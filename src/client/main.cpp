#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include <utils/finally.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>

namespace
{
DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
{
	component_loader::pre_destroy();
	exit(code);
}

VOID WINAPI initialize_critical_section(const LPCRITICAL_SECTION lpCriticalSection)
{
  component_loader::post_unpack();
  InitializeCriticalSection(lpCriticalSection);
}

void patch_imports()
{
  const utils::nt::library game{};
  const auto self = utils::nt::library::get_by_address(patch_imports);

  auto patch_steam_import = [&](const std::string& func) {
    const auto game_entry = game.get_iat_entry("steam_api64.dll", func);
    if (!game_entry) {
      throw std::runtime_error("Import '" + func + "' not found!");
    }

    const auto self_proc = self.get_proc<void*>(func);
    if (!self_proc) {
      throw std::runtime_error(func + " export not found");
    }
    utils::hook::set(game_entry, self_proc);
  };

  patch_steam_import("SteamAPI_RegisterCallback");
  patch_steam_import("SteamAPI_RegisterCallResult");
  patch_steam_import("SteamGameServer_Shutdown");
  patch_steam_import("SteamGameServer_RunCallbacks");
  patch_steam_import("SteamGameServer_GetHSteamPipe");
  patch_steam_import("SteamGameServer_GetHSteamUser");
  patch_steam_import("SteamInternal_GameServer_Init");
  patch_steam_import("SteamAPI_UnregisterCallResult");
  patch_steam_import("SteamAPI_UnregisterCallback");
  patch_steam_import("SteamAPI_RunCallbacks");
  //patch_steam_import("SteamAPI_Shutdown");
  patch_steam_import("SteamInternal_CreateInterface");
  patch_steam_import("SteamAPI_GetHSteamUser");
  patch_steam_import("SteamAPI_GetHSteamPipe");
  patch_steam_import("SteamAPI_Init");
  patch_steam_import("SteamAPI_RestartAppIfNecessary");
  
  utils::hook::set(game.get_iat_entry("kernel32.dll", "InitializeCriticalSection"), initialize_critical_section);
  utils::hook::set(game.get_iat_entry("kernel32.dll", "ExitProcess"), exit_hook);
}

bool run()
{
	srand(uint32_t(time(nullptr)) ^ (~GetTickCount()));

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

			if (!component_loader::post_load())
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
}

BOOL WINAPI DllMain(HINSTANCE, const DWORD reason, LPVOID)
{
  if (reason == DLL_PROCESS_ATTACH) {
    if(!run()) {
      return FALSE;
    }
  }
  return TRUE;
}

extern "C" __declspec(dllexport)
HRESULT D3D11CreateDevice(
  void* pAdapter,
  uint64_t DriverType,
  HMODULE Software,
  UINT Flags,
  const void* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  void** ppDevice,
  void* pFeatureLevel,
  void** ppImmediateContext
)
{
  static auto func = [] {
    char dir[MAX_PATH]{ 0 };
    GetSystemDirectoryA(dir, sizeof(dir));

    const auto d3d11 = utils::nt::library::load(dir + "/d3d11.dll"s);
    return d3d11.get_proc<decltype(&D3D11CreateDevice)>("D3D11CreateDevice");
  }();

  return func(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
