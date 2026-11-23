#pragma once

#include <bit>
#include <cassert>
#include <macros/macros.hpp>
#include <type_traits>

// Required for IMPL_SLICE_METHODS macros
#include <span> // IWYU pragma: export

#ifndef IMPL_ENUM_OPS_WARN_PUSH
#if defined(__clang__)
#define IMPL_ENUM_OPS_WARN_PUSH                                                \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic ignored \"-Wunused-function\"")
#define IMPL_ENUM_OPS_WARN_POP _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define IMPL_ENUM_OPS_WARN_PUSH                                                \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#define IMPL_ENUM_OPS_WARN_POP _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define IMPL_ENUM_OPS_WARN_PUSH                                                \
  __pragma(warning(push)) __pragma(warning(disable : 4505))
#define IMPL_ENUM_OPS_WARN_POP __pragma(warning(pop))
#else
#define IMPL_ENUM_OPS_WARN_PUSH
#define IMPL_ENUM_OPS_WARN_POP
#endif

#ifndef IMPL_ENUM_OPERATORS
#define IMPL_ENUM_OPERATORS(Enum)                                              \
  IMPL_ENUM_OPS_WARN_PUSH                                                      \
  inline constexpr Enum &operator++(Enum &s) noexcept {                        \
    s = static_cast<Enum>(std::to_underlying(s) + 1);                          \
    return s;                                                                  \
  }                                                                            \
  inline constexpr Enum operator++(Enum &s, int) noexcept {                    \
    Enum t = s;                                                                \
    ++s;                                                                       \
    return t;                                                                  \
  }                                                                            \
  inline constexpr Enum &operator--(Enum &s) noexcept {                        \
    s = static_cast<Enum>(std::to_underlying(s) - 1);                          \
    return s;                                                                  \
  }                                                                            \
  inline constexpr Enum operator--(Enum &s, int) noexcept {                    \
    Enum t = s;                                                                \
    --s;                                                                       \
    return t;                                                                  \
  }                                                                            \
  inline constexpr std::underlying_type_t<Enum> operator+(Enum s) noexcept {   \
    return std::to_underlying(s);                                              \
  }                                                                            \
  inline constexpr Enum operator-(Enum s) noexcept {                           \
    using U = std::underlying_type_t<Enum>;                                    \
    using UU = std::make_unsigned_t<U>;                                        \
    return static_cast<Enum>(                                                  \
        static_cast<U>(0u - static_cast<UU>(std::to_underlying(s))));          \
  }                                                                            \
  inline constexpr Enum operator~(Enum s) noexcept {                           \
    return static_cast<Enum>(~std::to_underlying(s));                          \
  }                                                                            \
                                                                               \
  inline constexpr Enum &operator+=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) + std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator+(Enum a, Enum b) noexcept { return a += b; }  \
                                                                               \
  inline constexpr Enum &operator-=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) - std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator-(Enum a, Enum b) noexcept { return a -= b; }  \
                                                                               \
  inline constexpr Enum &operator*=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) * std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator*(Enum a, Enum b) noexcept { return a *= b; }  \
                                                                               \
  inline constexpr Enum &operator/=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) / std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator/(Enum a, Enum b) noexcept { return a /= b; }  \
                                                                               \
  inline constexpr Enum &operator%=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) % std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator%(Enum a, Enum b) noexcept { return a %= b; }  \
                                                                               \
  inline constexpr Enum &operator&=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) & std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator&(Enum a, Enum b) noexcept { return a &= b; }  \
                                                                               \
  inline constexpr Enum &operator|=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) | std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator|(Enum a, Enum b) noexcept { return a |= b; }  \
                                                                               \
  inline constexpr Enum &operator^=(Enum &a, Enum b) noexcept {                \
    a = static_cast<Enum>(std::to_underlying(a) ^ std::to_underlying(b));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator^(Enum a, Enum b) noexcept { return a ^= b; }  \
                                                                               \
  inline constexpr Enum &operator<<=(Enum &a, int shift) noexcept {            \
    using U = std::underlying_type_t<Enum>;                                    \
    using UU = std::make_unsigned_t<U>;                                        \
    a = static_cast<Enum>(                                                     \
        static_cast<U>(static_cast<UU>(std::to_underlying(a)) << shift));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator<<(Enum a, int shift) noexcept {               \
    return a <<= shift;                                                        \
  }                                                                            \
                                                                               \
  inline constexpr Enum &operator>>=(Enum &a, int shift) noexcept {            \
    using U = std::underlying_type_t<Enum>;                                    \
    using UU = std::make_unsigned_t<U>;                                        \
    a = static_cast<Enum>(                                                     \
        static_cast<U>(static_cast<UU>(std::to_underlying(a)) >> shift));      \
    return a;                                                                  \
  }                                                                            \
  inline constexpr Enum operator>>(Enum a, int shift) noexcept {               \
    return a >>= shift;                                                        \
  }                                                                            \
  IMPL_ENUM_OPS_WARN_POP

#endif
#endif

template <typename T, T Actual, T Expected>
concept ValueMatches = (Actual == Expected);

#ifndef PACKED
#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
#define PACKED(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

// Automatically pad a partially defined (reverse-engineered, in our case)
// struct to a fixed, known-correct total length
#ifndef partial_def
#define partial_def(TotalSize, PrimType, Fixed, Verified)                               \
  PACKED(PrimType Fixed {                                                             \
    union {                                                                    \
      PrimType Verified;                                                       \
      uint8_t __raw[TotalSize];                                                \
    });                                                                         \
  }
#endif

#ifndef inline_partial_def
#define inline_partial_def(InlineNum, FixedTotalSize, PrimType, Verified)      \
  PACKED(union {                                                               \
    PrimType Verified verified_##InlineNum;                                    \
    uint8_t __raw_##InlineNum[FixedTotalSize];                                 \
  })
#endif

#ifndef ASSERT_ALIGNMENT
#define ASSERT_ALIGNMENT(type, align)                                          \
  static_assert(ValueMatches<size_t, alignof(type), align>,                    \
                "Alignment mismatch for " #type)
#endif

#ifndef ASSERT_PACKED
#define ASSERT_PACKED(type) ASSERT_ALIGNMENT(type, 1)
#endif

#define ASSERT_SIZE(type, size)                                                \
  static_assert(ValueMatches<size_t, sizeof(type), (size)>, "Size mismatch "   \
                                                            "for " #type)

#define ASSERT_OFFSET(type, field, offset)                                     \
  static_assert(ValueMatches<size_t, offsetof(type, field), (offset)>,         \
                "Offset mismatch for " #type "::" #field)

template <typename T>
concept CPP03PoD =
    std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> &&
    std::is_trivially_constructible_v<T> &&
    std::is_trivially_destructible_v<T> &&
    std::is_trivially_copy_constructible_v<T> &&
    std::is_trivially_assignable_v<T, T> &&
    std::is_trivially_copy_assignable_v<T> &&
    std::is_trivially_move_assignable_v<T> &&
    std::is_trivially_default_constructible_v<T> &&
    std::is_trivially_move_constructible_v<T> &&
    (std::is_aggregate_v<T> || std::is_scalar_v<T>);

/*
  The above `CPP03PoD` concept is a best-effort set of criteria to match that
  used by the LLVM and Microsoft CL IR generation backends for the x86-64
  Windows MSVC target.

  See the "Return values" section of Microsoft's x86-64 MSVC calling
  convention documentation [1] for more information.

  [1] https://learn.microsoft.com/cpp/build/x64-calling-convention#return-values
*/
template <typename T> consteval bool is_cpp03_pod() { return CPP03PoD<T>; }

#define ASSERT_CPP03_POD(name)                                                 \
  static_assert(ValueMatches<bool, std::is_standard_layout_v<name>, true>,     \
                #name " must be standard layout!");                            \
  static_assert(ValueMatches<bool, std::is_trivially_copyable_v<name>, true>,  \
                #name " must be trivially copyable!");                         \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_constructible_v<name>, true>,       \
      #name " must be trivially constructible!");                              \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_destructible_v<name>, true>,        \
      #name " must be trivially destructible!");                               \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_copy_constructible_v<name>, true>,  \
      #name " must be trivially copy constructible!");                         \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_assignable_v<name, name>, true>,    \
      #name " must be trivially assignable!");                                 \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_assignable_v<name &, const name>,   \
                   true>,                                                      \
      #name " must be trivially assignable!");                                 \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_copy_assignable_v<name>, true>,     \
      #name " must be trivially copy assignable!");                            \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_move_assignable_v<name>, true>,     \
      #name " must be trivially move assignable!");                            \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_default_constructible_v<name>,      \
                   true>,                                                      \
      #name " must be trivially default constructible!");                      \
  static_assert(                                                               \
      ValueMatches<bool, std::is_trivially_move_constructible_v<name>, true>,  \
      #name " must be trivially move constructible!");                         \
  static_assert(                                                               \
      ValueMatches<                                                            \
          bool, (std::is_scalar_v<name> || std::is_aggregate_v<name>), true>,  \
      #name " must be an aggregate or scalar type!");

inline constexpr auto BITS_PER_BYTE = CHAR_BIT;
template <typename T> alwaysinline constexpr auto bitsizeof() noexcept {
  return sizeof(T) * BITS_PER_BYTE;
}

template <typename T>
alwaysinline constexpr size_t
bitsizeof([[maybe_unused]] const T /*unused*/) noexcept {
  return sizeof(T) * BITS_PER_BYTE;
}

template <IntegralLike<uint64_t> T>
consteval int32_t min_bits_unsigned(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  if (val == 0) {
    return 1; // 0 needs at least 1 bit
  }

  // Total bits (64) minus leading zeros gives the bits used
  return static_cast<int32_t>(bitsizeof<uint64_t>()) - std::countl_zero(val);
}

template <IntegralLike<uint64_t> T> consteval uint32_t min_bits_mask(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  int32_t min_bits = min_bits_unsigned(val);
  return (1 << min_bits) - 1;
}

#ifndef IMPL_SLICE_METHODS
#define IMPL_SLICE_METHODS(field, count)                                       \
  inline std::span<                                                            \
      const std::remove_all_extents_t<strip_pointer_t<decltype(field)>>>       \
  get_##field() const noexcept {                                               \
    return std::span(field, static_cast<size_t>(count));                       \
  }                                                                            \
                                                                               \
  inline std::span<                                                            \
      std::remove_all_extents_t<strip_pointer_t<decltype(field)>>>             \
  get_##field() noexcept {                                                     \
    return std::span(field, static_cast<size_t>(count));                       \
  }
#endif

#ifndef IMPL_CONST_SLICE_METHODS
#define IMPL_CONST_SLICE_METHODS(field, count)                                 \
  inline std::span<                                                            \
      const std::remove_all_extents_t<strip_pointer_t<decltype(field)>>>       \
  get_##field() const noexcept {                                               \
    return std::span(field, static_cast<size_t>(count));                       \
  }                                                                            \
                                                                               \
  inline std::span<                                                            \
      const std::remove_all_extents_t<strip_pointer_t<decltype(field)>>>       \
  get_##field() noexcept {                                                     \
    return std::span(field, static_cast<size_t>(count));                       \
  }
#endif

#ifndef IMPL_VOLATILE_SLICE_METHODS
#define IMPL_VOLATILE_SLICE_METHODS(field, count)                              \
  inline std::span<const volatile std::remove_all_extents_t<                   \
      strip_pointer_t<decltype(field)>>>                                       \
  get_##field() const noexcept {                                               \
    return std::span(field, static_cast<size_t>(count));                       \
  }                                                                            \
                                                                               \
  inline std::span<                                                            \
      volatile std::remove_all_extents_t<strip_pointer_t<decltype(field)>>>    \
  get_##field() noexcept {                                                     \
    return std::span(field, static_cast<size_t>(count));                       \
  }
#endif

#ifndef DEF_PAD
#define DEF_PAD(hex_offset, size)                                              \
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers) */                  \
  uint8_t _padding##hex_offset[size]
#endif

#ifndef DEF_UNK
#define DEF_UNK(hex_offset, size)                                              \
  /* NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers) */                  \
  uint8_t _unknown##hex_offset[size]
#endif
