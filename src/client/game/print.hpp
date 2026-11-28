#pragma once

#include "macros/builtin.hpp"
#include <format>
#include <string_view>

namespace console {
void print_stdout(const std::string_view &message);
void print_stderr(const std::string_view &message);
} // namespace console

namespace game {

enum class PrintOutput : uint8_t { STDOUT = 0, STDERR = 1 };

template <typename... Args>
alwaysinline void println(const PrintOutput output,
                          std::format_string<Args...> fmt, Args &&...args) {
  std::string buffer = std::format(fmt, std::forward<Args>(args)...);
  buffer.push_back('\n');
  switch (output) {
  case PrintOutput::STDOUT:
    console::print_stdout(buffer);
    break;
  case PrintOutput::STDERR:
    console::print_stderr(buffer);
    break;
  }
}

template <typename... Args>
alwaysinline void println(std::format_string<Args...> fmt, Args &&...args) {
  println(PrintOutput::STDOUT, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
alwaysinline void print(const PrintOutput output,
                        std::format_string<Args...> fmt, Args &&...args) {
  std::string buffer = std::format(fmt, std::forward<Args>(args)...);
  switch (output) {
  case PrintOutput::STDOUT:
    console::print_stdout(buffer);
    break;
  case PrintOutput::STDERR:
    console::print_stderr(buffer);
    break;
  }
}

template <typename... Args>
alwaysinline void print(std::format_string<Args...> fmt, Args &&...args) {
  println(PrintOutput::STDOUT, fmt, std::forward<Args>(args)...);
}
} // namespace game
