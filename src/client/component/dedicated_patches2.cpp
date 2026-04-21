#include <mutex>
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"
#include "game/impl/snd/snd.hpp"

#include <string>
#include <unordered_map>
#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "scheduler.hpp"
#include "command.hpp"

namespace server_patches2 {
namespace {
// Connection rate limiting per IP
struct connect_attempt {
  uint32_t ip;
  std::chrono::steady_clock::time_point last_attempt;
  int attempts;
};

std::mutex rate_limit_mutex;
std::unordered_map<uint32_t, connect_attempt> connect_attempts;
constexpr int MAX_CONNECT_ATTEMPTS = 5;
constexpr auto RATE_LIMIT_WINDOW = std::chrono::seconds(10);

bool is_rate_limited(uint32_t ip) {
  std::lock_guard lock(rate_limit_mutex);
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
  std::lock_guard lock(rate_limit_mutex);
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

void disable_sv_cheats_cb(game::dvar_t *sv_cheats) {
  if (sv_cheats->current.value.enabled) {
    game::Dvar_SetBoolFromSource(sv_cheats, false, game::DVAR_SOURCE_INTERNAL);
  }
}

// Hook for G_Say to sanitize messages
utils::hook::detour g_say_hook;

void g_say_stub(game::level::gentity_s *ent, game::level::gentity_s *target,
                int mode, const char *chatText) {
  if (chatText) {
    const auto sanitized = sanitize_chat_message(chatText);
    g_say_hook.invoke(ent, target, mode, sanitized.data());
  } else {
    g_say_hook.invoke(ent, target, mode, chatText);
  }
}

// Hook for SV_DirectConnect to rate limit connections
utils::hook::detour sv_direct_connect_hook;

void sv_direct_connect_stub(game::net::netadr_t adr) {
  if (is_rate_limited(adr.addr)) {
    printf("[Security] Rejected connection from rate-limited IP\n");
    return;
  }

  sv_direct_connect_hook.invoke(adr);
}

utils::hook::detour sv_removeallclientsfromaddress_hook;
void sv_live_removeallclientsfromaddress_stub(game::net::client_s *client,
                                              const char *reason) {
  // Skip disconnecting other clients from the same IP -
  // just free the disconnected client's slot, and return.
  game::sv::SV_Live_RemoveClient(client, reason);
  return;
}

std::mutex reliable_cmd_mutex;
// Map of reliable command string -> Map of xuid -> svs->time of last sequencing
std::unordered_map<std::string, std::unordered_map<game::XUID, uint32_t>>
    client_openmenu_cmd_last_sequence_time;
// Map of xuid -> last sequenced reliable command string
std::unordered_map<game::XUID, std::string> client_last_cmd;

utils::hook::detour g_init_game_hook;
void g_init_game_stub(uint32_t levelTime, uint32_t randomSeed,
                      game::qboolean restart, game::qboolean registerDvars,
                      game::qboolean savegame) {
  std::lock_guard lock(reliable_cmd_mutex);

  // Reset tracked openmenu reliable cmds on starting a new game.
  for (auto &[cmd, client_map] : client_openmenu_cmd_last_sequence_time) {
    client_map.clear();
  }

  client_openmenu_cmd_last_sequence_time.clear();
  client_last_cmd.clear();

  g_init_game_hook.invoke(levelTime, randomSeed, restart, registerDvars,
                          savegame);
}

utils::hook::detour sv_addservercommand_hook;

void sv_addservercommand_stub(game::net::client_s *client,
                              game::net::svscmd_type type, const char *cmd) {

  std::string cmd_str = cmd ? std::string(cmd) : "";
  std::lock_guard lock(reliable_cmd_mutex);

  /*
    `openmenu` reliable commands have format "D %d %d %d %d", or "D %d %d %d".
    Note that the prefix "D " is its unique command type identifier.
  */
  if (utils::string::starts_with(cmd_str, "D ")) {
    // If this command was sent less than 1000 ms ago, skip.
    if (client_openmenu_cmd_last_sequence_time.contains(cmd_str) &&
        client_openmenu_cmd_last_sequence_time[cmd_str].contains(
            client->xuid) &&
        *(game::sv::svs_time.get()) -
                client_openmenu_cmd_last_sequence_time[cmd_str][client->xuid] <
            1000) {
      return;
    }

    // We also do not need to send a redundant openmenu command if it was the
    // last command sent, even if sent > 1 second ago. This is valid because we
    // can guarantee that menu state was not modified otherwise in the interim.
    if (client_last_cmd.contains(client->xuid) &&
        client_last_cmd[client->xuid] == cmd_str) {
      return;
    }
  }

  client_openmenu_cmd_last_sequence_time[cmd_str][client->xuid] =
      *(game::sv::svs_time.get());
  client_last_cmd[client->xuid] = cmd_str;

  sv_addservercommand_hook.invoke(client, type, cmd);
}

utils::hook::detour db_loadxfile_hook;
bool db_loadxfile_stub(const char *path, game::db::DBFile f,
                       game::db::xzone::XZoneBuffer *fileBuffer,
                       const char *filename, game::db::XBlock *blocks,
                       game::db::DB_Interrupt *interrupt, uint8_t *buf,
                       game::PMemStack side, int flags) {
  bool succeeded = db_loadxfile_hook.invoke<bool>(
      path, f, fileBuffer, filename, blocks, interrupt, buf, side, flags);

  if (succeeded && (game::db::load::g_load->flags & 0x1000C00) != 0) {
    game::snd::g_sb->loadGate = 0;
    game::snd::SND_LoadSoundsWait();
  }

  return succeeded;
}

void free_bank_allocations_before_clearing_address_stub(
    game::snd::SndBankLoad *load, int64_t offset, uint64_t len) {

  game::snd::sd_byte *loadedEntries =
      reinterpret_cast<game::snd::sd_byte *>(load->loadedEntries);
  if (loadedEntries) {
    game::snd::SD_Free_Impl(loadedEntries);
  }
  game::snd::sd_byte *loadedData = load->loadedData;
  if (loadedData) {
    game::snd::SD_Free_Impl(loadedData);
  }
  game::snd::sd_byte *loadAssetBankEntries =
      reinterpret_cast<game::snd::sd_byte *>(load->loadAssetBank.entries);
  if (loadAssetBankEntries) {
    game::snd::SD_Free_Impl(loadAssetBankEntries);
  }
  game::snd::sd_byte *streamAssetBankEntries =
      reinterpret_cast<game::snd::sd_byte *>(load->streamAssetBank.entries);
  if (streamAssetBankEntries) {
    game::snd::SD_Free_Impl(streamAssetBankEntries);
  }

  memset(reinterpret_cast<void *>(load), offset, len);
}

utils::hook::detour snd_init_hook;
void snd_init_stub() {
  snd_init_hook.invoke();
  *(game::snd::g_pc_nosnd.get()) = 0;
  game::snd::g_snd->verified_0.init = 1;
  game::snd::g_sb->bankMagic = 0x12233445;
}

utils::hook::detour snd_queueadd_hook;
void snd_queueadd_stub(game::snd::SndQueue *queue,
                       game::snd::SndCommandType cmd, uint32_t size,
                       const void *data) {
  game::snd::SND_CommandSND(cmd, static_cast<int64_t>(size),
                            const_cast<void *>(data));
}

utils::hook::detour snd_active_hook;
game::qboolean snd_active_stub() {
  game::snd::g_snd->verified_0.init = 1;
  const game::qboolean active = snd_active_hook.invoke<game::qboolean>();
  return active;
}

utils::hook::detour g_sndenabled_hook;
utils::hook::detour snd_shouldinit_hook;
bool return_true() { return true; }

utils::hook::detour snd_queueflush_hook;
utils::hook::detour snd_processsndqueue_hook;
void stub_func() { return; }

utils::hook::detour snd_enqueueloadedassets_hook;
utils::hook::detour snd_starttocread_hook;

/*
  Sound load, processing, and data access functionality was consistently either
  removed or disabled in dedicated server. This was a valid optimization for the
  stated intent - a dedicated, multiplayer server using only Treyarch maps - as
  Treyarch multiplayer maps never require server-side sound handling in any
  form.

  Treyarch zombies and custom maps for any gamemode, however, generally do
  require server-side sound processing, and the lack of it causes a wide variety
  of sound-related bugs. This function re-enables where possible and otherwise
  re-implements sound functionality in the dedicated server engine.

  This fixes most bugs related to server-side sound handling.

  For example:
  - Map music, sound effects, or voicelines not playing, playing on a loop,
  playing at the wrong time, or all playing at the same time - occurs in most
  zombies maps.
  - Maps with manual sound loops crashing the server with `G_Spawn: no free
  entities` error - Die Rise, for example.

  Does not fix:
  - Perk machine jingles inconsistently playing when player is in proximity.
*/

inline void enable_sound() {
  /*
   Disable usage of g_copyInfo - force using asset pools instead.
   XAsset dependency graph linking handles this already, and more
   effectively - this seems to be leftover from earlier engine versions with
   hardcoded asset load ordering, and crashes server when sound is enabled
   and g_copyInfo is used.
  */
  utils::hook::nop(0x1401D7C23_g, 9);
  utils::hook::nop(0x1401DA103_g, 5);
  /*
     Disable purposely crashing application by
     incrementing g_copyInfoCount by 16384 (over limit) on authload failure.
     Newer engine versions (e.g. Bo4) throw an error properly (`Sys_Error`)
     instead.
  */
  utils::hook::nop(0x1401A18E1_g, 10);
  utils::hook::nop(0x1401A1B5D_g, 10);

  /*
    In the lines of code where the client versions of SND_EnqueueLoadedAssets
    and SND_StartTocRead require usage of `SD_Alloc`, in dedicated server, a
    `nullptr` immediate value is used instead, causing these steps of bank
    load to immediately fail, and bank load to never occur.

    Hook these functions and replace them with the client-equivalent
    implementation.
  */
  snd_enqueueloadedassets_hook.create(game::snd::SND_EnqueueLoadedAssets.get(),
                                      game::snd::SND_EnqueueLoadedAssets_Impl);
  snd_starttocread_hook.create(game::snd::SND_StartTocRead.get(),
                               game::snd::SND_StartTocRead_Impl);

  /*
    In client, in SNDL_RemoveBank, SD_Free is called to free the
    heap-allocated bank data of a SndBankLoad, before clearing the
    SndBankLoad with memset.

    This obviously is not performed in server, as SD_Alloc is also not
    used in the unmodified engine; it was reimplemented and used in the above
    SND_EnqueueLoadedAssets_Impl.

    This call hooks the memset call to instead first free these allocations,
    if they are present, to prevent memory leak. The client frees these
    allocations in the same location.
  */
  utils::hook::call(0x14064AB30_g,
                    free_bank_allocations_before_clearing_address_stub);

  /*
    After loading level XPAK, block on loading its soundbanks, just as
    client does.
  */
  db_loadxfile_hook.create(game::db::load::DB_LoadXFile.get(),
                           db_loadxfile_stub);

  /*
    The dedicated server does not have an async sound queue, and the
    initialization in client is heavily arxan obfuscated.
    Suffice to say I have so far been unable (in a time-sensitive manner) to
    verify accurate structure and values for async queue initialization.

    Fortunately, the dedicated server actually doesn't need an async queue at
    all. Sounds only need to be processed at initial load, but not afterwards,
    except as requested by scripts.

    The below hooks circumvent attempted usage of the (non-existent) async
    sound queue, instead forwarding queue additions to the intended handler,
    immediately.
  */
  snd_queueadd_hook.create(game::snd::SND_QueueAdd.get(), snd_queueadd_stub);
  snd_processsndqueue_hook.create(game::snd::SND_ProcessSNDQueue.get(),
                                  stub_func);
  snd_queueflush_hook.create(game::snd::SND_QueueFlush.get(),
                             reinterpret_cast<void (*)(int)>(stub_func));

  /*
    Enable sound
  */
  snd_active_hook.create(game::snd::SND_Active.get(), snd_active_stub);
  snd_init_hook.create(game::snd::SND_Init.get(), snd_init_stub);
  g_sndenabled_hook.create(game::snd::G_SndEnabled.get(), return_true);
  snd_shouldinit_hook.create(game::snd::SND_ShouldInit.get(), return_true);
}
} // namespace

struct component final : server_component {
  void post_unpack() override {

    enable_sound();
    // Sanitize chat messages on server
    g_say_hook.create(game::G_Say.get(), g_say_stub);

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
      sv_direct_connect_hook.create(game::sv::SV_DirectConnect.get(),
                                    sv_direct_connect_stub);
    }

    // RCE Prevention: Patch Cmd_ParseArgs to prevent remote code execution
    // Makes the vulnerable function immediately return, blocking crafted
    // command strings from executing arbitrary code on the server
    utils::hook::set<uint8_t>(0x1404B2E00_g, 0xC3);

    // TeamOps arbitrary write fix: NOP the inlined arbitrary write
    // that allows attackers to write to arbitrary memory via team operations
    utils::hook::nop(0x1401155D5_g, 7);

    /*
      Disable removal of all clients from an IP address if
      one client from that IP address disconnects.

      Useful if e.g. server is hosted behind a reverse proxy or
      load balancer where multiple clients share the same IP.
    */
    sv_removeallclientsfromaddress_hook.create(
        game::sv::SV_Live_RemoveAllClientsFromAddress.get(),
        sv_live_removeallclientsfromaddress_stub);

    scheduler::once(
        [] {
          const game::dvar_t *sv_cheats = game::Dvar_FindVar("sv_cheats");
          game::Dvar_SetBoolFromSource(sv_cheats, false,
                                       game::DVAR_SOURCE_INTERNAL);

          // Enforce sv_cheats = 0
          game::Dvar_SetModifiedCallback(
              sv_cheats,
              reinterpret_cast<game::modifiedCallback>(disable_sv_cheats_cb));
        },
        scheduler::pipeline::async, 30000ms);

    if (!utils::flags::has_flag("noratelimit")) {
      // Cleanup old rate limit entries periodically
      scheduler::loop([] { cleanup_rate_limits(); }, scheduler::pipeline::async,
                      30000ms);
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
      results in a constant, massive flood of redundant `openmenu` reliable
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
      sv_addservercommand_hook.create(game::sv::SV_AddServerCommand.get(),
                                      sv_addservercommand_stub);
      g_init_game_hook.create(game::G_InitGame.get(), g_init_game_stub);
    }
  }
};
} // namespace server_patches2

REGISTER_COMPONENT(server_patches2::component)