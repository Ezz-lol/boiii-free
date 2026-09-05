#ifndef GAME_HPP
#define GAME_HPP

#include <filesystem>
#include <string>
#include <cstdint>

#include <utils/nt.hpp>   // IWYU pragma: export
#include <game/base.hpp>  // IWYU pragma: export
#include <game/flags.hpp> // IWYU pragma: export
#include <game/ptr.hpp>   // IWYU pragma: export

namespace arxan::detail {
void set_address_to_call(const void *address);
extern void *callstack_proxy_addr;
} // namespace arxan::detail

namespace game {

constexpr uint32_t APP_ID = 311210;
constexpr const char *APP_ID_STR = "311210";

bool alias();
bool quiet_crash();
bool is_headless();

void show_error(const std::string &text, const std::string &title = "Error");

std::filesystem::path get_appdata_path();
std::filesystem::path get_game_path();
std::vector<std::string> get_registered_dvar_names();
size_t get_registered_dvar_name_count();

#ifdef NDEBUG
template <typename... Args>
inline void trace([[maybe_unused]] const std::string_view &fmt,
                  [[maybe_unused]] Args &&...args) {}
#else
extern std::recursive_mutex log_mutex;

template <typename... Args>
void trace(const std::string_view &fmt, Args &&...args) {
  // Get current time for timestamp
  const std::chrono::time_point now = std::chrono::system_clock::now();
  const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  const std::chrono::milliseconds now_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch()) %
      1000;

  char time_buf[26];
  ctime_s(time_buf, sizeof(time_buf), &now_time_t);

  std::string now_str(time_buf);
  if (!now_str.empty() && now_str.back() == '\n') {
    now_str.pop_back(); // Remove trailing newline from ctime_s
  }

  const int32_t buf_len = std::snprintf(nullptr, 0, fmt.data(), args...);

  std::string buffer;
  if (buf_len > 0) {
    buffer.resize(buf_len);
    std::snprintf(buffer.data(), buf_len + 1, fmt.data(), args...);
  }

  std::scoped_lock<std::recursive_mutex> lock(log_mutex);
  std::ofstream &debug_log = game::tracing_logfile();
  if (debug_log.is_open()) {
    debug_log << "[" << now_str << "." << std::setfill('0') << std::setw(3)
              << now_ms.count() << "] [Debug]";

    // Trace location/category formatting
    if (!buffer.empty() && buffer[0] != '[') {
      debug_log << " ";
    }

    for (const char c : buffer) {
      if (c == '\n') {
        debug_log << "\\n";
      } else if (c == '\r') {
        debug_log << "\\r";
      } else if (std::isprint(static_cast<unsigned char>(c))) {
        debug_log << c;
      }
    }
    debug_log << std::endl;
  }
}
#endif

} // namespace game

#include "symbol.hpp"          // IWYU pragma: export
#include "structs/structs.hpp" // IWYU pragma: export
#include "symbols/symbols.hpp" // IWYU pragma: export
#include "impl/game/dvar.hpp"  // IWYU pragma: export
#endif