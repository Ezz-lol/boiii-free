#include <std_include.hpp>
#include "dedicated.hpp"
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "command.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"

#include <utils/hook.hpp>

namespace dedicated {
namespace {
constexpr const char *compatibility_commands[] = {
    "ffotdversion",     "bbdisable", "bbenable",
    "bitfieldBBPrints", "bbstart",   "setliveevent"};

const game::dvar_t *sv_lan_only;

void sv_con_tell_f_stub(game::sv::client_s *cl, game::net::svscmd_type type,
                        [[maybe_unused]] const char *fmt,
                        [[maybe_unused]] int c, char *text) {
  game::sv::SV_SendServerCommand(cl, type, "%c \"GAME_SERVER\x15: %s\"", 79,
                                 text);
}

void send_heartbeat_packet() {
  if (!game::get_dvar_bool(sv_lan_only)) {
    for (const game::net::netadr_t &target :
         server_list::get_master_servers()) {
      network::send(target, "heartbeat", "T7");
    }
  }
}

void register_server_compatibility_commands() {
  for (const char *command_name : compatibility_commands) {
    command::add_sv(command_name, [](const command::params_sv &) {});
  }
}
} // namespace

void send_heartbeat() {
  if (game::is_server()) {
    scheduler::once(send_heartbeat_packet, scheduler::pipeline::main, 5s);
  }
}

void trigger_map_rotation() {
  scheduler::once(
      [] {
        if (!game::get_dvar_string("sv_maprotation").empty()) {
          game::cbuf::Cbuf_AddText(0, "map_rotate\n");
          send_heartbeat();
        }
      },
      scheduler::pipeline::main, 1s);
}

struct component final : server_component {
  void post_unpack() override {
    // Ignore "bad stats"
    // utils::hook::set<uint8_t>(0x14052D523_g, 0xEB);
    // utils::hook::nop(0x14052D4E4_g, 2);

    // Fix tell command for IW4M
    utils::hook::call(0x14052A8CF_g, sv_con_tell_f_stub);

    scheduler::once(send_heartbeat, scheduler::pipeline::main);
    scheduler::loop(send_heartbeat, scheduler::pipeline::main, 5min);
    command::add("heartbeat", send_heartbeat);
    register_server_compatibility_commands();

    // Hook GScr_ExitLevel
    utils::hook::jump(0x1402D1AA0_g, trigger_map_rotation);

    sv_lan_only = game::register_dvar_bool("sv_lanOnly", false, game::DVAR_NONE,
                                           "Don't send heartbeats");
  }
};
} // namespace dedicated

REGISTER_COMPONENT(dedicated::component)