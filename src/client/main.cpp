#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include <utils/finally.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>
#include <utils/io.hpp>

#include <steam/steam.hpp>

namespace
{
	std::pair<void**, void*> g_original_import{};

	DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
	{
		component_loader::pre_destroy();
		exit(code);
	}

	std::pair<void**, void*> patch_steam_import(const std::string& func, void* function)
	{
		static const utils::nt::library game{};

		const auto game_entry = game.get_iat_entry("steam_api64.dll", func);
		if (!game_entry)
		{
			throw std::runtime_error("Import '" + func + "' not found!");
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

		utils::hook::set(utils::nt::library{}.get_iat_entry("kernel32.dll", "ExitProcess"), exit_hook);
	}

	bool run()
	{
		srand(uint32_t(time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

		{
			auto premature_shutdown = true;
			const auto _ = utils::finally([&premature_shutdown]()
			{
				if (premature_shutdown)
				{
					component_loader::pre_destroy();
				}
			});

			try
			{
				patch_imports();

				if (!component_loader::pre_start())
				{
					return false;
				}

				premature_shutdown = false;
			}
			catch (std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
				return false;
			}
		}

		return true;
	}

	class patch
	{
	public:
		patch() = default;

		patch(void* source, void* target)
			: source_(source)
		{
			memcpy(this->data_, source, sizeof(this->data_));
			utils::hook::jump(this->source_, target, true, true);
		}

		~patch()
		{
			if (source_)
			{
				utils::hook::copy(this->source_, this->data_, sizeof(this->data_));
			}
		}

		patch(patch&& obj) noexcept
			: patch()
		{
			this->operator=(std::move(obj));
		}

		patch& operator=(patch&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~patch();

				this->source_ = obj.source_;
				memcpy(this->data_, obj.data_, sizeof(this->data_));

				obj.source_ = nullptr;
			}

			return *this;
		}

	private:
		void* source_{nullptr};
		uint8_t data_[15]{};
	};

	std::vector<patch> initialization_hooks{};

	uint8_t* get_entry_point()
	{
		const utils::nt::library game{};
		return game.get_ptr() + game.get_optional_header()->AddressOfEntryPoint;
	}

	std::vector<uint8_t*> get_tls_callbacks()
	{
		const utils::nt::library game{};
		const auto& entry = game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
		if (!entry.VirtualAddress || !entry.Size)
		{
			return {};
		}

		const auto* tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(game.get_ptr() + entry.VirtualAddress);
		auto* callback = reinterpret_cast<uint8_t**>(tls_dir->AddressOfCallBacks);

		std::vector<uint8_t*> addresses{};
		while (callback && *callback)
		{
			addresses.emplace_back(*callback);
			++callback;
		}

		return addresses;
	}

	int patch_main()
	{
		if (!run())
		{
			return 1;
		}

		initialization_hooks.clear();
		return reinterpret_cast<int(*)()>(get_entry_point())();
	}

	void nullsub()
	{
	}

	void patch_entry_point()
	{
		initialization_hooks.emplace_back(get_entry_point(), patch_main);

		for (auto* tls_callback : get_tls_callbacks())
		{
			initialization_hooks.emplace_back(tls_callback, nullsub);
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE, const DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		patch_entry_point();
	}

	return TRUE;
}

extern "C" __declspec(dllexport)
HRESULT D3D11CreateDevice(void* adapter, const uint64_t driver_type,
                          const HMODULE software, const UINT flags,
                          const void* p_feature_levels, const UINT feature_levels,
                          const UINT sdk_version, void** device, void* feature_level,
                          void** immediate_context)
{
	static auto func = []
	{
		char dir[MAX_PATH]{0};
		GetSystemDirectoryA(dir, sizeof(dir));

		const auto d3d11 = utils::nt::library::load(dir + "/d3d11.dll"s);
		return d3d11.get_proc<decltype(&D3D11CreateDevice)>("D3D11CreateDevice");
	}();

	return func(adapter, driver_type, software, flags, p_feature_levels, feature_levels, sdk_version, device,
	            feature_level, immediate_context);
}
