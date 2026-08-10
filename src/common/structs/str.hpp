#pragma once

#include "array.hpp"

template <const auto N> using str = array<char, N>;

typedef str<8> str8_t;
typedef str<16> str16_t;
typedef str<24> str24_t;
typedef str<32> str32_t;
typedef str<64> str64_t;
typedef str<128> str128_t;
typedef str<256> str256_t;
typedef str<512> str512_t;
typedef str<1024> str1024_t;

struct ToStringResult {
  char *buf;
  char *ptr;

  inline constexpr operator char *() const noexcept { return buf; }
};

template <int32_t N> struct ConstString {
  char buf[N] = {};

  constexpr ConstString(const char (&s)[N]) {
    for (int32_t i = 0; i < N; ++i)
      buf[i] = s[i];
  }

  inline constexpr const char *c_str() const noexcept { return buf; }
  inline constexpr operator const char *() const noexcept { return c_str(); }
};

// Deduction guide
template <int32_t N> ConstString(const char (&)[N]) -> ConstString<N>;

template <const auto N> using str = char[static_cast<size_t>(N)];

// Helper to convert a byte/integer to hex characters at compile-time
template <IntegralLike<uint8_t> Index>
inline constexpr char hex_digit(Index v) {
  return "0123456789ABCDEF"[static_cast<uint8_t>(v) & 0xF];
}

namespace {
template <typename T> inline constexpr auto num_hex_characters() {
  constexpr auto NUM_HEX_CHARS_PER_BYTE = 2;
  return sizeof(T) * NUM_HEX_CHARS_PER_BYTE;
}
} // namespace

template <const size_t BaseSize, IntegralLike T>
constexpr const char *
append_hex(const str<BaseSize> &base, T val,
           str<BaseSize + num_hex_characters<T>()> &result = {0}) {
  // Determine max hex characters needed
  constexpr size_t hex_len = num_hex_characters<T>();

  int32_t idx = 0;
  // Copy base string (excluding old null-terminator)
  for (size_t i = 0; i < BaseSize - 1; ++i) {
    result[idx++] = base[i];
  }

  // Append hex representation backwards or forwards
  for (int32_t i = hex_len - 1; i >= 0; --i) {
    result[idx + i] = hex_digit(val >> (4 * (hex_len - 1 - i)));
  }
  idx += hex_len;
  result[idx] = '\0';

  return result;
}