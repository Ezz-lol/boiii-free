#pragma once

#include "macros/builtin.hpp"
#include "macros/type/core.hpp"
#include "macros/type/set.hpp"

#include <format>
#include <string>

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

template <typename T>
concept CharArrayLike =
    std::is_array_v<std::remove_cvref_t<T>> &&
    CharLike<std::remove_all_extents_t<std::remove_cvref_t<T>>>;

template <typename T>
concept CharPtrLike =
    AnyOf<std::remove_cvref_t<T>>::template In<char *> ||
    (CharArrayLike<T> &&
     !std::is_const_v<std::remove_all_extents_t<std::remove_cvref_t<T>>>);

template <typename T>
concept ConstCharPtrLike =
    AnyOf<std::remove_cvref_t<T>>::template In<const char *, char *> ||
    (CharArrayLike<T> &&
     std::is_const_v<std::remove_all_extents_t<std::remove_cvref_t<T>>>);

template <typename T>
concept StringLike = CharPtrLike<T> || requires(T t) {
  { t.data() } -> std::same_as<char *>;
};

template <typename T>
concept ConstStringLike =
    StringLike<T> || ConstCharPtrLike<T> || requires(const T t) {
      { t.data() } -> std::same_as<const char *>;
    };

template <ConstStringLike T> const char *stringlike_data(const T &str) {
  if constexpr (ConstCharPtrLike<T> || CharPtrLike<T>) {
    return str;
  } else {
    return str.data();
  }
}

template <StringLike T> char *stringlike_data(T &str) {
  if constexpr (CharPtrLike<T>) {
    return str;
  } else {
    return str.data();
  }
}

template <typename T>
concept CStrLike =
    std::is_pointer_v<std::remove_cvref_t<T>> &&
    std::is_same_v<
        std::remove_cv_t<std::remove_pointer_t<std::remove_cvref_t<T>>>, char>;

template <typename T>
concept ToStringable = requires(const T &val) {
  { std::to_string(val) } -> std::convertible_to<std::string>;
};

// e.g. std::bitset<N>::to_string()
template <typename T>
concept HasToStringMethod = requires(const T &val) {
  { val.to_string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept DefaultFormattable = std::formattable<T, char>;

template <typename T>
concept ImplicitlyStringConvertible = std::is_convertible_v<T, std::string>;

template <typename T>
concept ExplicitlyStringConvertible = std::is_constructible_v<std::string, T>;

template <typename T>
concept StringConvertible =
    IsString<T> || ImplicitlyStringConvertible<T> ||
    ExplicitlyStringConvertible<T> || ToStringable<T> ||
    DefaultFormattable<T> || CStrLike<T> || IsFilesystemPath<T> || IsEnum<T> ||
    HasToStringMethod<T> || IsByte<T> || IsException<T>;

template <StringConvertible T>
[[nodiscard]] alwaysinline std::string resolve_string(const T &val) {
  if constexpr (IsString<T> || ImplicitlyStringConvertible<T>) {
    return val;
  } else if constexpr (IsFilesystemPath<T>) {
    return val.string();
  } else if constexpr (IsEnum<T>) {
    return resolve_string(std::to_underlying(val));
  } else if constexpr (IsByte<T>) {
    return resolve_string(std::to_integer<uint32_t>(val));
  } else if constexpr (IsException<T>) {
    return val.what();
  } else if constexpr (HasToStringMethod<T>) {
    return val.to_string();
  } else if constexpr (ExplicitlyStringConvertible<T>) {
    return std::string(val);
  } else if constexpr (DefaultFormattable<T>) {
    return std::format("{}", val);
  } else if constexpr (ToStringable<T>) {
    return std::to_string(val);
  } else {
    static_assert(false,
                  "resolve_string: no conversion branch matched — this is a "
                  "StringConvertible concept/branch mismatch");
    std::unreachable();
  }
}

template <typename T>
  requires(std::is_pointer_v<std::remove_cvref_t<T>> && !StringConvertible<T> &&
           StringConvertible<
               std::remove_cv_t<std::remove_pointer_t<std::remove_cvref_t<T>>>>)
[[nodiscard]] alwaysinline std::string resolve_string(const T &val) {
  return resolve_string(*val);
}

// Wrapper of `std::format_to_n` that NUL-termintes the formatted string, just
// as `snprintf` does.
template <std::output_iterator<char> Out, class... Args>
alwaysinline constexpr std::format_to_n_result<Out>
snformatf(Out out, std::iter_difference_t<Out> n,
          std::format_string<Args...> fmt, Args &&...args) {
  if (n <= 0) {
    return std::format_to_n_result<Out>{out, 0};
  }

  const std::iter_difference_t<Out> max_chars =
      static_cast<std::iter_difference_t<Out>>(n - 1);
  std::format_to_n_result<Out> result =
      std::format_to_n(out, max_chars, fmt, std::forward<Args>(args)...);
  *result.out = '\0';
  ++result.out;
  result.size = std::formatted_size(fmt, std::forward<Args>(args)...);
  return result;
}

template <std::size_t N, class... Args>
alwaysinline constexpr std::format_to_n_result<char *>
snformatf(char (&out)[N], std::format_string<Args...> fmt, Args &&...args) {
  return snformatf(out,
                   static_cast<std::iter_difference_t<char *>>(std::size(out)),
                   fmt, std::forward<Args>(args)...);
}
