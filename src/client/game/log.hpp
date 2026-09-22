#pragma once

#include <std_include.hpp>

namespace game {
#ifndef NDEBUG
namespace log {
namespace thread {
void startup();
void shutdown();
} // namespace thread
void enqueue(const std::string &msg);
} // namespace log
#endif

#ifdef NDEBUG
template <typename... Args>
inline void tracef([[maybe_unused]] const std::string_view &fmt,
                   [[maybe_unused]] Args &&...args) {}
template <typename... Args>
inline void trace([[maybe_unused]] const std::string_view &fmt,
                  [[maybe_unused]] Args &&...args) {}
#else

template <typename... Args>
inline std::string __tracef_formatter(const std::string_view &fmt,
                                      Args &&...args) {
  const int32_t buf_len = std::snprintf(nullptr, 0, fmt.data(), args...);

  std::string buffer;
  if (buf_len > 0) {
    buffer.resize(buf_len);
    std::snprintf(buffer.data(), buf_len + 1, fmt.data(), args...);
  }
  return buffer;
}

template <typename... Args>
inline std::string __trace_formatter(const std::string_view &fmt,
                                     Args &&...args) {
  return std::vformat(fmt, std::make_format_args(args...));
}

template <typename... Args>
using FormatFunc =
    fastcall_t<std::string(const std::string_view &fmt, Args &&...args)>;
template <typename... Args>
void __trace_impl(FormatFunc<Args...> &formatter, const std::string_view &fmt,
                  Args &&...args) {
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

  // Generate printable output buffer in duplicated memory
  // to ensure minimal output lock time
  std::string out_buffer;
  {
    const std::string buffer = formatter(fmt, std::forward<Args>(args)...);
    out_buffer.reserve(buffer.size());

    out_buffer += std::format("[{}.{:03}][Debug]", now_str, now_ms.count());

    // Trace location/category formatting
    if (!buffer.empty() && buffer[0] != '[') {
      out_buffer += " ";
    }

    for (const char c : buffer) {
      if (c == '\n') {
        out_buffer += "\\n";
      } else if (c == '\r') {
        out_buffer += "\\r";
      } else if (std::isprint(static_cast<unsigned char>(c))) {
        out_buffer += c;
      }
    }
  }

  out_buffer += '\n';

  log::enqueue(std::move(out_buffer));
}

template <typename... Args>
inline void tracef(const std::string_view &fmt, Args &&...args) {
  return __trace_impl(__tracef_formatter<Args...>, fmt,
                      std::forward<Args>(args)...);
}
template <typename... Args>
inline void trace(const std::string_view &fmt, Args &&...args) {
  return __trace_impl(__trace_formatter<Args...>, fmt,
                      std::forward<Args>(args)...);
}
#endif
} // namespace game
