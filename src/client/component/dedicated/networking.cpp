#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <game/impl/snd/snd.hpp>
#include <game/impl/snd/sd/sd.hpp>

#include <component/scheduler.hpp>
#include <component/game_event.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <string>
#include <unordered_map>

namespace dedicated {
namespace networking {
namespace {
// Connection rate limiting per IP
struct connect_attempt {
  uint32_t ip;
  std::chrono::steady_clock::time_point last_attempt;
  int attempts;
};

static std::mutex rate_limit_mutex;
static std::unordered_map<uint32_t, connect_attempt> connect_attempts;
constexpr int MAX_CONNECT_ATTEMPTS = 5;
constexpr auto RATE_LIMIT_WINDOW = std::chrono::seconds(10);

bool rate_limited(uint32_t ip) {
  std::scoped_lock lock(rate_limit_mutex);
  const auto now = std::chrono::steady_clock::now();

  auto it = connect_attempts.find(ip);
  if (it == connect_attempts.end()) {
    connect_attempts[ip] = {ip, now, 1};
    return false;
  }

  auto &attempt = it->second;
  if ((now - attempt.last_attempt) > RATE_LIMIT_WINDOW) {
    attempt.attempts = 1;
    attempt.last_attempt = now;
    return false;
  }

  attempt.attempts++;
  attempt.last_attempt = now;

  if (attempt.attempts > MAX_CONNECT_ATTEMPTS) {
    printf("[Security] Rate limited connection from IP 0x%08X (%d attempts)\n",
           ip, attempt.attempts);
    return true;
  }

  return false;
}

void cleanup_rate_limits() {
  std::scoped_lock lock(rate_limit_mutex);
  const auto now = std::chrono::steady_clock::now();

  for (auto it = connect_attempts.begin(); it != connect_attempts.end();) {
    if ((now - it->second.last_attempt) > std::chrono::seconds(60))
      it = connect_attempts.erase(it);
    else
      ++it;
  }
}

// Chat message sanitization
std::string sanitize_chat_message(const std::string &msg) {
  std::string result;
  result.reserve(msg.size());

  for (size_t i = 0; i < msg.size(); i++) {
    const unsigned char c = static_cast<unsigned char>(msg[i]);

    // Block control characters (except newline and tab)
    if (c < 0x20 && c != '\n' && c != '\t')
      continue;

    // Block DEL
    if (c == 0x7F)
      continue;

    // Block format string exploits (all dangerous C format specifiers)
    if (c == '%' && (i + 1) < msg.size()) {
      const unsigned char next = static_cast<unsigned char>(msg[i + 1]);
      if (next == 'n' || next == 's' || next == 'x' || next == 'p' ||
          next == 'd' || next == 'i' || next == 'u' || next == 'o' ||
          next == 'f' || next == 'e' || next == 'g' || next == 'c' ||
          next == 'X')
        continue;
    }

    // Block extended ASCII control chars (can cause rendering issues)
    if (c >= 0x80 && c <= 0x9F)
      continue;

    result += static_cast<char>(c);
  }

  // Limit message length
  if (result.size() > 512)
    result.resize(512);

  return result;
}

// Hook for G_Say to sanitize messages
utils::hook::detour G_Say_hook;
void G_Say_Sanitize(game::level::gentity_s *ent, game::level::gentity_s *target,
                    int mode, const char *chatText) {
  if (chatText) {
    const std::string sanitized = sanitize_chat_message(chatText);
    G_Say_hook.invoke(ent, target, mode, sanitized.data());
  } else {
    G_Say_hook.invoke(ent, target, mode, chatText);
  }
}

// Hook for SV_DirectConnect to rate limit connections
utils::hook::detour SV_DirectConnect_hook;
void SV_DirectConnect_RateLimited(game::net::netadr_t adr) {
  if (rate_limited(adr.addr)) {
    fprintf(stderr, "[Security] Rejected connection from rate-limited IP\n");
    return;
  }

  SV_DirectConnect_hook.invoke(adr);
}

utils::hook::detour SV_LiveRemoveAllClientsFromAddress_hook;
void SV_LiveRemoveAllClientsFromAddress_RemoveSingle(game::sv::client_s *client,
                                                     const char *reason) {
  // Skip disconnecting other clients from the same IP -
  // just free the disconnected client's slot, and return.
  game::sv::SV_Live_RemoveClient(client, reason);
}

static std::mutex client_luinotify_cmd_last_sequence_time_mutex;
// Map of reliable command string -> Map of xuid -> svs->time of last sequencing
static std::unordered_map<std::string, std::unordered_map<game::XUID, uint32_t>>
    client_luinotify_cmd_last_sequence_time;

static std::mutex client_last_cmd_mutex;
// Map of xuid -> last sequenced reliable command string
static std::unordered_map<game::XUID, std::string> client_last_cmd;

void clear_luinotify_history() {
  {
    std::scoped_lock client_luinotify_cmd_last_sequence_time_lock(
        client_luinotify_cmd_last_sequence_time_mutex);

    // Reset tracked luinotify reliable cmds on starting a new game.
    for (auto &[cmd, client_map] : client_luinotify_cmd_last_sequence_time) {
      client_map.clear();
    }
    client_luinotify_cmd_last_sequence_time.clear();
  }

  {
    std::scoped_lock client_last_cmd_lock(client_last_cmd_mutex);
    client_last_cmd.clear();
  }
}

inline constexpr const str<2> LUI_NOTIFY_RELIABLE_CMD_PREFIX = {
    static_cast<char>(game::sv::ReliableCommand::LUI_NOTIFY), ' '};

utils::hook::detour SV_AddServerCommand_hook;

void SV_AddServerCommand_MinimizeLuiNotifyPackets(game::sv::client_s *client,
                                                  game::net::svscmd_type type,
                                                  const char *cmd) {
  std::string cmd_str = cmd ? std::string(cmd) : "";

  /*
    `luinotify` reliable commands have format "D %d %d %d %d", or "D %d %d %d".
    Note that the prefix "D " is its unique command type identifier.
  */
  if (utils::string::starts_with(cmd_str, LUI_NOTIFY_RELIABLE_CMD_PREFIX)) {
    {
      std::scoped_lock client_luinotify_cmd_last_sequence_time_lock(
          client_luinotify_cmd_last_sequence_time_mutex);

      // If this command was sent less than 1000 ms ago, skip.
      if (client_luinotify_cmd_last_sequence_time.contains(cmd_str) &&
          client_luinotify_cmd_last_sequence_time[cmd_str].contains(
              client->xuid) &&
          game::sv::svs->time -
                  client_luinotify_cmd_last_sequence_time[cmd_str]
                                                         [client->xuid] <
              1000) {
        return;
      }
    }

    {
      std::scoped_lock client_last_cmd_lock(client_last_cmd_mutex);
      // We also do not need to send a redundant luinotify command if it was the
      // last command sent, even if sent > 1 second ago. This is valid because
      // we can guarantee that menu state was not modified otherwise in the
      // interim.
      if (client_last_cmd.contains(client->xuid) &&
          client_last_cmd[client->xuid] == cmd_str) {
        return;
      }
    }
  }

  {
    std::scoped_lock client_luinotify_cmd_last_sequence_time_lock(
        client_luinotify_cmd_last_sequence_time_mutex);
    client_luinotify_cmd_last_sequence_time[cmd_str][client->xuid] =
        game::sv::svs->time;
  }

  {
    std::scoped_lock client_last_cmd_lock(client_last_cmd_mutex);
    client_last_cmd[client->xuid] = cmd_str;
  }

  SV_AddServerCommand_hook.invoke(client, type, cmd);
}

void stub_func() { return; }
utils::hook::detour R_Stream_ClearTechniqueSetShaders_hook;
void disable_unused_asset_loads() {
  /*
    In some cases of map switch between two usermaps, the dedicated server
    will unexpectedly load material technique sets, despite being unused.

    The below function takes a material technique set which is expected to be
    initialized and zeroes its shader-related values. This is performed to
    prepare the struct for subsequent copy of a loaded material technique set.

    Some of these values require dereference of pointers stored as fields in
    the material technique set. The material technique set is not actually
    initialized as expected, because the dedicated server is not intended to
    load or use these assets. As such, these pointer deferences result in a
    memory access violation (null pointer dereference).

    Stubbing this function could result in garbage data leftover in the
    technique set after copy of the loaded technique set, but these are unused
    on dedicated server, so this is inconsequential.
  */
  R_Stream_ClearTechniqueSetShaders_hook.create(
      game::db::load::R_Stream_ClearTechniqueSetShaders, stub_func);
}

} // namespace

struct component final : server_component {
  void post_unpack() override {

    disable_unused_asset_loads();

    /*
     Disable purposely crashing application by
     incrementing g_copyInfoCount by 16384 (over limit) on authload failure.
     Newer engine versions (e.g. Bo4) throw an error properly (`Sys_Error`)
     instead.
    */
    utils::hook::nop(0x1401A18E1_g, 10);
    utils::hook::nop(0x1401A1B5D_g, 10);

    // Sanitize chat messages on server
    G_Say_hook.create(game::G_Say.get(), G_Say_Sanitize);

    /*
     Some server configurations will require this to be disabled.
     For example, if the server operates behind a reverse proxy, all incoming
     connections will appear to be from the same IP. As such, clients will be
     erroneously rate limited, despite being unique connections.

     In this case, rate limiting should be enforced in the server which is
     executing the reverse proxy. Enforcement of rate limiting elsewhere will be
     the responsibility of the server operator using this abnormal
     configuration.
    */
    if (!utils::flags::has_flag("noratelimit")) {
      // Rate limit connections
      SV_DirectConnect_hook.create(game::sv::SV_DirectConnect.get(),
                                   SV_DirectConnect_RateLimited);
    }

    // RCE Prevention: Patch Cmd_ParseArgs to prevent remote code execution
    // Makes the vulnerable function immediately return, blocking crafted
    // command strings from executing arbitrary code on the server
    utils::hook::set<uint8_t>(0x1404B2E00_g, 0xC3);

    // TeamOps arbitrary write fix: NOP the inlined arbitrary write
    // that allows attackers to write to arbitrary memory via team operations
    utils::hook::nop(0x1401155D5_g, 7);

    /*
      Disable removal of all clients from an IP address when
      one client from the IP address disconnects.

      Useful if e.g. server is hosted behind a reverse proxy or
      load balancer where multiple clients share the same IP.
    */
    SV_LiveRemoveAllClientsFromAddress_hook.create(
        game::sv::SV_Live_RemoveAllClientsFromAddress.get(),
        SV_LiveRemoveAllClientsFromAddress_RemoveSingle);

    if (!utils::flags::has_flag("noratelimit")) {
      // Cleanup old rate limit entries periodically
      scheduler::loop(cleanup_rate_limits, scheduler::pipeline::async, 30000ms);
    }

    /*
      Some custom maps, especially those with custom HUDs, update HUD state with
      server-side logic. Often, this is implemented through a spin-loop which
      executes `luinotify`s every 50ms to update HUD state, regardless of
      whether there has been a change in state.

      When executed client-side, in a singleplayer game, this works well.

      Unfortunately, when executed in either dedicated server or when hosting a
      private match, each one of these `luinotify`s results in a packet sent to
      each non-host client.

      In two tested cases - in the custom maps Kowloon and Daybreak - this
      results in a constant, massive flood of redundant `luinotify` reliable
      commands being sent to each client. While inefficient, this is generally
      acceptable. However, when the client is completing load-in to the map, in
      the initial blackscreen before they begin playing, reliable commands are
      temporarily unhandled. In the case of the aforementioned examples, this
      results in a near-consistent inability for clients to succesfully load
      into the map, instead resulting in an `EXE_ERR_RELIABLE_CYCLED_OUT` error,
      as the flood of hundreds of menu update packets are unhandled.

      Inability to reliably play these maps online has been noted many times in
      the steamcommunity workshop pages for these maps. Neither has been updated
      to resolve the issue, and new maps are often created with HUD update logic
      which contains similarly poor design.

      To mitigate this, these duplicative menu update packets without state
      change can be handled and filtered out server-side.
    */
    if (utils::flags::has_flag("mitigatepacketspam")) {
      SV_AddServerCommand_hook.create(
          game::sv::SV_AddServerCommand.get(),
          SV_AddServerCommand_MinimizeLuiNotifyPackets);
      game_event::on_any(clear_luinotify_history);
    }
  }
};
} // namespace networking
} // namespace dedicated
REGISTER_COMPONENT(dedicated::networking::component)