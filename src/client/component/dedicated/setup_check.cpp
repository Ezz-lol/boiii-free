#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "../scheduler.hpp"
#include "../workshop/workshop.hpp"
#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/nt.hpp>
#include <utils/string.hpp>

namespace dedicated {
namespace {
constexpr std::string_view LOBBY_MODES[] = {"zm", "mp",  "cp",   "cpzm",
                                            "fr", "doa", "arena"};
constexpr std::string_view ROTATION_KEYS[] = {"gametype", "map", "exec"};
constexpr std::string_view MP_GAMETYPES[] = {
    "tdm",   "dm",     "ctf",  "dom",    "sd",  "koth", "conf", "gun",  "prop",
    "clean", "escort", "ball", "infect", "dem", "hq",   "sas",  "shrp", "oic"};
constexpr std::string_view ZM_GAMETYPES[] = {"zclassic", "zstandard", "zgrief",
                                             "zcleansed"};

bool contains(const auto &list, const std::string_view value) {
  return std::ranges::find(list, value) != std::ranges::end(list);
}

void warn(const std::string &message) {
  printf("[WARNING] %s\n", message.c_str());
}

bool path_exists(const std::filesystem::path &path) {
  std::error_code ec;
  return std::filesystem::exists(path, ec);
}

std::string get_string(const char *dvar) {
  return std::string(game::get_dvar_string(dvar).value_or(""));
}

std::string get_mod() {
  const std::string mod = get_string("fs_game");
  return mod == "usermaps" ? std::string{} : mod;
}

bool has_workshop_json(const std::filesystem::path &dir) {
  return path_exists(dir / "workshop.json") ||
         path_exists(dir / "zone" / "workshop.json");
}

bool has_zone_file(const std::filesystem::path &dir, const std::string &name) {
  return path_exists(dir / name) || path_exists(dir / "zone" / name);
}

std::filesystem::path find_usermap(const std::string &map) {
  const std::filesystem::path installed = workshop::find_installed(map);
  if (!installed.empty()) {
    return installed;
  }

  const std::filesystem::path &game_path = game::get_game_path();
  std::error_code ec;
  for (const std::filesystem::path &root :
       {game_path / "usermaps", game_path.parent_path().parent_path() /
                                    "workshop" / "content" /
                                    game::APP_ID_STR}) {
    for (const auto &entry : std::filesystem::directory_iterator(root, ec)) {
      if (entry.is_directory(ec) && has_zone_file(entry.path(), map + ".ff")) {
        return entry.path();
      }
    }
  }
  return {};
}

void check_data_files() {
  const utils::nt::library host{};
  const std::filesystem::path data = game::get_appdata_path() / "data";
  const std::filesystem::path script = std::filesystem::path("lobby_scripts") /
                                       "server_lobby_selector" / "__init__.lua";
  if (!path_exists(data / script) &&
      !path_exists(host.get_folder() / "boiii" / script)) {
    warn(std::format("The server lobby script is missing: {}. The server "
                     "can't start a match without it. Start boiii.exe once "
                     "without -noupdate so it can download its data files.",
                     (data / script).string()));
  }

  if (!path_exists(data / "gamesettings") &&
      !path_exists(host.get_folder() / "boiii" / "gamesettings")) {
    warn(std::format("The gamesettings folder is missing: {}. Some game "
                     "modes may not play correctly. Start boiii.exe once "
                     "without -noupdate so it can download its data files.",
                     (data / "gamesettings").string()));
  }

  const std::filesystem::path playlists =
      game::get_game_path() / "machinecfg" / "playlists.info";
  if (!path_exists(playlists)) {
    warn(std::format("{} is missing, so the server will stop with \"Unknown "
                     "gametype name '' in playlist\". Copy it from the Black "
                     "Ops III Unranked Dedicated Server files.",
                     playlists.string()));
  }
}

void check_config_file(std::string name) {
  if (std::filesystem::path(name).extension().empty()) {
    name += ".cfg";
  }

  const std::filesystem::path &game_path = game::get_game_path();
  std::vector<std::filesystem::path> dirs = {
      game_path / "zone", game_path / "players", game_path / "main",
      game_path / "local_storage", game_path / "machinecfg"};
  if (const std::string mod = get_mod(); !mod.empty()) {
    dirs.push_back(game_path / "mods" / mod);
    dirs.push_back(game_path / "mods" / mod / "zone");
  }

  if (std::ranges::any_of(
          dirs, [&](const auto &dir) { return path_exists(dir / name); })) {
    return;
  }

  const std::filesystem::path expected = game_path / "zone" / name;
  if (path_exists(game_path / name)) {
    warn(std::format("Config file '{}' is in the game folder, but the server "
                     "only reads it from the zone folder. Move it to {}",
                     name, expected.string()));
  } else if (path_exists(game_path / "zone" / (name + ".txt")) ||
             path_exists(game_path / (name + ".txt"))) {
    warn(std::format("Config file '{}' is saved as '{}.txt'. Turn on file "
                     "name extensions in Explorer and remove the .txt.",
                     name, name));
  } else {
    warn(std::format("Config file '{}' was not found, so none of your "
                     "server settings were loaded. Put it in {}",
                     name, expected.string()));
  }
}

void check_command_line() {
  int32_t count = 0;
  LPWSTR *const argv = CommandLineToArgvW(GetCommandLineW(), &count);
  if (!argv) {
    return;
  }

  for (int32_t i = 0; i < count; ++i) {
    if (_wcsicmp(argv[i], L"+exec") == 0 && i + 1 < count) {
      check_config_file(utils::string::convert(argv[i + 1]));
    } else if (_wcsicmp(argv[i], L"+map_rotate") == 0 ||
               _wcsicmp(argv[i], L"+map") == 0) {
      warn(std::format("Remove {} from the start command. The server starts "
                       "the map by itself once the lobby is ready, and "
                       "starting it early breaks the lobby.",
                       utils::string::convert(argv[i])));
    }
  }
  LocalFree(argv);
}

void check_mod() {
  const std::string mod = get_mod();
  if (mod.empty()) {
    return;
  }

  const std::filesystem::path folder = workshop::find_installed(mod);
  if (folder.empty()) {
    warn(std::format("Mod '{}' (fs_game) was not found. Put it in {}", mod,
                     (game::get_game_path() / "mods" / mod).string()));
  } else if (!has_workshop_json(folder)) {
    warn(std::format("Mod '{}' in {} has no workshop.json, so the game "
                     "can't load it.",
                     mod, folder.string()));
  }
}

std::string_view map_mode(const std::string &map) {
  if (map.starts_with("zm_")) {
    return "zm";
  }
  if (map.starts_with("mp_")) {
    return "mp";
  }
  if (map.starts_with("cp_")) {
    return "cp";
  }
  return {};
}

bool lobby_mode_fits(const std::string_view map_type,
                     const std::string &lobby_mode) {
  if (map_type == "zm") {
    return lobby_mode == "zm";
  }
  if (map_type == "mp") {
    return lobby_mode.empty() || lobby_mode == "mp" || lobby_mode == "arena" ||
           lobby_mode == "fr";
  }
  if (map_type == "cp") {
    return lobby_mode == "cp" || lobby_mode == "cpzm" || lobby_mode == "doa";
  }
  return true;
}

void check_map(const std::string &map, const std::string &gametype,
               const std::string &lobby_mode) {
  const std::filesystem::path &game_path = game::get_game_path();
  const std::string mod = get_mod();
  const std::string file = map + ".ff";

  if (!has_zone_file(game_path, file) &&
      (mod.empty() || !has_zone_file(game_path / "mods" / mod, file))) {
    const std::filesystem::path folder = find_usermap(map);
    if (folder.empty()) {
      warn(std::format(
          "Map '{}' was not found. Stock maps need their DLC "
          "installed, custom maps go in {}",
          map, (game_path / "usermaps" / map / "zone" / file).string()));
    } else if (!has_workshop_json(folder)) {
      warn(std::format("Custom map '{}' in {} has no workshop.json, so the "
                       "game can't load it.",
                       map, folder.string()));
    }
  }

  const std::string_view type = map_mode(map);
  if (!lobby_mode_fits(type, lobby_mode)) {
    warn(std::format("Map '{}' is a {} map, but sv_lobby_mode is \"{}\". Set "
                     "sv_lobby_mode \"{}\"",
                     map, type, lobby_mode, type));
  }

  if ((type == "zm" && contains(MP_GAMETYPES, gametype)) ||
      (type != "zm" && contains(ZM_GAMETYPES, gametype))) {
    warn(std::format("Gametype '{}' can't be played on map '{}'.", gametype,
                     map));
  }
}

void check_rotation(const std::string &lobby_mode) {
  const std::string rotation(game::maprotation().value_or(""));
  const bool skip_lobby = game::get_dvar_int("sv_skip_lobby").value_or(0) != 0;
  if (rotation.empty()) {
    if (skip_lobby) {
      warn("sv_maprotation is empty, so the server has no map to start. "
           "Example: set sv_maprotation \"gametype zclassic map zm_zod\"");
    }
    return;
  }

  std::vector<std::string> tokens;
  std::istringstream stream(rotation);
  for (std::string token; stream >> token;) {
    tokens.push_back(std::move(token));
  }

  std::string gametype = get_string("g_gametype");
  bool has_map = false;
  for (size_t i = 0; i < tokens.size(); i += 2) {
    if (!contains(ROTATION_KEYS, tokens[i])) {
      warn(std::format("sv_maprotation has an unknown word \"{}\". Only "
                       "gametype, map and exec are allowed.",
                       tokens[i]));
      return;
    }
    if (i + 1 == tokens.size()) {
      warn(std::format("sv_maprotation ends with \"{}\" but no value. Put the "
                       "whole rotation in quotes, for example: set "
                       "sv_maprotation \"gametype zclassic map zm_zod\"",
                       tokens[i]));
      return;
    }
    if (tokens[i] == "gametype") {
      gametype = tokens[i + 1];
    } else if (tokens[i] == "map") {
      has_map = true;
      check_map(tokens[i + 1], gametype, lobby_mode);
    }
  }

  if (!has_map) {
    warn("sv_maprotation has no map in it. Example: set sv_maprotation "
         "\"gametype zclassic map zm_zod\"");
  } else if (skip_lobby && (tokens.size() < 4 || tokens[0] != "gametype" ||
                            tokens[2] != "map")) {
    warn("With sv_skip_lobby 1, sv_maprotation must start with \"gametype "
         "<type> map <map>\", otherwise the first map is not used.");
  }
}

void check_dvars() {
  const std::string lobby_mode = get_string("sv_lobby_mode");
  if (!lobby_mode.empty() && !contains(LOBBY_MODES, lobby_mode)) {
    warn(std::format("sv_lobby_mode \"{}\" is not valid. Use one of: zm, mp, "
                     "cp, cpzm, fr, doa, arena.",
                     lobby_mode));
  }

  check_rotation(lobby_mode);

  if (get_string("rcon_password").empty()) {
    warn("rcon_password is empty, so rcon and admin tools like IW4MAdmin "
         "can't connect.");
  }
  if (get_string("g_log").empty()) {
    warn("g_log is empty, so no game log is written and admin tools like "
         "IW4MAdmin can't follow the match.");
  }
}

void run_checks() {
  check_data_files();
  check_command_line();
  check_mod();
  check_dvars();
}
} // namespace

struct setup_check final : server_component {
  DEFINE_COMPONENT_NAME("setup_check");

  void post_unpack() override {
    scheduler::once(run_checks, scheduler::pipeline::main, 2s);
  }
};
} // namespace dedicated

REGISTER_COMPONENT(dedicated::setup_check)
