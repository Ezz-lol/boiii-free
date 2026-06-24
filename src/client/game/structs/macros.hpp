#pragma once

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
  inline name &operator++(name &s) {                                           \
    using underlying = std::underlying_type_t<name>;                           \
    s = static_cast<name>(static_cast<underlying>(s) + 1);                     \
    return s;                                                                  \
  }                                                                            \
                                                                               \
  inline name operator++(name &s, int) {                                       \
    name temp = s;                                                             \
    ++s;                                                                       \
    return temp;                                                               \
  }                                                                            \
                                                                               \
  inline name &operator--(name &s) {                                           \
    using underlying = std::underlying_type_t<name>;                           \
    s = static_cast<name>(static_cast<underlying>(s) - 1);                     \
    return s;                                                                  \
  }                                                                            \
                                                                               \
  inline name operator--(name &s, int) {                                       \
    name temp = s;                                                             \
    --s;                                                                       \
    return temp;                                                               \
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