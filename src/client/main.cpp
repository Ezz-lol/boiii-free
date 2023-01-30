#include <std_include.hpp>

#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <utils/finally.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>

#include <steam/steam.hpp>

#include "game/game.hpp"
#include "launcher/launcher.hpp"

namespace
{
	volatile bool g_call_tls_callbacks = false;
	std::pair<void**, void*> g_original_import{};

	DECLSPEC_NORETURN void WINAPI exit_hook(const uint32_t code)
	{
		component_loader::pre_destroy();
		ExitProcess(code);
	}

	std::pair<void**, void*> patch_steam_import(const std::string& func, void* function)
	{
		static const utils::nt::library game{};

		const auto game_entry = game.get_iat_entry("steam_api64.dll", func);
		if (!game_entry)
		{
			//throw std::runtime_error("Import '" + func + "' not found!");
			return {nullptr, nullptr};
		}

		const auto original_import = game_entry;
		utils::hook::set(game_entry, function);
		return {game_entry, original_import};
	}

	bool restart_app_if_necessary_stub()
	{
		const std::string steam_path = steam::SteamAPI_GetSteamInstallPath();
		if (steam_path.empty() || !::utils::io::file_exists(steam_path + "/steam.exe"))
		{
			MessageBoxA(nullptr, "Steam must be installed for the game to run. Please install Steam!", "Error",
			            MB_ICONERROR);
			ShellExecuteA(nullptr, "open", "https://store.steampowered.com/about/", nullptr, nullptr, SW_SHOWNORMAL);
			TerminateProcess(GetCurrentProcess(), 1);
		}

		utils::hook::set(g_original_import.first, g_original_import.second);
		patch_steam_import("SteamAPI_Shutdown", steam::SteamAPI_Shutdown);

		component_loader::post_unpack();
		return steam::SteamAPI_RestartAppIfNecessary();
	}

	BOOL set_process_dpi_aware_stub()
	{
		component_loader::post_unpack();
		return SetProcessDPIAware();
	}

	void patch_imports()
	{
		patch_steam_import("SteamAPI_RegisterCallback", steam::SteamAPI_RegisterCallback);
		patch_steam_import("SteamAPI_RegisterCallResult", steam::SteamAPI_RegisterCallResult);
		patch_steam_import("SteamGameServer_Shutdown", steam::SteamGameServer_Shutdown);
		patch_steam_import("SteamGameServer_RunCallbacks", steam::SteamGameServer_RunCallbacks);
		patch_steam_import("SteamGameServer_GetHSteamPipe", steam::SteamGameServer_GetHSteamPipe);
		patch_steam_import("SteamGameServer_GetHSteamUser", steam::SteamGameServer_GetHSteamUser);
		patch_steam_import("SteamInternal_GameServer_Init", steam::SteamInternal_GameServer_Init);
		patch_steam_import("SteamAPI_UnregisterCallResult", steam::SteamAPI_UnregisterCallResult);
		patch_steam_import("SteamAPI_UnregisterCallback", steam::SteamAPI_UnregisterCallback);
		patch_steam_import("SteamAPI_RunCallbacks", steam::SteamAPI_RunCallbacks);
		patch_steam_import("SteamInternal_CreateInterface", steam::SteamInternal_CreateInterface);
		patch_steam_import("SteamAPI_GetHSteamUser", steam::SteamAPI_GetHSteamUser);
		patch_steam_import("SteamAPI_GetHSteamPipe", steam::SteamAPI_GetHSteamPipe);
		patch_steam_import("SteamAPI_Init", steam::SteamAPI_Init);
		//patch_steam_import("SteamAPI_Shutdown", steam::SteamAPI_Shutdown);
		g_original_import = patch_steam_import("SteamAPI_RestartAppIfNecessary", restart_app_if_necessary_stub);

		const utils::nt::library game{};
		utils::hook::set(game.get_iat_entry("kernel32.dll", "ExitProcess"), exit_hook);
		utils::hook::set(game.get_iat_entry("user32.dll", "SetProcessDPIAware"), set_process_dpi_aware_stub);
	}

	void remove_crash_file()
	{
		const utils::nt::library game{};
		const auto game_file = game.get_path();
		auto game_path = std::filesystem::path(game_file);
		game_path.replace_extension(".start");

		utils::io::remove_file(game_path);
	}

	PIMAGE_TLS_CALLBACK* get_tls_callbacks()
	{
		const utils::nt::library game{};
		const auto& entry = game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
		if (!entry.VirtualAddress || !entry.Size)
		{
			return nullptr;
		}

		const auto* tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(game.get_ptr() + entry.VirtualAddress);
		return reinterpret_cast<PIMAGE_TLS_CALLBACK*>(tls_dir->AddressOfCallBacks);
	}

	void run_tls_callbacks(const DWORD reason)
	{
		if (!g_call_tls_callbacks)
		{
			return;
		}

		auto* callback = get_tls_callbacks();
		while (callback && *callback)
		{
			(*callback)(GetModuleHandleA(nullptr), reason, nullptr);
			++callback;
		}
	}

	[[maybe_unused]] thread_local struct tls_runner
	{
		tls_runner()
		{
			run_tls_callbacks(DLL_THREAD_ATTACH);
		}

		~tls_runner()
		{
			run_tls_callbacks(DLL_THREAD_DETACH);
		}
	} tls_runner;

	FARPROC load_process(const std::string& procname)
	{
		const auto proc = loader::load_binary(procname);

		auto* const peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
		peb->Reserved3[1] = proc.get_ptr();
		static_assert(offsetof(PEB, Reserved3[1]) == 0x10);

		return FARPROC(proc.get_ptr() + proc.get_relative_entry_point());
	}

	bool handle_process_runner()
	{
		const auto* const command = "-proc ";
		const char* parent_proc = strstr(GetCommandLineA(), command);

		if (!parent_proc)
		{
			return false;
		}

		const auto pid = DWORD(atoi(parent_proc + strlen(command)));
		const utils::nt::handle<> process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
		if (process_handle)
		{
			WaitForSingleObject(process_handle, INFINITE);
		}

		return true;
	}

	void enable_dpi_awareness()
	{
		const utils::nt::library user32{"user32.dll"};
		const auto set_dpi = user32
			                     ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
				                     "SetProcessDpiAwarenessContext")
			                     : nullptr;
		if (set_dpi)
		{
			set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		}
	}

	int main()
	{
		if (handle_process_runner())
		{
			return 0;
		}

		FARPROC entry_point{};
		srand(uint32_t(time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

		enable_dpi_awareness();

		{
			auto premature_shutdown = true;
			const auto _ = utils::finally([&premature_shutdown]
			{
				if (premature_shutdown)
				{
					component_loader::pre_destroy();
				}
			});

			try
			{
				remove_crash_file();

				if (!launcher::run())
				{
					return 0;
				}

				const auto client_binary = "BlackOps3.exe"s;
				const auto server_binary = "BlackOps3_UnrankedDedicatedServer.exe"s;
				const auto has_server = !utils::io::file_exists(client_binary) || utils::flags::has_flag("dedicated");

				if (!component_loader::activate(has_server))
				{
					return 1;
				}

				entry_point = load_process(has_server ? server_binary : client_binary);
				if (!entry_point)
				{
					throw std::runtime_error("Unable to load binary into memory");
				}

				if (has_server != game::is_server())
				{
					throw std::runtime_error("Bad binary loaded into memory");
				}

				patch_imports();

				if (!component_loader::post_load())
				{
					return 1;
				}

				premature_shutdown = false;
			}
			catch (std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				return 1;
			}
		}

		g_call_tls_callbacks = true;
		return static_cast<int>(entry_point());
	}
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
