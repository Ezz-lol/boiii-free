#pragma once
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
     std::is_convertible_v<std::underlying_type_t<From>, To>);

template <typename From, typename To = uint64_t>
concept IntegralLike =
    std::is_convertible_v<From, To> || ScopedIntegralLike<From, To>;

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

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif