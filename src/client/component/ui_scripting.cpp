#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <game/utils.hpp>

#include <game/ui_scripting/execution.hpp>

#include "command.hpp"
#include "script.hpp"
#include "ui_scripting.hpp"
#include "scheduler.hpp"
#include "friends.hpp"
#include "getinfo.hpp"
#include "discord.hpp"
#include "name.hpp"

#include <steam/steam.hpp>
#include <steam/interfaces/matchmaking_servers.hpp>
#include "toast.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>
#include <utils/http.hpp>

#include <cmath>
#include <filesystem>
#include <unordered_map>
#include <atomic>
#include <frozen/unordered_set.h>
#include "frozen/string.h"

#include <game/impl/ugc/ugc.hpp>
#include <game/impl/com/com.hpp>

using namespace game::db;
using namespace game::db::xasset;
using namespace game::ugc;
using namespace game::ui;
using namespace game::lua;
using namespace game::lua::cod;
using namespace game::lua::hks;

namespace ui_scripting {
static std::atomic<bool> ui_initialized = false;

static std::atomic<bool> unsafe_function_called_message_shown = false;
static std::atomic<bool> unsafe_lua_approved_for_session = false;

void show_unsafe_lua_dialog() {
  if (unsafe_function_called_message_shown) {
    return;
  }

  unsafe_function_called_message_shown.store(true, std::memory_order_seq_cst);

  scheduler::once(
      [] {
        const int32_t result = MessageBoxA(
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
          unsafe_lua_approved_for_session.store(true,
                                                std::memory_order_seq_cst);
        }
      },
      scheduler::pipeline::main);
}

namespace {
std::unordered_map<cclosure *,
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

std::unordered_map<uintptr_t, std::string> rawfile_source_cache{};

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
std::atomic<bool> hot_reload_running = false;
std::atomic<bool> hot_reload_in_game = false;

bool execute_raw_lua(const std::string &code,
                     const char *chunk_name = "hot_reload") {
  lua_State *state = *primary_luaVM;
  if (!state)
    return false;

  try {
    const table lua = state->globals.v.table;
    state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;
    const script_value load_results = lua["loadstring"](code, chunk_name);
    state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::SECURE;

    if (load_results[0].is<function>()) {
      const script_value results = lua["pcall"](load_results);
      if (!results[0].as<bool>()) {
        auto err = results[1].as<std::string>();
        game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                              game::consoleLabel_e::DEFAULT,
                              "^1Lua Error [%s]: %s\n", chunk_name,
                              err.c_str());
        return false;
      }
      return true;
    } else if (load_results[1].is<std::string>()) {
      const std::string err = load_results[1].as<std::string>();
      game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                            game::consoleLabel_e::DEFAULT,
                            "^1Lua Compile Error [%s]: %s\n", chunk_name,
                            err.c_str());
    }
  } catch (const std::exception &ex) {
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT,
                          "^1Lua Error [%s]: %s\n", chunk_name, ex.what());
  }

  return false;
}

void fire_debug_reload(const char *root_name) {
  const std::string_view mapname = game::get_mapname().value_or("");
  const std::string code =
      utils::string::va("pcall(function() LUI.roots.%s:processEvent({ name = "
                        "'debug_reload', mapname = '%s' }) end)",
                        root_name, mapname.data());
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
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT,
                          "^1Hot Reload: Error scanning: %s\n", ex.what());
    return 0;
  }

  if (changed.empty())
    return 0;

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^2Hot Reload: Found %d file(s) to reload\n",
                        static_cast<int>(changed.size()));

  rawfile_source_cache.clear();

  for (const std::filesystem::directory_entry &entry : changed) {
    const std::string path_str = entry.path().string();
    std::string data;
    if (utils::io::read_file(path_str, &data)) {
      // Use relative path as chunk name
      std::string chunk = path_str;
      if (chunk.starts_with(hot_reload_path)) {
        chunk = chunk.substr(hot_reload_path.size());
      }

      if (execute_raw_lua(data, chunk.c_str())) {
        game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                              game::consoleLabel_e::DEFAULT,
                              "^2Hot Reload: Reloaded %s\n", chunk.c_str());
      } else {
        game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                              game::consoleLabel_e::DEFAULT,
                              "^1Hot Reload: Error reloading %s\n",
                              chunk.c_str());
      }
    }
  }

  // Fire debug_reload events to refresh UI
  fire_debug_reload("UIRootFull");
  if (hot_reload_in_game.load(std::memory_order_seq_cst)) {
    fire_debug_reload("UIRoot0");
    fire_debug_reload("UIRoot1");
  }

  return 1;
}

void start_hot_reload(const std::string &path) {
  hot_reload_path = path;
  hot_reload_files.clear();
  hot_reload_running.store(true, std::memory_order_seq_cst);

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
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^2Hot Reload: Watching '%s'\n", path.c_str());
}

void stop_hot_reload() {
  if (!hot_reload_running.load(std::memory_order_seq_cst)) {
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT,
                          "^3Hot Reload: Not currently watching.\n");
    return;
  }

  hot_reload_running.store(false, std::memory_order_seq_cst);
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

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^2Hot Reload: Stopped watching.\n");
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
  lua_State *state = *primary_luaVM;
  return state->globals.v.table;
}

const char *get_hks_type_name(const HksObjectType type) {
  const int32_t index = static_cast<int32_t>(type);
  if (index < static_cast<int32_t>(HksObjectType::TNIL) ||
      index >= static_cast<int32_t>(HksObjectType::COUNT)) {
    return "unknown";
  }

  const char *name = s_compilerTypeName->pool[index];
  return name ? name : "unknown";
}

bool try_stringify_with_lua(const script_value &value, std::string &out) {
  try {
    const table lua = get_globals();
    const auto tostring_fn = lua["tostring"];
    if (!tostring_fn.is<function>()) {
      return false;
    }

    const auto results = tostring_fn(value);
    if (!results.empty() && results[0].is<std::string>()) {
      out = results[0].as<std::string>();
      return true;
    }
  } catch (...) {
  }

  return false;
}

std::string stringify_print_arg(const script_value &value) {
  const auto &raw = value.get_raw();

  switch (raw.t) {
  case HksObjectType::TNONE:
  case HksObjectType::TNIL:
    return "nil";

  case HksObjectType::TBOOLEAN:
    return raw.v.boolean ? "true" : "false";

  case HksObjectType::TNUMBER: {
    const auto number = raw.v.number;
    if (std::isfinite(number) && std::floor(number) == number) {
      return utils::string::va("%.0f", number);
    }

    return utils::string::va("%g", number);
  }

  case HksObjectType::TSTRING:
    return value.as<std::string>();

  default: {
    std::string converted;
    if (try_stringify_with_lua(value, converted)) {
      return converted;
    }

    return utils::string::va("%s: %p", get_hks_type_name(raw.t), raw.v.ptr);
  }
  }
}

constexpr std::string_view BLACKLISTED_LUA_PRINT_CONTENTS[] = {
    "LUI_NULL_FUNCTION:"};

arguments lua_print(variadic_args args) {
  std::string message;

  for (size_t i = 0; i < args.size(); ++i) {
    if (i > 0) {
      message.push_back('\t');
    }

    message += stringify_print_arg(args[i]);
  }

  for (const std::string_view &blacklisted_contents :
       BLACKLISTED_LUA_PRINT_CONTENTS) {
    if (message.find(blacklisted_contents) != std::string::npos) {
      return {};
    }
  }

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s\n", message.c_str());
  fprintf(stdout, "%s\n", message.c_str());
  fflush(stdout);
#ifndef NDEBUG
  game::trace("[Lua] %s", message.c_str());
#endif

  return {};
}

void print_error(const std::string &error) {
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^1************** LUI script error **************\n");
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "^1%s\n", error.data());
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^1**********************************************\n");

  auto popup_msg = error;
  scheduler::once(
      [popup_msg] {
        UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0, game::errorCode::UI,
                                     popup_msg.c_str());
      },
      scheduler::main, 1s);
}

void print_loading_script(const std::string &name) {
  printf("Loading LUI script '%s'\n", name.data());
  game::trace("Loading LUI script '%s'", name.data());
}

std::string get_current_script(lua_State *state) {
  lua_Debug info{};
  hksi_lua_getstack(state, 1, &info);
  hksi_lua_getinfo(state, "nSl", &info);
  return info.short_src;
}

int load_buffer(const std::string &name, const std::string &data) {
  lua_State *state = *primary_luaVM;
  const HksBytecodeSharingMode sharing_mode =
      state->m_global->m_bytecodeSharingMode;
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;

  const auto _0 = utils::finally(
      [&] { state->m_global->m_bytecodeSharingMode = sharing_mode; });

  HksCompilerSettings compiler_settings{};
  return hksi_hksL_loadbuffer(state, &compiler_settings, data.data(),
                              data.size(), name.data());
}

void load_script(const std::string &name, const std::string &data,
                 const std::string &display_name = "") {
  globals.loaded_scripts[name] = name;
  const std::string &chunk = display_name.empty() ? name : display_name;
  if (!display_name.empty() && display_name != name) {
    globals.loaded_scripts[display_name] = name;
  }

  lua_State *state = *primary_luaVM;
  const table lua = get_globals();
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;
  const script_value load_results = lua["loadstring"](data, chunk);
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::SECURE;

  if (load_results[0].is<function>()) {
    const script_value results = lua["pcall"](load_results);
    if (!results[0].as<bool>()) {
      print_error(results[1].as<std::string>());
    }
  } else if (load_results[1].is<std::string>()) {
    print_error(load_results[1].as<std::string>());
  }
}

void load_local_script_files(const std::string &script_dir) {
  if (utils::io::directory_exists(script_dir)) {
    const std::vector<std::filesystem::path> scripts =
        utils::io::list_files(script_dir);

    for (const std::filesystem::path &script : scripts) {
      const std::string script_file = script.generic_string();

      if (std::filesystem::is_regular_file(script)) {
        const std::string file_path =
            script_file.substr(script_file.find("ui_scripts") + 11);
        globals.local_scripts[file_path] = script_file;
      } else if (std::filesystem::is_directory(script)) {
        load_local_script_files(script_file);
      }
    }
  }
}

void load_scripts(const std::string &script_dir) {
  if (!utils::io::directory_exists(script_dir)) {
    return;
  }

  load_local_script_files(script_dir);

  const std::vector<std::filesystem::path> scripts =
      utils::io::list_files(script_dir);

  for (const std::filesystem::path &script : scripts) {
    std::string data;
    const std::string script_file = script.generic_string();
    if (std::filesystem::is_directory(script) &&
        utils::io::read_file(script_file + "/__init__.lua", &data)) {
      print_loading_script(script_file);
      const std::string full_path = script_file + "/__init__.lua";
      const std::string display_name =
          script.filename().generic_string() + "/__init__.lua";
      load_script(full_path, data, display_name);
    }
  }
}

void setup_functions() {
  const table lua = get_globals();
  lua["game"] = table();

  lua["game"]["getfriendcount"] = function(
      convert_function([]() -> int32_t { return friends::get_friend_count(); }),
      HksObjectType::TCFUNCTION);

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
               HksObjectType::TCFUNCTION);

  lua["game"]["refreshfriends"] =
      function(convert_function([] { friends::refresh_presence(); }),
               HksObjectType::TCFUNCTION);

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
               HksObjectType::TCFUNCTION);

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
               HksObjectType::TCFUNCTION);

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
               HksObjectType::TCFUNCTION);

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
               HksObjectType::TCFUNCTION);

  lua["game"]["issocialfriend"] =
      function(convert_function([](const std::string &id_hex) -> bool {
                 try {
                   return friends::is_friend(std::stoull(id_hex, nullptr, 16));
                 } catch (...) {
                   return false;
                 }
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["connectsocialfriend"] = function(
      convert_function([](const std::string &id_hex) -> bool {
        try {
          return friends::connect_to_friend(std::stoull(id_hex, nullptr, 16));
        } catch (...) {
          return false;
        }
      }),
      HksObjectType::TCFUNCTION);

  lua["game"]["getkickableplayers"] =
      function(convert_function([]() -> table {
                 table players{};
                 if (!getinfo::is_host())
                   return players;

                 int list_index = 1;
                 game::foreach_connected_client([&players, &list_index](
                                                    game::sv::client_s &client,
                                                    const size_t client_index) {
                   if (client_index == 0 ||
                       game::sv::SV_IsTestClient(
                           static_cast<game::ClientNum_t>(client_index))) {
                     return;
                   }

                   char name_buffer[64]{};
                   std::string display_name;
                   if (game::cl::CL_GetClientName(
                           game::LOCAL_CLIENT_0, static_cast<int>(client_index),
                           name_buffer, sizeof(name_buffer), false) &&
                       name_buffer[0]) {
                     display_name = name_buffer;
                   } else if (client.name[0]) {
                     display_name = client.name;
                   } else {
                     display_name = "Player " + std::to_string(client_index);
                   }

                   table player{};
                   player.set("client_num", static_cast<int>(client_index));
                   player.set("name", display_name);
                   players.set(list_index++, player);
                 });
                 return players;
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["ishost"] =
      function(convert_function([]() -> bool { return getinfo::is_host(); }),
               HksObjectType::TCFUNCTION);

  lua["game"]["kickplayer"] = function(
      convert_function([](const int client_num) -> bool {
        if (!getinfo::is_host() || client_num <= 0)
          return false;

        bool kicked = false;
        std::string player_name;
        game::access_connected_client(
            static_cast<size_t>(client_num), [&](game::sv::client_s &client) {
              if (game::sv::SV_IsTestClient(
                      static_cast<game::ClientNum_t>(client_num))) {
                return;
              }
              player_name = client.name;
              game::sv::SV_DropClient(&client, "EXE_PLAYERKICKED", true, true);
              kicked = true;
            });

        if (kicked)
          toast::warn("PLAYER KICKED", player_name + " was removed.");
        return kicked;
      }),
      HksObjectType::TCFUNCTION);

  // HTTP functions
  lua["game"]["httpget"] =
      function(convert_function([](const std::string &url) -> std::string {
                 const auto result = utils::http::get_data(url);
                 return result.value_or("");
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["httppost"] =
      function(convert_function([](const std::string &url,
                                   const std::string &body) -> std::string {
                 const auto result = utils::http::post_data(url, body);
                 return result.value_or("");
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["setDiscordPlayerScore"] = function(
      convert_function([](int score) { discord::set_player_score(score); }),
      HksObjectType::TCFUNCTION);

  lua["game"]["setDiscordEnemyScore"] = function(
      convert_function([](int score) { discord::set_enemy_score(score); }),
      HksObjectType::TCFUNCTION);

  lua["game"]["setDiscordRoundsPlayed"] = function(
      convert_function([](int round) { discord::set_rounds_played(round); }),
      HksObjectType::TCFUNCTION);

  // Hot reload functions (callable from Lua timers)
  lua["game"]["hotreloadcheck"] =
      function(convert_function([]() {
                 if (hot_reload_running.load(std::memory_order_seq_cst)) {
                   hot_reload_check_files();
                 }
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["hotreloadstart"] = function(
      convert_function([](const std::string &path) { start_hot_reload(path); }),
      HksObjectType::TCFUNCTION);

  lua["game"]["getappdatapath"] =
      function(convert_function([]() -> std::string {
                 return game::get_appdata_path().generic_string();
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["getclientoverridename"] = function(
      convert_function([](const int32_t client_num) -> std::string {
        const auto cn = static_cast<game::ClientNum_t>(client_num);
        if (!game::valid_client_num(cn) || !name::has_name_override(cn)) {
          return "";
        }

        return name::get_name_override(cn).value_or("");
      }),
      HksObjectType::TCFUNCTION);

  lua["game"]["getclientoverridetag"] =
      function(convert_function([](const int32_t client_num) -> std::string {
                 const auto cn = static_cast<game::ClientNum_t>(client_num);
                 if (!game::valid_client_num(cn) ||
                     !name::has_clan_abbrev_override(cn)) {
                   return "";
                 }

                 return name::get_clan_abbrev_override(cn).value_or("");
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["getrawservercount"] =
      function(convert_function([]() -> int32_t {
                 return steam::get_raw_internet_server_count();
               }),
               HksObjectType::TCFUNCTION);

  lua["game"]["isserverlistrefreshing"] =
      function(convert_function(
                   []() -> bool { return steam::is_server_list_refreshing(); }),
               HksObjectType::TCFUNCTION);

  lua["game"]["getrawserverinfo"] =
      function(convert_function([](const int32_t index) -> table {
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
               HksObjectType::TCFUNCTION);
}

void enable_globals() {
  const table lua = get_globals();
  const std::string code = "local g = getmetatable(_G)\n"
                           "if not g then\n"
                           "g = {}\n"
                           "setmetatable(_G, g)\n"
                           "end\n"
                           "g.__newindex = nil\n";

  lua_State *state = *primary_luaVM;
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;
  const script_value _f = lua["loadstring"](code)[0]();
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::SECURE;
}

// List of mods by publisher ID that attempt to detect usage of a BOIII client
// and purposely trigger game crash, load failures, or other unexpected or
// malicious behaviour when detected.
constexpr frozen::string KNOWN_MALICIOUS_MODS_ARRAY[] = {
    // AAE
    "2631943123",
    // AAE Lite
    "2994481309",
    // AAE beta
    "2283794212",
    // UEM
    "2942053577",
    // UEM beta
    "3035353403",
    // Unknown workshop ID listed by UEM internally - workshop page is empty or
    // private
    "2987384358",
    // UEM dev
    "2975670972",
    // UEM barebones
    "3064538158"};
constexpr frozen::unordered_set<frozen::string,
                                std::size(KNOWN_MALICIOUS_MODS_ARRAY)>
    KNOWN_MALICIOUS_MODS =
        frozen::make_unordered_set<frozen::string,
                                   std::size(KNOWN_MALICIOUS_MODS_ARRAY)>(
            KNOWN_MALICIOUS_MODS_ARRAY);

inline bool malicious_mod_loaded() {
  return active_mod->publisherId[0] &&
         KNOWN_MALICIOUS_MODS.contains(
             std::string_view(active_mod->publisherId));
}

void setup_lua_globals() {
  globals = {};

  const table lua = get_globals();
  enable_globals();

  setup_functions();

  if (!game::is_server()) {
    lua["print"] =
        function(convert_function(lua_print), HksObjectType::TCFUNCTION);
  }
  lua["table"]["unpack"] = lua["unpack"];
  lua["luiglobals"] = lua;

  if (malicious_mod_loaded()) {
    lua["Engine"]["IsBOIII"] = *NilValue;
    lua["Engine"]["IsEZZBOIII"] = *NilValue;

  } else {
    lua["Engine"]["IsBOIII"] = true;
    lua["Engine"]["IsEZZBOIII"] = true;
  }

  lua["T7Overcharged"] = true;
}

void start() {
  setup_lua_globals();

  const utils::nt::library host{};
  const char *folder = game::is_server() ? "lobby_scripts/" : "ui_scripts/";
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

void reload_ingame_menu_scripts() {
  const utils::nt::library host{};
  const std::filesystem::path roots[] = {
      game::get_appdata_path() / "data/ui_scripts",
      host.get_folder() / "boiii/ui_scripts",
  };
  const char *files[] = {
      "party/datasources_start_menu_game_options.lua",
      "party/__init__.lua",
      "kick_menu/__init__.lua",
      "tweaks/__init__.lua",
      "social_friends/__init__.lua",
  };

  for (const std::filesystem::path &root : roots) {
    if (!utils::io::directory_exists(root.string())) {
      continue;
    }

    load_local_script_files((root / "party").string());
    load_local_script_files((root / "kick_menu").string());
    load_local_script_files((root / "tweaks").string());
    load_local_script_files((root / "social_friends").string());
    for (const char *file : files) {
      const std::filesystem::path path = root / file;
      std::string data;
      if (!utils::io::read_file(path.string(), &data)) {
        continue;
      }

      load_script(path.generic_string(), data, file);
    }
  }
}

void schedule_ingame_menu_reload() {
  scheduler::once(
      [] {
        try {
          reload_ingame_menu_scripts();
          toast::patch_hud();
        } catch (...) {
        }
      },
      scheduler::main, 2s);
}

void ui_init_stub(lua_Alloc allocFunction, void *outOfMemoryFunction) {
  ui_init_hook.invoke(allocFunction, outOfMemoryFunction);

  setup_lua_globals();
}

std::atomic<bool> doneFirstSnapshot = false;
std::atomic<bool> reloadIngameMenusAfterRestart = false;

void ui_cod_init_stub(const bool frontend) {
  ui_cod_init_hook.invoke(frontend);

  if (!game::is_server() && game::com::Com_IsRunningUILevel()) {
    // Fetch the names of the local files so file overrides are already handled
    globals = {};
    const utils::nt::library host{};
    doneFirstSnapshot.store(false, std::memory_order_seq_cst);
    reloadIngameMenusAfterRestart.store(false, std::memory_order_seq_cst);

    load_local_script_files(
        (game::get_appdata_path() / "data/ui_scripts/").string());
    load_local_script_files((host.get_folder() / "boiii/ui_scripts/").string());
    return;
  }
  try_start();
  ui_initialized.store(true, std::memory_order_seq_cst);
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

void cl_first_snapshot_stub(game::LocalClientNum_t localClientNum) {
  cl_first_snapshot_hook.invoke(localClientNum);
  if (game::com::Com_IsRunningUILevel()) {
    return;
  }

  if (doneFirstSnapshot.exchange(true, std::memory_order_seq_cst)) {
    if (!reloadIngameMenusAfterRestart.exchange(false,
                                                std::memory_order_seq_cst)) {
      return;
    }

    schedule_ingame_menu_reload();
    return;
  }

  hot_reload_in_game.store(true, std::memory_order_seq_cst);
  try_start();
  try {
    reload_ingame_menu_scripts();
  } catch (...) {
  }

  toast::patch_hud();

  try {
    inject_discord_score_subscriptions();
  } catch (...) {
  }
}

void ui_shutdown_stub() {
  hot_reload_in_game.store(false, std::memory_order_seq_cst);
  if (!utils::flags::has_flag("unsafe-lua")) {
    unsafe_function_called_message_shown.store(false,
                                               std::memory_order_seq_cst);
    unsafe_lua_approved_for_session.store(false, std::memory_order_seq_cst);
  }
  ui_shutdown_hook.invoke<void>();
  converted_functions.clear();
  rawfile_source_cache.clear();
  globals = {};

  ui_initialized.store(false, std::memory_order_seq_cst);
}

void *hks_package_require_stub(lua_State *state) {
  const std::string script = get_current_script(state);
  const std::string root = get_root_script(script);
  globals.in_require_script = root;
  return hks_package_require_hook.invoke<void *>(state);
}

hksInt32 hks_load_stub(lua_State *state, HksCompilerSettings *compiler_options,
                       lua_Reader *reader, RawFile *reader_data,
                       lua_Reader *debug_reader, void *debug_reader_data,
                       const char *chunk_name) {
  if (globals.load_raw_script) {
    globals.load_raw_script = false;
    globals.loaded_scripts[globals.raw_script_name] = globals.in_require_script;
    return load_buffer(globals.raw_script_name,
                       utils::io::read_file(globals.raw_script_name));
  }

  return Compiler(state, compiler_options, reader, reader_data, debug_reader,
                  debug_reader_data, chunk_name);
}

xasset::XAssetHeader lua_cod_getrawfile_stub(char *filename) {
  if (!is_loaded_script(globals.in_require_script) &&
      !is_local_script(filename)) {
    return lua_cod_getrawfile_hook.invoke<xasset::XAssetHeader>(filename);
  }

  const std::string name_ = filename;
  std::string target_script;
  if (is_loaded_script(globals.in_require_script)) {
    const std::string folder = globals.in_require_script.substr(
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
        .rawfile = reinterpret_cast<xasset::RawFile *>(1) //
    };
  }

  return lua_cod_getrawfile_hook.invoke<xasset::XAssetHeader>(filename);
}

luaReturnCount_e lua_stub_func([[maybe_unused]] lua_State *l) {
  return luaReturnCount_e::NONE;
}

template <size_t Key>
int32_t lua_unsafe_function_require_permissions(lua_State *l) {
  if (unsafe_lua_approved_for_session) {
    return unsafe_function_detours[Key].invoke<int>(l);
  }

  show_unsafe_lua_dialog();
  return 0;
}

template <size_t Key> void hook_unsafe_function(size_t address) {
  unsafe_function_detours[Key].create(
      address,
      reinterpret_cast<void *>(lua_unsafe_function_require_permissions<Key>));
}

#define HOOK_UNSAFE_FUNCTION(addr) hook_unsafe_function<addr>(addr##_g)

constexpr frozen::string BLACKLISTED_COMMANDS_ARRAY[] = {"quit"};
constexpr frozen::unordered_set<frozen::string,
                                std::size(BLACKLISTED_COMMANDS_ARRAY)>
    BLACKLISTED_COMMANDS =
        frozen::make_unordered_set<frozen::string,
                                   std::size(BLACKLISTED_COMMANDS_ARRAY)>(
            BLACKLISTED_COMMANDS_ARRAY);
utils::hook::detour Lua_CoD_LuaCall_OpenURL_hook;
utils::hook::detour Lua_CoD_LuaCall_Exec_hook;
luaReturnCount_e Lua_CoD_LuaCall_Exec_DisableBlacklisted(lua_State *luaVM) {
  if (lua_gettop(luaVM) == 2) {
    if (lua_isstring(luaVM, 2)) {
      const game::ControllerIndex_t controllerIndex =
          lua_isnumber(luaVM, 1)
              ? static_cast<game::ControllerIndex_t>(lua_tointeger(luaVM, 1))
              : game::com::Com_ControllerIndexes_GetPrimary();
      const char *cmd = lua_tostring(luaVM, 2);

      if (cmd && !BLACKLISTED_COMMANDS.contains(std::string_view(cmd))) {
        const game::LocalClientNum_t localClientNum =
            game::com::Com_ControllerIndex_GetLocalClientNum(controllerIndex);

        game::cbuf::Cbuf_AddText(localClientNum, cmd);
        game::cbuf::Cbuf_AddText(localClientNum, "\n");
      }
#ifndef NDEBUG
      else if (cmd) {
        game::trace(
            "[Lua][Exec] Blocked execution of blacklisted command \"%s\"", cmd);
      }
#endif
    } else {
      hksi_luaL_error(luaVM, "%s", "lua_isstring( luaVM, 2 )");
    }
  } else {
    hksi_luaL_error(luaVM, "%s", "lua_gettop( luaVM ) == 2");
  }
  return luaReturnCount_e::NONE;
}
void patch_unsafe_lua_functions() {
  /*
     Disable the `OpenURL` API function. This is never required for in-game
     functionality, and has historically almost always been used for obnoxious
     advertising.
  */
  Lua_CoD_LuaCall_OpenURL_hook.create(api::Lua_CoD_LuaCall_OpenURL,
                                      lua_stub_func);
  Lua_CoD_LuaCall_Exec_hook.create(api::Lua_CoD_LuaCall_Exec,
                                   Lua_CoD_LuaCall_Exec_DisableBlacklisted);
  if (utils::flags::has_flag("unsafe-lua")) {
    unsafe_lua_approved_for_session.store(true, std::memory_order_release);
  } else {

    // Do not allow the HKS vm to open LUA's libraries
    // Disable unsafe functions (debug library stays completely blocked)
    utils::hook::jump(0x141D34190_g, lua_stub_func); // debug

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
}
} // namespace

luaReturnCount_e main_handler(lua_State *state) {
  HksObject *value = &state->m_apistack.base[-1];
  if (value->t != HksObjectType::TCFUNCTION) {
    return luaReturnCount_e::NONE;
  }

  cclosure *closure = value->v.cClosure;
  if (!converted_functions.contains(closure)) {
    return luaReturnCount_e::NONE;
  }

  const auto &function = converted_functions[closure];

  try {
    const auto args = get_return_values();
    const auto results = function(args);

    for (const auto &result : results) {
      push_value(result);
    }

    return static_cast<luaReturnCount_e>(results.size());
  } catch (const std::exception &ex) {
    hksi_luaL_error(state, ex.what());
  }

  return luaReturnCount_e::NONE;
}

template <typename F> cclosure *convert_function(F f) {
  lua_State *state = *primary_luaVM;
  cclosure *closure = cclosure_Create(state, main_handler, 0, 0, 0);
  converted_functions[closure] = wrap_function(f);
  return closure;
}

namespace {
thread_local char getinfo_name_buf[256]{};
thread_local char getinfo_source_buf[512]{};

const char *resolve_c_function_name(uintptr_t c_func_ptr) {
  if (!c_func_ptr || game::is_server())
    return nullptr;
  uintptr_t list_head = *reinterpret_cast<uintptr_t *>(0x14365C5E0_g);
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

  auto it = rawfile_source_cache.find(bytecode_header);
  if (it != rawfile_source_cache.end()) {
    return it->second.empty() ? nullptr : it->second.c_str();
  }

  struct lookup_ctx {
    uintptr_t target;
    const char *found;
  };
  lookup_ctx ctx{bytecode_header, nullptr};

  xasset::DB_EnumXAssets(
      xasset::XAssetType::RAWFILE,
      [](xasset::XAssetHeader header, void *data) {
        lookup_ctx *c = static_cast<lookup_ctx *>(data);
        if (c->found)
          return;
        if (header.rawfile && header.rawfile->name && header.rawfile->buffer) {
          if (reinterpret_cast<uintptr_t>(header.rawfile->buffer) == c->target)
            c->found = header.rawfile->name;
        }
      },
      &ctx, false);

  rawfile_source_cache[bytecode_header] = ctx.found ? ctx.found : "";
  return ctx.found;
}

int hksi_lua_getinfo_stub(lua_State *s, const char *what, lua_Debug *ar) {
  const int32_t result = hksi_lua_getinfo_detour.invoke<int32_t>(s, what, ar);
  if (!result || !s || !ar)
    return result;
  if (!what || !strchr(what, 'n'))
    return result;

  CallStack *callstack = &s->m_callStack;
  if (!callstack->m_records || !callstack->m_current)
    return result;

  const int32_t stack_level = ar->callstack_level;
  const int32_t num_records =
      static_cast<int32_t>((reinterpret_cast<uintptr_t>(callstack->m_current) -
                            reinterpret_cast<uintptr_t>(callstack->m_records)) /
                           sizeof(CallStack::ActivationRecord));

  HksObject *func_obj = nullptr;
  if (stack_level >= num_records) {
    if (s->m_apistack.bottom)
      func_obj = s->m_apistack.bottom - 1;
  } else if (stack_level + 1 <= num_records) {
    CallStack::ActivationRecord *next_record =
        &callstack->m_records[stack_level + 1];
    if (next_record->m_base)
      func_obj = next_record->m_base - 1;
  }

  if (!func_obj)
    return result;

  const HksObjectType obj_type = func_obj->t;
  const uintptr_t obj_value = reinterpret_cast<uintptr_t>(func_obj->v.cClosure);
  if (!obj_value)
    return result;

  if (obj_type == HksObjectType::TCFUNCTION) {
    uintptr_t c_func_ptr = *reinterpret_cast<uintptr_t *>(obj_value + 16);
    const char *resolved = resolve_c_function_name(c_func_ptr);
    if (resolved && resolved[0])
      ar->name = resolved;
    else if (!ar->name || !ar->name[0])
      ar->name = "(luaC_unknown)";
  } else if (obj_type == HksObjectType::TIFUNCTION) {
    uintptr_t proto = *reinterpret_cast<uintptr_t *>(obj_value + 16);
    if (proto) {
      uint32_t m_hash = *reinterpret_cast<uint32_t *>(proto + 16);
      uint8_t m_numParams = *reinterpret_cast<uint8_t *>(proto + 0x18);
      uintptr_t m_debug = *reinterpret_cast<uintptr_t *>(proto + 80);

      bool name_fixed = false;
      if (m_debug) {
        uintptr_t debug_name_ptr = *reinterpret_cast<uintptr_t *>(m_debug + 48);
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
        using getPC_t = fastcallPtr_t<uintptr_t, lua_State *, lua_Debug *>;
        getPC_t fn_getPC = reinterpret_cast<getPC_t>(0x141D46310_g);
        pc = fn_getPC(s, ar);
      }

      const char *resolved_source = nullptr;
      if (pc) {
        uintptr_t scan = pc & ~static_cast<uintptr_t>(0xF);
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

    std::string trimmed = line;
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
      size_t quote_start = trimmed.find('\'');
      size_t quote_end = trimmed.rfind('\'');
      if (quote_start != std::string::npos && quote_end > quote_start) {
        std::string func_name =
            trimmed.substr(quote_start + 1, quote_end - quote_start - 1);
        result += "\t<^1native^7>: in function '^3" + func_name + "^7'\n";
      } else {
        result += "\t<^1native^7>: " + trimmed + "\n";
      }
    } else if (trimmed.find("(tail call)") != std::string::npos) {
      result += "\t^7(tail call): ?\n";
    } else {
      size_t first_colon = trimmed.find(':');
      if (first_colon != std::string::npos) {
        std::string filename = trimmed.substr(0, first_colon);
        std::string rest = trimmed.substr(first_colon + 1);
        size_t second_colon = rest.find(':');
        if (second_colon != std::string::npos) {
          std::string line_num = rest.substr(0, second_colon);
          std::string remainder = rest.substr(second_colon + 1);
          result += "\t^5" + filename + "^7:^2" + line_num + "^7:";

          size_t quote_start = remainder.find('\'');
          size_t quote_end = remainder.rfind('\'');
          if (quote_start != std::string::npos && quote_end > quote_start) {
            std::string before = remainder.substr(0, quote_start);
            std::string func_name =
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

const char *safe_get_lua_error_stack(lua_State *luaVM) {
  __try {
    HksObject *api_top = luaVM->m_apistack.top;
    HksObject *api_bottom = luaVM->m_apistack.bottom;

    if (api_top && api_bottom && (api_top - 1) >= api_bottom) {
      HksObject *top_obj = api_top - 1;
      if (top_obj->t == HksObjectType::TSTRING && top_obj->v.str) {
        return top_obj->v.str->m_data;
      }
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return nullptr;
}

void lua_cod_luastatemanager_error_stub(const char *error, lua_State *luaVM) {
  if (error && luaVM) {
    // Suppress duplicate errors - the engine often fires the same error twice
    static std::string last_error;
    static std::chrono::steady_clock::time_point last_error_time;
    const std::chrono::time_point now = std::chrono::steady_clock::now();

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
        std::string trimmed = check_line;
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
      const std::string colored =
          colorize_lua_error("LUI script (suppressed)", error_stack);
      game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                            game::consoleLabel_e::DEFAULT, "%s",
                            colored.c_str());
      return;
    }

    const bool is_ui = (*primary_luaVM == luaVM);
    const char *error_loc =
        is_ui ? "LUI script execution error" : "LobbyVM script execution error";

    const std::string colored = colorize_lua_error(error_loc, resolved_stack);

    try {
      const std::filesystem::path root_path =
          utils::nt::library{}.get_path().parent_path();
      const std::filesystem::path logs_dir = root_path / "logs";
      std::filesystem::create_directories(logs_dir);
      const std::string log_path = (logs_dir / "boiii_lua_errors.log").string();

      std::chrono::time_point now_sys = std::chrono::system_clock::now();
      time_t now = std::chrono::system_clock::to_time_t(now_sys);
      tm ltime{};
      localtime_s(&ltime, &now);
      char timestamp[64]{};
      strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &ltime);

      std::ofstream log_file(log_path, std::ios::app);
      if (log_file.is_open()) {
        log_file << "\n[" << timestamp << "] " << error_loc << "\n"
                 << resolved_stack << "\n";
      }
    } catch (...) {
    }

    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT, "%s", colored.c_str());

    // Show colored error popup with delay to ensure UI is ready
    std::string popup_text = colorize_lua_error(nullptr, resolved_stack);
    scheduler::once(
        [popup_text] {
          UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0,
                                       game::errorCode::UI, popup_text.c_str());
        },
        scheduler::main, 500ms);
  }
}

utils::hook::detour load_dll_hook;
luaReturnCount_e load_dll_disable(lua_State *s, const char *filename,
                                  const char *func_name) {
  lua_pushfunction(s, lua_stub_func, func_name);
  return luaReturnCount_e::ONE;
}

void lua_error_print_stub(int, const char *, ...) {}

inline void lui_reload() {
  converted_functions.clear();
  rawfile_source_cache.clear();

  globals.loaded_scripts.clear();
  globals.local_scripts.clear();

  UI_CoD_Shutdown();
  UI_CoD_Init(true);

  // Com_LoadFrontEnd stripped
  Lua_CoD_LoadLuaFile(*primary_luaVM, "ui_mp.T6.main");
  UI_AddMenu(UI_CoD_GetRootNameForController(0), "main", -1, *primary_luaVM);

  UI_CoD_LobbyUI_Init();
}

inline void register_lui_commands() {
  command::add("boiii_prepare_menu_restart", [](const command::params &) {
    reloadIngameMenusAfterRestart.store(true, std::memory_order_seq_cst);
  });

  command::add("luiReload", [] {
    if (game::com::Com_IsRunningUILevel()) {
      lui_reload();
    } else {
      // TODO: Find a way to do a full shutdown & restart like in frontend,
      // that opens up the loading screen that can't be easily closed
      rawfile_source_cache.clear();
      game::cg::CG_LUIHUDRestart(game::LOCAL_CLIENT_0);
      schedule_ingame_menu_reload();
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
            int32_t count = 0;
            std::string errors;
            const std::function<void(const std::string &script_dir)>
                reload_dir = [&](const std::string &script_dir) {
                  if (!utils::io::directory_exists(script_dir))
                    return;
                  for (const std::filesystem::directory_entry &entry :
                       std::filesystem::recursive_directory_iterator(
                           script_dir)) {
                    if (!entry.is_regular_file())
                      continue;
                    if (entry.path().extension() != ".lua")
                      continue;

                    std::string data;
                    if (utils::io::read_file(entry.path().string(), &data)) {
                      std::string chunk = entry.path().string();
                      if (chunk.starts_with(script_dir))
                        chunk = chunk.substr(script_dir.size());
                      if (execute_raw_lua(data, chunk.c_str()))
                        count++;
                      else
                        errors += chunk + "\n";
                    }
                  }
                };

            rawfile_source_cache.clear();

            reload_dir(dir);

            const utils::nt::library host{};
            reload_dir((host.get_folder() / "boiii" / "ui_scripts").string());

            game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                                  game::consoleLabel_e::DEFAULT,
                                  "^2Lua Reload: Reloaded %d file(s)\n", count);
            const std::string toast_msg =
                "Reloaded " + std::to_string(count) + " file(s)";
            scheduler::once(
                [toast_msg] {
                  toast::success("Lua Reload", toast_msg.c_str());
                },
                scheduler::pipeline::renderer, 2s);

            // Refresh current page
            fire_debug_reload("UIRootFull");
            if (hot_reload_in_game.load(std::memory_order_seq_cst)) {
              fire_debug_reload("UIRoot0");
              fire_debug_reload("UIRoot1");
            }

            // Show collected errors in one popup after reload is done
            if (!errors.empty()) {
              std::string popup_msg =
                  std::string("^1Lua Reload Errors:\n") + errors;
              scheduler::once(
                  [popup_msg] {
                    UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0,
                                                 game::errorCode::UI,
                                                 popup_msg.c_str());
                  },
                  scheduler::pipeline::renderer, 1s);
            }
          } catch (const std::exception &ex) {
            game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                                  game::consoleLabel_e::DEFAULT,
                                  "^1Lua Reload: Error: %s\n", ex.what());
          }
        },
        scheduler::pipeline::renderer);
  });

  command::add("lua_reload_mod", [](const command::params & /*params*/) {
    const std::string mod_id = game::ugc::UGC_ActiveMod_PublisherId();
    if (mod_id.empty() || mod_id == "usermaps") {
      scheduler::once([] { toast::success("Lua Reload Mod", "No mod loaded"); },
                      scheduler::pipeline::renderer, 2s);
      game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                            game::consoleLabel_e::DEFAULT,
                            "^3Lua Reload Mod: No mod currently loaded\n");
      return;
    }

    // Find the mod's content folder from the workshop pool
    std::string mod_content_path;
    for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
      const game::ugc::WorkshopData *mod_data = &game::ugc::modsPool.data[i];
      if (mod_data->publisherId == mod_id || mod_data->internalName == mod_id) {
        mod_content_path = mod_data->absolutePathContentDirectory;
        break;
      }
    }

    if (mod_content_path.empty()) {
      game::com::Com_Printf(
          game::consoleChannel_e::CHANNEL_DONT_FILTER,
          game::consoleLabel_e::DEFAULT,
          "^3Lua Reload Mod: Could not find content folder for mod '%s'\n",
          mod_id.c_str());
      return;
    }

    const std::string script_dir =
        (std::filesystem::path(mod_content_path) / "mods" / mod_id).string();

    scheduler::once(
        [script_dir, mod_id] {
          try {
            int32_t count = 0;
            std::string errors;
            if (utils::io::directory_exists(script_dir)) {
              for (const std::filesystem::directory_entry &entry :
                   std::filesystem::recursive_directory_iterator(script_dir)) {
                if (!entry.is_regular_file())
                  continue;
                if (entry.path().extension() != ".lua")
                  continue;

                std::string data;
                if (utils::io::read_file(entry.path().string(), &data)) {
                  std::string chunk = entry.path().string();
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
                game::consoleChannel_e::CHANNEL_DONT_FILTER,
                game::consoleLabel_e::DEFAULT,
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
            if (hot_reload_in_game.load(std::memory_order_seq_cst)) {
              fire_debug_reload("UIRoot0");
              fire_debug_reload("UIRoot1");
            }

            if (!errors.empty()) {
              std::string popup_msg =
                  std::string("^1Lua Reload Mod Errors:\n") + errors;
              scheduler::once(
                  [popup_msg] {
                    UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0,
                                                 game::errorCode::UI,
                                                 popup_msg.c_str());
                  },
                  scheduler::pipeline::renderer, 1s);
            }
          } catch (const std::exception &ex) {
            game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                                  game::consoleLabel_e::DEFAULT,
                                  "^1Lua Reload Mod: Error: %s\n", ex.what());
          }
        },
        scheduler::pipeline::renderer);
  });

  command::add("lua_exec", [](const command::params &params) {
    if (params.size() < 2) {
      game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                            game::consoleLabel_e::DEFAULT,
                            "Usage: lua_exec <file.lua>\n");
      return;
    }

    const std::string file = params.get(1);
    std::string data;
    if (!utils::io::read_file(file, &data)) {
      game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                            game::consoleLabel_e::DEFAULT,
                            "^1Failed to read file: %s\n", file.c_str());
      return;
    }

    scheduler::once(
        [data, file] {
          const std::string name =
              std::filesystem::path(file).filename().string();
          if (execute_raw_lua(data, file.c_str())) {
            game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                                  game::consoleLabel_e::DEFAULT,
                                  "^2Executed Lua file successfully\n");
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
}
} // namespace

utils::hook::detour Lua_CoD_FFReader_hook;
inline const uint8_t *
Lua_CoD_FFReader_EnforceOverride(lua_State *luaVM, RawFile *ud, size_t *size) {
  if (ud == nullptr) {
    return nullptr;
  }
  if (ud->name && ud->name[0]) {
    RawFile *override = script::get_loaded_rawfile(ud->name);
    if (override && override->buffer != ud->buffer) {
// TODO: how and why does this happen? How do scripts arrive here if not fetched
// with `DB_FindXAssetHeader`?
#ifndef NDEBUG
      game::trace(
          "Rawfile override failed for script \"%s\". Override buffer: 0x%p, "
          "original buffer: 0x%p, override len: 0x%016X, original len: 0x%016X",
          ud->name, override->buffer, ud->buffer, override->len, ud->len);
#endif
      ud->buffer = override->buffer;
      ud->len = override->len;
    }
  }
  *size = ud->len;
  if (*size == 0) {
    return nullptr;
  }
  return ud->buffer;
}

utils::hook::detour R_CopyTextureRegionMips_hook;
void R_CopyTextureRegionMips_Safe(void *a1, void *a2, uint32_t a3, int32_t a4,
                                  int32_t a5, void *a6, int32_t a7, int32_t a8,
                                  int32_t a9, int32_t a10, int32_t a11,
                                  int32_t a12, int32_t a13, int32_t a14) {
  if (game::is_server() || a6 == nullptr || a2 == nullptr ||
      /*
        See note above `R_CopyTextureRegionMips`'s symbol definition.
        None of the data structures used in the function arguments are presently
        known or reverse engineered.

        However, a crash occurs here upon AAE load if not for the following
        (ugly) check using inlined offsets, so this needed to be implemented
        immediately.

        TODO: reverse engineer this function and the types used therein, then
        use struct fields for this check instead.
      */
      game::readable_ptr(reinterpret_cast<void *>(
          *reinterpret_cast<void (**)(void *, int64_t *)>(
              **(reinterpret_cast<int64_t **>(a6) + 0x15) + 0x38LL)))) {
    R_CopyTextureRegionMips_hook.invoke(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                        a11, a12, a13, a14);
  }
}

#ifndef NDEBUG
void print_info(game::consoleLabel_e label, const std::string_view &msg) {
  game::trace("[Lua][Info] %s", msg.data());
  fprintf(stdout, "[Lua][Info] %s\n", msg.data());
  fflush(stdout);
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER, label,
                        "%s\n", msg.data());
}

void print_error(game::consoleLabel_e label, const std::string_view &msg) {
  game::trace("[Lua][Error] %s", msg.data());
  fprintf(stderr, "[Lua][Error] %s\n", msg.data());
  fflush(stderr);
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER, label,
                        "%s\n", msg.data());
}

void print_warning(game::consoleLabel_e label, const std::string_view &msg) {
  game::trace("[Lua][Warn] %s", msg.data());
  fprintf(stdout, "[Lua][Warn] %s\n", msg.data());
  fflush(stdout);
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER, label,
                        "%s\n", msg.data());
}

std::string concat_string_args(lua_State *s, hksInt32 startIdx = 1) {
  std::string text;
  for (hksInt32 i = startIdx; i <= lua_gettop(s); ++i) {
    text += lua_tostring(s, i);
  }
  return text;
}

utils::hook::detour Lua_CoD_LuaCall_PrintInfo_hook;
luaReturnCount_e Lua_CoD_LuaCall_PrintInfo_AllOutputs(lua_State *s) {
  if (lua_gettop(s) >= 2) {
    const game::consoleLabel_e label =
        static_cast<game::consoleLabel_e>(lua_tonumber(s, 1));
    print_info(label, concat_string_args(s, 2));
  }
  return luaReturnCount_e::NONE;
}

utils::hook::detour Lua_CoD_LuaCall_PrintError_hook;
luaReturnCount_e Lua_CoD_LuaCall_PrintError_AllOutputs(lua_State *s) {
  if (lua_gettop(s) >= 2) {
    const game::consoleLabel_e label =
        static_cast<game::consoleLabel_e>(lua_tonumber(s, 1));
    print_error(label, concat_string_args(s, 2));
  }
  return luaReturnCount_e::NONE;
}

utils::hook::detour Lua_CoD_LuaCall_PrintWarning_hook;
luaReturnCount_e Lua_CoD_LuaCall_PrintWarning_AllOutputs(lua_State *s) {
  if (lua_gettop(s) >= 2) {
    const game::consoleLabel_e label =
        static_cast<game::consoleLabel_e>(lua_tonumber(s, 1));
    print_warning(label, concat_string_args(s, 2));
  }
  return luaReturnCount_e::NONE;
}
#endif

utils::hook::detour Com_GetBuildIntField_hook;
utils::hook::detour Com_GetBuildStringField_hook;
utils::hook::detour Lua_CoD_LuaCall_Mods_SetMod_hook;
luaReturnCount_e Lua_CoD_LuaCall_Mods_SetMod_LoadImmediately(lua_State *luaVM) {
  if (lua_gettop(luaVM) > 0 && lua_isstring(luaVM, 1)) {
    const char *publisherId = lua_tostring(luaVM, 1);
#ifndef NDEBUG
    game::trace("[Lua] Mods_SetMod called with publisherId: \"%s\"",
                publisherId);
#endif
    if (publisherId) {
      scheduler::once(
          [publisherId]() {
            UGC_LoadModByPublisherId_Impl(
                game::LocalClientNum_t::LOCAL_CLIENT_0, publisherId, true);
          },
          scheduler::pipeline::main);
    }
  }
  return luaReturnCount_e::NONE;
}

typedef fastcallPtr_t<void(lua_State *luaVM)> LobbyVM_CallFunc_Handler;
constexpr std::pair<frozen::string, LobbyVM_CallFunc_Handler>
    LOBBYVM_CALLFUNC_HANDLER_ARRAY[] = {
        {"LoadMod", nullptr},
        {"CanClientLaunch", nullptr},
        {"CanLobbyCanMigrate", nullptr},
        {"ChangeSigninState", nullptr},
        {"CheckAdvertizeSteamServer", nullptr},
        {"CheckDLCBit", nullptr},
        {"CheckInitSteamServer", nullptr},
        {"CheckNeedInstallUGC", nullptr},
        {"CheckSpecialPlaylistRules", nullptr},
        {"CheckStarterPack", nullptr},
        {"ClearLobbyStatus", nullptr},
        {"ClientLaunch", nullptr},
        {"ClientLaunchClear", nullptr},
        {"ClientLaunchInit", nullptr},
        {"ClientLaunchPump", nullptr},
        {"ComErrorCodeToString", nullptr},
        {"CreateDedicatedLANLobby", nullptr},
        {"CreateDedicatedLobby", nullptr},
        {"CreateDedicatedModsLobby", nullptr},
        {"DemoEndFinished", nullptr},
        {"DevGui", nullptr},
        {"Devmap", nullptr},
        {"DevmapClient", nullptr},
        {"DLCMapCheck", nullptr},
        {"DoChunksAllowJoin", nullptr},
        {"ErrorShutdown", nullptr},
        {"ErrorShutdownMessage", nullptr},
        {"ExecuteLobbyVMRequest", nullptr},
        {"ForceToMenu", nullptr},
        {"GameLobbyClientDataUpdate", nullptr},
        {"GameLobbyGameServerDataUpdate", nullptr},
        {"GameModeChanged", nullptr},
        {"GetJoinProcess", nullptr},
        {"GetLootItemCategory", nullptr},
        {"GetLootItemList", nullptr},
        {"GetLootPossibleCount", nullptr},
        {"GetMatchmakingExperimentActive", nullptr},
        {"GetNeededDLCBits", nullptr},
        {"GetRecentItemTags", nullptr},
        {"GoBack", nullptr},
        {"GoForward", nullptr},
        {"Gunsmith", nullptr},
        {"HopperClientJoin", nullptr},
        {"HopperIsParked", nullptr},
        {"HostLaunch", nullptr},
        {"HostLaunchClear", nullptr},
        {"HostLaunchInit", nullptr},
        {"HostLaunchPump", nullptr},
        {"InGameJoin", nullptr},
        {"IngameMonitor", nullptr},
        {"InitilizeGunsmithBuffer", nullptr},
        {"InitilizeZMLoadoutBuffer", nullptr},
        {"Invite", nullptr},
        {"IsHostLaunching", nullptr},
        {"IsInTheaterLobby", nullptr},
        {"Join", nullptr},
        {"JoinableCheck", nullptr},
        {"JoinResultToString", nullptr},
        {"JoinSystemlink", nullptr},
        {"LaunchDemo", nullptr},
        {"LaunchDemoExec", nullptr},
        {"LaunchGame", nullptr},
        {"LaunchGameExec", nullptr},
        {"Leaderboard_CalculateLBColValue", nullptr},
        {"Leaderboard_PopulateCustomList", nullptr},
        {"LobbyClientLeftEvent", nullptr},
        {"LobbyClientPromoteToHost", nullptr},
        {"LobbyHost_ClientSelectionReceived", nullptr},
        {"LobbyHostLeft_InGameMigrateFinished", nullptr},
        {"LobbyHostLeftNoMigration", nullptr},
        {"LobbyLeaveWithParty", nullptr},
        {"LobbyLocalClientLeave", nullptr},
        {"LobbyMonitor", nullptr},
        {"LobbySettings", nullptr},
        {"LobbyStatusUpdate", nullptr},
        {"LogGlobalData", nullptr},
        {"ManagePartyLeave", nullptr},
        {"MatchmakingPriorityQuit", nullptr},
        {"NetworkModeChanged", nullptr},
        {"OnBuyCrate", nullptr},
        {"OnCanBroadcastHostInfo", nullptr},
        {"OnCanFitLobbys", nullptr},
        {"OnCheckPrestigeFeatureBan", nullptr},
        {"OnClientAdded", nullptr},
        {"OnClientRemoved", nullptr},
        {"OnClientSelectionReceived", nullptr},
        {"OnComError", nullptr},
        {"OnComErrorCleanup", nullptr},
        {"OnCookGobbleGumRecipe", nullptr},
        {"OnDediQosReady", nullptr},
        {"OnDisconnect", nullptr},
        {"OnDWDisconnect", nullptr},
        {"OnEnableJoins", nullptr},
        {"OnErrorShutdown", nullptr},
        {"OnExperimentReset", nullptr},
        {"OnFeatureBan", nullptr},
        {"OnGametypeSettingsChange", nullptr},
        {"OnGetAnticheatReputation", nullptr},
        {"OnGetBanTimeRemaining", nullptr},
        {"OnInit", nullptr},
        {"OnInitializeLoadouts", nullptr},
        {"OnInitializeStats", nullptr},
        {"OnInventoryFetched", nullptr},
        {"OnIsFeatureBanned", nullptr},
        {"OnIsPermaBanned", nullptr},
        {"OnJoinComplete", nullptr},
        {"OnJoinPartyPrivacyCheck", nullptr},
        {"OnKVSFlush", nullptr},
        {"OnLeaveWithParty", nullptr},
        {"OnLimitedItemPromoUpdate", nullptr},
        {"OnLobbyOnlineUpdate", nullptr},
        {"OnLobbyServerCountUpdated", nullptr},
        {"OnLobbyServerListRetrieved", nullptr},
        {"OnLobbyServerListSorted", nullptr},
        {"OnMatchChangeGameType", nullptr},
        {"OnMatchChangeMap", nullptr},
        {"OnMatchEnd", nullptr},
        {"OnMatchLaunchClient", nullptr},
        {"OnMatchRecordStart", nullptr},
        {"OnMatchStart", nullptr},
        {"OnMaxClientsChanged", nullptr},
        {"OnModUpdate", nullptr},
        {"OnPartyPrivacyChange", nullptr},
        {"OnPlatformJoin", nullptr},
        {"OnPlatformPlayTogether", nullptr},
        {"OnPlatformResume", nullptr},
        {"OnPlatformSessionDataUpdate", nullptr},
        {"OnPlatformSessionMultiplayerSessionChanged", nullptr},
        {"OnPlatformSessionMultiplayerSubscriptionLost", nullptr},
        {"OnPlatformSuspend", nullptr},
        {"OnPlayerBanned", nullptr},
        {"OnPopAnticheatMessage", nullptr},
        {"OnPostExecFFOTD", nullptr},
        {"OnPreExecFFOTD", nullptr},
        {"OnPushAnticheatMessageToUI", nullptr},
        {"OnRecordComScoreEvent", nullptr},
        {"OnSessionEnd", nullptr},
        {"OnSessionModeChanged", nullptr},
        {"OnSessionStart", nullptr},
        {"OnShouldWriteLeaderboard", nullptr},
        {"OnSpendVials", nullptr},
        {"OnStorageOperationReadResult", nullptr},
        {"OnStorageRead", nullptr},
        {"OnStorageWrite", nullptr},
        {"OnStorageWriteDispatch", nullptr},
        {"OnUILoad", nullptr},
        {"OnUpdateAdvertising", nullptr},
        {"OnUsermapUpdate", nullptr},
        {"PlaySound", nullptr},
        {"PopulateMutableClientDDLBuff", nullptr},
        {"PrivateLobbyServerDataUpdate", nullptr},
        {"ProcessCompleteError", nullptr},
        {"ProcessCompleteFailure", nullptr},
        {"ProcessCompleteSuccess", nullptr},
        {"ProcessUpdate", nullptr},
        {"Pump", nullptr},
        {"ReceiveMutableClientDDLBuff", nullptr},
        {"SessionSQJRefreshInfo", nullptr},
        {"SetDefaultShowcaseWeapon", nullptr},
        {"SetMaxLocalPlayers", nullptr},
        {"ShouldShowContentChangedMessage", nullptr},
        {"ShutdownCleanup", nullptr},
        {"ShutdownCleanupCP", nullptr},
        {"ShutdownCleanupMP", nullptr},
        {"ShutdownCleanupZM", nullptr},
        {"StopLobbyTimer", nullptr},
        {"SwitchTeam", nullptr},
        {"UGCOffensiveEmblemAdd", nullptr},
        {"UpdateLobbyStatusInfo", nullptr},
        {"UpdateUI", nullptr}};
constexpr frozen::unordered_map<frozen::string, LobbyVM_CallFunc_Handler,
                                std::size(LOBBYVM_CALLFUNC_HANDLER_ARRAY)>
    LOBBYVM_CALLFUNC_HANDLERS =
        frozen::make_unordered_map<frozen::string, LobbyVM_CallFunc_Handler,
                                   std::size(LOBBYVM_CALLFUNC_HANDLER_ARRAY)>(
            LOBBYVM_CALLFUNC_HANDLER_ARRAY);

utils::hook::detour LobbyVM_CallFunc_hook;
luaReturnCount_e LobbyVM_CallFunc_Redirect(lua_State *luaVM) {
  if (lua_gettop(luaVM) > 0 && lua_isstring(luaVM, 1)) {
    const char *func = lua_tostring(luaVM, 1);
    if (func && func[0]) {
      const std::string_view func_view = func;
#ifndef NDEBUG
      game::trace("LobbyVM_CallFunc called with func: %s, argc: %d",
                  func_view.data(), lua_gettop(luaVM));
#endif
      if (LOBBYVM_CALLFUNC_HANDLERS.contains(func_view)) {
        LobbyVM_CallFunc_Handler handler =
            LOBBYVM_CALLFUNC_HANDLERS.at(func_view);
        if (handler != nullptr) {
          handler(luaVM);
          return luaReturnCount_e::NONE;
        }
      }
    }
  }

  return LobbyVM_CallFunc_hook.invoke<luaReturnCount_e>(luaVM);
}

class component final : public generic_component {
public:
  void post_unpack() override {
#ifndef NDEBUG
    Lua_CoD_LuaCall_PrintInfo_hook.create(api::Lua_CoD_LuaCall_PrintInfo,
                                          Lua_CoD_LuaCall_PrintInfo_AllOutputs);
    if (game::is_client()) {
      Lua_CoD_LuaCall_PrintError_hook.create(
          api::Lua_CoD_LuaCall_PrintError,
          Lua_CoD_LuaCall_PrintError_AllOutputs);
      Lua_CoD_LuaCall_PrintWarning_hook.create(
          api::Lua_CoD_LuaCall_PrintWarning,
          Lua_CoD_LuaCall_PrintWarning_AllOutputs);
    }
#endif

    /*
       Spoof client build info returned to lua scripts to
       circumvent build info checks in mod scripts intended to disable usage of
       boiii
    */
    Com_GetBuildIntField_hook.create(game::com::Com_GetBuildIntField,
                                     game::com::Com_GetBuildIntField_Impl);
    Com_GetBuildStringField_hook.create(
        game::com::Com_GetBuildStringField,
        game::com::Com_GetBuildStringField_Impl);
    Lua_CoD_LuaCall_Mods_SetMod_hook.create(
        api::Lua_CoD_LuaCall_Mods_SetMod,
        Lua_CoD_LuaCall_Mods_SetMod_LoadImmediately);
    LobbyVM_CallFunc_hook.create(api::Lua_CoD_LuaCall_LobbyVM_CallFunc,
                                 LobbyVM_CallFunc_Redirect);

    R_CopyTextureRegionMips_hook.create(game::r::R_CopyTextureRegionMips,
                                        R_CopyTextureRegionMips_Safe);
    Lua_CoD_FFReader_hook.create(Lua_CoD_FFReader,
                                 Lua_CoD_FFReader_EnforceOverride);
    utils::hook::call(game::select(0x141D4979A, 0x1403F233A), hks_load_stub);
    load_dll_hook.create(load_dll, load_dll_disable);

    hks_package_require_hook.create(game::select(0x141D28EF0, 0x1403D7FC0),
                                    hks_package_require_stub);
    ui_cod_init_hook.create(UI_CoD_Init, ui_cod_init_stub);
    ui_cod_lobbyui_init_hook.create(UI_CoD_LobbyUI_Init,
                                    ui_cod_lobbyui_init_stub);
    ui_shutdown_hook.create(game::select(0x14270DE00, 0x1404A1280),
                            ui_shutdown_stub);
    lua_cod_getrawfile_hook.create(Lua_CoD_GetRawFile.get(),
                                   lua_cod_getrawfile_stub);

    hksi_lua_getinfo_detour.create(game::select(0x141D4D8D0, 0x1403F64B0),
                                   hksi_lua_getinfo_stub);

    if (game::is_client()) {

      ui_init_hook.create(UI_Init.get(), ui_init_stub);
      cl_first_snapshot_hook.create(game::cl::CL_FirstSnapshot.get(),
                                    cl_first_snapshot_stub);

      lua_error_hook.create(0x141F11DA0_g, lua_cod_luastatemanager_error_stub);
      lua_error_print_hook.create(0x141F132B0_g, lua_error_print_stub);

      scheduler::once(
          []() {
            game::ui_error_callstack_ship->flags().clear();
            game::ui_error_callstack_ship->set(true);

            game::ui_error_report_delay->flags().clear();
            game::ui_error_report_delay->set(true);
          },
          scheduler::pipeline::renderer);

      register_lui_commands();
      patch_unsafe_lua_functions();
    }
  }
};
} // namespace ui_scripting

REGISTER_COMPONENT(ui_scripting::component)