#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <game/game.hpp>

#include <Windows.h>
#include <detours.h>


namespace extension
{
	typedef BOOL(WINAPI* PTERMINATE_PROCESS)(HANDLE hProcess, UINT uExitCode);
	typedef VOID(WINAPI* PEXIT_PROCESS)(UINT uExitCode);

	PTERMINATE_PROCESS g_pTerminateProcess = nullptr;
	PEXIT_PROCESS g_pExitProcess = nullptr;

	BOOL WINAPI MyTerminateProcess(HANDLE hProcess, UINT uExitCode)
	{
		// oh
		return TRUE;
	}

	VOID WINAPI MyExitProcess(UINT uExitCode)
	{
		// damn
	}

	struct component final : generic_component
	{
		component()
		{
			this->extension_ = utils::nt::library::load(game::get_appdata_path() / "ext.dll");

			g_pTerminateProcess = (PTERMINATE_PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "TerminateProcess");
			g_pExitProcess = (PEXIT_PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "ExitProcess");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(LPVOID&)g_pTerminateProcess, MyTerminateProcess);
			DetourTransactionCommit();

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(LPVOID&)g_pExitProcess, MyExitProcess);
			DetourTransactionCommit();
		}

		~component() override
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(LPVOID&)g_pTerminateProcess, MyTerminateProcess);
			DetourTransactionCommit();

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(LPVOID&)g_pExitProcess, MyExitProcess);
			DetourTransactionCommit();

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
