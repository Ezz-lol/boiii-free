#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

#include "game/ui_scripting/execution.hpp"

#include "command.hpp"
#include "ui_scripting.hpp"
#include "scheduler.hpp"
#include "friends.hpp"
#include "discord.hpp"
#include "network.hpp"

#include "../steam/steam.hpp"
#include "../steam/interfaces/matchmaking_servers.hpp"
#include "getinfo.hpp"
#include "toast.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>
#include <utils/http.hpp>

#include <filesystem>
#include <unordered_map>

using namespace game::db;
using namespace game::db::xasset;
using namespace game::ugc;

namespace ui_scripting {
namespace {
std::unordered_map<game::ui::lua::hks::cclosure *,
                   std::function<arguments(const function_arguments &args)>>
    converted_functions;

utils::hook::detour ui_init_hook;
utils::hook::detour ui_cod_init_hook;
utils::hook::detour ui_cod_lobbyui_init_hook;
utils::hook::detour cl_first_snapshot_hook;
utils::hook::detour ui_shutdown_hook;
utils::hook::detour hks_package_require_hook;
utils::hook::detour lua_cod_getrawfile_hook;
utils::hook::detour lua_error_hook;
utils::hook::detour lua_error_print_hook;
utils::hook::detour hksi_lua_getinfo_detour;

bool unsafe_function_called_message_shown = false;
bool unsafe_lua_approved_for_session = false;

using lua_function_t = int (*)(game::ui::lua::hks::lua_State *);
std::unordered_map<size_t, utils::hook::detour> unsafe_function_detours;

struct globals_t {
  std::string in_require_script;
  std::unordered_map<std::string, std::string> loaded_scripts;
  std::unordered_map<std::string, std::string> local_scripts;
  bool load_raw_script{};
  std::string raw_script_name{};
};

globals_t globals;

// Hot reload state
std::string hot_reload_path;
std::map<std::string, std::filesystem::file_time_type> hot_reload_files;
bool hot_reload_running = false;
bool hot_reload_in_game = false;

bool execute_raw_lua(const std::string &code,
                     const char *chunk_name = "hot_reload") {
  const auto state = *game::ui::lua::hks::lua_state;
  if (!state)
    return false;

  try {
    const table lua = state->globals.v.table;
    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;
    const auto load_results = lua["loadstring"](code, chunk_name);
    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_SECURE;

    if (load_results[0].is<function>()) {
      const auto results = lua["pcall"](load_results);
      if (!results[0].as<bool>()) {
        auto err = results[1].as<std::string>();
        game::com::Com_Printf(0, 0, "^1Lua Error [%s]: %s\n", chunk_name,
                              err.c_str());
        return false;
      }
      return true;
    } else if (load_results[1].is<std::string>()) {
      auto err = load_results[1].as<std::string>();
      game::com::Com_Printf(0, 0, "^1Lua Compile Error [%s]: %s\n", chunk_name,
                            err.c_str());
    }
  } catch (const std::exception &ex) {
    game::com::Com_Printf(0, 0, "^1Lua Error [%s]: %s\n", chunk_name,
                          ex.what());
  }

  return false;
}

void fire_debug_reload(const char *root_name) {
  const auto mapname = game::get_dvar_string("mapname");
  const auto code =
      utils::string::va("pcall(function() LUI.roots.%s:processEvent({ name = "
                        "'debug_reload', mapname = '%s' }) end)",
                        root_name, mapname.c_str());
  execute_raw_lua(code, "DebugReload");
}

int hot_reload_check_files() {
  if (hot_reload_path.empty())
    return 0;

  std::vector<std::filesystem::directory_entry> changed;

  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(hot_reload_path)) {
      if (!entry.is_regular_file())
        continue;
      if (entry.path().extension() != ".lua")
        continue;

      const auto path_str = entry.path().string();
      const auto mod_time = entry.last_write_time();

      auto it = hot_reload_files.find(path_str);
      if (it == hot_reload_files.end()) {
        hot_reload_files[path_str] = mod_time;
        changed.push_back(entry);
      } else if (it->second < mod_time) {
        it->second = mod_time;
        changed.push_back(entry);
      }
    }
  } catch (const std::exception &ex) {
    game::com::Com_Printf(0, 0, "^1Hot Reload: Error scanning: %s\n",
                          ex.what());
    return 0;
  }

  if (changed.empty())
    return 0;

  game::com::Com_Printf(0, 0, "^2Hot Reload: Found %d file(s) to reload\n",
                        static_cast<int>(changed.size()));

  for (const auto &entry : changed) {
    const auto path_str = entry.path().string();
    std::string data;
    if (!utils::io::read_file(path_str, &data))
      continue;

    // Use relative path as chunk name
    auto chunk = path_str;
    if (chunk.starts_with(hot_reload_path)) {
      chunk = chunk.substr(hot_reload_path.size());
    }

    if (execute_raw_lua(data, chunk.c_str())) {
      game::com::Com_Printf(0, 0, "^2Hot Reload: Reloaded %s\n", chunk.c_str());
    } else {
      game::com::Com_Printf(0, 0, "^1Hot Reload: Error reloading %s\n",
                            chunk.c_str());
    }
  }

  // Fire debug_reload events to refresh UI
  fire_debug_reload("UIRootFull");
  if (hot_reload_in_game) {
    fire_debug_reload("UIRoot0");
    fire_debug_reload("UIRoot1");
  }

  return 1;
}

void start_hot_reload(const std::string &path) {
  hot_reload_path = path;
  hot_reload_files.clear();
  hot_reload_running = true;

  // Initial scan to populate timestamps
  hot_reload_check_files();

  // Install a Lua timer that calls back into C++ every second
  const std::string lua_code =
      "pcall(function() "
      "local UIRootFull = LUI.roots.UIRootFull; "
      "if UIRootFull.HotReloadTimer then "
      "UIRootFull:removeElement(UIRootFull.HotReloadTimer) end; "
      "UIRootFull.HotReloadTimer = LUI.UITimer.newElementTimer(1000, false, "
      "function() "
      "game.hotreloadcheck(); "
      "end); "
      "UIRootFull:addElement(UIRootFull.HotReloadTimer); "
      "end)";

  execute_raw_lua(lua_code, "HotReloadTimer");
  game::com::Com_Printf(0, 0, "^2Hot Reload: Watching '%s'\n", path.c_str());
}

void stop_hot_reload() {
  if (!hot_reload_running) {
    game::com::Com_Printf(0, 0, "^3Hot Reload: Not currently watching.\n");
    return;
  }

  hot_reload_running = false;
  hot_reload_files.clear();
  hot_reload_path.clear();

  // Remove the Lua timer
  const std::string lua_code =
      "pcall(function() "
      "local UIRootFull = LUI.roots.UIRootFull; "
      "if UIRootFull.HotReloadTimer then "
      "UIRootFull:removeElement(UIRootFull.HotReloadTimer); "
      "UIRootFull.HotReloadTimer = nil end; "
      "end)";
  execute_raw_lua(lua_code, "HotReloadTimerStop");

  game::com::Com_Printf(0, 0, "^2Hot Reload: Stopped watching.\n");
}

bool is_loaded_script(const std::string &name) {
  return globals.loaded_scripts.contains(name);
}

bool is_local_script(const std::string &name) {
  if (globals.local_scripts.contains(name))
    return true;
  if (!name.ends_with(".lua") && globals.local_scripts.contains(name + ".lua"))
    return true;
  return false;
}

std::string get_root_script(const std::string &name) {
  const auto itr = globals.loaded_scripts.find(name);
  return (itr == globals.loaded_scripts.end()) ? std::string() : itr->second;
}

table get_globals() {
  const auto state = *game::ui::lua::hks::lua_state;
  return state->globals.v.table;
}

void print_error(const std::string &error) {
  game::com::Com_Printf(0, 0,
                        "^1************** LUI script error **************\n");
  game::com::Com_Printf(0, 0, "^1%s\n", error.data());
  game::com::Com_Printf(0, 0,
                        "^1**********************************************\n");

  auto popup_msg = error;
  scheduler::once(
      [popup_msg] {
        game::ui::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
                                               popup_msg.c_str());
      },
      scheduler::main, 1s);
}

void print_loading_script(const std::string &name) {
  printf("Loading LUI script '%s'\n", name.data());
}

std::string get_current_script(game::ui::lua::hks::lua_State *state) {
  game::ui::lua::hks::lua_Debug info{};
  game::ui::lua::hks::hksi_lua_getstack(state, 1, &info);
  game::ui::lua::hks::hksi_lua_getinfo(state, "nSl", &info);
  return info.short_src;
}

int load_buffer(const std::string &name, const std::string &data) {
  const auto state = *game::ui::lua::hks::lua_state;
  const auto sharing_mode = state->m_global->m_bytecodeSharingMode;
  state->m_global->m_bytecodeSharingMode =
      game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;

  const auto _0 = utils::finally(
      [&] { state->m_global->m_bytecodeSharingMode = sharing_mode; });

  game::ui::lua::hks::HksCompilerSettings compiler_settings{};
  return game::ui::lua::hks::hksi_hksL_loadbuffer(
      state, &compiler_settings, data.data(), data.size(), name.data());
}

void load_script(const std::string &name, const std::string &data,
                 const std::string &display_name = "") {
  globals.loaded_scripts[name] = name;
  const auto chunk = display_name.empty() ? name : display_name;
  if (!display_name.empty() && display_name != name) {
    globals.loaded_scripts[display_name] = name;
  }

  const auto state = *game::ui::lua::hks::lua_state;
  const auto lua = get_globals();
  state->m_global->m_bytecodeSharingMode =
      game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;
  const auto load_results = lua["loadstring"](data, chunk);
  state->m_global->m_bytecodeSharingMode =
      game::ui::lua::hks::HKS_BYTECODE_SHARING_SECURE;

  if (load_results[0].is<function>()) {
    const auto results = lua["pcall"](load_results);
    if (!results[0].as<bool>()) {
      print_error(results[1].as<std::string>());
    }
  } else if (load_results[1].is<std::string>()) {
    print_error(load_results[1].as<std::string>());
  }
}

void load_local_script_files(const std::string &script_dir) {
  if (!utils::io::directory_exists(script_dir)) {
    return;
  }

  const auto scripts = utils::io::list_files(script_dir);

  for (const auto &script : scripts) {
    const auto script_file = script.generic_string();

    if (std::filesystem::is_regular_file(script)) {
      const std::string file_path =
          script_file.substr(script_file.find("ui_scripts") + 11);
      globals.local_scripts[file_path] = script_file;
    } else if (std::filesystem::is_directory(script)) {
      load_local_script_files(script_file);
    }
  }
}

void load_scripts(const std::string &script_dir) {
  if (!utils::io::directory_exists(script_dir)) {
    return;
  }

  load_local_script_files(script_dir);

  const auto scripts = utils::io::list_files(script_dir);

  for (const auto &script : scripts) {
    std::string data;
    const auto script_file = script.generic_string();
    if (std::filesystem::is_directory(script) &&
        utils::io::read_file(script_file + "/__init__.lua", &data)) {
      print_loading_script(script_file);
      const auto full_path = script_file + "/__init__.lua";
      const auto display_name =
          script.filename().generic_string() + "/__init__.lua";
      load_script(full_path, data, display_name);
    }
  }
}

void setup_functions() {
  const auto lua = get_globals();
  lua["game"] = table();

  lua["game"]["getfriendcount"] = function(
      convert_function([]() -> int { return friends::get_friend_count(); }),
      game::ui::lua::hks::TCFUNCTION);

  lua["game"]["getfriend"] =
      function(convert_function([](int index) -> table {
                 auto f = friends::get_friend_by_index(index);
                 auto t = table();
                 t.set("steam_id", utils::string::va("%llu", f.steam_id));
                 t.set("name", std::string(f.name));
                 t.set("status", static_cast<int>(f.state));
                 t.set("server", std::string(f.server_address));
                 return t;
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["addfriend"] =
      function(convert_function([](const std::string &steam_id_str,
                                   const std::string &name) {
                 uint64_t steam_id = 0;
                 try {
                   steam_id = std::stoull(steam_id_str);
                 } catch (...) {
                   return;
                 }
                 friends::add_friend(steam_id, name);
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["removefriend"] =
      function(convert_function([](const std::string &steam_id_str) {
                 uint64_t steam_id = 0;
                 try {
                   steam_id = std::stoull(steam_id_str);
                 } catch (...) {
                   return;
                 }
                 friends::remove_friend(steam_id);
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["invitefriend"] =
      function(convert_function([](const std::string &id_str) -> bool {
                 uint64_t id = 0;
                 try {
                   id = std::stoull(id_str);
                 } catch (...) {
                   return false;
                 }
                 return friends::invite_to_game(id);
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["connecttofriend"] =
      function(convert_function([](const std::string &id_str) -> bool {
                 uint64_t id = 0;
                 try {
                   id = std::stoull(id_str);
                 } catch (...) {
                   return false;
                 }
                 return friends::connect_to_friend(id);
               }),
               game::ui::lua::hks::TCFUNCTION);

  // HTTP functions
  lua["game"]["httpget"] =
      function(convert_function([](const std::string &url) -> std::string {
                 const auto result = utils::http::get_data(url);
                 return result.value_or("");
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["httppost"] =
      function(convert_function([](const std::string &url,
                                   const std::string &body) -> std::string {
                 const auto result = utils::http::post_data(url, body);
                 return result.value_or("");
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["setDiscordPlayerScore"] = function(
      convert_function([](int score) { discord::set_player_score(score); }),
      game::ui::lua::hks::TCFUNCTION);

  lua["game"]["setDiscordEnemyScore"] = function(
      convert_function([](int score) { discord::set_enemy_score(score); }),
      game::ui::lua::hks::TCFUNCTION);

  lua["game"]["setDiscordRoundsPlayed"] = function(
      convert_function([](int round) { discord::set_rounds_played(round); }),
      game::ui::lua::hks::TCFUNCTION);

  // Hot reload functions (callable from Lua timers)
  lua["game"]["hotreloadcheck"] = function(convert_function([]() {
                                             if (hot_reload_running) {
                                               hot_reload_check_files();
                                             }
                                           }),
                                           game::ui::lua::hks::TCFUNCTION);

  lua["game"]["hotreloadstart"] = function(
      convert_function([](const std::string &path) { start_hot_reload(path); }),
      game::ui::lua::hks::TCFUNCTION);

  lua["game"]["getrawservercount"] =
      function(convert_function([]() -> int {
                 return steam::get_raw_internet_server_count();
               }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["isserverlistrefreshing"] =
      function(convert_function(
                   []() -> bool { return steam::is_server_list_refreshing(); }),
               game::ui::lua::hks::TCFUNCTION);

  lua["game"]["getrawserverinfo"] =
      function(convert_function([](const int index) -> table {
                 auto t = table();
                 const auto *item = steam::get_raw_internet_server_item(index);
                 if (!item) {
                   return t;
                 }

                 t.set("name", std::string(item->m_szServerName));
                 t.set("map", std::string(item->m_szMap));
                 t.set("desc", std::string(item->m_szGameDescription));
                 t.set("ping", item->m_nPing);
                 t.set("playerCount", item->m_nPlayers);
                 t.set("maxPlayers", item->m_nMaxPlayers);
                 t.set("botCount", item->m_nBotPlayers);
                 t.set("password", item->m_bPassword);
                 t.set("secure", item->m_bSecure);

                 const auto tags = std::string(item->m_szGameTags);
                 const auto get_tag = [&](const char *key) -> std::string {
                   const auto *val =
                       game::info::Info_ValueForKey(tags.c_str(), key);
                   return val ? val : "";
                 };

                 t.set("gametype", get_tag("gametype"));
                 t.set("dedicated", get_tag("dedicated") == "true");
                 t.set("ranked", get_tag("ranked") == "true");
                 t.set("hardcore", get_tag("hardcore") == "true");
                 t.set("zombies", get_tag("zombies") == "true");
                 t.set("campaign", get_tag("campaign") == "true" ? 1 : 0);
                 t.set("rounds", std::atoi(get_tag("rounds").c_str()));
                 t.set("modName", get_tag("modName"));

                 const auto ip = item->m_NetAdr.m_unIP;
                 const auto port = item->m_NetAdr.m_usConnectionPort;
                 t.set("connectAddr",
                       utils::string::va("%u.%u.%u.%u:%u", (ip >> 24) & 0xFF,
                                         (ip >> 16) & 0xFF, (ip >> 8) & 0xFF,
                                         ip & 0xFF, port));
                 t.set("serverIndex", index);

                 return t;
               }),
               game::ui::lua::hks::TCFUNCTION);
}

void enable_globals() {
  const auto lua = get_globals();
  const std::string code = "local g = getmetatable(_G)\n"
                           "if not g then\n"
                           "g = {}\n"
                           "setmetatable(_G, g)\n"
                           "end\n"
                           "g.__newindex = nil\n";

  const auto state = *game::ui::lua::hks::lua_state;
  state->m_global->m_bytecodeSharingMode =
      game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;
  lua["loadstring"](code)[0]();
  state->m_global->m_bytecodeSharingMode =
      game::ui::lua::hks::HKS_BYTECODE_SHARING_SECURE;
}

void setup_lua_globals() {
  globals = {};

  const auto lua = get_globals();
  enable_globals();

  setup_functions();

  if (!game::is_server()) {
    lua["print"] = function(reinterpret_cast<game::ui::lua::hks::lua_function>(
        0x141D30290_g)); // hks::base_print
  }
  lua["table"]["unpack"] = lua["unpack"];
  lua["luiglobals"] = lua;

  // Expose IsBOIII for mod compatibility - both as a value and function
  lua["Engine"]["IsBOIII"] = true;
  lua["Engine"]["IsEZZBOIII"] = true;
  // lua["IsBOIII"] = function([]() { return false; });
}

void start() {
  setup_lua_globals();

  const utils::nt::library host{};
  const auto folder = game::is_server() ? "lobby_scripts/" : "ui_scripts/";
  load_scripts((game::get_appdata_path() / "data" / folder).string());
  load_scripts((host.get_folder() / "boiii" / folder).string());
}

void try_start() {
  try {
    start();
  } catch (const std::exception &ex) {
    printf("Failed to load LUI scripts: %s\n", ex.what());
  }
}

void ui_init_stub(void *allocFunction, void *outOfMemoryFunction) {
  ui_init_hook.invoke(allocFunction, outOfMemoryFunction);

  setup_lua_globals();
}

bool doneFirstSnapshot = false;

void ui_cod_init_stub(const bool frontend) {
  ui_cod_init_hook.invoke(frontend);

  if (!game::is_server() && game::com::Com_IsRunningUILevel()) {
    // Fetch the names of the local files so file overrides are already handled
    globals = {};
    const utils::nt::library host{};
    doneFirstSnapshot = false;

    load_local_script_files(
        (game::get_appdata_path() / "data/ui_scripts/").string());
    load_local_script_files((host.get_folder() / "boiii/ui_scripts/").string());
    return;
  }
  try_start();
}

void ui_cod_lobbyui_init_stub() {
  ui_cod_lobbyui_init_hook.invoke();
  try_start();
}

void inject_discord_score_subscriptions() {
  const std::string lua_code =
      "LUI.roots.UIRoot0:subscribeToGlobalModel(0, 'GameScore', 'playerScore', "
      "function(model) "
      "local score = Engine.GetModelValue(model); "
      "if score and not Engine.IsVisibilityBitSet( 0, "
      "Enum.UIVisibilityBit.BIT_IN_KILLCAM ) then "
      "game.setDiscordPlayerScore(score); "
      "end; "
      "end); "
      "LUI.roots.UIRoot0:subscribeToGlobalModel(0, 'GameScore', 'enemyScore', "
      "function(model) "
      "local score = Engine.GetModelValue(model); "
      "if score and not Engine.IsVisibilityBitSet( 0, "
      "Enum.UIVisibilityBit.BIT_IN_KILLCAM ) then "
      "game.setDiscordEnemyScore(score); "
      "end; "
      "end); "
      "LUI.roots.UIRoot0:subscribeToGlobalModel(0, 'GameScore', "
      "'roundsPlayed', function(model) "
      "local roundsPlayed = Engine.GetModelValue(model); "
      "if roundsPlayed then "
      "game.setDiscordRoundsPlayed(roundsPlayed - 1); "
      "end; "
      "end); ";

  execute_raw_lua(lua_code, "discord_score_hooks");
}

void cl_first_snapshot_stub(int a1) {
  cl_first_snapshot_hook.invoke(a1);

  if (game::com::Com_IsRunningUILevel() || doneFirstSnapshot) {
    return;
  }
  doneFirstSnapshot = true;
  hot_reload_in_game = true;
  try_start();

  try {
    inject_discord_score_subscriptions();
  } catch (...) {
  }
}

void ui_shutdown_stub() {
  converted_functions.clear();
  globals = {};
  hot_reload_in_game = false;
  unsafe_function_called_message_shown = false;
  unsafe_lua_approved_for_session = false;
  return ui_shutdown_hook.invoke<void>();
}

void *hks_package_require_stub(game::ui::lua::hks::lua_State *state) {
  const auto script = get_current_script(state);
  const auto root = get_root_script(script);
  globals.in_require_script = root;
  return hks_package_require_hook.invoke<void *>(state);
}

int hks_load_stub(game::ui::lua::hks::lua_State *state, void *compiler_options,
                  void *reader, void *reader_data, void *debug_reader,
                  void *debug_reader_data, const char *chunk_name) {
  if (globals.load_raw_script) {
    globals.load_raw_script = false;
    globals.loaded_scripts[globals.raw_script_name] = globals.in_require_script;
    return load_buffer(globals.raw_script_name,
                       utils::io::read_file(globals.raw_script_name));
  }

  return utils::hook::invoke<int>(game::select(0x141D3AFB0, 0x1403E4090), state,
                                  compiler_options, reader, reader_data,
                                  debug_reader, debug_reader_data, chunk_name);
}

xasset::XAssetHeader lua_cod_getrawfile_stub(char *filename) {
  if (!is_loaded_script(globals.in_require_script) &&
      !is_local_script(filename)) {
    return lua_cod_getrawfile_hook.invoke<xasset::XAssetHeader>(filename);
  }

  const std::string name_ = filename;
  std::string target_script;
  if (is_loaded_script(globals.in_require_script)) {
    const auto folder = globals.in_require_script.substr(
        0, globals.in_require_script.find_last_of("/\\"));
    if (name_.ends_with(".lua")) {
      target_script = folder + "/" + name_;
    } else {
      target_script = folder + "/" + name_ + ".lua";
    }
  } else {
    if (globals.local_scripts.contains(name_)) {
      target_script = globals.local_scripts[name_];
    } else if (!name_.ends_with(".lua") &&
               globals.local_scripts.contains(name_ + ".lua")) {
      target_script = globals.local_scripts[name_ + ".lua"];
    }
  }

  if (utils::io::file_exists(target_script)) {
    globals.load_raw_script = true;
    globals.raw_script_name = target_script;

    return xasset::XAssetHeader{
        .luaFile = reinterpret_cast<xasset::RawFile *>(1) //
    };
  }

  return lua_cod_getrawfile_hook.invoke<xasset::XAssetHeader>(filename);
}

int luaopen_stub([[maybe_unused]] game::ui::lua::hks::lua_State *l) {
  return 0;
}

void show_unsafe_lua_dialog() {
  if (unsafe_function_called_message_shown) {
    return;
  }

  unsafe_function_called_message_shown = true;

  scheduler::once(
      [] {
        const int result = MessageBoxA(
            nullptr,
            "The map/mod you are playing tried to run code that can be "
            "unsafe.\n\n"
            "This can include:\n"
            "  - Writing or reading files on your system\n"
            "  - Accessing environment variables\n"
            "  - Running system commands\n"
            "  - Loading DLLs\n\n"
            "These features are usually used for storing data across games, "
            "integrating third party software like Discord, or fetching data "
            "from a server.\n\n"
            "However, malicious mods could use these to harm your system.\n\n"
            "Do you want to enable unsafe lua functions for this session?\n\n"
            "Click 'Yes' to enable for this session only.\n"
            "Click 'No' to keep them blocked (recommended if you don't trust "
            "this mod).",
            "Unsafe Lua Function Called",
            MB_YESNO | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND);

        if (result == IDYES) {
          unsafe_lua_approved_for_session = true;
        }
      },
      scheduler::pipeline::main);
}

template <size_t Key>
int lua_unsafe_function_stub(game::ui::lua::hks::lua_State *l) {
  if (unsafe_lua_approved_for_session) {
    return unsafe_function_detours[Key].invoke<int>(l);
  }

  show_unsafe_lua_dialog();
  return 0;
}

template <size_t Key> void hook_unsafe_function(size_t address) {
  unsafe_function_detours[Key].create(
      address, reinterpret_cast<void *>(lua_unsafe_function_stub<Key>));
}

#define HOOK_UNSAFE_FUNCTION(addr) hook_unsafe_function<addr>(addr##_g)

void patch_unsafe_lua_functions() {
  if (utils::flags::has_flag("unsafe-lua")) {
    return;
  }

  // Do not allow the HKS vm to open LUA's libraries
  // Disable unsafe functions (debug library stays completely blocked)
  utils::hook::jump(0x141D34190_g, luaopen_stub); // debug

  HOOK_UNSAFE_FUNCTION(0x141D300B0); // base_loadfile
  HOOK_UNSAFE_FUNCTION(0x141D31EE0); // base_load
  HOOK_UNSAFE_FUNCTION(0x141D2CF00); // string_dump
  HOOK_UNSAFE_FUNCTION(0x141FD3220); // engine_openurl

  HOOK_UNSAFE_FUNCTION(0x141D2AFF0); // os_getenv
  HOOK_UNSAFE_FUNCTION(0x141D2B790); // os_exit
  HOOK_UNSAFE_FUNCTION(0x141D2B7C0); // os_remove
  HOOK_UNSAFE_FUNCTION(0x141D2BB70); // os_rename
  HOOK_UNSAFE_FUNCTION(0x141D2B360); // os_tmpname
  HOOK_UNSAFE_FUNCTION(0x141D2B0F0); // os_sleep
  HOOK_UNSAFE_FUNCTION(0x141D2AF90); // os_execute

  // io helpers
  HOOK_UNSAFE_FUNCTION(0x141D32390); // io_tostring
  HOOK_UNSAFE_FUNCTION(0x141D2FDC0); // io_close_file
  HOOK_UNSAFE_FUNCTION(0x141D2FD50); // io_flush
  HOOK_UNSAFE_FUNCTION(0x141D31260); // io_lines
  HOOK_UNSAFE_FUNCTION(0x141D305C0); // io_read_file
  HOOK_UNSAFE_FUNCTION(0x141D320A0); // io_seek_file
  HOOK_UNSAFE_FUNCTION(0x141D321E0); // io_setvbuf
  HOOK_UNSAFE_FUNCTION(0x141D2FCD0); // io_write

  // io functions
  HOOK_UNSAFE_FUNCTION(0x141D2FD10); // io_write
  HOOK_UNSAFE_FUNCTION(0x141D30F40); // io_read
  HOOK_UNSAFE_FUNCTION(0x141D2FF00); // io_close
  HOOK_UNSAFE_FUNCTION(0x141D2FD90); // io_flush
  HOOK_UNSAFE_FUNCTION(0x141D313A0); // io_lines
  HOOK_UNSAFE_FUNCTION(0x141D31BA0); // io_input
  HOOK_UNSAFE_FUNCTION(0x141D31BC0); // io_output
  HOOK_UNSAFE_FUNCTION(0x141D31BE0); // io_type
  HOOK_UNSAFE_FUNCTION(0x141D31DD0); // io_open
  HOOK_UNSAFE_FUNCTION(0x141D31D70); // io_tmpfile
  HOOK_UNSAFE_FUNCTION(0x141D33C00); // io_popen

  HOOK_UNSAFE_FUNCTION(0x141D2D0C0); // serialize_persist
  HOOK_UNSAFE_FUNCTION(0x141D2D480); // serialize_unpersist

  HOOK_UNSAFE_FUNCTION(0x141D2F560); // havokscript_compiler_settings
  HOOK_UNSAFE_FUNCTION(0x141D2F660); // havokscript_setgcweights
  HOOK_UNSAFE_FUNCTION(0x141D2FB10); // havokscript_getgcweights

  HOOK_UNSAFE_FUNCTION(0x141D299C0); // package_loadlib
}
} // namespace

int main_handler(game::ui::lua::hks::lua_State *state) {
  const auto value = state->m_apistack.base[-1];
  if (value.t != game::ui::lua::hks::TCFUNCTION) {
    return 0;
  }

  const auto closure = value.v.cClosure;
  if (!converted_functions.contains(closure)) {
    return 0;
  }

  const auto &function = converted_functions[closure];

  try {
    const auto args = get_return_values();
    const auto results = function(args);

    for (const auto &result : results) {
      push_value(result);
    }

    return static_cast<int>(results.size());
  } catch (const std::exception &ex) {
    game::ui::lua::hks::hksi_luaL_error(state, ex.what());
  }

  return 0;
}

template <typename F> game::ui::lua::hks::cclosure *convert_function(F f) {
  const auto state = *game::ui::lua::hks::lua_state;
  const auto closure =
      game::ui::lua::hks::cclosure_Create(state, main_handler, 0, 0, 0);
  converted_functions[closure] = wrap_function(f);
  return closure;
}

namespace {
thread_local char getinfo_name_buf[256]{};
thread_local char getinfo_source_buf[512]{};

const char *resolve_c_function_name(uintptr_t c_func_ptr) {
  if (!c_func_ptr || game::is_server())
    return nullptr;
  auto list_head = *reinterpret_cast<uintptr_t *>(0x14365C5E0_g);
  while (list_head) {
    if (*reinterpret_cast<uintptr_t *>(list_head + 0x8) == c_func_ptr)
      return *reinterpret_cast<const char **>(list_head);
    list_head = *reinterpret_cast<uintptr_t *>(list_head + 0x18);
  }
  return nullptr;
}

const char *resolve_source_from_rawfiles(uintptr_t bytecode_header) {
  if (!bytecode_header)
    return nullptr;

  struct lookup_ctx {
    uintptr_t target;
    const char *found;
  };
  lookup_ctx ctx{bytecode_header, nullptr};

  xasset::DB_EnumXAssets(
      xasset::XAssetType::ASSET_TYPE_RAWFILE,
      [](xasset::XAssetHeader header, void *data) {
        auto *c = static_cast<lookup_ctx *>(data);
        if (c->found)
          return;
        if (header.luaFile && header.luaFile->name && header.luaFile->buffer) {
          if (reinterpret_cast<uintptr_t>(header.luaFile->buffer) == c->target)
            c->found = header.luaFile->name;
        }
      },
      &ctx, false);

  return ctx.found;
}

int hksi_lua_getinfo_stub(game::ui::lua::hks::lua_State *s, const char *what,
                          game::ui::lua::hks::lua_Debug *ar) {
  const auto result = hksi_lua_getinfo_detour.invoke<int>(s, what, ar);
  if (!result || !s || !ar)
    return result;
  if (!what || !strchr(what, 'n'))
    return result;

  auto *callstack = &s->m_callStack;
  if (!callstack->m_records || !callstack->m_current)
    return result;

  const int stack_level = ar->callstack_level;
  const auto num_records =
      static_cast<int>((reinterpret_cast<uintptr_t>(callstack->m_current) -
                        reinterpret_cast<uintptr_t>(callstack->m_records)) /
                       sizeof(game::ui::lua::hks::ActivationRecord));

  game::ui::lua::hks::HksObject *func_obj = nullptr;
  if (stack_level >= num_records) {
    if (s->m_apistack.bottom)
      func_obj = s->m_apistack.bottom - 1;
  } else if (stack_level + 1 <= num_records) {
    auto *next_record = &callstack->m_records[stack_level + 1];
    if (next_record->m_base)
      func_obj = next_record->m_base - 1;
  }

  if (!func_obj)
    return result;

  const auto obj_type = func_obj->t;
  const auto obj_value = reinterpret_cast<uintptr_t>(func_obj->v.cClosure);
  if (!obj_value)
    return result;

  if (obj_type == game::ui::lua::hks::TCFUNCTION) {
    auto c_func_ptr = *reinterpret_cast<uintptr_t *>(obj_value + 16);
    auto resolved = resolve_c_function_name(c_func_ptr);
    if (resolved && resolved[0])
      ar->name = resolved;
    else if (!ar->name || !ar->name[0])
      ar->name = "(luaC_unknown)";
  } else if (obj_type == game::ui::lua::hks::TIFUNCTION) {
    auto proto = *reinterpret_cast<uintptr_t *>(obj_value + 16);
    if (proto) {
      auto m_hash = *reinterpret_cast<uint32_t *>(proto + 16);
      auto m_numParams = *reinterpret_cast<uint8_t *>(proto + 0x18);
      auto m_debug = *reinterpret_cast<uintptr_t *>(proto + 80);

      bool name_fixed = false;
      if (m_debug) {
        auto debug_name_ptr = *reinterpret_cast<uintptr_t *>(m_debug + 48);
        if (debug_name_ptr) {
          ar->name = reinterpret_cast<const char *>(debug_name_ptr + 20);
          name_fixed = true;
        }
      }

      if (!name_fixed && m_hash &&
          (!ar->name || strcmp(ar->name, "(*stripped)") == 0 || !ar->name[0])) {
        snprintf(getinfo_name_buf, sizeof(getinfo_name_buf), "func_%X(%d)",
                 m_hash, m_numParams);
        ar->name = getinfo_name_buf;
      }

      uintptr_t pc = 0;
      if (!game::is_server()) {
        using getPC_t = uintptr_t(__fastcall *)(
            game::ui::lua::hks::lua_State *, game::ui::lua::hks::lua_Debug *);
        auto fn_getPC = reinterpret_cast<getPC_t>(0x141D46310_g);
        pc = fn_getPC(s, ar);
      }

      const char *resolved_source = nullptr;
      if (pc) {
        auto scan = pc & ~static_cast<uintptr_t>(0xF);
        for (int i = 0; i < 0x10000; i++, scan -= 0x10) {
          if (*reinterpret_cast<uint32_t *>(scan) == 0x61754C1B) {
            resolved_source = resolve_source_from_rawfiles(scan);
            break;
          }
        }
      }

      if (resolved_source) {
        snprintf(getinfo_source_buf, sizeof(getinfo_source_buf), "%s",
                 resolved_source);
        ar->source = getinfo_source_buf;
        snprintf(ar->short_src, sizeof(ar->short_src), "%s", resolved_source);
      }
    }
  }

  return result;
}

std::string colorize_lua_error(const char *error_loc, const char *error_stack) {
  std::string result;
  result.reserve(2048);

  if (error_loc) {
    result += "^1************** ";
    result += error_loc;
    result += " **************\n";
  }

  std::istringstream stream(error_stack);
  std::string line;
  bool first_line = true;
  bool in_traceback = false;

  while (std::getline(stream, line)) {
    if (first_line) {
      result += "^1";
      result += line;
      result += "\n";
      first_line = false;
      continue;
    }

    auto trimmed = line;
    while (!trimmed.empty() && (trimmed[0] == ' ' || trimmed[0] == '\t'))
      trimmed.erase(trimmed.begin());

    if (trimmed == "stack traceback:") {
      result += "^1stack traceback:\n";
      in_traceback = true;
      continue;
    }

    if (!in_traceback) {
      result += "^7" + line + "\n";
      continue;
    }

    if (trimmed.find("[C]:") == 0 || trimmed.find("<native>") == 0) {
      auto quote_start = trimmed.find('\'');
      auto quote_end = trimmed.rfind('\'');
      if (quote_start != std::string::npos && quote_end > quote_start) {
        auto func_name =
            trimmed.substr(quote_start + 1, quote_end - quote_start - 1);
        result += "\t<^1native^7>: in function '^3" + func_name + "^7'\n";
      } else {
        result += "\t<^1native^7>: " + trimmed + "\n";
      }
    } else if (trimmed.find("(tail call)") != std::string::npos) {
      result += "\t^7(tail call): ?\n";
    } else {
      auto first_colon = trimmed.find(':');
      if (first_colon != std::string::npos) {
        auto filename = trimmed.substr(0, first_colon);
        auto rest = trimmed.substr(first_colon + 1);
        auto second_colon = rest.find(':');
        if (second_colon != std::string::npos) {
          auto line_num = rest.substr(0, second_colon);
          auto remainder = rest.substr(second_colon + 1);
          result += "\t^5" + filename + "^7:^2" + line_num + "^7:";

          auto quote_start = remainder.find('\'');
          auto quote_end = remainder.rfind('\'');
          if (quote_start != std::string::npos && quote_end > quote_start) {
            auto before = remainder.substr(0, quote_start);
            auto func_name =
                remainder.substr(quote_start + 1, quote_end - quote_start - 1);
            result += before + "'^3" + func_name + "^7'\n";
          } else {
            result += "^7" + remainder + "\n";
          }
        } else {
          result += "\t^5" + filename + "^7:" + rest + "\n";
        }
      } else {
        result += "\t^7" + trimmed + "\n";
      }
    }
  }

  if (error_loc)
    result += "^1**********************************************\n";
  return result;
}

const char *safe_get_lua_error_stack(game::ui::lua::hks::lua_State *luaVM) {
  __try {
    auto *api_top = luaVM->m_apistack.top;
    auto *api_bottom = luaVM->m_apistack.bottom;

    if (api_top && api_bottom && (api_top - 1) >= api_bottom) {
      auto *top_obj = api_top - 1;
      if (top_obj->t == game::ui::lua::hks::TSTRING && top_obj->v.str) {
        return top_obj->v.str->m_data;
      }
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return nullptr;
}

void lua_cod_luastatemanager_error_stub(const char *error,
                                        game::ui::lua::hks::lua_State *luaVM) {
  if (!error || !luaVM) {
    return;
  }

  // Suppress duplicate errors - the engine often fires the same error twice
  static std::string last_error;
  static std::chrono::steady_clock::time_point last_error_time;
  const auto now = std::chrono::steady_clock::now();

  const char *error_stack = safe_get_lua_error_stack(luaVM);
  if (!error_stack) {
    error_stack = error;
  }

  // Skip empty/useless errors with no real traceback info
  const std::string stack_str(error_stack);
  if (stack_str.find('\n') != std::string::npos) {
    // Check if traceback is empty (just "stack traceback:" with no frames)
    bool has_frames = false;
    std::istringstream check(stack_str);
    std::string check_line;
    while (std::getline(check, check_line)) {
      auto trimmed = check_line;
      while (!trimmed.empty() && (trimmed[0] == ' ' || trimmed[0] == '\t'))
        trimmed.erase(trimmed.begin());
      if (trimmed.find(':') != std::string::npos &&
          trimmed != "stack traceback:") {
        has_frames = true;
        break;
      }
    }
    if (!has_frames) {
      return; // Skip errors with empty stack traces (duplicates from engine)
    }
  }

  // Deduplicate: skip if same error within 2 seconds
  if (stack_str == last_error &&
      (now - last_error_time) < std::chrono::seconds(2)) {
    return;
  }
  last_error = stack_str;
  last_error_time = now;

  const char *resolved_stack = error_stack;

  // Suppress known benign nil errors from server_browser scripts
  if (stack_str.find("Attempt to call a nil value") != std::string::npos &&
      stack_str.find("server_browser/") != std::string::npos) {
    const auto colored =
        colorize_lua_error("LUI script (suppressed)", error_stack);
    game::com::Com_Printf(0, 0, "%s", colored.c_str());
    return;
  }

  const bool is_ui = (*game::ui::lua::hks::lua_state == luaVM);
  const char *error_loc =
      is_ui ? "LUI script execution error" : "LobbyVM script execution error";

  const auto colored = colorize_lua_error(error_loc, resolved_stack);

  try {
    const auto root_path = utils::nt::library{}.get_path().parent_path();
    const auto logs_dir = root_path / "logs";
    std::filesystem::create_directories(logs_dir);
    const auto log_path = (logs_dir / "boiii_lua_errors.log").string();

    auto now_sys = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now_sys);
    tm ltime{};
    localtime_s(&ltime, &time_t);
    char timestamp[64]{};
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &ltime);

    std::ofstream log_file(log_path, std::ios::app);
    if (log_file.is_open()) {
      log_file << "\n[" << timestamp << "] " << error_loc << "\n"
               << resolved_stack << "\n";
    }
  } catch (...) {
  }

  game::com::Com_Printf(0, 0, "%s", colored.c_str());

  // Show colored error popup with delay to ensure UI is ready
  auto popup_text = colorize_lua_error(nullptr, resolved_stack);
  scheduler::once(
      [popup_text] {
        game::ui::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
                                               popup_text.c_str());
      },
      scheduler::main, 500ms);
}

void lua_error_print_stub(int, const char *, ...) {}
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    utils::hook::call(game::select(0x141D4979A, 0x1403F233A), hks_load_stub);

    hks_package_require_hook.create(game::select(0x141D28EF0, 0x1403D7FC0),
                                    hks_package_require_stub);
    ui_cod_init_hook.create(game::ui::UI_CoD_Init, ui_cod_init_stub);
    ui_cod_lobbyui_init_hook.create(game::ui::UI_CoD_LobbyUI_Init,
                                    ui_cod_lobbyui_init_stub);
    ui_shutdown_hook.create(game::select(0x14270DE00, 0x1404A1280),
                            ui_shutdown_stub);
    lua_cod_getrawfile_hook.create(game::select(0x141F0EFE0, 0x1404BCB70),
                                   lua_cod_getrawfile_stub);

    hksi_lua_getinfo_detour.create(game::select(0x141D4D8D0, 0x1403F64B0),
                                   hksi_lua_getinfo_stub);

    if (game::is_server()) {
      return;
    }

    ui_init_hook.create(0x142704FF0_g, ui_init_stub);
    cl_first_snapshot_hook.create(0x141320E60_g, cl_first_snapshot_stub);

    lua_error_hook.create(0x141F11DA0_g, lua_cod_luastatemanager_error_stub);
    lua_error_print_hook.create(0x141F132B0_g, lua_error_print_stub);

    scheduler::once(
        []() {
          game::dvar_t *dvar_callstack_ship =
              game::Dvar_FindVar("ui_error_callstack_ship");
          dvar_callstack_ship->flags = static_cast<game::dvarFlags_e>(0);
          game::dvar_t *dvar_report_delay =
              game::Dvar_FindVar("ui_error_report_delay");
          dvar_report_delay->flags = static_cast<game::dvarFlags_e>(0);

          game::Dvar_SetFromStringByName("ui_error_callstack_ship", "1", true);
          game::Dvar_SetFromStringByName("ui_error_report_delay", "0", true);
        },
        scheduler::pipeline::renderer);

    command::add("luiReload", [] {
      if (game::com::Com_IsRunningUILevel()) {
        converted_functions.clear();

        globals.loaded_scripts.clear();
        globals.local_scripts.clear();

        game::ui::UI_CoD_Shutdown();
        game::ui::UI_CoD_Init(true);

        // Com_LoadFrontEnd stripped
        game::ui::lua::Lua_CoD_LoadLuaFile(*game::ui::lua::hks::lua_state,
                                           "ui_mp.T6.main");
        game::ui::UI_AddMenu(game::ui::UI_CoD_GetRootNameForController(0),
                             "main", -1, *game::ui::lua::hks::lua_state);

        game::ui::UI_CoD_LobbyUI_Init();
      } else {
        // TODO: Find a way to do a full shutdown & restart like in frontend,
        // that opens up the loading screen that can't be easily closed
        game::cg::CG_LUIHUDRestart(0);
      }
    });

    command::add("lua_hotreload", [](const command::params &params) {
      std::string dir;
      if (params.size() >= 2) {
        dir = params.get(1);
      } else {
        dir = (game::get_appdata_path() / "data" / "ui_scripts").string();
      }

      scheduler::once(
          [dir] {
            start_hot_reload(dir);
            scheduler::once([] { toast::info("Lua", "Hot-reload started"); },
                            scheduler::pipeline::renderer, 1s);
          },
          scheduler::pipeline::renderer);
    });

    command::add("lua_hotreload_stop", [](const command::params &) {
      scheduler::once(
          [] {
            stop_hot_reload();
            scheduler::once([] { toast::info("Lua", "Hot-reload stopped"); },
                            scheduler::pipeline::renderer, 1s);
          },
          scheduler::pipeline::renderer);
    });

    command::add("lua_reload", [](const command::params &params) {
      std::string dir;
      if (params.size() >= 2) {
        dir = params.get(1);
      } else {
        dir = (game::get_appdata_path() / "data" / "ui_scripts").string();
      }

      scheduler::once(
          [dir] {
            try {
              int count = 0;
              std::string errors;
              const auto reload_dir = [&](const std::string &script_dir) {
                if (!utils::io::directory_exists(script_dir))
                  return;
                for (const auto &entry :
                     std::filesystem::recursive_directory_iterator(
                         script_dir)) {
                  if (!entry.is_regular_file())
                    continue;
                  if (entry.path().extension() != ".lua")
                    continue;

                  std::string data;
                  if (utils::io::read_file(entry.path().string(), &data)) {
                    auto chunk = entry.path().string();
                    if (chunk.starts_with(script_dir))
                      chunk = chunk.substr(script_dir.size());
                    if (execute_raw_lua(data, chunk.c_str()))
                      count++;
                    else
                      errors += chunk + "\n";
                  }
                }
              };

              reload_dir(dir);

              const utils::nt::library host{};
              reload_dir((host.get_folder() / "boiii" / "ui_scripts").string());

              game::com::Com_Printf(0, 0, "^2Lua Reload: Reloaded %d file(s)\n",
                                    count);
              const std::string toast_msg =
                  "Reloaded " + std::to_string(count) + " file(s)";
              scheduler::once(
                  [toast_msg] {
                    toast::success("Lua Reload", toast_msg.c_str());
                  },
                  scheduler::pipeline::renderer, 2s);

              // Refresh current page
              fire_debug_reload("UIRootFull");
              if (hot_reload_in_game) {
                fire_debug_reload("UIRoot0");
                fire_debug_reload("UIRoot1");
              }

              // Show collected errors in one popup after reload is done
              if (!errors.empty()) {
                auto popup_msg = std::string("^1Lua Reload Errors:\n") + errors;
                scheduler::once(
                    [popup_msg] {
                      game::ui::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
                                                             popup_msg.c_str());
                    },
                    scheduler::pipeline::renderer, 1s);
              }
            } catch (const std::exception &ex) {
              game::com::Com_Printf(0, 0, "^1Lua Reload: Error: %s\n",
                                    ex.what());
            }
          },
          scheduler::pipeline::renderer);
    });

    command::add("lua_reload_mod", [](const command::params & /*params*/) {
      const std::string mod_id = game::ugc::getPublisherIdFromLoadedMod();
      if (mod_id.empty() || mod_id == "usermaps") {
        scheduler::once(
            [] { toast::success("Lua Reload Mod", "No mod loaded"); },
            scheduler::pipeline::renderer, 2s);
        game::com::Com_Printf(0, 0,
                              "^3Lua Reload Mod: No mod currently loaded\n");
        return;
      }

      // Find the mod's content folder from the workshop pool
      std::string mod_content_path;
      for (unsigned int i = 0; i < *game::ugc::modsCount; ++i) {
        const auto &mod_data = game::ugc::modsPool[i];
        if (mod_data.publisherId == mod_id || mod_data.folderName == mod_id) {
          mod_content_path = mod_data.absolutePathContentFolder;
          break;
        }
      }

      if (mod_content_path.empty()) {
        game::com::Com_Printf(
            0, 0,
            "^3Lua Reload Mod: Could not find content folder for mod '%s'\n",
            mod_id.c_str());
        return;
      }

      // const auto folder = game::is_server() ? "lobby_scripts" : "ui_scripts";
      // const auto script_dir = (std::filesystem::path(mod_content_path) /
      // folder).string();

      const auto script_dir =
          (std::filesystem::path(mod_content_path) / "mods" / mod_id).string();

      scheduler::once(
          [script_dir, mod_id] {
            try {
              int count = 0;
              std::string errors;
              if (utils::io::directory_exists(script_dir)) {
                for (const auto &entry :
                     std::filesystem::recursive_directory_iterator(
                         script_dir)) {
                  if (!entry.is_regular_file())
                    continue;
                  if (entry.path().extension() != ".lua")
                    continue;

                  std::string data;
                  if (utils::io::read_file(entry.path().string(), &data)) {
                    auto chunk = entry.path().string();
                    if (chunk.starts_with(script_dir))
                      chunk = chunk.substr(script_dir.size());
                    if (execute_raw_lua(data, chunk.c_str()))
                      count++;
                    else
                      errors += chunk + "\n";
                  }
                }
              }

              game::com::Com_Printf(
                  0, 0,
                  "^2Lua Reload Mod: Reloaded %d file(s) for mod "
                  "'%s' from %s\n",
                  count, mod_id.c_str(), script_dir.c_str());
              const std::string toast_msg = std::string("Mod '") + mod_id +
                                            "': " + std::to_string(count) +
                                            " file(s)";
              scheduler::once(
                  [toast_msg] {
                    toast::success("Lua Reload Mod", toast_msg.c_str());
                  },
                  scheduler::pipeline::renderer, 2s);

              fire_debug_reload("UIRootFull");
              if (hot_reload_in_game) {
                fire_debug_reload("UIRoot0");
                fire_debug_reload("UIRoot1");
              }

              if (!errors.empty()) {
                auto popup_msg =
                    std::string("^1Lua Reload Mod Errors:\n") + errors;
                scheduler::once(
                    [popup_msg] {
                      game::ui::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
                                                             popup_msg.c_str());
                    },
                    scheduler::pipeline::renderer, 1s);
              }
            } catch (const std::exception &ex) {
              game::com::Com_Printf(0, 0, "^1Lua Reload Mod: Error: %s\n",
                                    ex.what());
            }
          },
          scheduler::pipeline::renderer);
    });

    command::add("lua_exec", [](const command::params &params) {
      if (params.size() < 2) {
        game::com::Com_Printf(0, 0, "Usage: lua_exec <file.lua>\n");
        return;
      }

      const std::string file = params.get(1);
      std::string data;
      if (!utils::io::read_file(file, &data)) {
        game::com::Com_Printf(0, 0, "^1Failed to read file: %s\n",
                              file.c_str());
        return;
      }

      scheduler::once(
          [data, file] {
            const auto name = std::filesystem::path(file).filename().string();
            if (execute_raw_lua(data, file.c_str())) {
              game::com::Com_Printf(0, 0, "^2Executed Lua file successfully\n");
              const std::string msg = "Executed " + name;
              scheduler::once([msg] { toast::success("Lua", msg.c_str()); },
                              scheduler::pipeline::renderer, 1s);
            } else {
              const std::string msg = "Failed: " + name;
              scheduler::once([msg] { toast::error("Lua", msg.c_str()); },
                              scheduler::pipeline::renderer, 1s);
            }
          },
          scheduler::pipeline::renderer);
    });

    patch_unsafe_lua_functions();
  }
};
} // namespace ui_scripting

REGISTER_COMPONENT(ui_scripting::component)