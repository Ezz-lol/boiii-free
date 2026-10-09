#include <std_include.hpp>

#include "steamcmd.hpp"

#include <game/game.hpp>

#include <utils/compression.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/nt.hpp>

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

} // namespace steamcmd
