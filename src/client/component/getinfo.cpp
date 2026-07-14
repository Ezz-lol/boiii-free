#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <steam/steam.hpp>

#include "network.hpp"
#include "network_password.hpp"
#include "workshop.hpp"
#include "scheduler.hpp"

#include <utils/string.hpp>
#include <utils/info_string.hpp>

// In case of clangd compilation
#if __has_include("version.hpp")
#include "version.hpp"
#else
#ifndef VERSION
#define VERSION "0"
#endif
#ifndef SHORTVERSION
#define SHORTVERSION "0"
#endif
#endif

#include <game/utils.hpp>

namespace getinfo {
template <typename T> int get_client_count(T *client_states) {
  if (client_states == nullptr) {
    return 0;
  }

  int32_t count = 0;
  for (int32_t i = 0; i < game::get_com_maxclients(); ++i) {
    if (client_states[i].client_state > 0) {
      ++count;
    }
  }

  return count;
}

size_t get_client_count() {
  size_t count = 0;
  game::foreach_connected_client(
      [&count](const game::sv::client_s &) { ++count; });

  return count;
}

size_t get_bot_count() {
  size_t count = 0;

  game::foreach_connected_client(
      [&count](const game::sv::client_s &, const size_t index) {
        if (game::sv::SV_IsTestClient(static_cast<game::ClientNum_t>(index))) {
          ++count;
        }
      });

  return count;
}

int get_assigned_team() { return (rand() % 2) + 1; }

bool is_host() {
  return game::sv::SV_Loaded() &&
         (game::is_server() || !game::com::Com_IsRunningUILevel());
}

struct component final : generic_component {
  void post_unpack() override {

    game::sv_wwwDownload = game::register_dvar_bool(
        "sv_wwwDownload", false, game::DVAR_NONE, "Enable http downloads");
    game::sv_wwwDlDisconnected = game::register_dvar_bool(
        "sv_wwwDlDisconnected", true, game::DVAR_NONE,
        "Should clients stay connected while downloading?");
    scheduler::once(
        []() {
          game::sv_wwwBaseURL = game::register_dvar_string(
              "sv_wwwBaseURL", "", game::DVAR_NONE,
              "The base url for files downloaded via http");
          game::workshop_id = game::register_dvar_string(
              "workshop_id", "", game::DVAR_NONE,
              "Steam workshop ID of loaded mod, if any, "
              "or loaded usermap otherwise.");
        },
        scheduler::pipeline::main);

    // utils::hook::jump(game::select(0x142254EF0, 0x140537730),
    // get_assigned_team);

    network::on("getInfo", [](const game::net::netadr_t &target,
                              const network::data_view &data,
                              game::LocalClientNum_t clientNum) {
      utils::info_string info{};
      info.set("challenge", std::string{data.begin(), data.end()});
      info.set("gamename", "T7");
      info.set("hostname", game::hostname().value_or(""));
      info.set("gametype", game::gametype().value_or(""));
      /*
         Unsure why this is commented out, but important notes in case of future
         restoration:
          - The `sv_motd` dvar does not exist in the engine.
          - The engine _does_ have:
              - g_motd - most closely matching the usage of `sv_motd`.
                Used to set SV config string index 11 on `G_InitGame`.
                Description is "The message of the day".
                Never seems to be set in engine.
              - `cl_motdstring` - unused after initial registration
              - `motd_enabled` - used for the `LiveStats` MOTD
              - `live_motdEnabled` - used to check if LiveStorage should fetch
                latest pub online data MOTD.

                This may be the same MOTD used later,
                conditionally upon the potentially duplicitous
                `motd_enabled` dvar.

                Unsure currently.
              - `motddelay` - used for the `LiveStats` MOTD

        TL;DR: this should either be set with the value of `g_motd` or
        we should register a new `sv_motd` dvar.
      */
      // info.set("sv_motd", get_dvar_string("sv_motd"));
      info.set("description",
               game::is_server()
                   ? game::get_live_steam_server_description().value_or("")
                   : "");
      info.set("xuid", utils::string::va(
                           "%llX", steam::SteamUser()->GetSteamID().bits));
      info.set("mapname", game::get_mapname().value_or(""));
      info.set("isPrivate", game::password().value_or("").empty() ? "0" : "1");
      info.set("clients", std::to_string(get_client_count()));
      info.set("bots", std::to_string(get_bot_count()));
      info.set("sv_maxclients", std::to_string(game::get_max_client_count()));
      info.set("protocol", std::to_string(PROTOCOL));
      info.set("sub_protocol", std::to_string(SUB_PROTOCOL));
      info.set("playmode", std::to_string(static_cast<int32_t>(
                               game::com::Com_SessionMode_GetMode())));
      info.set("gamemode",
               std::to_string(game::com::Com_SessionMode_GetGameMode()));
      info.set("sv_running", std::to_string(game::server_running()));
      info.set("dedicated", game::is_server() ? "1" : "0");
      info.set("hc", std::to_string(game::com::Com_GametypeSettings_GetUInt(
                         "hardcoremode", false)));
      info.set("modName", workshop::get_mod_resized_name());
      info.set("modId", workshop::get_mod_publisher_id());
      info.set("rounds_played",
               std::to_string(*game::level::level_rounds_played));
      info.set("shortversion", SHORTVERSION);

      info.set("sv_wwwBaseURL", game::fastdl_uri().value_or(""));
      info.set("sv_wwwBaseUrl", game::fastdl_uri().value_or(""));
      info.set("workshop_id", game::get_workshop_id().value_or(""));
      info.set("usermapId", game::get_workshop_id().value_or(""));

      if (network_password::is_password_set()) {
        info.set("net_password_hash",
                 network_password::get_password_hash_string());
      }

      network::send(target, "infoResponse", info.build(), '\n');
    });
  }
};
} // namespace getinfo

REGISTER_COMPONENT(getinfo::component)