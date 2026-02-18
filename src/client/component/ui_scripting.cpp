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

#include <utils/io.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>
#include <utils/http.hpp>

#include <filesystem>
#include <unordered_map>

namespace ui_scripting
{
	namespace
	{
		std::unordered_map<game::hks::cclosure*, std::function<arguments(const function_arguments& args)>>
		converted_functions;

		utils::hook::detour ui_init_hook;
		utils::hook::detour ui_cod_init_hook;
		utils::hook::detour ui_cod_lobbyui_init_hook;
		utils::hook::detour cl_first_snapshot_hook;
		utils::hook::detour ui_shutdown_hook;
		utils::hook::detour hks_package_require_hook;
		utils::hook::detour lua_cod_getrawfile_hook;

		bool unsafe_function_called_message_shown = false;
		bool unsafe_lua_approved_for_session = false;

		using lua_function_t = int(*)(game::hks::lua_State*);
		std::unordered_map<size_t, utils::hook::detour> unsafe_function_detours;

		struct globals_t
		{
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

		bool execute_raw_lua(const std::string& code, const char* chunk_name = "hot_reload")
		{
			const auto state = *game::hks::lua_state;
			if (!state) return false;

			game::hks::HksCompilerSettings compiler_settings{};
			const auto result = game::hks::hksi_hksL_loadbuffer(
				state, &compiler_settings, code.data(),
				static_cast<unsigned __int64>(code.size()), chunk_name);

			if (result != 0)
			{
				game::Com_Printf(0, 0, "^1Hot Reload: Failed to compile Lua chunk '%s'\n", chunk_name);
				return false;
			}

			game::hks::vm_call_internal(state, 0, 0, nullptr);
			return true;
		}

		void fire_debug_reload(const char* root_name)
		{
			const auto mapname = game::get_dvar_string("mapname");
			const auto code = utils::string::va(
				"pcall(function() LUI.roots.%s:processEvent({ name = 'debug_reload', mapname = '%s' }) end)",
				root_name, mapname.c_str());
			execute_raw_lua(code, "DebugReload");
		}

		int hot_reload_check_files()
		{
			if (hot_reload_path.empty()) return 0;

			std::vector<std::filesystem::directory_entry> changed;

			try
			{
				for (const auto& entry : std::filesystem::recursive_directory_iterator(hot_reload_path))
				{
					if (!entry.is_regular_file()) continue;
					if (entry.path().extension() != ".lua") continue;

					const auto path_str = entry.path().string();
					const auto mod_time = entry.last_write_time();

					auto it = hot_reload_files.find(path_str);
					if (it == hot_reload_files.end())
					{
						hot_reload_files[path_str] = mod_time;
						changed.push_back(entry);
					}
					else if (it->second < mod_time)
					{
						it->second = mod_time;
						changed.push_back(entry);
					}
				}
			}
			catch (const std::exception& ex)
			{
				game::Com_Printf(0, 0, "^1Hot Reload: Error scanning: %s\n", ex.what());
				return 0;
			}

			if (changed.empty()) return 0;

			game::Com_Printf(0, 0, "^2Hot Reload: Found %d file(s) to reload\n", static_cast<int>(changed.size()));

			for (const auto& entry : changed)
			{
				const auto path_str = entry.path().string();
				std::string data;
				if (!utils::io::read_file(path_str, &data)) continue;

				// Use relative path as chunk name
				auto chunk = path_str;
				if (chunk.starts_with(hot_reload_path))
				{
					chunk = chunk.substr(hot_reload_path.size());
				}

				if (execute_raw_lua(data, chunk.c_str()))
				{
					game::Com_Printf(0, 0, "^2Hot Reload: Reloaded %s\n", chunk.c_str());
				}
				else
				{
					game::Com_Printf(0, 0, "^1Hot Reload: Error reloading %s\n", chunk.c_str());
				}
			}

			// Fire debug_reload events to refresh UI
			fire_debug_reload("UIRootFull");
			if (hot_reload_in_game)
			{
				fire_debug_reload("UIRoot0");
				fire_debug_reload("UIRoot1");
			}

			return 1;
		}

		void start_hot_reload(const std::string& path)
		{
			hot_reload_path = path;
			hot_reload_files.clear();
			hot_reload_running = true;

			// Initial scan to populate timestamps
			hot_reload_check_files();

			// Install a Lua timer that calls back into C++ every second
			const std::string lua_code =
				"pcall(function() "
				"local UIRootFull = LUI.roots.UIRootFull; "
				"if UIRootFull.HotReloadTimer then UIRootFull:removeElement(UIRootFull.HotReloadTimer) end; "
				"UIRootFull.HotReloadTimer = LUI.UITimer.newElementTimer(1000, false, function() "
				"game.hotreloadcheck(); "
				"end); "
				"UIRootFull:addElement(UIRootFull.HotReloadTimer); "
				"end)";

			execute_raw_lua(lua_code, "HotReloadTimer");
			game::Com_Printf(0, 0, "^2Hot Reload: Watching '%s'\n", path.c_str());
		}

		void stop_hot_reload()
		{
			if (!hot_reload_running)
			{
				game::Com_Printf(0, 0, "^3Hot Reload: Not currently watching.\n");
				return;
			}

			hot_reload_running = false;
			hot_reload_files.clear();
			hot_reload_path.clear();

			// Remove the Lua timer
			const std::string lua_code =
				"pcall(function() "
				"local UIRootFull = LUI.roots.UIRootFull; "
				"if UIRootFull.HotReloadTimer then UIRootFull:removeElement(UIRootFull.HotReloadTimer); UIRootFull.HotReloadTimer = nil end; "
				"end)";
			execute_raw_lua(lua_code, "HotReloadTimerStop");

			game::Com_Printf(0, 0, "^2Hot Reload: Stopped watching.\n");
		}

		bool is_loaded_script(const std::string& name)
		{
			return globals.loaded_scripts.contains(name);
		}

		bool is_local_script(const std::string& name)
		{
			if (globals.local_scripts.contains(name)) return true;
			if (!name.ends_with(".lua") && globals.local_scripts.contains(name + ".lua")) return true;
			return false;
		}

		std::string get_root_script(const std::string& name)
		{
			const auto itr = globals.loaded_scripts.find(name);
			return (itr == globals.loaded_scripts.end()) ? std::string() : itr->second;
		}

		table get_globals()
		{
			const auto state = *game::hks::lua_state;
			return state->globals.v.table;
		}

		void print_error(const std::string& error)
		{
			printf("************** LUI script execution error **************\n");
			printf("%s\n", error.data());
			printf("********************************************************\n");
			MessageBoxA(nullptr, error.c_str(), "LUI Script Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		}

		void print_loading_script(const std::string& name)
		{
			printf("Loading LUI script '%s'\n", name.data());
		}

		std::string get_current_script(game::hks::lua_State* state)
		{
			game::hks::lua_Debug info{};
			game::hks::hksi_lua_getstack(state, 1, &info);
			game::hks::hksi_lua_getinfo(state, "nSl", &info);
			return info.short_src;
		}

		int load_buffer(const std::string& name, const std::string& data)
		{
			const auto state = *game::hks::lua_state;
			const auto sharing_mode = state->m_global->m_bytecodeSharingMode;
			state->m_global->m_bytecodeSharingMode = game::hks::HKS_BYTECODE_SHARING_ON;

			const auto _0 = utils::finally([&]
			{
				state->m_global->m_bytecodeSharingMode = sharing_mode;
			});

			game::hks::HksCompilerSettings compiler_settings{};
			return game::hks::hksi_hksL_loadbuffer(state, &compiler_settings, data.data(), data.size(), name.data());
		}

		void load_script(const std::string& name, const std::string& data)
		{
			globals.loaded_scripts[name] = name;

			const auto state = *game::hks::lua_state;
			const auto lua = get_globals();
			state->m_global->m_bytecodeSharingMode = game::hks::HKS_BYTECODE_SHARING_ON;
			const auto load_results = lua["loadstring"](data, name);
			state->m_global->m_bytecodeSharingMode = game::hks::HKS_BYTECODE_SHARING_SECURE;

			if (load_results[0].is<function>())
			{
				const auto results = lua["pcall"](load_results);
				if (!results[0].as<bool>())
				{
					print_error(results[1].as<std::string>());
				}
			}
			else if (load_results[1].is<std::string>())
			{
				print_error(load_results[1].as<std::string>());
			}
		}

		void load_local_script_files(const std::string& script_dir)
		{
			if (!utils::io::directory_exists(script_dir))
			{
				return;
			}

			const auto scripts = utils::io::list_files(script_dir);

			for (const auto& script : scripts)
			{
				const auto script_file = script.generic_string();

				if (std::filesystem::is_regular_file(script))
				{
					const std::string file_path = script_file.substr(script_file.find("ui_scripts") + 11);
					globals.local_scripts[file_path] = script_file;
				}
				else if (std::filesystem::is_directory(script))
				{
					load_local_script_files(script_file);
				}
			}
		}

		void load_scripts(const std::string& script_dir)
		{
			if (!utils::io::directory_exists(script_dir))
			{
				return;
			}

			load_local_script_files(script_dir);

			const auto scripts = utils::io::list_files(script_dir);

			for (const auto& script : scripts)
			{
				std::string data;
				const auto script_file = script.generic_string();
				if (std::filesystem::is_directory(script) && utils::io::read_file(script_file + "/__init__.lua", &data))
				{
					print_loading_script(script_file);
					load_script(script_file + "/__init__.lua", data);
				}
			}
		}

		void setup_functions()
		{
			const auto lua = get_globals();
			lua["game"] = table();

			lua["game"]["getfriendcount"] = function(convert_function([]() -> int
			{
				return friends::get_friend_count();
			}), game::hks::TCFUNCTION);

			lua["game"]["getfriend"] = function(convert_function([](int index) -> table
			{
				auto f = friends::get_friend_by_index(index);
				auto t = table();
				t.set("steam_id", utils::string::va("%llu", f.steam_id));
				t.set("name", std::string(f.name));
				t.set("status", static_cast<int>(f.state));
				t.set("server", std::string(f.server_address));
				return t;
			}), game::hks::TCFUNCTION);

			lua["game"]["addfriend"] = function(convert_function([](const std::string& steam_id_str, const std::string& name)
			{
				uint64_t steam_id = 0;
				try { steam_id = std::stoull(steam_id_str); } catch (...) { return; }
				friends::add_friend(steam_id, name);
			}), game::hks::TCFUNCTION);

			lua["game"]["removefriend"] = function(convert_function([](const std::string& steam_id_str)
			{
				uint64_t steam_id = 0;
				try { steam_id = std::stoull(steam_id_str); } catch (...) { return; }
				friends::remove_friend(steam_id);
			}), game::hks::TCFUNCTION);

			lua["game"]["invitefriend"] = function(convert_function([](const std::string& id_str) -> bool
			{
				uint64_t id = 0;
				try { id = std::stoull(id_str); } catch (...) { return false; }
				return friends::invite_to_game(id);
			}), game::hks::TCFUNCTION);

			lua["game"]["connecttofriend"] = function(convert_function([](const std::string& id_str) -> bool
			{
				uint64_t id = 0;
				try { id = std::stoull(id_str); } catch (...) { return false; }
				return friends::connect_to_friend(id);
			}), game::hks::TCFUNCTION);

			// HTTP functions
			lua["game"]["httpget"] = function(convert_function([](const std::string& url) -> std::string
			{
				const auto result = utils::http::get_data(url);
				return result.value_or("");
			}), game::hks::TCFUNCTION);

			lua["game"]["httppost"] = function(convert_function([](const std::string& url, const std::string& body) -> std::string
			{
				const auto result = utils::http::post_data(url, body);
				return result.value_or("");
			}), game::hks::TCFUNCTION);

			lua["game"]["setDiscordPlayerScore"] = function(convert_function([](int score)
			{
				discord::set_player_score(score);
			}), game::hks::TCFUNCTION);

			lua["game"]["setDiscordEnemyScore"] = function(convert_function([](int score)
			{
				discord::set_enemy_score(score);
			}), game::hks::TCFUNCTION);

			// Hot reload functions (callable from Lua timers)
			lua["game"]["hotreloadcheck"] = function(convert_function([]()
			{
				if (hot_reload_running)
				{
					hot_reload_check_files();
				}
			}), game::hks::TCFUNCTION);

			lua["game"]["hotreloadstart"] = function(convert_function([](const std::string& path)
			{
				start_hot_reload(path);
			}), game::hks::TCFUNCTION);
		}

		void enable_globals()
		{
			const auto lua = get_globals();
			const std::string code =
				"local g = getmetatable(_G)\n"
				"if not g then\n"
				"g = {}\n"
				"setmetatable(_G, g)\n"
				"end\n"
				"g.__newindex = nil\n";

			const auto state = *game::hks::lua_state;
			state->m_global->m_bytecodeSharingMode = game::hks::HKS_BYTECODE_SHARING_ON;
			lua["loadstring"](code)[0]();
			state->m_global->m_bytecodeSharingMode = game::hks::HKS_BYTECODE_SHARING_SECURE;
		}

		void setup_lua_globals()
		{
			globals = {};

			const auto lua = get_globals();
			enable_globals();

			setup_functions();

			lua["print"] = function(reinterpret_cast<game::hks::lua_function>(0x141D30290_g)); // hks::base_print
			lua["table"]["unpack"] = lua["unpack"];
			lua["luiglobals"] = lua;

			// Expose IsBOIII for mod compatibility - both as a value and function
			lua["Engine"]["IsBOIII"] = true;
			lua["Engine"]["IsEZZBOIII"] = true;

			//lua["IsBOIII"] = function([]() { return false; });
		}

		void start()
		{
			setup_lua_globals();

			const utils::nt::library host{};
			const auto folder = game::is_server() ? "lobby_scripts/" : "ui_scripts/";
			load_scripts((game::get_appdata_path() / "data" / folder).string());
			load_scripts((host.get_folder() / "boiii" / folder).string());
		}

		void try_start()
		{
			try
			{
				start();
			}
			catch (const std::exception& ex)
			{
				printf("Failed to load LUI scripts: %s\n", ex.what());
			}
		}

		void ui_init_stub(void* allocFunction, void* outOfMemoryFunction)
		{
			ui_init_hook.invoke(allocFunction, outOfMemoryFunction);

			setup_lua_globals();
		}

		bool doneFirstSnapshot = false;

		void ui_cod_init_stub(const bool frontend)
		{
			ui_cod_init_hook.invoke(frontend);

			if (!game::is_server() && game::Com_IsRunningUILevel())
			{
				// Fetch the names of the local files so file overrides are already handled
				globals = {};
				const utils::nt::library host{};
				doneFirstSnapshot = false;

				load_local_script_files((game::get_appdata_path() / "data/ui_scripts/").string());
				load_local_script_files((host.get_folder() / "boiii/ui_scripts/").string());
				return;
			}
			try_start();
		}

		void ui_cod_lobbyui_init_stub()
		{
			ui_cod_lobbyui_init_hook.invoke();
			try_start();
		}

		void inject_discord_score_subscriptions()
		{
			const auto state = *game::hks::lua_state;
			if (!state) return;

			const std::string lua_code =
				"pcall(function() "
				"LUI.roots.UIRoot0:subscribeToGlobalModel(0, 'GameScore', 'playerScore', function(model) "
				"local score = Engine.GetModelValue(model); "
				"if score and not Engine.IsVisibilityBitSet(0, Enum.UIVisibilityBit.BIT_IN_KILLCAM) then "
				"game.setDiscordPlayerScore(score); "
				"end; "
				"end); "
				"LUI.roots.UIRoot0:subscribeToGlobalModel(0, 'GameScore', 'enemyScore', function(model) "
				"local score = Engine.GetModelValue(model); "
				"if score and not Engine.IsVisibilityBitSet(0, Enum.UIVisibilityBit.BIT_IN_KILLCAM) then "
				"game.setDiscordEnemyScore(score); "
				"end; "
				"end); "
				"end)";

			game::hks::HksCompilerSettings compiler_settings{};
			const auto load_result = game::hks::hksi_hksL_loadbuffer(
				state, &compiler_settings, lua_code.data(),
				static_cast<unsigned __int64>(lua_code.size()), "discord_score_hooks");

			if (load_result == 0)
			{
				game::hks::vm_call_internal(state, 0, 0, nullptr);
			}
		}

		void cl_first_snapshot_stub(int a1)
		{
			cl_first_snapshot_hook.invoke(a1);

			if (game::Com_IsRunningUILevel() || doneFirstSnapshot)
			{
				return;
			}
			doneFirstSnapshot = true;
			hot_reload_in_game = true;
			try_start();

			try
			{
				inject_discord_score_subscriptions();
			}
			catch (...)
			{
			}
		}

		void ui_shutdown_stub()
		{
			converted_functions.clear();
			globals = {};
			hot_reload_in_game = false;
			unsafe_function_called_message_shown = false;
			unsafe_lua_approved_for_session = false;
			return ui_shutdown_hook.invoke<void>();
		}

		void* hks_package_require_stub(game::hks::lua_State* state)
		{
			const auto script = get_current_script(state);
			const auto root = get_root_script(script);
			globals.in_require_script = root;
			return hks_package_require_hook.invoke<void*>(state);
		}

		int hks_load_stub(game::hks::lua_State* state, void* compiler_options, void* reader, void* reader_data,
		                  void* debug_reader, void* debug_reader_data, const char* chunk_name)
		{
			if (globals.load_raw_script)
			{
				globals.load_raw_script = false;
				globals.loaded_scripts[globals.raw_script_name] = globals.in_require_script;
				return load_buffer(globals.raw_script_name, utils::io::read_file(globals.raw_script_name));
			}

			return utils::hook::invoke<int>(game::select(0x141D3AFB0, 0x1403E4090), state, compiler_options, reader,
			                                reader_data, debug_reader,
			                                debug_reader_data, chunk_name);
		}

		game::XAssetHeader lua_cod_getrawfile_stub(char* filename)
		{
			if (!is_loaded_script(globals.in_require_script) && !is_local_script(filename))
			{
				return lua_cod_getrawfile_hook.invoke<game::XAssetHeader>(filename);
			}

			const std::string name_ = filename;
			std::string target_script;
			if (is_loaded_script(globals.in_require_script))
			{
				const auto folder = globals.in_require_script.substr(0, globals.in_require_script.find_last_of("/\\"));
				if (name_.ends_with(".lua"))
				{
					target_script = folder + "/" + name_;
				}
				else
				{
					target_script = folder + "/" + name_ + ".lua";
				}
			}
			else
			{
				if (globals.local_scripts.contains(name_))
				{
					target_script = globals.local_scripts[name_];
				}
				else if (!name_.ends_with(".lua") && globals.local_scripts.contains(name_ + ".lua"))
				{
					target_script = globals.local_scripts[name_ + ".lua"];
				}
			}

			if (utils::io::file_exists(target_script))
			{
				globals.load_raw_script = true;
				globals.raw_script_name = target_script;

				return game::XAssetHeader{
					.luaFile = reinterpret_cast<game::RawFile*>(1) //
				};
			}

			return lua_cod_getrawfile_hook.invoke<game::XAssetHeader>(filename);
		}

		int luaopen_stub([[maybe_unused]] game::hks::lua_State* l)
		{
			return 0;
		}

		void show_unsafe_lua_dialog()
		{
			if (unsafe_function_called_message_shown)
			{
				return;
			}

			unsafe_function_called_message_shown = true;

			scheduler::once([]
			{
				const int result = MessageBoxA(
					nullptr,
					"The map/mod you are playing tried to run code that can be unsafe.\n\n"
					"This can include:\n"
					"  - Writing or reading files on your system\n"
					"  - Accessing environment variables\n"
					"  - Running system commands\n"
					"  - Loading DLLs\n\n"
					"These features are usually used for storing data across games, "
					"integrating third party software like Discord, or fetching data from a server.\n\n"
					"However, malicious mods could use these to harm your system.\n\n"
					"Do you want to enable unsafe lua functions for this session?\n\n"
					"Click 'Yes' to enable for this session only.\n"
					"Click 'No' to keep them blocked (recommended if you don't trust this mod).",
					"Unsafe Lua Function Called",
					MB_YESNO | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND
				);

				if (result == IDYES)
				{
					unsafe_lua_approved_for_session = true;
				}
			}, scheduler::pipeline::main);
		}

		template <size_t Key>
		int lua_unsafe_function_stub(game::hks::lua_State* l)
		{
			if (unsafe_lua_approved_for_session)
			{
				return unsafe_function_detours[Key].invoke<int>(l);
			}

			show_unsafe_lua_dialog();
			return 0;
		}

		template <size_t Key>
		void hook_unsafe_function(size_t address)
		{
			unsafe_function_detours[Key].create(address, lua_unsafe_function_stub<Key>);
		}

#define HOOK_UNSAFE_FUNCTION(addr) hook_unsafe_function<addr>(addr##_g)

		void patch_unsafe_lua_functions()
		{
			if (utils::flags::has_flag("unsafe-lua"))
			{
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
	}

	int main_handler(game::hks::lua_State* state)
	{
		const auto value = state->m_apistack.base[-1];
		if (value.t != game::hks::TCFUNCTION)
		{
			return 0;
		}

		const auto closure = value.v.cClosure;
		if (!converted_functions.contains(closure))
		{
			return 0;
		}

		const auto& function = converted_functions[closure];

		try
		{
			const auto args = get_return_values();
			const auto results = function(args);

			for (const auto& result : results)
			{
				push_value(result);
			}

			return static_cast<int>(results.size());
		}
		catch (const std::exception& ex)
		{
			game::hks::hksi_luaL_error(state, ex.what());
		}

		return 0;
	}

	template <typename F>
	game::hks::cclosure* convert_function(F f)
	{
		const auto state = *game::hks::lua_state;
		const auto closure = game::hks::cclosure_Create(state, main_handler, 0, 0, 0);
		converted_functions[closure] = wrap_function(f);
		return closure;
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			utils::hook::call(game::select(0x141D4979A, 0x1403F233A), hks_load_stub);

			hks_package_require_hook.create(game::select(0x141D28EF0, 0x1403D7FC0), hks_package_require_stub);
			ui_cod_init_hook.create(game::UI_CoD_Init, ui_cod_init_stub);
			ui_cod_lobbyui_init_hook.create(game::UI_CoD_LobbyUI_Init, ui_cod_lobbyui_init_stub);
			ui_shutdown_hook.create(game::select(0x14270DE00, 0x1404A1280), ui_shutdown_stub);
			lua_cod_getrawfile_hook.create(game::select(0x141F0EFE0, 0x1404BCB70), lua_cod_getrawfile_stub);

			if (game::is_server())
			{
				return;
			}

			ui_init_hook.create(0x142704FF0_g, ui_init_stub);
			cl_first_snapshot_hook.create(0x141320E60_g, cl_first_snapshot_stub);

			scheduler::once([]()
			{
				game::dvar_t* dvar_callstack_ship = game::Dvar_FindVar("ui_error_callstack_ship");
				dvar_callstack_ship->flags = static_cast<game::dvarFlags_e>(0);
				game::dvar_t* dvar_report_delay = game::Dvar_FindVar("ui_error_report_delay");
				dvar_report_delay->flags = static_cast<game::dvarFlags_e>(0);

				game::Dvar_SetFromStringByName("ui_error_callstack_ship", "1", true);
				game::Dvar_SetFromStringByName("ui_error_report_delay", "0", true);
			}, scheduler::pipeline::renderer);

			command::add("luiReload", []
			{
				if (game::Com_IsRunningUILevel())
				{
					converted_functions.clear();

					globals.loaded_scripts.clear();
					globals.local_scripts.clear();

					game::UI_CoD_Shutdown();
					game::UI_CoD_Init(true);

					// Com_LoadFrontEnd stripped
					game::Lua_CoD_LoadLuaFile(*game::hks::lua_state, "ui_mp.T6.main");
					game::UI_AddMenu(game::UI_CoD_GetRootNameForController(0), "main", -1, *game::hks::lua_state);

					game::UI_CoD_LobbyUI_Init();
				}
				else
				{
					// TODO: Find a way to do a full shutdown & restart like in frontend, that opens up the loading screen that can't be easily closed
					game::CG_LUIHUDRestart(0);
				}
			});

			command::add("lua_hotreload", [](const command::params& params)
			{
				std::string dir;
				if (params.size() >= 2)
				{
					dir = params.get(1);
				}
				else
				{
					dir = (game::get_appdata_path() / "data" / "ui_scripts").string();
				}

				scheduler::once([dir]
				{
					start_hot_reload(dir);
				}, scheduler::pipeline::renderer);
			});

			command::add("lua_hotreload_stop", [](const command::params&)
			{
				scheduler::once([]
				{
					stop_hot_reload();
				}, scheduler::pipeline::renderer);
			});

			command::add("lua_reload", [](const command::params& params)
			{
				std::string dir;
				if (params.size() >= 2)
				{
					dir = params.get(1);
				}
				else
				{
					dir = (game::get_appdata_path() / "data" / "ui_scripts").string();
				}

				scheduler::once([dir]
				{
					try
					{
						int count = 0;
						for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
						{
							if (!entry.is_regular_file()) continue;
							if (entry.path().extension() != ".lua") continue;

							std::string data;
							if (utils::io::read_file(entry.path().string(), &data))
							{
								auto chunk = entry.path().string();
								if (chunk.starts_with(dir)) chunk = chunk.substr(dir.size());
								execute_raw_lua(data, chunk.c_str());
								count++;
							}
						}

						game::Com_Printf(0, 0, "^2Lua Reload: Reloaded %d file(s)\n", count);

						fire_debug_reload("UIRootFull");
						if (hot_reload_in_game)
						{
							fire_debug_reload("UIRoot0");
							fire_debug_reload("UIRoot1");
						}
					}
					catch (const std::exception& ex)
					{
						game::Com_Printf(0, 0, "^1Lua Reload: Error: %s\n", ex.what());
					}
				}, scheduler::pipeline::renderer);
			});

			command::add("lua_exec", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					game::Com_Printf(0, 0, "Usage: lua_exec <file.lua>\n");
					return;
				}

				const std::string file = params.get(1);
				std::string data;
				if (!utils::io::read_file(file, &data))
				{
					game::Com_Printf(0, 0, "^1Failed to read file: %s\n", file.c_str());
					return;
				}

				scheduler::once([data, file]
				{
					if (execute_raw_lua(data, file.c_str()))
					{
						game::Com_Printf(0, 0, "^2Executed Lua file successfully\n");
					}
				}, scheduler::pipeline::renderer);
			});

			patch_unsafe_lua_functions();
		}
	};
}

REGISTER_COMPONENT(ui_scripting::component)
