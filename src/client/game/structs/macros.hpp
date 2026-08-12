#pragma once

#include <macros.hpp>
#include <bit>
#include <cassert>
#include <type_traits>

// Automatically pad a partially defined (reverse-engineered, in our case)
// struct to a fixed, known-correct total length
#define partial_def(TotalSize, PrimType, Fixed, Verified)                      \
  PrimType Fixed {                                                             \
    union {                                                                    \
      PrimType Verified;                                                       \
      uint8_t __raw[TotalSize];                                                \
    };                                                                         \
  }

#define inline_partial_def(InlineNum, FixedTotalSize, PrimType, Verified)      \
  union {                                                                      \
    PrimType Verified verified_##InlineNum;                                    \
    uint8_t __raw_##InlineNum[FixedTotalSize];                                 \
  }

#define IMPL_ENUM_OPERATORS(name)                                              \
  inline constexpr name &operator++(name &s) {                                 \
    using underlying = std::underlying_type_t<name>;                           \
    s = static_cast<name>(static_cast<underlying>(s) + 1);                     \
    return s;                                                                  \
  }                                                                            \
                                                                               \
  inline constexpr name operator++(name &s, int) {                             \
    name temp = s;                                                             \
    ++s;                                                                       \
    return temp;                                                               \
  }                                                                            \
                                                                               \
  inline constexpr name &operator--(name &s) {                                 \
    using underlying = std::underlying_type_t<name>;                           \
    s = static_cast<name>(static_cast<underlying>(s) - 1);                     \
    return s;                                                                  \
  }                                                                            \
                                                                               \
  inline constexpr name operator--(name &s, int) {                             \
    name temp = s;                                                             \
    --s;                                                                       \
    return temp;                                                               \
  }                                                                            \
  inline constexpr std::underlying_type_t<name> operator+(name s) {            \
    return static_cast<std::underlying_type_t<name>>(s);                       \
  }

template <typename T, T Actual, T Expected>
concept ValueMatches = (Actual == Expected);

#ifndef PACKED
#if defined(_MSC_VER) || defined(__clang__) || defined(__GNUC__)
#define PACKED(...) __pragma(pack(push, 1)) __VA_ARGS__ __pragma(pack(pop))
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
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

#ifndef INLINE_MEMSET
#if defined(__clang__) || defined(__GNUC__)
#define INLINE_MEMSET(buf, val, count) __builtin_memset(buf, val, count)
#elif defined(_MSC_VER)
#pragma instrinsic(memset)
#define INLINE_MEMSET(buf, val, count) memset(buf, val, count)
#else
#error "Unsupported compiler. Only MSVC, Clang and GCC are supported."
#endif
#endif

#define BITS_PER_BYTE CHAR_BIT
template <typename T> inline constexpr size_t bitsizeof() noexcept {
  return sizeof(T) * BITS_PER_BYTE;
}

template <typename T>
inline constexpr size_t bitsizeof([[maybe_unused]] const T) noexcept {
  return sizeof(T) * BITS_PER_BYTE;
}

template <IntegralLike<uint64_t> T>
consteval int32_t min_bits_unsigned(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  if (val == 0) {
    return 1; // 0 needs at least 1 bit
  }

  // Total bits (64) minus leading zeros gives the bits used
  return bitsizeof<uint64_t>() - std::countl_zero(val);
}

template <IntegralLike<uint64_t> T> consteval uint32_t min_bits_mask(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  int32_t min_bits = min_bits_unsigned(val);
  return (1 << min_bits) - 1;
}