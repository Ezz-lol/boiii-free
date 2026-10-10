#include <std_include.hpp>

#include <game/game.hpp>
#include <loader/component_loader.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace dedicated {
namespace {
utils::hook::detour spawn_server_hook;

void scr_are_textures_loaded_stub() {
  game::scr::Scr_AddInt(game::scr::SCRIPTINSTANCE_SERVER, 1);
}

game::eNetworkModes get_online_mode_stub() {
  return game::eNetworkModes::ONLINE;
}

bool is_online_stub() { return true; }

bool is_mod_loaded_stub() { return false; }

void patch_is_mod_loaded_checks() {
  constexpr uintptr_t is_mod_loaded_addresses[] = {
      0x14019CFC4, 0x14024D4A0, 0x14024D669, 0x14024D939,
      0x14024DC64, 0x14024E13A, 0x14024E5A3, 0x14024FFB9,
      0x140251E9E, 0x140253680, 0x140257BF6, 0x1402D296D,
      0x1402D58E9, 0x140468374, 0x14046B796, 0x14048003D,
  };

  for (const uintptr_t address : is_mod_loaded_addresses) {
    utils::hook::call(game::relocate(address), is_mod_loaded_stub);
  }
}

void spawn_server_stub(game::ControllerIndex_t controllerIndex,
                       const char *server, game::MapPreload preload,
                       bool savegame) {
  game::com::Com_SessionMode_SetNetworkMode(game::eNetworkModes::ONLINE);
  game::com::Com_SessionMode_SetGameMode(
      game::eGameModes::MATCHMAKING_PLAYLIST);

  spawn_server_hook.invoke(controllerIndex, server, preload, savegame);
}

uint64_t sv_get_player_xuid_stub(const int client_num) {
  const game::sv::client_s *clients = *game::sv::svs_clients;
  return clients ? clients[client_num].xuid : 0;
}

void info_set_value_for_key_stub(char *s, const char *key,
                                 [[maybe_unused]] const char *value) {
  game::info::Info_SetValueForKey(s, key, "Unknown Soldier");
}

const char *va_stub([[maybe_unused]] const char *fmt, const char *name,
                    [[maybe_unused]] const int client_num) {
  return utils::string::va("%s", name);
}

template <const uint8_t Count> inline void set_max_name_characters() {
  constexpr uintptr_t patch_addresses[] = {/* SV_UserinfoChanged */
                                           0x14053136A,
                                           /* G_ClientSessionInfoChanged */
                                           0x1402799E9, 0x140279A04,
                                           0x140279A21, 0x140279A85};
  for (const uintptr_t addr : patch_addresses) {
    utils::hook::set<uint8_t>(game::relocate(addr), Count);
  }
}

std::string add_missing_gametype_names(const std::string &data) {
  std::vector<std::string> lines;
  std::istringstream stream(data);
  for (std::string line; std::getline(stream, line);) {
    lines.push_back(std::move(line));
  }

  const auto trimmed = [](std::string_view line) {
    while (!line.empty() && std::isspace(static_cast<uint8_t>(line.back()))) {
      line.remove_suffix(1);
    }
    return line;
  };

  std::string out;
  for (size_t i = 0; i < lines.size(); ++i) {
    out += lines[i] + '\n';
    const std::string_view line = trimmed(lines[i]);
    if (!line.starts_with("gametype ")) {
      continue;
    }

    bool has_name = false;
    for (size_t j = i + 1; j < lines.size(); ++j) {
      const std::string_view next = trimmed(lines[j]);
      if (next.empty() || next.starts_with("gametype ") ||
          next.starts_with("playlist ")) {
        break;
      }
      has_name |= next.starts_with("nameref ");
    }

    if (!has_name) {
      out += "nameref PLAYLIST_GAMETYPE_" +
             utils::string::to_upper(std::string(line.substr(9))) + '\n';
    }
  }
  return out;
}

int read_playlists_stub(const char *qpath, void **buffer) {
  const int len = utils::hook::invoke<int>(0x140564F70_g, qpath, buffer);
  if (len <= 0 || !*buffer) {
    return len;
  }

  const std::string data(static_cast<const char *>(*buffer),
                         static_cast<size_t>(len));
  const std::string fixed = add_missing_gametype_names(data);
  if (fixed.size() == data.size()) {
    return len;
  }

  utils::hook::invoke<void>(0x140564F30_g, *buffer);
  ++(*game::fs::fs_loadStack);
  char *buf = game::fs::FS_AllocMem(fixed.size() + 1);
  fixed.copy(buf, fixed.size());
  buf[fixed.size()] = '\0';
  *buffer = buf;
  return static_cast<int>(fixed.size());
}

} // namespace

struct component final : server_component {
  DEFINE_COMPONENT_NAME("core");

  void post_unpack() override {
    // Fix infinite loop
    utils::hook::jump(0x1402E86B0_g, scr_are_textures_loaded_stub);

    // Online classes
    utils::hook::jump(0x1405003E0_g, get_online_mode_stub);
    utils::hook::jump(0x1405003B0_g, get_online_mode_stub);

    // Progression / Ranked
    utils::hook::jump(0x140500A50_g, is_online_stub);
    utils::hook::jump(0x140500980_g, is_online_stub);
    utils::hook::jump(0x1402565D0_g, is_online_stub);
    patch_is_mod_loaded_checks();

    spawn_server_hook.create(game::sv::SV_SpawnServer, spawn_server_stub);

    utils::hook::call(0x14055BB10_g, read_playlists_stub);
    utils::hook::call(0x140560100_g, read_playlists_stub);

    // Don't count server as client
    utils::hook::jump(0x14052F0F5_g, 0x14052F139_g);

    utils::hook::call(0x1402853D7_g,
                      sv_get_player_xuid_stub); // PlayerCmd_GetXuid

    // Stop executing default_dedicated.cfg & language_settings.cfg
    utils::hook::set<uint8_t>(0x1405063C0_g, 0xC3);

    // change 32 character max name limit to 15
    set_max_name_characters<15>();

    // Disable Unknown Soldier with a number
    utils::hook::call(0x140531311_g, info_set_value_for_key_stub);
    utils::hook::call(0x1405311E0_g, va_stub);
    utils::hook::call(0x140531227_g, va_stub);
  }
};
} // namespace dedicated

REGISTER_COMPONENT(dedicated::component)
