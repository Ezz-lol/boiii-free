#pragma once

#include "game/structs/macros.hpp"
#include <functional>
#include <string_view>
#include <windows.h>
namespace console {

enum class ColorCode : char {
  BLACK = '0',
  RED = '1',
  GREEN = '2',
  YELLOW = '3',
  BLUE = '4',
  CYAN = '5',
  MAGENTA = '6',
  WHITE = '7',
  ORANGE = '8',
  GRAY = '9',
};
IMPL_ENUM_OPERATORS(ColorCode);
constexpr bool validate(const ColorCode code) {
  return code >= ColorCode::BLACK && code <= ColorCode::GRAY;
}

constexpr COLORREF BLACK = RGB(0, 0, 0);
constexpr COLORREF RED = RGB(237, 92, 92);
constexpr COLORREF GREEN = RGB(96, 200, 120);
constexpr COLORREF YELLOW = RGB(232, 209, 96);
constexpr COLORREF BLUE = RGB(96, 150, 237);
constexpr COLORREF CYAN = RGB(96, 210, 220);
constexpr COLORREF MAGENTA = RGB(220, 120, 220);
constexpr COLORREF WHITE = RGB(235, 235, 238);
constexpr COLORREF ORANGE = RGB(235, 160, 90);
constexpr COLORREF GRAY = RGB(150, 150, 156);

constexpr COLORREF ERROR_COLOR = RGB(248, 113, 113);
constexpr COLORREF WARNING_COLOR = RGB(232, 194, 84);
constexpr COLORREF INFO_COLOR = RGB(161, 161, 170);
constexpr COLORREF BRACKET_TAG_COLOR = RGB(96, 165, 250);
constexpr COLORREF DEFAULT_CONSOLE_COLOR = RGB(228, 228, 231);
constexpr COLORREF BACKGROUND_COLOR = RGB(24, 24, 27);
constexpr COLORREF HINT_BACKGROUND_COLOR = RGB(38, 38, 46);

constexpr COLORREF get_cod_color(const ColorCode code) {
  switch (code) {
  case ColorCode::BLACK:
    return BLACK;
  case ColorCode::RED:
    return RED;
  case ColorCode::GREEN:
    return GREEN;
  case ColorCode::YELLOW:
    return YELLOW;
  case ColorCode::BLUE:
    return BLUE;
  case ColorCode::CYAN:
    return CYAN;
  case ColorCode::MAGENTA:
    return MAGENTA;
  case ColorCode::WHITE:
    return WHITE;
  case ColorCode::ORANGE:
    return ORANGE;
  case ColorCode::GRAY:
    return GRAY;
  default:
    return DEFAULT_CONSOLE_COLOR;
  }
}

void set_title(const std::string_view &title);
void print(const char *message);
void print_stdout(const std::string_view &message);
void print_stderr(const std::string_view &message);
void set_interceptor(
    std::function<void(const std::string_view &message)> callback);
void remove_interceptor();
bool is_ready();

struct scoped_interceptor {
  scoped_interceptor(
      std::function<void(const std::string_view &message)> callback) {
    set_interceptor(std::move(callback));
  }

  ~scoped_interceptor() { remove_interceptor(); }

  scoped_interceptor(scoped_interceptor &&) = delete;
  scoped_interceptor(const scoped_interceptor &) = delete;
  scoped_interceptor &operator=(scoped_interceptor &&) = delete;
  scoped_interceptor &operator=(const scoped_interceptor &) = delete;
};
} // namespace console
