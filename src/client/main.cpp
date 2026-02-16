#include <std_include.hpp>

#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <utils/finally.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/flags.hpp>
#include <utils/com.hpp>

#include <steam/steam.hpp>

#include "game/game.hpp"
#include "launcher/launcher.hpp"
#include "launcher/html/html_window.hpp"
#include "component/updater.hpp"

#include <ShlObj.h>

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
		if (steam_path.empty() || !utils::io::file_exists(steam_path + "/steam.exe"))
		{
			game::show_error("Steam must be installed for the game to run. Please install Steam!");
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

		const auto pid = static_cast<DWORD>(atoi(parent_proc + strlen(command)));
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

		{
			const auto set_dpi = user32
				                     ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
					                     "SetProcessDpiAwarenessContext")
				                     : nullptr;
			if (set_dpi)
			{
				set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
				return;
			}
		}

		{
			const utils::nt::library shcore{"shcore.dll"};
			const auto set_dpi = shcore
				                     ? shcore.get_proc<HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS)>(
					                     "SetProcessDpiAwareness")
				                     : nullptr;
			if (set_dpi)
			{
				set_dpi(PROCESS_PER_MONITOR_DPI_AWARE);
				return;
			}
		}

		{
			const auto set_dpi = user32
				                     ? user32.get_proc<BOOL(WINAPI*)()>(
					                     "SetProcessDPIAware")
				                     : nullptr;
			if (set_dpi)
			{
				set_dpi();
			}
		}
	}

	void trigger_high_performance_gpu_switch()
	{
		// Make sure to link D3D11, as this might trigger high performance GPU
		static volatile auto _ = &D3D11CreateDevice;

		const auto key = utils::nt::open_or_create_registry_key(
			HKEY_CURRENT_USER, R"(Software\Microsoft\DirectX\UserGpuPreferences)");
		if (!key)
		{
			return;
		}

		const auto self = utils::nt::library::get_by_address(&trigger_high_performance_gpu_switch);
		const auto path = self.get_path().make_preferred().wstring();

		if (RegQueryValueExW(key, path.data(), nullptr, nullptr, nullptr, nullptr) != ERROR_FILE_NOT_FOUND)
		{
			return;
		}

		const std::wstring data = L"GpuPreference=2;";
		RegSetValueExW(key, self.get_path().make_preferred().wstring().data(), 0, REG_SZ,
		               reinterpret_cast<const BYTE*>(data.data()),
		               static_cast<DWORD>((data.size() + 1u) * 2));
	}

	void validate_non_network_share()
	{
		const auto self = utils::nt::library::get_by_address(&validate_non_network_share);
		const auto path = self.get_path().make_preferred();
		const auto wpath = path.wstring();
		if (wpath.size() >= 2 && wpath[0] == L'\\' && wpath[1] == L'\\')
		{
			throw std::runtime_error(
				"You seem to be using a network share:\n\n" + path.string() + "\n\nNetwork shares are not supported!");
		}
	}

	std::filesystem::path get_game_path_file()
	{
		return game::get_appdata_path() / "user" / "game_path.txt";
	}

	bool is_valid_game_folder(const std::filesystem::path& folder)
	{
		return std::filesystem::exists(folder / "BlackOps3.exe")
			|| std::filesystem::exists(folder / "BlackOps3_UnrankedDedicatedServer.exe");
	}

	std::string find_steam_game_path()
	{
		const char* default_path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty Black Ops III";
		if (is_valid_game_folder(default_path))
		{
			return default_path;
		}

		const std::string steam_path = steam::SteamAPI_GetSteamInstallPath();
		if (steam_path.empty()) return {};

		auto steamapps = std::filesystem::path(steam_path) / "steamapps" / "common" / "Call of Duty Black Ops III";
		if (is_valid_game_folder(steamapps))
		{
			return steamapps.string();
		}

		auto vdf_path = std::filesystem::path(steam_path) / "steamapps" / "libraryfolders.vdf";
		std::string vdf_data;
		if (utils::io::read_file(vdf_path.string(), &vdf_data))
		{
			std::istringstream stream(vdf_data);
			std::string line;
			while (std::getline(stream, line))
			{
				auto pos = line.find("\"path\"");
				if (pos == std::string::npos) continue;

				auto first_quote = line.find('"', pos + 6);
				auto last_quote = line.rfind('"');
				if (first_quote == std::string::npos || last_quote <= first_quote) continue;

				auto lib_path = line.substr(first_quote + 1, last_quote - first_quote - 1);
				auto game_folder = std::filesystem::path(lib_path) / "steamapps" / "common" / "Call of Duty Black Ops III";
				if (is_valid_game_folder(game_folder))
				{
					return game_folder.string();
				}
			}
		}

		return {};
	}

	bool resolve_game_path()
	{
		const auto path_file = get_game_path_file();

		if (is_valid_game_folder("."))
		{
			char cwd[MAX_PATH];
			if (GetCurrentDirectoryA(sizeof(cwd), cwd))
			{
				std::error_code ec;
				std::filesystem::create_directories(path_file.parent_path(), ec);
				utils::io::write_file(path_file.string(), std::string(cwd));
			}
			return true;
		}

		{
			std::string stored_path;
			if (utils::io::read_file(path_file.string(), &stored_path) && !stored_path.empty())
			{
				while (!stored_path.empty() && (stored_path.back() == '\n' || stored_path.back() == '\r' || stored_path.back() == ' '))
					stored_path.pop_back();

				if (is_valid_game_folder(stored_path))
				{
					SetCurrentDirectoryA(stored_path.c_str());
					return true;
				}
			}
		}

		{
			auto steam_game = find_steam_game_path();
			if (!steam_game.empty())
			{
				std::error_code ec;
				std::filesystem::create_directories(path_file.parent_path(), ec);
				utils::io::write_file(path_file.string(), steam_game);
				SetCurrentDirectoryA(steam_game.c_str());
				return true;
			}
		}

		bool path_set = false;

		html_window setup_window("BOIII - Game Setup", 480, 300,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);

		setup_window.get_html_frame()->register_callback(
			"selectFolder", [&path_set, &setup_window, &path_file](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				std::string selected_str;
				try
				{
					if (!utils::com::select_folder(selected_str, "Select your Black Ops 3 installation folder"))
					{
						return CComVariant("cancelled");
					}
				}
				catch (...)
				{
					return CComVariant("error");
				}

				if (!is_valid_game_folder(selected_str))
				{
					return CComVariant("invalid");
				}

				std::error_code ec;
				std::filesystem::create_directories(path_file.parent_path(), ec);
				utils::io::write_file(path_file.string(), selected_str);
				SetCurrentDirectoryA(selected_str.c_str());

				path_set = true;
				setup_window.get_window()->close();

				return CComVariant(selected_str.c_str());
			});

		setup_window.get_html_frame()->load_html(R"html(
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
	background: #1a1a1a; color: #e0e0e0; font-family: 'Segoe UI', sans-serif;
	display: flex; flex-direction: column; align-items: center; justify-content: center;
	height: 100vh; padding: 30px; user-select: none;
}
.icon { font-size: 42px; margin-bottom: 12px; }
h2 { font-size: 17px; font-weight: 600; margin-bottom: 8px; color: #fff; }
p { font-size: 13px; color: #aaa; text-align: center; line-height: 1.5; margin-bottom: 20px; }
button {
	background: #2d7d2d; color: #fff; border: none; padding: 10px 28px;
	font-size: 14px; border-radius: 4px; cursor: pointer; font-weight: 500;
}
button:hover { background: #359935; }
#status { font-size: 12px; color: #e74c3c; margin-top: 12px; min-height: 18px; }
</style>
</head>
<body>
<div class="icon">&#9888;</div>
<h2>Game Not Found</h2>
<p>Could not locate Black Ops 3 installation.<br>Please select your game folder to continue.</p>
<button onclick="doSelect()">Set Game Path</button>
<div id="status"></div>
<script>
function doSelect() {
	var result = window.external.selectFolder();
	if (result === 'invalid') {
		document.getElementById('status').innerText = 'Selected folder does not contain BlackOps3.exe';
	} else if (result === 'error') {
		document.getElementById('status').innerText = 'An error occurred. Please try again.';
	} else if (result === 'cancelled') {
		document.getElementById('status').innerText = '';
	}
}
</script>
</body>
</html>
)html");

		window::run();

		return path_set;
	}
}

int main()
{
	if (handle_process_runner())
	{
		return 0;
	}

	FARPROC entry_point{};
	srand(static_cast<uint32_t>(time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

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
			validate_non_network_share();
			remove_crash_file();

			if (!resolve_game_path())
			{
				return 0;
			}

			updater::update();

			if (!utils::io::file_exists(launcher::get_launcher_ui_file().generic_wstring()))
			{
				throw std::runtime_error("BOIII needs an active internet connection for the first time you launch it.");
			}

			const auto client_binary = "BlackOps3.exe"s;
			const auto server_binary = "BlackOps3_UnrankedDedicatedServer.exe"s;

			auto has_client = utils::io::file_exists(client_binary);
			auto has_server = utils::io::file_exists(server_binary);

			while (!has_client && !has_server)
			{
				if (!resolve_game_path())
				{
					return 0;
				}
				has_client = utils::io::file_exists(client_binary);
				has_server = utils::io::file_exists(server_binary);
			}

			const auto is_server = utils::flags::has_flag("dedicated") || (!has_client && has_server);

			if (!is_server)
			{
				trigger_high_performance_gpu_switch();

				const auto launch = utils::flags::has_flag("launch");
				if (!launch && !utils::nt::is_wine() && !launcher::run())
				{
					return 0;
				}
			}

			if (!component_loader::activate(is_server))
			{
				return 1;
			}

			entry_point = load_process(is_server ? server_binary : client_binary);
			if (!entry_point)
			{
				throw std::runtime_error("Unable to load binary into memory");
			}

			if (is_server != game::is_server())
			{
				throw std::runtime_error("Bad binary loaded into memory");
			}

			if (!is_server && !game::is_client())
			{
				if (game::is_legacy_client())
				{
					throw std::runtime_error(
						"You are using the outdated BlackOps3.exe. This version is not supported anymore. Please use the latest binary from Steam.");
				}

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
			game::show_error(e.what());
			return 1;
		}
	}

	g_call_tls_callbacks = true;
	return static_cast<int>(entry_point());
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}