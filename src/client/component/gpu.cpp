#include <std_include.hpp>

#include "exception.hpp"
#include "gpu.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <game/ui_scripting/execution.hpp>
#include <loader/component_loader.hpp>

#include <utils/string.hpp>

namespace gpu {
namespace {
constexpr auto preferences_key =
    L"Software\\Microsoft\\DirectX\\UserGpuPreferences";
constexpr std::wstring_view preference_field = L"GpuPreference=";

std::atomic<bool> warning_pending{false};

std::wstring exe_path() {
  wchar_t path[MAX_PATH * 4]{};
  DWORD size = static_cast<DWORD>(std::size(path));
  QueryFullProcessImageNameW(GetCurrentProcess(), 0, path, &size);
  return path;
}

std::wstring read_preferences() {
  wchar_t value[512]{};
  DWORD size = sizeof(value) - sizeof(wchar_t);
  if (RegGetValueW(HKEY_CURRENT_USER, preferences_key, exe_path().c_str(),
                   RRF_RT_REG_SZ, nullptr, value, &size) != ERROR_SUCCESS) {
    return {};
  }
  return value;
}

std::string adapter_name(const wchar_t *description) {
  return utils::string::convert(std::wstring(description));
}

bool same_adapter(const DXGI_ADAPTER_DESC &a, const DXGI_ADAPTER_DESC1 &b) {
  return a.AdapterLuid.LowPart == b.AdapterLuid.LowPart &&
         a.AdapterLuid.HighPart == b.AdapterLuid.HighPart;
}

bool on_main_menu() {
  game::lua::hks::lua_State *state = *game::lua::hks::primary_luaVM;
  if (!state || game::com::Com_IsInGame()) {
    return false;
  }

  try {
    const ui_scripting::table globals{state->globals.v.table};
    const ui_scripting::arguments screen =
        globals["Engine"]["GetLobbyUIScreen"]();
    return !screen.empty() && screen[0].as<int>() != 0;
  } catch (const std::exception &) {
    return false;
  }
}

std::optional<DXGI_ADAPTER_DESC1> fastest_adapter() {
  IDXGIFactory1 *factory = nullptr;
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
    return {};
  }

  std::optional<DXGI_ADAPTER_DESC1> result;
  IDXGIAdapter1 *adapter = nullptr;
  for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, &adapter)); ++i) {
    DXGI_ADAPTER_DESC1 desc{};
    if (SUCCEEDED(adapter->GetDesc1(&desc)) &&
        !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) &&
        (!result || desc.DedicatedVideoMemory > result->DedicatedVideoMemory)) {
      result = desc;
    }
    adapter->Release();
  }

  factory->Release();
  return result;
}
} // namespace

int get_preference() {
  const std::wstring preferences = read_preferences();
  const size_t pos = preferences.find(preference_field);
  if (pos == std::wstring::npos) {
    return 0;
  }
  return _wtoi(preferences.c_str() + pos + preference_field.size());
}

void set_preference(const int preference) {
  if (preference == get_preference()) {
    return;
  }

  std::wstring preferences = read_preferences();
  const std::wstring field =
      std::format(L"{}{};", preference_field, preference);

  const size_t pos = preferences.find(preference_field);
  if (pos == std::wstring::npos) {
    preferences += field;
  } else {
    const size_t end = preferences.find(L';', pos);
    preferences.replace(
        pos, end == std::wstring::npos ? std::wstring::npos : end - pos + 1,
        field);
  }

  RegSetKeyValueW(
      HKEY_CURRENT_USER, preferences_key, exe_path().c_str(), REG_SZ,
      preferences.c_str(),
      static_cast<DWORD>((preferences.size() + 1) * sizeof(wchar_t)));

  if (MessageBoxA(nullptr,
                  "The game needs to restart to switch graphics cards.\n\n"
                  "Restart now?",
                  "Graphics Card",
                  MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST |
                      MB_SETFOREGROUND) == IDOK) {
    exception::restart_game();
  }
}

void check_device(ID3D11Device *device) {
  IDXGIDevice *dxgi_device = nullptr;
  if (FAILED(device->QueryInterface(IID_PPV_ARGS(&dxgi_device)))) {
    return;
  }

  IDXGIAdapter *adapter = nullptr;
  DXGI_ADAPTER_DESC current{};
  const bool found = SUCCEEDED(dxgi_device->GetAdapter(&adapter)) &&
                     SUCCEEDED(adapter->GetDesc(&current));
  if (adapter) {
    adapter->Release();
  }
  dxgi_device->Release();
  if (!found) {
    return;
  }

  const std::optional<DXGI_ADAPTER_DESC1> fastest = fastest_adapter();
  std::string message;
  if (current.VendorId == 0x1414 && current.DeviceId == 0x8C) {
    message = "[GPU] Heads up: your CPU is drawing the game instead of a "
              "graphics card, that's why it feels slow.\n";
  } else if (fastest && !same_adapter(current, *fastest)) {
    message = std::format("[GPU] Heads up: the game is running on {} instead "
                          "of your faster {}, expect lag.\n",
                          adapter_name(current.Description),
                          adapter_name(fastest->Description));
  } else {
    return;
  }

  message += "[GPU] Fix: Settings > Graphics > Graphics Card > High "
             "Performance. Still slow? Update your graphics drivers.\n";
  printf("%s", message.c_str());
  warning_pending = true;
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("gpu");

  void post_unpack() override {
    scheduler::loop(
        [] {
          if (warning_pending && on_main_menu()) {
            warning_pending = false;
            toast::warn("Not running on your GPU",
                        "Set Graphics Card to High Performance in Graphics "
                        "settings.");
          }
        },
        scheduler::main, 1s);
  }
};
} // namespace gpu

REGISTER_COMPONENT(gpu::component)
