#include <std_include.hpp>
#include <curl/curl.h>

#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include "../common/utils/finally.hpp"
#include "../common/utils/hook.hpp"
#include "../common/utils/nt.hpp"
#include "../common/utils/io.hpp"
#include "../common/utils/http.hpp"
#include "../common/utils/flags.hpp"
#include "../common/utils/com.hpp"
#include "../common/utils/progress_ui.hpp"

#include <steam/steam.hpp>

#include "game/game.hpp"
#include "launcher/launcher.hpp"
#include "launcher/html/html_window.hpp"
#include "component/updater.hpp"

#include <shlobj.h>
#include <tlhelp32.h>

namespace {
volatile bool g_call_tls_callbacks = false;
std::pair<void **, void *> g_original_import{};

DECLSPEC_NORETURN void WINAPI exit_hook(const uint32_t code) {
  component_loader::pre_destroy();
  ExitProcess(code);
}

std::pair<void **, void *> patch_steam_import(const std::string &func,
                                              void *function) {
  static const utils::nt::library game{};

  void **game_entry = game.get_iat_entry("steam_api64.dll", func);
  if (!game_entry) {
    // throw std::runtime_error("Import '" + func + "' not found!");
    return {nullptr, nullptr};
  }

  void *original_import = reinterpret_cast<void *>(game_entry);
  utils::hook::set(game_entry, function);
  return {game_entry, original_import};
}

bool restart_app_if_necessary_stub() {
  const std::string steam_path = steam::SteamAPI_GetSteamInstallPath();
  if (steam_path.empty() ||
      !utils::io::file_exists(steam_path + "/steam.exe")) {
    game::show_error(
        "Steam must be installed for the game to run. Please install Steam!");
    ShellExecuteA(nullptr, "open", "https://store.steampowered.com/about/",
                  nullptr, nullptr, SW_SHOWNORMAL);
    TerminateProcess(GetCurrentProcess(), 1);
  }

  utils::hook::set(g_original_import.first, g_original_import.second);
  patch_steam_import("SteamAPI_Shutdown", steam::SteamAPI_Shutdown);

  component_loader::post_unpack();
  return steam::SteamAPI_RestartAppIfNecessary();
}

BOOL set_process_dpi_aware_stub() {
  component_loader::post_unpack();
  return SetProcessDPIAware();
}

void patch_imports() {
  patch_steam_import("SteamAPI_RegisterCallback",
                     steam::SteamAPI_RegisterCallback);
  patch_steam_import("SteamAPI_RegisterCallResult",
                     steam::SteamAPI_RegisterCallResult);
  patch_steam_import("SteamGameServer_Shutdown",
                     steam::SteamGameServer_Shutdown);
  patch_steam_import("SteamGameServer_RunCallbacks",
                     steam::SteamGameServer_RunCallbacks);
  patch_steam_import("SteamGameServer_GetHSteamPipe",
                     steam::SteamGameServer_GetHSteamPipe);
  patch_steam_import("SteamGameServer_GetHSteamUser",
                     steam::SteamGameServer_GetHSteamUser);
  patch_steam_import("SteamInternal_GameServer_Init",
                     steam::SteamInternal_GameServer_Init);
  patch_steam_import("SteamAPI_UnregisterCallResult",
                     steam::SteamAPI_UnregisterCallResult);
  patch_steam_import("SteamAPI_UnregisterCallback",
                     steam::SteamAPI_UnregisterCallback);
  patch_steam_import("SteamAPI_RunCallbacks", steam::SteamAPI_RunCallbacks);
  patch_steam_import("SteamInternal_CreateInterface",
                     steam::SteamInternal_CreateInterface);
  patch_steam_import("SteamAPI_GetHSteamUser", steam::SteamAPI_GetHSteamUser);
  patch_steam_import("SteamAPI_GetHSteamPipe", steam::SteamAPI_GetHSteamPipe);
  patch_steam_import("SteamAPI_Init", steam::SteamAPI_Init);
  // patch_steam_import("SteamAPI_Shutdown", steam::SteamAPI_Shutdown);
  g_original_import = patch_steam_import("SteamAPI_RestartAppIfNecessary",
                                         restart_app_if_necessary_stub);

  const utils::nt::library game{};
  utils::hook::set(game.get_iat_entry("kernel32.dll", "ExitProcess"),
                   exit_hook);
  utils::hook::set(game.get_iat_entry("user32.dll", "SetProcessDPIAware"),
                   set_process_dpi_aware_stub);
}

void remove_crash_file() {
  const utils::nt::library game{};
  const auto game_file = game.get_path();
  auto game_path = std::filesystem::path(game_file);
  game_path.replace_extension(".start");

  utils::io::remove_file(game_path);
}

struct patch_install_cancelled {};

constexpr uint32_t supported_client_checksum = 0x888C368;
constexpr uint32_t supported_newsteamclient_checksum = 0x6517980;
constexpr uint32_t legacy_client_checksum = 0x8880704;

constexpr const char *supported_client_patch_url =
    "https://archive.org/download/t7_full_game/BlackOps3.exe";

enum class client_binary_state {
  supported,
  legacy,
  incompatible,
  unreadable,
};

uint32_t get_expected_client_checksum() {
  return utils::flags::has_flag("newsteamclient")
             ? supported_newsteamclient_checksum
             : supported_client_checksum;
}

std::optional<uint32_t> get_pe_checksum(const std::filesystem::path &file) {
  std::ifstream stream(file, std::ios::binary);
  if (!stream.is_open()) {
    return std::nullopt;
  }

  IMAGE_DOS_HEADER dos_header{};
  stream.read(reinterpret_cast<char *>(&dos_header), sizeof(dos_header));
  if (!stream || dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
    return std::nullopt;
  }

  stream.seekg(dos_header.e_lfanew, std::ios::beg);

  unsigned long signature = 0;
  stream.read(reinterpret_cast<char *>(&signature), sizeof(signature));
  if (!stream || signature != IMAGE_NT_SIGNATURE) {
    return std::nullopt;
  }

  IMAGE_FILE_HEADER file_header{};
  stream.read(reinterpret_cast<char *>(&file_header), sizeof(file_header));
  if (!stream) {
    return std::nullopt;
  }

  WORD optional_magic = 0;
  stream.read(reinterpret_cast<char *>(&optional_magic),
              sizeof(optional_magic));
  if (!stream) {
    return std::nullopt;
  }

  stream.seekg(-static_cast<std::streamoff>(sizeof(optional_magic)),
               std::ios::cur);

  if (optional_magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    IMAGE_OPTIONAL_HEADER64 optional_header{};
    stream.read(reinterpret_cast<char *>(&optional_header),
                sizeof(optional_header));
    if (!stream) {
      return std::nullopt;
    }

    return optional_header.CheckSum;
  }

  if (optional_magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    IMAGE_OPTIONAL_HEADER32 optional_header{};
    stream.read(reinterpret_cast<char *>(&optional_header),
                sizeof(optional_header));
    if (!stream) {
      return std::nullopt;
    }

    return optional_header.CheckSum;
  }

  return std::nullopt;
}

client_binary_state
classify_client_binary(const std::filesystem::path &client_binary) {
  const std::optional<uint32_t> checksum = get_pe_checksum(client_binary);
  if (!checksum) {
    return client_binary_state::unreadable;
  }

  if (*checksum == get_expected_client_checksum()) {
    return client_binary_state::supported;
  }

  if (*checksum == legacy_client_checksum) {
    return client_binary_state::legacy;
  }

  return client_binary_state::incompatible;
}

std::vector<unsigned long> get_running_client_binary_process_ids() {
  std::vector<unsigned long> pids{};
  const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return pids;
  }

  auto _ = utils::finally([&]() { CloseHandle(snapshot); });

  PROCESSENTRY32W process_entry{};
  process_entry.dwSize = sizeof(process_entry);
  const auto self_pid = GetCurrentProcessId();

  if (!Process32FirstW(snapshot, &process_entry)) {
    return pids;
  }

  do {
    if (process_entry.th32ProcessID == self_pid) {
      continue;
    }

    if (_wcsicmp(process_entry.szExeFile, L"BlackOps3.exe") == 0) {
      pids.emplace_back(process_entry.th32ProcessID);
    }
  } while (Process32NextW(snapshot, &process_entry));

  return pids;
}

bool is_client_binary_process_running() {
  return !get_running_client_binary_process_ids().empty();
}

void close_running_client_binary_processes() {
  const std::vector<unsigned long> pids =
      get_running_client_binary_process_ids();
  for (const auto pid : pids) {
    const auto process =
        OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (!process) {
      throw std::runtime_error(
          "Failed to access the running BlackOps3.exe process before "
          "installing the patch.");
    }

    auto _ = utils::finally([&]() { CloseHandle(process); });

    if (!TerminateProcess(process, 0)) {
      const auto error = GetLastError();
      throw std::runtime_error(
          std::string("Failed to close the running BlackOps3.exe process "
                      "before installing the patch (error ") +
          std::to_string(error) + ").");
    }

    const auto wait_result = WaitForSingleObject(process, 15000);
    if (wait_result != WAIT_OBJECT_0) {
      throw std::runtime_error(
          "BlackOps3.exe did not close in time for the patch install.");
    }
  }
}

std::string
get_manual_client_patch_message(const client_binary_state /*state*/) {
  return "Incompatible game version detected:\n"
         "The installed BlackOps3.exe version is not compatible with this "
         "BOIII build.\n"
         "This usually happens because Black Ops 3 was updated and BOIII "
         "needs the older compatible BlackOps3.exe version.\n"
         "To fix this, download the compatible BlackOps3.exe and replace the "
         "one in your game directory.\n"
         "You can find a download link under Settings => Patches.";
}

std::string get_client_patch_prompt_message(const client_binary_state /*state*/,
                                            const bool close_running_game) {
  const auto close_message =
      close_running_game
          ? "\n\nBlack Ops 3 is already running. BOIII will close it when "
            "the patch is ready, then continue launch."
          : "";

  return std::string(
             "The installed BlackOps3.exe version is not compatible with "
             "this BOIII build.\n\n"
             "This usually means Black Ops 3 was updated and BOIII needs "
             "the older compatible BlackOps3.exe version.\n\n"
             "BOIII can download and install that compatible version "
             "automatically before launch.") +
         close_message + "\n\nPress OK to continue or Cancel to stop.";
}

bool prompt_to_install_client_patch(const client_binary_state state,
                                    const bool close_running_game) {
  if (game::is_headless()) {
    return false;
  }

  const auto result = MessageBoxA(
      nullptr,
      get_client_patch_prompt_message(state, close_running_game).c_str(),
      "BOIII Patch Installer",
      MB_OKCANCEL | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST);

  return result == IDOK;
}

std::string format_download_size(const size_t bytes) {
  const double kilobytes = static_cast<double>(bytes) / 1024.0;
  const double megabytes = kilobytes / 1024.0;
  const double gigabytes = megabytes / 1024.0;

  char buffer[64] = {};
  if (gigabytes >= 1.0) {
    snprintf(buffer, sizeof(buffer), "%.2f GB", gigabytes);
  } else if (megabytes >= 1.0) {
    snprintf(buffer, sizeof(buffer), "%.2f MB", megabytes);
  } else if (kilobytes >= 1.0) {
    snprintf(buffer, sizeof(buffer), "%.2f KB", kilobytes);
  } else {
    snprintf(buffer, sizeof(buffer), "%zu B", bytes);
  }

  return buffer;
}

void install_supported_client_binary(
    const std::filesystem::path &client_binary,
    const bool allow_close_running_client_binary) {
  utils::progress_ui progress(false);
  progress.set_title("BOIII Patch Installer");
  progress.set_line(1, "Downloading compatible BlackOps3.exe...");
  progress.set_line(2, "Preparing download...");
  progress.show(true);

  const auto temp_binary =
      std::filesystem::path(client_binary.string() + ".boiii_download");
  const auto backup_binary =
      std::filesystem::path(client_binary.string() + ".boiii_backup");

  auto cleanup_temp =
      utils::finally([&]() { utils::io::remove_file(temp_binary); });

  std::ofstream temp_stream(temp_binary, std::ios::binary | std::ios::trunc);
  if (!temp_stream.is_open()) {
    throw std::runtime_error("Failed to create the temporary patch file in "
                             "your game directory.");
  }

  auto last_progress_update = std::chrono::steady_clock::time_point{};
  bool has_total_size = false;
  size_t latest_total_size = 0;
  const auto curl_code = utils::http::get_data_stream(
      supported_client_patch_url, {},
      [&](const size_t downloaded, const size_t total_size) {
        if (progress.is_cancelled()) {
          throw patch_install_cancelled{};
        }

        latest_total_size = total_size;

        const auto now = std::chrono::steady_clock::now();
        if (last_progress_update == std::chrono::steady_clock::time_point{} ||
            (now - last_progress_update) >= 125ms) {
          if (total_size > 0) {
            if (!has_total_size) {
              progress.set_marquee(false);
              has_total_size = true;
            }

            progress.set_progress(downloaded, total_size);
            progress.set_line(2, "Downloaded " +
                                     format_download_size(downloaded) + " / " +
                                     format_download_size(total_size));
          } else {
            progress.set_line(2,
                              "Downloaded " + format_download_size(downloaded));
          }

          last_progress_update = now;
        }
      },
      [&](const char *data, const size_t size) {
        temp_stream.write(data, static_cast<std::streamsize>(size));
        if (!temp_stream) {
          throw std::runtime_error(
              "Failed while writing the downloaded BlackOps3.exe patch.");
        }
      });

  temp_stream.flush();
  temp_stream.close();

  if (progress.is_cancelled()) {
    throw patch_install_cancelled{};
  }

  if (curl_code != CURLE_OK) {
    throw std::runtime_error("Failed to download the compatible BlackOps3.exe "
                             "patch. Please try again later.");
  }

  progress.set_marquee(false);
  progress.set_progress(latest_total_size > 0 ? latest_total_size : 1,
                        latest_total_size > 0 ? latest_total_size : 1);
  progress.set_line(1, "Verifying downloaded BlackOps3.exe...");
  progress.set_line(2, temp_binary.filename().string());

  const auto downloaded_checksum = get_pe_checksum(temp_binary);
  if (!downloaded_checksum ||
      *downloaded_checksum != get_expected_client_checksum()) {
    throw std::runtime_error(
        "The downloaded BlackOps3.exe patch did not match the BOIII-"
        "compatible version that this build expects.");
  }

  if (is_client_binary_process_running()) {
    if (!allow_close_running_client_binary) {
      throw std::runtime_error(
          "Black Ops 3 is still running, so BOIII cannot replace "
          "BlackOps3.exe.");
    }

    progress.set_line(1, "Closing running BlackOps3.exe...");
    progress.set_line(2, "Waiting for Black Ops 3 to exit...");
    close_running_client_binary_processes();
  }

  utils::io::remove_file(backup_binary);
  const bool moved_original = MoveFileExW(
      client_binary.wstring().c_str(), backup_binary.wstring().c_str(),
      MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
  if (!moved_original) {
    throw std::runtime_error("Failed to move your current BlackOps3.exe out of "
                             "the way before installing the patch.");
  }

  bool installed = false;
  const auto restore_backup = utils::finally([&]() {
    if (!installed) {
      MoveFileExW(backup_binary.wstring().c_str(),
                  client_binary.wstring().c_str(),
                  MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    }
  });

  progress.set_line(1, "Replacing BlackOps3.exe...");
  progress.set_line(2, client_binary.string());

  if (!MoveFileExW(temp_binary.wstring().c_str(),
                   client_binary.wstring().c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    throw std::runtime_error(
        "Failed to replace BlackOps3.exe with the downloaded patch.");
  }

  const auto installed_state = classify_client_binary(client_binary);
  if (installed_state != client_binary_state::supported) {
    throw std::runtime_error("BlackOps3.exe was replaced, but the new file is "
                             "still not compatible with this BOIII build.");
  }

  installed = true;
  utils::io::remove_file(backup_binary);

  progress.set_line(1, "BlackOps3.exe updated successfully.");
  progress.set_line(2, "Continuing launch...");
  std::this_thread::sleep_for(350ms);
}

void ensure_compatible_client_binary(
    const std::filesystem::path &client_binary) {
  const auto state = classify_client_binary(client_binary);
  if (state == client_binary_state::supported ||
      state == client_binary_state::unreadable) {
    return;
  }

  const auto close_running_game = is_client_binary_process_running();
  if (!prompt_to_install_client_patch(state, close_running_game)) {
    throw patch_install_cancelled{};
  }

  install_supported_client_binary(client_binary, close_running_game);
}

PIMAGE_TLS_CALLBACK *get_tls_callbacks() {
  const utils::nt::library game{};
  const auto &entry =
      game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
  if (!entry.VirtualAddress || !entry.Size) {
    return nullptr;
  }

  const auto *tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY *>(
      game.get_ptr() + entry.VirtualAddress);
  return reinterpret_cast<PIMAGE_TLS_CALLBACK *>(tls_dir->AddressOfCallBacks);
}

void run_tls_callbacks(const unsigned long reason) {
  if (!g_call_tls_callbacks) {
    return;
  }

  auto *callback = get_tls_callbacks();
  while (callback && *callback) {
    (*callback)(GetModuleHandleA(nullptr), reason, nullptr);
    ++callback;
  }
}

[[maybe_unused]] thread_local struct tls_runner {
  tls_runner() { run_tls_callbacks(DLL_THREAD_ATTACH); }

  ~tls_runner() { run_tls_callbacks(DLL_THREAD_DETACH); }
} tls_runner;

FARPROC load_process(const std::string &procname) {
  const auto proc = loader::load_binary(procname);

  auto *const peb = reinterpret_cast<PPEB>(__readgsqword(0x60));
  peb->Reserved3[1] = proc.get_ptr();
  static_assert(offsetof(PEB, Reserved3[1]) == 0x10);

  return FARPROC(proc.get_ptr() + proc.get_relative_entry_point());
}

bool handle_process_runner() {
  const char *const command = "-proc ";
  const char *parent_proc = strstr(GetCommandLineA(), command);

  if (!parent_proc) {
    return false;
  }

  const unsigned long pid =
      static_cast<unsigned long>(atoi(parent_proc + strlen(command)));
  const utils::nt::handle<> process_handle =
      OpenProcess(SYNCHRONIZE, FALSE, pid);
  if (process_handle) {
    WaitForSingleObject(process_handle, INFINITE);
  }

  return true;
}

void enable_dpi_awareness() {
  const utils::nt::library user32{"user32.dll"};

  {
    BOOL(WINAPI * set_dpi)(DPI_AWARENESS_CONTEXT) =
        user32 ? user32.get_proc<BOOL(WINAPI *)(DPI_AWARENESS_CONTEXT)>(
                     "SetProcessDpiAwarenessContext")
               : nullptr;
    if (set_dpi) {
      set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
      return;
    }
  }

  {
    const utils::nt::library shcore{"shcore.dll"};
    HRESULT(WINAPI * set_dpi)(PROCESS_DPI_AWARENESS) =
        shcore ? shcore.get_proc<HRESULT(WINAPI *)(PROCESS_DPI_AWARENESS)>(
                     "SetProcessDpiAwareness")
               : nullptr;
    if (set_dpi) {
      set_dpi(PROCESS_PER_MONITOR_DPI_AWARE);
      return;
    }
  }

  {
    BOOL(WINAPI * set_dpi)() =
        user32 ? user32.get_proc<BOOL(WINAPI *)()>("SetProcessDPIAware")
               : nullptr;
    if (set_dpi) {
      set_dpi();
    }
  }
}

void trigger_high_performance_gpu_switch() {
  // Make sure to link D3D11, as this might trigger high performance GPU
  [[maybe_unused]] static volatile auto _ = &D3D11CreateDevice;

  const utils::nt::registry_key key = utils::nt::open_or_create_registry_key(
      HKEY_CURRENT_USER, R"(Software\Microsoft\DirectX\UserGpuPreferences)");
  if (!key) {
    return;
  }

  const utils::nt::library self =
      utils::nt::library::get_by_address(&trigger_high_performance_gpu_switch);
  const std::wstring path = self.get_path().make_preferred().wstring();

  if (RegQueryValueExW(key, path.data(), nullptr, nullptr, nullptr, nullptr) !=
      ERROR_FILE_NOT_FOUND) {
    return;
  }

  const std::wstring data = L"GpuPreference=2;";
  RegSetValueExW(key, self.get_path().make_preferred().wstring().data(), 0,
                 REG_SZ, reinterpret_cast<const BYTE *>(data.data()),
                 static_cast<unsigned long>((data.size() + 1u) * 2));
}

void validate_non_network_share() {
  const utils::nt::library self =
      utils::nt::library::get_by_address(&validate_non_network_share);
  const std::filesystem::path path = self.get_path().make_preferred();
  const std::wstring wpath = path.wstring();
  if (wpath.size() >= 2 && wpath[0] == L'\\' && wpath[1] == L'\\') {
    throw std::runtime_error("You seem to be using a network share:\n\n" +
                             path.string() +
                             "\n\nNetwork shares are not supported!");
  }
}

std::filesystem::path get_game_path_file() {
  return game::get_appdata_path() / "user" / "game_path.txt";
}

bool is_valid_game_folder(const std::filesystem::path &folder) {
  return std::filesystem::exists(folder / "BlackOps3.exe") ||
         std::filesystem::exists(folder /
                                 "BlackOps3_UnrankedDedicatedServer.exe");
}

std::string find_steam_game_path() {
  const char *default_path =
      "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Call of Duty Black "
      "Ops III";
  if (is_valid_game_folder(default_path)) {
    return default_path;
  }

  const std::string steam_path = steam::SteamAPI_GetSteamInstallPath();
  if (steam_path.empty())
    return {};

  const std::filesystem::path steamapps = std::filesystem::path(steam_path) /
                                          "steamapps" / "common" /
                                          "Call of Duty Black Ops III";
  if (is_valid_game_folder(steamapps)) {
    return steamapps.string();
  }

  std::filesystem::path vdf_path =
      std::filesystem::path(steam_path) / "steamapps" / "libraryfolders.vdf";
  std::string vdf_data;
  if (utils::io::read_file(vdf_path.string(), &vdf_data)) {
    std::istringstream stream(vdf_data);
    std::string line;
    while (std::getline(stream, line)) {
      size_t pos = line.find("\"path\"");
      if (pos != std::string::npos) {

        size_t first_quote = line.find('"', pos + 6);
        size_t last_quote = line.rfind('"');
        if (first_quote != std::string::npos && last_quote > first_quote) {

          std::string lib_path =
              line.substr(first_quote + 1, last_quote - first_quote - 1);
          const std::filesystem::path game_folder =
              std::filesystem::path(lib_path) / "steamapps" / "common" /
              "Call of Duty Black Ops III";
          if (is_valid_game_folder(game_folder)) {
            return game_folder.string();
          }
        }
      }
    }
  }

  return {};
}

bool resolve_game_path() {
  const std::filesystem::path path_file = get_game_path_file();

  if (is_valid_game_folder(".")) {
    char cwd[MAX_PATH];
    if (GetCurrentDirectoryA(sizeof(cwd), cwd)) {
      std::error_code ec;
      std::filesystem::create_directories(path_file.parent_path(), ec);
      utils::io::write_file(path_file.string(), std::string(cwd));
    }
    return true;
  }

  {
    std::string stored_path;
    if (utils::io::read_file(path_file.string(), &stored_path) &&
        !stored_path.empty()) {
      while (!stored_path.empty() &&
             (stored_path.back() == '\n' || stored_path.back() == '\r' ||
              stored_path.back() == ' '))
        stored_path.pop_back();

      if (is_valid_game_folder(stored_path)) {
        SetCurrentDirectoryA(stored_path.c_str());
        return true;
      }
    }
  }

  {
    std::string steam_game = find_steam_game_path();
    if (!steam_game.empty()) {
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

  std::mutex setup_folder_mutex;
  std::string setup_folder_result;
  std::atomic<bool> setup_folder_busy{false};
  std::atomic<bool> setup_folder_done{false};

  setup_window.get_html_frame()->register_callback(
      "openFolderPicker",
      [&setup_folder_busy, &setup_folder_done, &setup_folder_mutex,
       &setup_folder_result, &path_set, &setup_window, &path_file](
          const std::vector<html_argument> & /*params*/) -> CComVariant {
        if (setup_folder_busy.exchange(true))
          return CComVariant("busy");
        setup_folder_done = false;
        {
          std::lock_guard lock(setup_folder_mutex);
          setup_folder_result.clear();
        }
        std::thread([&setup_folder_busy, &setup_folder_done,
                     &setup_folder_mutex, &setup_folder_result, &path_set,
                     &setup_window, &path_file]() {
          CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
          std::string selected;
          try {
            IFileOpenDialog *pfd = nullptr;
            HRESULT hr =
                CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                 CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
            if (SUCCEEDED(hr) && pfd) {
              unsigned long opts = 0;
              pfd->GetOptions(&opts);
              pfd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
              pfd->SetTitle(L"Select your Black Ops 3 installation folder");

              hr = pfd->Show(nullptr);
              if (SUCCEEDED(hr)) {
                IShellItem *psi = nullptr;
                if (SUCCEEDED(pfd->GetResult(&psi))) {
                  LPWSTR path_buf = nullptr;
                  if (SUCCEEDED(
                          psi->GetDisplayName(SIGDN_FILESYSPATH, &path_buf)) &&
                      path_buf) {
                    const std::wstring wp(path_buf);
                    const int len =
                        WideCharToMultiByte(CP_UTF8, 0, wp.c_str(), -1, nullptr,
                                            0, nullptr, nullptr);
                    if (len > 0) {
                      selected.resize(len - 1);
                      WideCharToMultiByte(CP_UTF8, 0, wp.c_str(), -1,
                                          &selected[0], len, nullptr, nullptr);
                    }
                    CoTaskMemFree(path_buf);
                  }
                  psi->Release();
                }
              }
              pfd->Release();
            }
          } catch (...) {
          }

          if (!selected.empty()) {
            if (!is_valid_game_folder(selected)) {
              std::lock_guard lock(setup_folder_mutex);
              setup_folder_result = "invalid";
            } else {
              std::error_code ec;
              std::filesystem::create_directories(path_file.parent_path(), ec);
              utils::io::write_file(path_file.string(), selected);
              SetCurrentDirectoryA(selected.c_str());
              path_set = true;
              {
                std::lock_guard lock(setup_folder_mutex);
                setup_folder_result = selected;
              }
              PostMessage(static_cast<HWND>(*(setup_window.get_window())),
                          WM_CLOSE, 0, 0);
            }
          } else {
            std::lock_guard lock(setup_folder_mutex);
            setup_folder_result = "cancelled";
          }
          CoUninitialize();
          setup_folder_done = true;
          setup_folder_busy = false;
        }).detach();
        return CComVariant("ok");
      });

  setup_window.get_html_frame()->register_callback(
      "getFolderPickerResult",
      [&setup_folder_done, &setup_folder_mutex, &setup_folder_result](
          const std::vector<html_argument> & /*params*/) -> CComVariant {
        if (!setup_folder_done.load())
          return CComVariant("pending");
        std::lock_guard lock(setup_folder_mutex);
        return CComVariant(setup_folder_result.c_str());
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
<button id="selectBtn" onclick="doSelect()">Set Game Path</button>
<div id="status"></div>
<script>
function doSelect() {
	var btn = document.getElementById('selectBtn');
	var r = window.external.openFolderPicker();
	if (r === 'busy') return;
	btn.disabled = true;
	document.getElementById('status').innerText = '';
	var poll = setInterval(function() {
		var result = window.external.getFolderPickerResult();
		if (result === 'pending') return;
		clearInterval(poll);
		btn.disabled = false;
		if (result === 'invalid') {
			document.getElementById('status').innerText = 'Selected folder does not contain BlackOps3.exe';
		} else if (result === 'cancelled') {
			document.getElementById('status').innerText = '';
		}
	}, 200);
}
</script>
</body>
</html>
)html");

  window::run();

  return path_set;
}
} // namespace

int parse_flags(int argc, char *argv[]) {
  if (handle_process_runner()) {
    return 0;
  }

  FARPROC entry_point{};
  srand(static_cast<uint32_t>(time(nullptr)) ^
        ~(GetTickCount() * GetCurrentProcessId()));

  if (utils::flags::parse_flags(argc, argv)) {
    return 1;
  }

  enable_dpi_awareness();

  {
    auto premature_shutdown = true;
    const auto _ = utils::finally([&premature_shutdown] {
      if (premature_shutdown) {
        component_loader::pre_destroy();
      }
    });

    try {
      validate_non_network_share();
      remove_crash_file();

      if (!resolve_game_path()) {
        return 0;
      }

      const std::string client_binary = "BlackOps3.exe"s;
      const std::string server_binary =
          "BlackOps3_UnrankedDedicatedServer.exe"s;

      bool has_client = utils::io::file_exists(client_binary);
      bool has_server = utils::io::file_exists(server_binary);

      while (!has_client && !has_server) {
        if (!resolve_game_path()) {
          return 0;
        }
        has_client = utils::io::file_exists(client_binary);
        has_server = utils::io::file_exists(server_binary);
      }

      const bool is_server =
          utils::flags::has_flag("dedicated") || (!has_client && has_server);

      if (!is_server && !launcher::is_game_process_running()) {
        updater::update();
      }

      if (!utils::io::file_exists(
              launcher::get_launcher_ui_file().generic_wstring())) {
        throw std::runtime_error("BOIII needs an active internet connection "
                                 "for the first time you launch it.");
      }

      if (!is_server) {
        trigger_high_performance_gpu_switch();

        const bool launch = utils::flags::has_flag("launch");
        if (!launch && !utils::nt::is_wine() && !launcher::run()) {
          return 0;
        }

        ensure_compatible_client_binary(client_binary);
      }

      if (!component_loader::activate(is_server)) {
        return 1;
      }

      entry_point = load_process(is_server ? server_binary : client_binary);
      if (!entry_point) {
        throw std::runtime_error("Unable to load binary into memory");
      }

      if (is_server != game::is_server()) {
        throw std::runtime_error(
            get_manual_client_patch_message(client_binary_state::incompatible));
      }

      if (!is_server && !game::is_client()) {
        throw std::runtime_error(get_manual_client_patch_message(
            game::is_legacy_client() ? client_binary_state::legacy
                                     : client_binary_state::incompatible));
      }

      patch_imports();

      if (!component_loader::post_load()) {
        return 1;
      }

      premature_shutdown = false;
    } catch (patch_install_cancelled &) {
      return 0;
    } catch (std::exception &e) {
      game::show_error(e.what());
      return 1;
    }
  }

  g_call_tls_callbacks = true;
  return static_cast<int>(entry_point());
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int) { return main(); }