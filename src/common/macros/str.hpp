#pragma once

#include "macros/builtin.hpp"
#include <cstdint>
#include <type_traits>

template <typename T>
concept CharLike = sizeof(T) == sizeof(char);

template <typename T> inline constexpr const char *reflect_name() {
  return __PRETTY_FUNCTION__;
}

#ifndef APPENDCHAR
#define APPENDCHAR(ptr, c)                                                     \
  ptr[0] = c;                                                                  \
  ++ptr
#endif

inline constexpr auto ASCII_UPPERCASE_RANGE_START = 'A';
inline constexpr auto ASCII_UPPERCASE_RANGE_END = 'Z';

inline constexpr auto ASCII_LOWERCASE_RANGE_START = 'a';
inline constexpr auto ASCII_LOWERCASE_RANGE_END = 'z';

template <CharLike T> alwaysinline constexpr bool is_uppercase(const T c) {
  return static_cast<char>(c) >= ASCII_UPPERCASE_RANGE_START &&
         static_cast<char>(c) <= ASCII_UPPERCASE_RANGE_END;
}

template <CharLike T> alwaysinline constexpr bool is_lowercase(const T c) {
  return static_cast<char>(c) >= ASCII_LOWERCASE_RANGE_START &&
         static_cast<char>(c) <= ASCII_LOWERCASE_RANGE_END;
}

inline constexpr auto UPPER_TO_LOWER_INC =
    ASCII_LOWERCASE_RANGE_START - ASCII_UPPERCASE_RANGE_START;
inline constexpr auto LOWER_TO_UPPER_INC =
    ASCII_UPPERCASE_RANGE_START - ASCII_LOWERCASE_RANGE_START;

template <CharLike T> alwaysinline constexpr T tolower(const T c) noexcept {
  return is_uppercase(c)
             ? static_cast<T>(c + static_cast<T>(UPPER_TO_LOWER_INC))
             : c;
}

template <CharLike T> alwaysinline constexpr T toupper(const T c) noexcept {
  return is_lowercase(c)
             ? static_cast<T>(c + static_cast<T>(LOWER_TO_UPPER_INC))
             : c;
}
