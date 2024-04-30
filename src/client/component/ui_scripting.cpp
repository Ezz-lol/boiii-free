#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "game/ui_scripting/execution.hpp"

#include "command.hpp"
#include "ui_scripting.hpp"
#include "scheduler.hpp"

#include <utils/io.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/finally.hpp>

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

		struct globals_t
		{
			std::string in_require_script;
			std::unordered_map<std::string, std::string> loaded_scripts;
			std::unordered_map<std::string, std::string> local_scripts;
			bool load_raw_script{};
			std::string raw_script_name{};
		};

		globals_t globals;

		bool is_loaded_script(const std::string& name)
		{
			return globals.loaded_scripts.contains(name);
		}

		bool is_local_script(const std::string& name)
		{
			return globals.local_scripts.contains(name);
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
			lua["Engine"]["IsBOIII"] = true;
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

		void cl_first_snapshot_stub(int a1)
		{
			cl_first_snapshot_hook.invoke(a1);

			if (game::Com_IsRunningUILevel() || doneFirstSnapshot)
			{
				return;
			}
			doneFirstSnapshot = true;
			try_start();
		}

		void ui_shutdown_stub()
		{
			converted_functions.clear();
			globals = {};
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
				target_script = folder + "/" + name_ + ".lua";
			}
			else
			{
				target_script = globals.local_scripts[name_];
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

		int lua_unsafe_function_stub([[maybe_unused]] game::hks::lua_State* l)
		{
			if (!unsafe_function_called_message_shown)
			{
				auto state = get_globals();
				// TODO: Is it possible to do this with a confirm dialog? Doing this in LUI seems unsafe to me because mods will be able to change this aswell
				state["LuaUtils"]["ShowMessageDialog"](
					0, 0,
					"The map/mod you are playing tried to run code that can be unsafe. This can include writing or reading files on your system, accessing environment variables, running system commands or loading a dll. These are usually used for storing data across games, integrating third party software like Discord or fetching data from a server to make the gameplay for dynamic.\nThis can also cause a lot of harm by the wrong people.\n\nIf you trust this map/mod and want to enable these features, restart Black Ops 3 with the -unsafe-lua commandline argument.",
					"Unsafe lua function called");
				unsafe_function_called_message_shown = true;
			}

			return 0;
		}

		void patch_unsafe_lua_functions()
		{
			if (utils::flags::has_flag("unsafe-lua"))
			{
				return;
			}

			// Do not allow the HKS vm to open LUA's libraries
			// Disable unsafe functions
			utils::hook::jump(0x141D34190_g, luaopen_stub); // debug

			utils::hook::jump(0x141D300B0_g, lua_unsafe_function_stub); // base_loadfile
			utils::hook::jump(0x141D31EE0_g, lua_unsafe_function_stub); // base_load
			utils::hook::jump(0x141D2CF00_g, lua_unsafe_function_stub); // string_dump
			utils::hook::jump(0x141FD3220_g, lua_unsafe_function_stub); // engine_openurl

			utils::hook::jump(0x141D2AFF0_g, lua_unsafe_function_stub); // os_getenv
			utils::hook::jump(0x141D2B790_g, lua_unsafe_function_stub); // os_exit
			utils::hook::jump(0x141D2B7C0_g, lua_unsafe_function_stub); // os_remove
			utils::hook::jump(0x141D2BB70_g, lua_unsafe_function_stub); // os_rename
			utils::hook::jump(0x141D2B360_g, lua_unsafe_function_stub); // os_tmpname
			utils::hook::jump(0x141D2B0F0_g, lua_unsafe_function_stub); // os_sleep
			utils::hook::jump(0x141D2AF90_g, lua_unsafe_function_stub); // os_execute
			utils::hook::jump(0x141D2AFF0_g, lua_unsafe_function_stub); // os_getenv

			// io helpers
			utils::hook::jump(0x141D32390_g, lua_unsafe_function_stub); // io_tostring
			utils::hook::jump(0x141D2FDC0_g, lua_unsafe_function_stub); // io_close_file
			utils::hook::jump(0x141D2FD50_g, lua_unsafe_function_stub); // io_flush
			utils::hook::jump(0x141D31260_g, lua_unsafe_function_stub); // io_lines
			utils::hook::jump(0x141D305C0_g, lua_unsafe_function_stub); // io_read_file
			utils::hook::jump(0x141D305C0_g, lua_unsafe_function_stub); // io_read_file
			utils::hook::jump(0x141D320A0_g, lua_unsafe_function_stub); // io_seek_file
			utils::hook::jump(0x141D321E0_g, lua_unsafe_function_stub); // io_setvbuf
			utils::hook::jump(0x141D2FCD0_g, lua_unsafe_function_stub); // io_write

			// io functions
			utils::hook::jump(0x141D2FD10_g, lua_unsafe_function_stub); // io_write
			utils::hook::jump(0x141D30F40_g, lua_unsafe_function_stub); // io_read
			utils::hook::jump(0x141D2FF00_g, lua_unsafe_function_stub); // io_close
			utils::hook::jump(0x141D2FD90_g, lua_unsafe_function_stub); // io_flush
			utils::hook::jump(0x141D313A0_g, lua_unsafe_function_stub); // io_lines
			utils::hook::jump(0x141D31BA0_g, lua_unsafe_function_stub); // io_input
			utils::hook::jump(0x141D31BC0_g, lua_unsafe_function_stub); // io_output
			utils::hook::jump(0x141D31BE0_g, lua_unsafe_function_stub); // io_type
			utils::hook::jump(0x141D31DD0_g, lua_unsafe_function_stub); // io_open
			utils::hook::jump(0x141D31D70_g, lua_unsafe_function_stub); // io_tmpfile
			utils::hook::jump(0x141D33C00_g, lua_unsafe_function_stub); // io_popen

			utils::hook::jump(0x141D2D0C0_g, lua_unsafe_function_stub); // serialize_persist
			utils::hook::jump(0x141D2D480_g, lua_unsafe_function_stub); // serialize_unpersist

			utils::hook::jump(0x141D2F560_g, lua_unsafe_function_stub); // havokscript_compiler_settings
			utils::hook::jump(0x141D2F660_g, lua_unsafe_function_stub); // havokscript_setgcweights
			utils::hook::jump(0x141D2FB10_g, lua_unsafe_function_stub); // havokscript_getgcweights

			utils::hook::jump(0x141D299C0_g, lua_unsafe_function_stub); // package_loadlib
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

			patch_unsafe_lua_functions();
		}
	};
}

REGISTER_COMPONENT(ui_scripting::component)
