#include <std_include.hpp>

#include "game.hpp"

#include <utils/flags.hpp>
#include <utils/finally.hpp>

#include <combaseapi.h>

namespace game {
bool quiet_crash() {
  static const bool quiet_crash = utils::flags::has_flag("quiet-crash");
  return quiet_crash;
}

#ifndef NDEBUG
bool alias() {
  static const bool alias = utils::flags::has_flag("alias");
  return alias;
}
#endif

bool is_headless() {
  static const bool headless = utils::flags::has_flag("headless");
  return headless;
}

void show_error(const std::string &text, const std::string &title) {
  if (quiet_crash()) {
    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "%s\n%s\n", title.c_str(), text.c_str());

    fflush(stderr);
  } else if (is_headless()) {
    puts(text.data());
  } else {
    MessageBoxA(nullptr, text.data(), title.data(),
                MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
  }
}

std::filesystem::path get_appdata_path() {
  static const std::filesystem::path appdata_path =
      []() -> std::filesystem::path {
    PWSTR path = nullptr;
    if (FAILED(
            SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
      throw std::runtime_error("Failed to read APPDATA path!");
    }

    auto _ = utils::finally([&path] { CoTaskMemFree(path); });

    // Ensure proper handling of wide character paths
    return std::filesystem::path(path) / L"boiii";
  }();

  return appdata_path;
}

std::filesystem::path get_game_path() {
  return std::filesystem::current_path();
}

#ifndef NDEBUG
#define UNIX_EPOCH                                                             \
  std::chrono::system_clock::time_point(std::chrono::seconds(0))
static std::chrono::system_clock::time_point last_log_time = UNIX_EPOCH;
static std::recursive_mutex log_mutex;
void printfdebug(const char *format, ...) {

  // Get current time for timestamp
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::chrono::milliseconds now_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch()) %
      1000;

  char time_buf[26];
  ctime_s(time_buf, sizeof(time_buf), &now_time_t);

  std::string now_str(time_buf);
  if (!now_str.empty() && now_str.back() == '\n') {
    now_str.pop_back(); // Remove trailing newline from ctime_s
  }

  va_list args;
  va_start(args, format);
  int32_t buf_len = vsnprintf(nullptr, 0, format, args);
  std::string buffer;
  buffer.resize(buf_len + 1);
  va_start(args, format);
  vsnprintf(buffer.data(), buffer.size(), format, args);

  va_end(args);

  // strip newline characters from buffer
  for (size_t i = 0; i < buffer.size() - 1; ++i) {
    if (buffer[i] == '\n' || buffer[i] == '\r') {
      buffer[i] = ' ';
    }
  }
  const char *buf_str = const_cast<const char *>(buffer.data());

  std::lock_guard<std::recursive_mutex> lock(log_mutex);
  std::ofstream debug_log = std::ofstream(
      game::is_client() ? "debug.log" : "debug-server.log", std::ios_base::app);
  if (!debug_log.is_open()) {
    return;
  }

  debug_log << "[" << now_str << "." << std::setfill('0') << std::setw(3)
            << now_ms.count() << "] [Debug] ";

  debug_log << buf_str << std::endl;
  debug_log.flush();
  debug_log.close();
  last_log_time = (std::max)(last_log_time, now);
}
#endif
} // namespace game