#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <game/game.hpp>

#include <Windows.h>
#include <MinHook.h>

namespace extension
{
	typedef BOOL(WINAPI* PTERMINATE_PROCESS)(HANDLE hProcess, UINT uExitCode);
	typedef VOID(WINAPI* PEXIT_PROCESS)(UINT uExitCode);

	PTERMINATE_PROCESS g_pTerminateProcess = nullptr;
	PEXIT_PROCESS g_pExitProcess = nullptr;

	BOOL WINAPI T_Ezz(HANDLE hProcess, UINT uExitCode)
	{
		// :o
		return TRUE;
	}

	VOID WINAPI E_Ezz(UINT uExitCode)
	{
		// :O
	}

	struct component final : generic_component
	{
		component()
		{
			this->extension_ = utils::nt::library::load(game::get_appdata_path() / "ext.dll");

			g_pTerminateProcess = (PTERMINATE_PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "TerminateProcess");
			g_pExitProcess = (PEXIT_PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "ExitProcess");

			MH_Initialize();

			MH_CreateHook(g_pTerminateProcess, T_Ezz, reinterpret_cast<LPVOID*>(&g_pTerminateProcess));
			MH_EnableHook(g_pTerminateProcess);

			MH_CreateHook(g_pExitProcess, E_Ezz, reinterpret_cast<LPVOID*>(&g_pExitProcess));
			MH_EnableHook(g_pExitProcess);
		}

		~component() override
		{
			MH_DisableHook(g_pTerminateProcess);
			MH_RemoveHook(g_pTerminateProcess);

			MH_DisableHook(g_pExitProcess);
			MH_RemoveHook(g_pExitProcess);

			MH_Uninitialize();

			this->extension_.free();
		}

		void post_load() override
		{
			this->extension_.invoke<void>("_1");
		}

		void post_unpack() override
		{
			this->extension_.invoke<void>("_2");
		}

		void pre_destroy() override
		{
			this->extension_.invoke<void>("_3");
		}

		utils::nt::library extension_{};
	};
}

REGISTER_COMPONENT(extension::component)
