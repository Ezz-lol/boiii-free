#include <std_include.hpp>

#include "download_overlay.hpp"
#include "scheduler.hpp"
#include "steamcmd.hpp"
#include "workshop.hpp"

#include <game/game.hpp>

#include <utils/compression.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>

namespace steamcmd {
namespace {
constexpr uint64_t MIN_UPDATED_EXE_SIZE = 3 * 1024 * 1024;

constexpr std::string_view LINUX_SCRIPT = R"sh(cd "$(dirname "$0")" || exit 1
[ -f steamcmd.sh ] || tar -xzf steamcmd_linux.tar.gz || { echo 127 > exit_code; exit 1; }
HOME="$PWD" ./steamcmd.sh "$@" > console.txt 2>&1 &
pid=$!
echo $pid > steamcmd.pid
(while [ ! -f stop ] && [ ! -f exit_code ]; do sleep 1; done
[ -f stop ] && kill $(cat /proc/$pid/task/$pid/children 2>/dev/null) $pid 2>/dev/null) &
watcher=$!
wait $pid
echo $? > exit_code
kill $watcher 2>/dev/null
)sh";

std::string read_text(const std::filesystem::path &path) {
  std::ifstream stream(path, std::ios::binary);
  std::stringstream text;
  text << stream.rdbuf();
  return text.str();
}

std::string unix_path(const std::filesystem::path &path) {
  using wine_get_unix_file_name_t = char *(__cdecl *)(const wchar_t *);
  static const auto convert =
      reinterpret_cast<wine_get_unix_file_name_t>(GetProcAddress(
          GetModuleHandleW(L"kernel32.dll"), "wine_get_unix_file_name"));
  char *const result = convert ? convert(path.wstring().c_str()) : nullptr;
  if (!result) {
    return {};
  }
  std::string value = result;
  HeapFree(GetProcessHeap(), 0, result);
  return value;
}

uint64_t size_on_disk(const std::filesystem::path &path) {
  std::error_code ec;
  const uint64_t size = std::filesystem::file_size(path, ec);
  return ec ? 0 : size;
}
} // namespace

process::process(std::filesystem::path dir, const bool wine)
    : dir_(std::move(dir)), wine_(wine) {}

process::~process() {
  if (pi_.hProcess)
    CloseHandle(pi_.hProcess);
  if (pi_.hThread)
    CloseHandle(pi_.hThread);
}

bool process::start(const std::string &args) {
  if (!wine_) {
    std::string command =
        "\"" + (dir_ / "steamcmd.exe").string() + "\" " + args;
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    return CreateProcessA(nullptr, command.data(), nullptr, nullptr, FALSE,
                          CREATE_NO_WINDOW, nullptr, dir_.string().c_str(), &si,
                          &pi_);
  }

  std::error_code ec;
  std::filesystem::remove(dir_ / "exit_code", ec);
  std::filesystem::remove(dir_ / "stop", ec);
  std::filesystem::remove(dir_ / "steamcmd.pid", ec);
  const std::string script = unix_path(dir_ / "run.sh");
  if (script.empty() ||
      !utils::io::write_file((dir_ / "run.sh").string(),
                             std::string(LINUX_SCRIPT), false)) {
    return false;
  }
  std::wstring command = L"/bin/sh \"" +
                         std::wstring(script.begin(), script.end()) + L"\" " +
                         std::wstring(args.begin(), args.end());
  STARTUPINFOW si{};
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi{};
  if (!CreateProcessW(L"Z:\\bin\\sh", command.data(), nullptr, nullptr, FALSE,
                      0, nullptr, nullptr, &si, &pi)) {
    return false;
  }
  if (pi.hProcess)
    CloseHandle(pi.hProcess);
  if (pi.hThread)
    CloseHandle(pi.hThread);
  return true;
}

bool process::wait(const std::chrono::milliseconds timeout) {
  if (!wine_) {
    return WaitForSingleObject(pi_.hProcess,
                               static_cast<DWORD>(timeout.count())) !=
           WAIT_TIMEOUT;
  }
  std::this_thread::sleep_for(timeout);
  std::error_code ec;
  return std::filesystem::exists(dir_ / "exit_code", ec);
}

void process::terminate() {
  if (!wine_) {
    TerminateProcess(pi_.hProcess, 1);
    return;
  }
  utils::io::write_file((dir_ / "stop").string(), "", false);
  for (int i = 0; i < 20 && !wait(std::chrono::milliseconds(500)); i++) {
  }
}

uint32_t process::exit_code() {
  if (!wine_) {
    DWORD code = 0;
    GetExitCodeProcess(pi_.hProcess, &code);
    return code;
  }
  return std::strtoul(read_text(dir_ / "exit_code").c_str(), nullptr, 10);
}

uint64_t process::transferred_bytes() {
  if (!wine_) {
    IO_COUNTERS counters{};
    GetProcessIoCounters(pi_.hProcess, &counters);
    return counters.OtherTransferCount;
  }
  const std::string pid = std::to_string(
      std::strtoul(read_text(dir_ / "steamcmd.pid").c_str(), nullptr, 10));
  std::istringstream children(
      read_text("Z:/proc/" + pid + "/task/" + pid + "/children"));
  uint64_t total = 0;
  std::string child;
  while (children >> child) {
    const std::string io = read_text("Z:/proc/" + child + "/io");
    const size_t written = io.find("wchar: ");
    if (written != std::string::npos) {
      total += std::strtoull(io.c_str() + written + 7, nullptr, 10);
    }
  }
  return total;
}

bool process::counts_disk_writes() const { return wine_; }

bool process::missing_libraries() const {
  return wine_ && read_text(dir_ / "console.txt")
                          .find("error while loading shared libraries") !=
                      std::string::npos;
}

progress::progress(const std::filesystem::path &dir) {
  for (const std::filesystem::path &log :
       {dir / "logs" / "content_log.txt",
        dir / "Steam" / "logs" / "content_log.txt"}) {
    logs_.emplace_back(log, size_on_disk(log));
  }
}

void progress::update(process &steamcmd) {
  for (auto &[log, offset] : logs_) {
    std::ifstream log_file(log, std::ios::binary);
    if (!log_file) {
      continue;
    }
    log_file.seekg(static_cast<std::streamoff>(offset));
    std::string line;
    while (std::getline(log_file, line) && !log_file.eof()) {
      offset += line.size() + 1;
      const size_t started = line.find("update started : download ");
      if (started != std::string::npos) {
        by_writes_ = steamcmd.counts_disk_writes();
        char *end = nullptr;
        const uint64_t download_resumed =
            std::strtoull(line.c_str() + started + 26, &end, 10);
        const uint64_t download_total = std::strtoull(end + 1, nullptr, 10);
        uint64_t install_resumed = download_resumed;
        install_total_ = download_total;
        const size_t stage = line.find("stage ");
        if (stage != std::string::npos) {
          install_resumed = std::strtoull(line.c_str() + stage + 6, &end, 10);
          install_total_ = std::strtoull(end + 1, nullptr, 10);
        }
        resumed_ = by_writes_ ? install_resumed : download_resumed;
        total_ = by_writes_ ? install_total_ : download_total;
        baseline_ = previous_ = steamcmd.transferred_bytes();
        last_sample_ = std::chrono::steady_clock::now();
        speed_ = 0.0;
      } else if (line.find("Committing") != std::string::npos) {
        committing_ = true;
      }
    }
  }

  if (!started()) {
    return;
  }
  const uint64_t now_bytes = steamcmd.transferred_bytes();
  const auto now = std::chrono::steady_clock::now();
  const double seconds =
      std::chrono::duration<double>(now - last_sample_).count();
  if (seconds > 0.0) {
    const double sample = static_cast<double>(now_bytes - previous_) / seconds;
    speed_ = speed_ <= 0.0 ? sample : 0.3 * sample + 0.7 * speed_;
  }
  previous_ = now_bytes;
  last_sample_ = now;
  received_ = std::min(resumed_ + now_bytes - baseline_, total_);
}

bool progress::started() const { return total_ > 0; }

bool progress::committing() const { return committing_; }

uint64_t progress::installed_bytes() const {
  return static_cast<uint64_t>(fraction() *
                               static_cast<double>(install_total_));
}

uint64_t progress::install_total() const { return install_total_; }

double progress::fraction() const {
  return total_ ? static_cast<double>(received_) / static_cast<double>(total_)
                : 0.0;
}

double progress::speed() const { return speed_; }

int64_t progress::eta_seconds() const {
  return speed_ > 1024.0 ? static_cast<int64_t>(
                               static_cast<double>(total_ - received_) / speed_)
                         : -1;
}

bool ensure_installed(const std::filesystem::path &dir, const bool wine,
                      const std::function<void(const std::string &)> &status,
                      std::string &error) {
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);

  if (wine) {
    if (std::filesystem::exists(dir / "steamcmd.sh", ec) ||
        std::filesystem::exists(dir / "steamcmd_linux.tar.gz", ec)) {
      return true;
    }
    status("Downloading SteamCMD...");
    const std::optional<std::string> tarball = utils::http::get_data(
        "https://steamcdn-a.akamaihd.net/client/installer/"
        "steamcmd_linux.tar.gz",
        {}, {}, 3);
    if (!tarball || tarball->empty() ||
        !utils::io::write_file((dir / "steamcmd_linux.tar.gz").string(),
                               *tarball, false)) {
      error = "Could not download SteamCMD. Check your internet connection.";
      return false;
    }
    return true;
  }

  const std::filesystem::path exe = dir / "steamcmd.exe";
  if (!std::filesystem::exists(exe, ec)) {
    status("Downloading SteamCMD...");
    const std::optional<std::string> zip = utils::http::get_data(
        "https://steamcdn-a.akamaihd.net/client/installer/steamcmd.zip", {}, {},
        3);
    if (!zip || zip->empty()) {
      error = "Could not download SteamCMD. Check your internet connection.";
      return false;
    }
    try {
      for (const auto &[name, data] : utils::compression::zip::extract(*zip)) {
        utils::io::write_file((dir / name).string(), data, false);
      }
    } catch (const std::exception &) {
    }
    if (!std::filesystem::exists(exe, ec)) {
      error = "SteamCMD could not be extracted.";
      return false;
    }
  }

  if (size_on_disk(exe) < MIN_UPDATED_EXE_SIZE) {
    status("Updating SteamCMD (first run)...");
    process updater(dir, false);
    if (updater.start("+quit") && !updater.wait(std::chrono::minutes(3))) {
      updater.terminate();
    }
    if (size_on_disk(exe) < MIN_UPDATED_EXE_SIZE) {
      error = "SteamCMD could not update itself. Check your internet "
              "connection and try again.";
      return false;
    }
  }
  return true;
}

void clear_downloads(const std::filesystem::path &dir) {
  std::error_code ec;
  std::filesystem::remove_all(dir / "steamapps", ec);
  std::filesystem::remove_all(dir / "Steam" / "steamapps", ec);
}

std::vector<std::filesystem::path>
workshop_roots(const std::filesystem::path &dir, const bool wine) {
  return {dir / "steamapps" / "workshop",
          (wine ? dir / "Steam" : game::get_game_path()) / "steamapps" /
              "workshop"};
}

namespace {
enum class download_result {
  success,
  cancelled,
  move_failed,
  setup_failed,
  failed,
  missing_libraries
};

void move_downloaded_folder(const std::filesystem::path &source,
                            const std::filesystem::path &destination) {
  std::filesystem::create_directories(destination);
  for (const std::filesystem::directory_entry &entry :
       std::filesystem::directory_iterator(source)) {
    std::filesystem::rename(entry.path(),
                            destination / entry.path().filename());
  }
  std::filesystem::remove(source);
}

download_result download_workshop_item(
    const std::string &workshop_id, const std::string &modtype,
    const std::function<void(const progress *, const std::string &)> &report) {
  const std::filesystem::path dir = game::get_game_path() / "steamcmd";
  const bool wine = utils::nt::is_wine();

  std::string error;
  if (!ensure_installed(
          dir, wine,
          [&](const std::string &status) { report(nullptr, status); }, error)) {
    printf("[ Workshop ] %s\n", error.c_str());
    return download_result::setup_failed;
  }

  std::vector<std::filesystem::path> content_folders;
  for (const std::filesystem::path &root : workshop_roots(dir, wine)) {
    content_folders.push_back(root / "content" / game::APP_ID_STR /
                              workshop_id);
  }
  const auto find_content = [&]() -> std::filesystem::path {
    std::error_code ec;
    for (const std::filesystem::path &folder : content_folders) {
      if (std::filesystem::exists(folder, ec)) {
        return folder;
      }
    }
    return {};
  };

  clear_downloads(dir);

  const int max_tries = workshop::get_workshop_retry_attempts();
  int tries = 0;
  int fast_fail_count = 0;
  constexpr int FAST_FAIL_THRESHOLD = 5;

  while (find_content().empty()) {
    if (!workshop::downloading_workshop_item.load()) {
      return download_result::cancelled;
    }
    if (tries >= max_tries) {
      if (!download_overlay::show_confirmation_blocking(
              "Retry Limit Reached",
              "Download has used all " + std::to_string(max_tries) +
                  " retry attempts without completing.\n\n"
                  "Do you want to continue downloading?\n"
                  "(Your progress will be preserved)")) {
        return download_result::failed;
      }
      tries = 0;
    }
    tries++;

    if (fast_fail_count >= FAST_FAIL_THRESHOLD) {
      std::error_code ec;
      for (const char *dir_name : {"steamapps", "dumps", "logs", "depotcache",
                                   "appcache", "userdata", "Steam"}) {
        std::filesystem::remove_all(dir / dir_name, ec);
      }
      fast_fail_count = 0;
    }

    printf("[ Workshop ] Downloading (attempt %d/%d)...\n", tries, max_tries);
    process steamcmd(dir, wine);
    if (!steamcmd.start("+login anonymous +workshop_download_item " +
                        std::string(game::APP_ID_STR) + " " + workshop_id +
                        " +quit")) {
      report(nullptr, "Could not start SteamCMD, retrying...");
      fast_fail_count++;
      std::this_thread::sleep_for(std::chrono::seconds(2));
      continue;
    }

    const auto attempt_start = std::chrono::steady_clock::now();
    progress tracker(dir);
    while (!steamcmd.wait(std::chrono::seconds(1))) {
      if (!workshop::downloading_workshop_item.load()) {
        steamcmd.terminate();
        return download_result::cancelled;
      }
      tracker.update(steamcmd);
      report(&tracker, tries > 1 ? "Attempt " + std::to_string(tries) : "");
    }

    if (steamcmd.missing_libraries()) {
      return download_result::missing_libraries;
    }
    if (std::chrono::steady_clock::now() - attempt_start <
            std::chrono::seconds(15) &&
        find_content().empty()) {
      fast_fail_count++;
    }
  }

  const std::filesystem::path destination =
      game::get_game_path() / (modtype == "Map" ? "usermaps" : "mods") /
      workshop_id;
  try {
    move_downloaded_folder(find_content(), destination);
  } catch (const std::filesystem::filesystem_error &ex) {
    printf("[ Workshop ] %s\n", ex.what());
    return download_result::move_failed;
  }
  return download_result::success;
}
} // namespace

void initialize_download(std::string workshop_id, std::string modtype) {
  if (workshop::launcher_downloading.load()) {
    scheduler::once(
        [] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI,
              "A download is already in progress from the launcher. Wait for "
              "it to finish.");
        },
        scheduler::main);
    return;
  }

  workshop::downloading_workshop_item = true;

  const auto ws_info = workshop::get_steam_workshop_info(workshop_id);
  const std::string workshop_title =
      ws_info.title.empty()
          ? ((modtype == "Map" ? "Map: " : "Mod: ") + workshop_id)
          : ws_info.title;

  const auto report = [&](const progress *tracker, const std::string &status) {
    download_overlay::download_state state;
    state.active = true;
    state.item_name = workshop_title;
    state.on_cancel = [] { workshop::downloading_workshop_item = false; };
    if (tracker && tracker->committing()) {
      state.status_line = "Finishing download...";
    } else if (tracker && tracker->started()) {
      state.downloaded_bytes = tracker->installed_bytes();
      state.total_bytes = tracker->install_total();
      state.speed_bps = static_cast<float>(tracker->speed());
      state.eta_seconds = static_cast<int>(tracker->eta_seconds());
      state.status_line =
          workshop::human_readable_size(tracker->installed_bytes()) + " / " +
          workshop::human_readable_size(tracker->install_total());
      if (!status.empty()) {
        state.status_line += " | " + status;
      }
    } else {
      state.status_line = status.empty() ? "Waiting for SteamCMD..." : status;
    }
    download_overlay::update(state);
  };
  report(nullptr, "Setting up SteamCMD...");

  const download_result result =
      download_workshop_item(workshop_id, modtype, report);

  if (result == download_result::cancelled) {
    clear_downloads(game::get_game_path() / "steamcmd");
  }

  const char *error_msg = nullptr;
  switch (result) {
  case download_result::failed:
    error_msg = "Problem downloading the workshop item. Max tries used.";
    break;
  case download_result::setup_failed:
    error_msg = "Cannot install SteamCMD. Please try again.";
    break;
  case download_result::move_failed:
    error_msg =
        "There was a problem moving the workshop item to the correct "
        "folder.\nYou can try moving it manually and joining the server again.";
    break;
  case download_result::cancelled:
    error_msg = "Download cancelled.";
    break;
  case download_result::missing_libraries:
    error_msg = "SteamCMD needs 32-bit libraries on Linux. Install "
                "lib32gcc-s1 (Debian/Ubuntu), glibc.i686 and libgcc.i686 "
                "(Fedora) or lib32-gcc-libs (Arch), then try again.";
    break;
  case download_result::success:
    break;
  }

  if (error_msg) {
    scheduler::once(
        [error_msg] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI, error_msg);
        },
        scheduler::main);
  } else {
    const std::string reconnect_addr =
        workshop::get_pending_download_reconnect();
    if (!reconnect_addr.empty()) {
      scheduler::once(
          [reconnect_addr] {
            download_overlay::show_confirmation(
                "Download Complete",
                "Workshop item downloaded successfully!\n\nDo you want to "
                "connect to the server?",
                [reconnect_addr] {
                  game::cbuf::Cbuf_AddText(
                      game::LOCAL_CLIENT_0,
                      utils::string::va("connect %s\n",
                                        reconnect_addr.c_str()));
                });
          },
          scheduler::main);
    } else {
      scheduler::once(
          [] {
            game::ui::UI_OpenErrorPopupWithMessage(
                game::LOCAL_CLIENT_0, game::errorCode::UI,
                "Workshop item downloaded successfully!");
          },
          scheduler::main);
    }
  }

  download_overlay::clear();
  game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "userContentReload\n");
  workshop::downloading_workshop_item = false;
}
} // namespace steamcmd
