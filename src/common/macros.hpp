#pragma once

#include <array>
#include <format>
#include <string>
#include <type_traits>

#ifndef __inline_def
#if defined(__clang__) || defined(__GNUC__)
#define __inline_def __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __inline_def __forceinline
#define TEMPLATE_INVALID_HANDLE_VALUE -1
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef __optimize
#if defined(__clang__) || defined(__GNUC__)
#define __optimize __attribute__((hot))
#elif defined(_MSC_VER)
// No equivalent that I can find. Add if found or known otherwise.
#define __optimize
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#ifndef unreachable
#ifdef NDEBUG
#if defined(__GNUC__) || defined(__clang__)
#define unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define unreachable() __assume(false)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#else
#define unreachable() assert(false && "This code should be unreachable.")
#endif
#endif

#ifndef __trivial_abi
#if defined(__clang__) && __has_cpp_attribute(clang::trivial_abi)
#define __trivial_abi [[clang::trivial_abi]]
#elif defined(__GNUC__) && __has_cpp_attribute(gnu::trivial_abi)
#define __trivial_abi [[gnu::trivial_abi]]
#else
// An equivalent does not exist in MSVC. This is a new language extension,
// added in 2017 in clang and GCC.
#define __trival_abi
#endif
#endif

template <typename T>
concept ScopedEnum =
    std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;
template <typename T, typename V>
concept ScopedUnderlying =
    ScopedEnum<T> && std::is_same_v<std::underlying_type_t<T>, V>;

template <ScopedEnum T>
inline constexpr std::underlying_type_t<T> to_underlying(T e) noexcept {
  return static_cast<std::underlying_type_t<T>>(e);
}

template <typename From, typename To = uint64_t>
concept ScopedIntegralLike =
    ScopedEnum<From> &&
    (ScopedUnderlying<From, int8_t> || ScopedUnderlying<From, uint8_t> ||
     ScopedUnderlying<From, int16_t> || ScopedUnderlying<From, uint16_t> ||
     ScopedUnderlying<From, int32_t> || ScopedUnderlying<From, uint32_t> ||
     ScopedUnderlying<From, int64_t> || ScopedUnderlying<From, uint64_t> ||
     std::is_integral_v<std::underlying_type_t<From>> ||
     std::is_convertible_v<std::underlying_type_t<From>, To>);

template <typename From, typename To = uint64_t>
concept IntegralLike =
    std::is_integral_v<To> &&
    (std::is_convertible_v<From, To> || ScopedIntegralLike<From, To>);

template <typename T, std::size_t N>
concept SizeGreaterThan = sizeof(T) > N;

template <typename From, typename To>
concept SizedIntegralLike =
    (std::is_convertible_v<From, To> || ScopedIntegralLike<From, To>) &&
    !SizeGreaterThan<From, sizeof(To)>;

template <typename T> using element_of = std::remove_all_extents_t<T>;

#ifndef WITH_DIAG_DISABLED

// Helper macro to properly evaluate and stringify arguments for standard
// _Pragma
#define DETAIL_DIAG_PRAGMA(x) _Pragma(#x)

#if defined(__clang__)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  _Pragma("clang diagnostic push")                                             \
      DETAIL_DIAG_PRAGMA(clang diagnostic ignored clang_diag)                  \
          __VA_ARGS__ _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  _Pragma("GCC diagnostic push")                                               \
      DETAIL_DIAG_PRAGMA(GCC diagnostic ignored gnu_diag)                      \
          __VA_ARGS__ _Pragma("GCC diagnostic pop")

#elif defined(_MSC_VER)
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...)                \
  __pragma(warning(push)) __pragma(warning(disable : msc_diag)) __VA_ARGS__    \
  __pragma(warning(pop))

#else
#pragma message(                                                               \
    "WITH_DIAG_DISABLED: Unsupported compiler. Diagnostics will not be disabled.")
#define WITH_DIAG_DISABLED(clang_diag, gnu_diag, msc_diag, ...) __VA_ARGS__

#endif
#endif // WITH_DIAG_DISABLED

#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

#ifndef ARRAYSIZE
template <typename Element, const size_t Count>
inline constexpr auto ARRAYSIZE(const Element (&)[Count]) {
  return Count;
}

template <typename Element, const size_t Count>
inline constexpr auto ARRAYSIZE(const std::array<Element, Count> &arr) {
  return arr.size();
}
#endif

template <typename T> inline constexpr const char *reflect_name() {
  return __PRETTY_FUNCTION__;
}

inline constexpr uint64_t byteswap(uint64_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap64(val);
#elif defined(_MSC_VER)
  return _byteswap_uint64(val);
#else
  // Fallback if compiler is unknown
  return ((val & 0xFF00000000000000ULL) >> 56) |
         ((val & 0x00FF000000000000ULL) >> 40) |
         ((val & 0x0000FF0000000000ULL) >> 24) |
         ((val & 0x000000FF00000000ULL) >> 8) |
         ((val & 0x00000000FF000000ULL) << 8) |
         ((val & 0x0000000000FF0000ULL) << 24) |
         ((val & 0x000000000000FF00ULL) << 40) |
         ((val & 0x00000000000000FFULL) << 56);
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint64_t) && !std::is_same_v<T, uint64_t> &&
           std::is_convertible_v<T, uint64_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint64_t>(val)));
}

inline constexpr uint32_t byteswap(uint32_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap32(val);
#elif defined(_MSC_VER)
  return _byteswap_ulong(val);
#else
  // Fallback if compiler is unknown
  return ((val & 0xFF000000U) >> 24) | ((val & 0x00FF0000U) >> 8) |
         ((val & 0x0000FF00U) << 8) | ((val & 0x000000FFU) << 24);
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint32_t) && !std::is_same_v<T, uint32_t> &&
           std::is_convertible_v<T, uint32_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint32_t>(val)));
}

inline constexpr uint16_t byteswap(uint16_t val) {
#if defined(__GNUC__) || defined(__clang__)
  return __builtin_bswap16(val);
#elif defined(_MSC_VER)
  return _byteswap_ushort(val);
#else
  // Fallback if compiler is unknown
  return static_cast<uint16_t>(((val & 0xFF00U) >> 8) | ((val & 0x00FFU) << 8));
#endif
}

template <typename T>
  requires(sizeof(T) == sizeof(uint16_t) && !std::is_same_v<T, uint16_t>,
           std::is_convertible_v<T, uint16_t>)
inline constexpr T byteswap(T val) {
  return static_cast<T>(byteswap(static_cast<uint16_t>(val)));
}

#ifndef APPENDCHAR
#define APPENDCHAR(ptr, c)                                                     \
  ptr[0] = c;                                                                  \
  ++ptr
#endif

template <typename T> struct remove_pointer_recursive {
  using type = std::remove_cv_t<T>;
};

template <typename T> struct remove_pointer_recursive<T *> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *const> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *volatile> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T> struct remove_pointer_recursive<T *const volatile> {
  using type = typename remove_pointer_recursive<T>::type;
};

template <typename T>
using remove_pointer_recursive_t = typename remove_pointer_recursive<T>::type;

// Same as std::remove_pointer_t, but also removes CV qualifiers from both data
// and pointer
template <typename T>
using strip_pointer_t =
    std::remove_cv_t<std::remove_pointer_t<std::remove_cv_t<T>>>;

template <typename T, typename = std::void_t<>>
struct has_serialize : std::false_type {};

template <typename T>
struct has_serialize<T, std::void_t<decltype(std::declval<T &>().serialize())>>
    : std::true_type {};

template <typename T> constexpr bool has_serialize_v = has_serialize<T>::value;

template <typename T>
concept Serializable = requires(T t) { t.serialize(); };

template <typename T>
concept SerializablePtr = !std::is_reference_v<T> && std::is_pointer_v<T> &&
                          Serializable<strip_pointer_t<T>>;

// Require bool argument in template rather than in argument type to ensure this
// is an _explicit_ boolean argument, and not a value implicitly casted to
// boolean
template <typename T>
  requires(std::same_as<T, bool>)
inline constexpr const char *serialize(const T val) {
  return val ? "true" : "false";
}

inline constexpr std::string serialize(const char *val) {
  return val ? std::format("\"{}\"", val) : "NULL";
}

template <SerializablePtr T> inline std::string serialize(const T &ptr) {
  return ptr ? ptr->serialize() : "NULL";
}

template <Serializable T> inline std::string serialize(const T &ptr) {
  return ptr.serialize();
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_same_v<T, char> &&
           !std::is_same_v<T, bool>)
inline std::string serialize(const T *const(&val)) {
  return val ? std::format("0x{:X}", *val) : "NULL";
}

template <typename T>
  requires(std::floating_point<T> && !std::is_same_v<T, char> &&
           !std::is_same_v<T, bool>)
inline std::string serialize(const T *const(&val)) {
  return val ? std::to_string(*val) : "NULL";
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format("0x{:X}", arr[0]);

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", 0x{:X}", arr[i]);
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    result += std::format("0x{:X}", *arr[0]);

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", 0x{:X}", *arr[i]);
      }
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    result += std::format("{}", *arr[0]);

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", {}", *arr[i]);
      }
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format("{}", arr[0]);

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", arr[i]);
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool>)
inline std::string serialize(const T *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    result += std::format(", {}", serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", serialize(arr[i]));
    }
  }
  result += "]";
  return result;
}

template <typename T>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T *const *const arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    if constexpr (std::is_same_v<T, char>) {
      result += std::format(", {}", serialize(arr[0]));
    } else {
      result += std::format(", {}", serialize(*arr[0]));
    }

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        if constexpr (std::is_same_v<T, char>) {
          result += std::format(", {}", serialize(arr[i]));
        } else {
          result += std::format(", {}", serialize(*arr[i]));
        }
      }
    }
  }
  result += "]";
  return result;
}

template <SerializablePtr T>
inline std::string serialize(const T arr, const size_t N) {
  std::string result = "[";
  if (N && arr) {
    // Delegate to global ::serialize to allow recursive requirement resolution
    result += std::format("{}", ::serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      result += std::format(", {}", ::serialize(arr[i]));
    }
  }
  result += "]";
  return result;
}

template <SerializablePtr T>
inline std::string serialize(const T *arr, const size_t N) {
  std::string result = "[";
  if (N && arr && arr[0]) {
    // Delegate to global ::serialize to allow recursive requirement resolution
    result += std::format("{}", ::serialize(arr[0]));

    for (size_t i = 1; i < N; ++i) {
      if (arr[i]) {
        result += std::format(", {}", ::serialize(arr[i]));
      }
    }
  }
  result += "]";
  return result;
}

template <typename T, const size_t N>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <Serializable T, const size_t N>
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_integral_v<T> && !std::is_enum_v<T> &&
           !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_floating_point_v<T>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <typename T, const size_t N>
  requires(std::is_enum_v<T> || std::is_same_v<T, bool> ||
           std::is_same_v<T, char>)
inline std::string serialize(const T *const (&arr)[N]) {
  return serialize(&arr[0], N);
}

template <SerializablePtr T, const size_t N>
inline std::string serialize(const T (&arr)[N]) {
  return serialize(&arr[0], N);
}
