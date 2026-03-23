#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "../game_event.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace script
{
	int64_t find_export_address(const std::string& script_name, const std::string& func_name);
}

namespace gsc_funcs
{
	namespace
	{
		using vm_opcode_handler_t = void(__fastcall*)(int32_t inst, int64_t* fs_0, int64_t vmc, bool* terminate);
		using ScrVm_GetInt_t = int64_t(__fastcall*)(unsigned int inst, unsigned int index);
		using ScrVm_GetString_t = const char*(__fastcall*)(unsigned int inst, unsigned int index);

		ScrVm_GetInt_t ScrVm_GetInt = nullptr;
		ScrVm_GetString_t ScrVm_GetString = nullptr;

		std::unordered_map<int32_t, void(*)(int)> custom_builtins;
		std::unordered_map<int64_t, int64_t> function_replacements;
		bool detours_enabled = false;

		vm_opcode_handler_t orig_SafeCreateLocalVariables = nullptr;
		vm_opcode_handler_t orig_CheckClearParams = nullptr;

		constexpr uint32_t fnv1a(const char* str)
		{
			uint32_t hash = 0x811c9dc5;
			while (*str)
			{
				hash ^= static_cast<uint8_t>(*str++);
				hash *= 0x01000193;
			}
			return hash;
		}


		bool try_redirect(int32_t inst, int64_t* fs_0)
		{
			if (!detours_enabled || inst != 0) return false;
			const auto it = function_replacements.find(*fs_0 - 2);
			if (it != function_replacements.end())
			{
				*fs_0 = it->second;
				return true;
			}
			return false;
		}

		void __fastcall hk_SafeCreateLocalVariables(int32_t inst, int64_t* fs_0, int64_t vmc, bool* terminate)
		{
			if (!try_redirect(inst, fs_0))
				orig_SafeCreateLocalVariables(inst, fs_0, vmc, terminate);
		}

		void __fastcall hk_CheckClearParams(int32_t inst, int64_t* fs_0, int64_t vmc, bool* terminate)
		{
			if (!try_redirect(inst, fs_0))
				orig_CheckClearParams(inst, fs_0, vmc, terminate);
		}

		void hook_opcode(uint16_t opcode, vm_opcode_handler_t hook, vm_opcode_handler_t* out_orig)
		{
			const auto table1 = game::select(0x1432E6350, 0x0);
			const auto table2 = game::select(0x143306350, 0x0);

			for (const auto table : { table1, table2 })
			{
				if (!table) continue;
				auto* entry = reinterpret_cast<int64_t*>(table + opcode * 8);
				if (!*out_orig)
					*out_orig = reinterpret_cast<vm_opcode_handler_t>(*entry);
				if (*entry == reinterpret_cast<int64_t>(*out_orig))
					*entry = reinterpret_cast<int64_t>(hook);
			}
		}

		void builtin_dispatcher(game::scriptInstance_t inst)
		{
			if (!ScrVm_GetInt) return;
			try
			{
				const auto hash = static_cast<int32_t>(ScrVm_GetInt(static_cast<unsigned int>(inst), 0));
				const auto it = custom_builtins.find(hash);
				if (it != custom_builtins.end())
					it->second(static_cast<int>(inst));
			}
			catch (const std::exception& e)
			{
				printf("^1[builtin] Exception: %s\n", e.what());
			}
			catch (...)
			{
				printf("^1[builtin] Unknown exception\n");
			}

			// EEEEE i think it should fix it
			game::Scr_AddInt(inst, 0);
		}


		// replacefunc: redirect all calls to target_func to replacement_func
		void gscr_replacefunc(int inst)
		{
			const auto target_script = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
			const auto target_func = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
			const auto replace_script = ScrVm_GetString(static_cast<unsigned int>(inst), 3);
			const auto replace_func = ScrVm_GetString(static_cast<unsigned int>(inst), 4);

			if (!target_script || !target_func || !replace_script || !replace_func)
				return;

			const auto target_addr = script::find_export_address(target_script, target_func);
			const auto replace_addr = script::find_export_address(replace_script, replace_func);

			if (target_addr && replace_addr)
			{
				function_replacements[target_addr] = replace_addr;
				detours_enabled = true;
			}
		}

		void gscr_println(int inst)
		{
			const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
			printf("%s\n", msg ? msg : "");
		}

		void gscr_executecommand(int inst)
		{
			const auto cmd = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
			if (cmd)
				game::Cbuf_AddText(0, utils::string::va("%s\n", cmd));
		}

		// say: broadcast a chat message to all players
		// GSC: say("Hello world");
		void gscr_say(int inst)
		{
			const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 1);
			if (msg)
				game::SV_GameSendServerCommand(-1, game::SV_CMD_CAN_IGNORE_0,
					utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
		}

		// tell: send a private chat message to a specific client
		// GSC: player tell("Hello");
		void gscr_tell(int inst)
		{
			const auto client_num = static_cast<int>(ScrVm_GetInt(static_cast<unsigned int>(inst), 1));
			const auto msg = ScrVm_GetString(static_cast<unsigned int>(inst), 2);
			if (client_num >= 0 && client_num < 18 && msg)
				game::SV_GameSendServerCommand(client_num, game::SV_CMD_CAN_IGNORE_0,
					utils::string::va("v \"%Iu %d %d %s\"", -1, 0, 0, msg));
		}
	}

	void add_detour(int64_t target_addr, int64_t replacement_addr)
	{
		function_replacements[target_addr] = replacement_addr;
		detours_enabled = true;
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			ScrVm_GetInt = reinterpret_cast<ScrVm_GetInt_t>(game::relocate(0x1412EB7F0));
			ScrVm_GetString = reinterpret_cast<ScrVm_GetString_t>(game::relocate(0x1412EBAA0));

			custom_builtins[static_cast<int32_t>(fnv1a("replacefunc"))] = gscr_replacefunc;
			custom_builtins[static_cast<int32_t>(fnv1a("executecommand"))] = gscr_executecommand;
			custom_builtins[static_cast<int32_t>(fnv1a("say"))] = gscr_say;
			custom_builtins[static_cast<int32_t>(fnv1a("tell"))] = gscr_tell;
			custom_builtins[static_cast<int32_t>(fnv1a("println"))] = gscr_println;

			auto* builtin_def = reinterpret_cast<game::BuiltinFunctionDef*>(game::relocate(0x1432D7D70));
			builtin_def->max_args = 255;
			builtin_def->actionFunc = reinterpret_cast<void*>(builtin_dispatcher);

			// Hook function-entry opcodes (SafeCreateLocalVariables + CheckClearParams).
			hook_opcode(0x01D2, hk_SafeCreateLocalVariables, &orig_SafeCreateLocalVariables);
			hook_opcode(0x000D, hk_CheckClearParams, &orig_CheckClearParams);

			game_event::on_g_shutdown_game([] {
				function_replacements.clear();
				detours_enabled = false;
			});
		}
	};
}

REGISTER_COMPONENT(gsc_funcs::component)
