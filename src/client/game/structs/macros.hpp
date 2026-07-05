#pragma once
#include <bit>

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

template <size_t Actual, size_t Expected>
concept ValueMatches = (Actual == Expected);

#define ASSERT_SIZE(type, size)                                                \
  static_assert(ValueMatches<sizeof(type), (size)>, "Size mismatch "           \
                                                    "for " #type)

#define ASSERT_OFFSET(type, field, offset)                                     \
  static_assert(ValueMatches<offsetof(type, field), (offset)>,                 \
                "Offset mismatch for " #type "::" #field)

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

#define BITS(x) (8 * x)
template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, uint64_t>::value>::type>
consteval int32_t min_bits_unsigned(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  if (val == 0)
    return 1; // 0 needs at least 1 bit

  // Total bits (64) minus leading zeros gives the bits used
  return BITS(sizeof(uint64_t)) - std::countl_zero(val);
}

template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, uint64_t>::value>::type>
consteval uint32_t min_bits_mask(T val_in) {
  uint64_t val = static_cast<uint64_t>(val_in);
  int32_t min_bits = min_bits_unsigned(val);
  return (1 << min_bits) - 1;
}