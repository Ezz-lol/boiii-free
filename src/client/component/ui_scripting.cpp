#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "game/ui_scripting/execution.hpp"

#include "ui_scripting.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/finally.hpp>

namespace ui_scripting
{
	namespace
	{
		std::unordered_map<game::hks::cclosure*, std::function<arguments(const function_arguments& args)>> converted_functions;

		utils::hook::detour ui_cod_init_hook;
		utils::hook::detour ui_shutdown_hook;
		utils::hook::detour hks_package_require_hook;
		utils::hook::detour lua_cod_getrawfile_hook;

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
				if (std::filesystem::is_regular_file(script))
				{
					const std::string file_path = script.substr(script.find("ui_scripts") + 11);
					globals.local_scripts[file_path.c_str()] = script;
				}
				else if (std::filesystem::is_directory(script))
				{
					load_local_script_files(script);
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

				if (std::filesystem::is_directory(script) && utils::io::read_file(script + "/__init__.lua", &data))
				{
					print_loading_script(script);
					load_script(script + "/__init__.lua", data);
				}
			}
		}

		void setup_functions()
		{
			const auto lua = get_globals();

			using game = table;
			auto game_type = game();
			lua["game"] = game_type;
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

		void start()
		{
			globals = {};

			const auto lua = get_globals();
			enable_globals();

			setup_functions();

			lua["print"] = function(reinterpret_cast<game::hks::lua_function>(0x141D30290_g)); // hks::base_print
			lua["table"]["unpack"] = lua["unpack"];
			lua["luiglobals"] = lua;

			load_scripts(game::get_host_library().get_folder().append("/data/ui_scripts/").string());
			load_scripts("boiii/ui_scripts/");
			load_scripts("data/ui_scripts/");
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

		void ui_cod_init_stub(void* a1, void* a2)
		{
			ui_cod_init_hook.invoke(a1, a2);
			const auto _0 = utils::finally(&try_start);
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

		int hks_load_stub(game::hks::lua_State* state, void* compiler_options, void* reader, void* reader_data, void* debug_reader, void* debug_reader_data, const char* chunk_name)
		{
			if (globals.load_raw_script)
			{
				globals.load_raw_script = false;
				globals.loaded_scripts[globals.raw_script_name] = globals.in_require_script;
				return load_buffer(globals.raw_script_name, utils::io::read_file(globals.raw_script_name));
			}

			return utils::hook::invoke<int>(0x141D3AFB0_g, state, compiler_options, reader, reader_data, debug_reader, debug_reader_data, chunk_name);
		}

		game::XAssetHeader lua_cod_getrawfile_stub(char* filename)
		{
			game::XAssetHeader header{ .luaFile = nullptr };

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
				header.luaFile = reinterpret_cast<game::LuaFile*>(1);

				return header;
			}

			return lua_cod_getrawfile_hook.invoke<game::XAssetHeader>(filename);
		}

		int luaopen_stub([[maybe_unused]] game::hks::lua_State* l)
		{
			return 0;
		}

		int hks_base_stub([[maybe_unused]] game::hks::lua_State* l)
		{
			return 0;
		}
	};

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

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			// Do not allow the HKS vm to open LUA's libraries
			utils::hook::jump(0x141D33510_g, luaopen_stub); // io
			utils::hook::jump(0x141D33D20_g, luaopen_stub); // os
			utils::hook::jump(0x141D34B40_g, luaopen_stub); // serialize
			utils::hook::jump(0x141D34B10_g, luaopen_stub); // havokscript
			utils::hook::jump(0x141D34190_g, luaopen_stub); // debug

			// Disable unsafe functions
			utils::hook::jump(0x141D300B0_g, hks_base_stub); // base_loadfile
			utils::hook::jump(0x141D31EE0_g, hks_base_stub); // base_load

			utils::hook::jump(0x141D2CF00_g, hks_base_stub); // string_dump
			utils::hook::jump(0x141D2AF90_g, hks_base_stub); // os_execute

			utils::hook::call(0x141D4979A_g, hks_load_stub);

			hks_package_require_hook.create(0x141D28EF0_g, hks_package_require_stub);
			ui_cod_init_hook.create(0x141F298B0_g, ui_cod_init_stub);
			ui_shutdown_hook.create(0x14270E9C0_g, ui_shutdown_stub);
			lua_cod_getrawfile_hook.create(0x141F0F880_g, lua_cod_getrawfile_stub);

			scheduler::once([]() {
				game::dvar_t* dvar_callstack_ship = game::Dvar_FindVar("ui_error_callstack_ship");
				dvar_callstack_ship->flags = (game::dvarFlags_e)0;
				game::dvar_t* dvar_report_delay= game::Dvar_FindVar("ui_error_report_delay");
				dvar_report_delay->flags = (game::dvarFlags_e)0;

				game::Dvar_SetFromStringByName("ui_error_callstack_ship", "1", true);
				game::Dvar_SetFromStringByName("ui_error_report_delay", "0", true);
			}, scheduler::pipeline::main);
		}
	};
}

REGISTER_COMPONENT(ui_scripting::component)
