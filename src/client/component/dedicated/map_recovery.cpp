#include <std_include.hpp>

#include "map_recovery.hpp"

#include <loader/component_loader.hpp>

#include <component/scheduler.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace map_recovery {
namespace {
game::EngineDependentDvar restart_delay;
std::atomic<int32_t> consecutive_failures{0};
std::atomic<int64_t> launch_not_before{0};
std::atomic<bool> launch_queued{false};
game::cmd::xcommand_t launchgame_original{};
std::string failed_map;

int64_t now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void print(const char *message) {
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s\n", message);
}

void launchgame_stub() {
  const int64_t remaining = launch_not_before.load() - now_ms();
  if (remaining <= 0) {
    launchgame_original();
    return;
  }

  if (launch_queued.exchange(true)) {
    return;
  }

  print(utils::string::va("^3Map recovery: launching '%s' in %.1fs",
                          failed_map.c_str(),
                          static_cast<double>(remaining) / 1000.0));
  scheduler::once(
      [] {
        launch_queued = false;
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "launchgame\n");
      },
      scheduler::pipeline::main, std::chrono::milliseconds(remaining));
}

void wrap_launchgame() {
  if (launchgame_original) {
    return;
  }

  game::cmd::cmd_function_s *command =
      *reinterpret_cast<game::cmd::cmd_function_s **>(
          static_cast<game::cmd::cmd_function_s *>(game::cmd::cmd_functions));
  for (; command; command = command->next) {
    if (command->name && !_stricmp(command->name, "launchgame") &&
        command->function) {
      launchgame_original = command->function;
      command->function = launchgame_stub;
      return;
    }
  }

  print("^1Map recovery: 'launchgame' command not found, the map "
        "will not be delayed");
}

struct dropped_client {
  game::net::netadr_t address;
  std::chrono::steady_clock::time_point expires;
};

constexpr std::chrono::minutes DISCONNECT_NOTICE_LIFETIME{2};
constexpr size_t MAX_DISCONNECT_REASON = 900;
std::vector<dropped_client> dropped_clients;
std::string disconnect_packet;

bool same_address(const game::net::netadr_t &a, const game::net::netadr_t &b) {
  return a.type == b.type && a.addr == b.addr && a.port == b.port;
}

bool reply_to_unknown_client(const game::net::netsrc_t sock,
                             game::net::netadr_t *address, const char *data) {
  const auto now = std::chrono::steady_clock::now();
  std::erase_if(dropped_clients, [now](const dropped_client &client) {
    return client.expires <= now;
  });

  const bool dropped = std::ranges::any_of(
      dropped_clients, [address](const dropped_client &client) {
        return same_address(client.address, *address);
      });

  return game::net::NET_OutOfBandPrint(
      sock, address, dropped ? disconnect_packet.c_str() : data);
}
} // namespace

void notify_clients(const std::string &reason) {
  game::sv::client_s *clients = *game::sv::svs_clients;
  if (!clients) {
    return;
  }

  std::string message = "^7" + reason.substr(0, MAX_DISCONNECT_REASON);
  std::ranges::replace(message, '"', '\'');
  disconnect_packet = std::format("disconnect \"{}\"", message);

  dropped_clients.clear();
  const auto expires =
      std::chrono::steady_clock::now() + DISCONNECT_NOTICE_LIFETIME;
  for (size_t i = 0; i < game::get_max_client_count(); ++i) {
    game::sv::client_s &client = clients[i];
    if (client.state < game::net::clientState_t::RECONNECTING ||
        client.address.type < game::net::NA_RAWIP) {
      continue;
    }

    dropped_clients.push_back({client.address, expires});
    game::net::NET_OutOfBandPrint(game::net::NS_SERVER, &client.address,
                                  disconnect_packet.c_str());
  }
}

void on_map_started() {
  const int32_t failures = consecutive_failures.exchange(0);
  if (failures == 0) {
    return;
  }

  launch_not_before = 0;
  print(utils::string::va(
      "^2Map recovery: '%s' started cleanly after %d restart(s)",
      std::string(game::get_mapname().value_or("")).c_str(), failures));
}

std::string on_map_stopped() {
  wrap_launchgame();

  const int32_t delay = restart_delay.get_int();
  const int32_t failures = ++consecutive_failures;
  failed_map = game::get_mapname().value_or("");
  launch_not_before = now_ms() + delay * 1000ll;

  return delay > 0
             ? std::format("map stopped, next launch in {}s "
                           "(restart #{})",
                           delay, failures)
             : std::format("map stopped, relaunching (restart #{})", failures);
}

struct component final : server_component {
#ifndef NDEBUG
  std::string name() override { return "map_recovery"; }
#endif

  void post_unpack() override {
    utils::hook::call(game::sv::SV_PacketEvent.offset(0x124),
                      reply_to_unknown_client);

    restart_delay = game::register_dvar_int(
        "sv_errorRestartDelay", 5, 0, 300, game::DVAR_NONE,
        "Seconds to wait before launching the map again after a script error "
        "or crash stopped it");
  }
};
} // namespace map_recovery

REGISTER_COMPONENT(map_recovery::component)
