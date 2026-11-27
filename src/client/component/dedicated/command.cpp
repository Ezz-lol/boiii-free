#include <std_include.hpp>

#include "command.hpp"
#include <loader/component_loader.hpp>

#include "../command.hpp"
#include "../network.hpp"
#include "../scheduler.hpp"
#include "../server_list.hpp"
#include <component/game_event.hpp>

#include <game/utils.hpp>

#include <utils/hook.hpp>

#include <sstream>

namespace dedicated {
game::EngineDependentDvar sv_lan_only;

void send_heartbeat_packet() {
  if (!sv_lan_only.get_bool()) {
    for (const game::net::netadr_t &target :
         server_list::get_master_servers()) {
      network::send(target, "heartbeat", "T7");
    }
  }
}
void send_heartbeat() {
  if (game::is_server()) {
    scheduler::once(send_heartbeat_packet, scheduler::pipeline::main, 5s);
  }
}

namespace command {
namespace {
constexpr const char *COMPATIBILITY_COMMANDS[] = {
    "ffotdversion",     "bbdisable", "bbenable",
    "bitfieldBBPrints", "bbstart",   "setliveevent"};

game::EngineDependentDvar sv_auto_gametype_settings;
utils::hook::detour SV_MapRotate_f_hook;

std::string next_rotation_gametype(std::string_view rotation) {
  std::istringstream tokens{std::string(rotation)};
  std::string mode;
  std::string key;
  std::string value;
  while (tokens >> key >> value) {
    if (key == "map") {
      return mode;
    }
    if (key != "gametype" ||
        value.find_first_not_of("abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") !=
            std::string::npos) {
      return {};
    }
    mode = value;
  }
  return {};
}

void SV_MapRotate_f_ExecGametypeSettings() {
  if (sv_auto_gametype_settings.get_bool() &&
      game::com::Com_SessionMode_IsMode(game::eModes::MULTIPLAYER)) {
    std::string_view rotation = game::maprotationcurrent().value_or("");
    if (rotation.find_first_not_of(" \t\r\n") == std::string_view::npos) {
      rotation = game::maprotation().value_or("");
    }
    const std::string mode = next_rotation_gametype(rotation);
    if (!mode.empty() && mode != game::get_g_gametype().value_or("")) {
      const std::string cmd = std::format(
          "exec gamedata/gamesettings/mp/gamesettings_{}.cfg", mode);
      game::cmd::Cmd_ExecuteSingleCommand(0, game::CONTROLLER_INDEX_FIRST,
                                          cmd.c_str(), false);
    }
  }
  SV_MapRotate_f_hook.invoke<void>();
}

void sv_con_tell_f_stub(game::sv::client_s *cl, game::net::svscmd_type type,
                        [[maybe_unused]] const char *fmt,
                        [[maybe_unused]] int32_t c, char *text) {
  game::sv::SV_SendServerCommand(cl, type, "%c \"GAME_SERVER\x15: %s\"", 79,
                                 text);
}

void register_server_compatibility_commands() {
  for (const char *command_name : COMPATIBILITY_COMMANDS) {
    ::command::add_sv(command_name, [](const ::command::params_sv &) {});
  }
}
} // namespace

void trigger_map_rotation() {
  scheduler::once(
      [] {
        const std::optional<std::string_view> maprotation = game::maprotation();
        if (maprotation.has_value() && !maprotation.value().empty()) {
          game::sv::SV_MapRotate_f();
          send_heartbeat();
        }
      },
      scheduler::pipeline::main, 1s);
}

struct component final : server_component {
  DEFINE_COMPONENT_NAME("command");

  void post_unpack() override {
    // Ignore "bad stats"
    // utils::hook::set<uint8_t>(0x14052D523_g, 0xEB);
    // utils::hook::nop(0x14052D4E4_g, 2);

    // Fix tell command for IW4M
    utils::hook::call(0x14052A8CF_g, sv_con_tell_f_stub);

    scheduler::once(send_heartbeat, scheduler::pipeline::main);
    game_event::on_g_init_game(send_heartbeat);
    scheduler::loop(send_heartbeat, scheduler::pipeline::main, 5min);
    ::command::add("heartbeat", send_heartbeat);
    register_server_compatibility_commands();

    sv_auto_gametype_settings = game::register_dvar_bool(
        "sv_autoGametypeSettings", true, game::dvarFlags_e::NONE,
        "Load the MP gametype config for each gametype entry in map rotation");
    SV_MapRotate_f_hook.create(game::sv::SV_MapRotate_f.get(),
                               SV_MapRotate_f_ExecGametypeSettings);

    // Hook GScr_ExitLevel
    utils::hook::jump(0x1402D1AA0_g, trigger_map_rotation);

    sv_lan_only = game::register_dvar_bool(
        "sv_lanOnly", false, game::dvarFlags_e::NONE, "Don't send heartbeats");
  }
};
} // namespace command
} // namespace dedicated
REGISTER_COMPONENT(dedicated::command::component)
