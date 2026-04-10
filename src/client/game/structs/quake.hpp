#pragma once

#include <cstdint>
#include <type_traits>
namespace game {

#pragma pack(push, 1)
struct qboolean {
  int32_t value;

  constexpr qboolean() noexcept : value(0) {}
  constexpr qboolean(bool b) noexcept : value(b ? 1 : 0) {}
  constexpr qboolean(int32_t i) noexcept : value(i) {}
  constexpr qboolean(uint32_t i) noexcept : value((int32_t)i) {}

  constexpr operator bool() const noexcept { return value != 0; }
  constexpr operator int32_t() const noexcept { return value; }
  constexpr operator uint32_t() const noexcept { return (uint32_t)value; }

  constexpr qboolean operator!() const noexcept { return value == 0; }
  constexpr qboolean operator~() const noexcept { return ~value; }
  constexpr qboolean operator-() const noexcept { return -value; }
  constexpr qboolean operator+() const noexcept { return value; }

  constexpr qboolean &operator++() noexcept {
    ++value;
    return *this;
  }
  constexpr qboolean operator++(int32_t) noexcept {
    qboolean temp = *this;
    ++value;
    return temp;
  }
  constexpr qboolean &operator--() noexcept {
    --value;
    return *this;
  }
  constexpr qboolean operator--(int32_t) noexcept {
    qboolean temp = *this;
    --value;
    return temp;
  }

  constexpr qboolean &operator+=(const qboolean &rhs) noexcept {
    value += rhs.value;
    return *this;
  }
  constexpr qboolean &operator-=(const qboolean &rhs) noexcept {
    value -= rhs.value;
    return *this;
  }
  constexpr qboolean &operator*=(const qboolean &rhs) noexcept {
    value *= rhs.value;
    return *this;
  }
  constexpr qboolean &operator/=(const qboolean &rhs) noexcept {
    value /= rhs.value;
    return *this;
  }
  constexpr qboolean &operator%=(const qboolean &rhs) noexcept {
    value %= rhs.value;
    return *this;
  }
  constexpr qboolean &operator&=(const qboolean &rhs) noexcept {
    value &= rhs.value;
    return *this;
  }
  constexpr qboolean &operator|=(const qboolean &rhs) noexcept {
    value |= rhs.value;
    return *this;
  }
  constexpr qboolean &operator^=(const qboolean &rhs) noexcept {
    value ^= rhs.value;
    return *this;
  }
  constexpr qboolean &operator<<=(const qboolean &rhs) noexcept {
    value <<= rhs.value;
    return *this;
  }
  constexpr qboolean &operator>>=(const qboolean &rhs) noexcept {
    value >>= rhs.value;
    return *this;
  }

  friend constexpr qboolean operator+(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs += rhs;
  }
  friend constexpr qboolean operator-(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs -= rhs;
  }
  friend constexpr qboolean operator*(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs *= rhs;
  }
  friend constexpr qboolean operator/(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs /= rhs;
  }
  friend constexpr qboolean operator%(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs %= rhs;
  }

  friend constexpr qboolean operator&(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs &= rhs;
  }
  friend constexpr qboolean operator|(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs |= rhs;
  }
  friend constexpr qboolean operator^(qboolean lhs,
                                      const qboolean &rhs) noexcept {
    return lhs ^= rhs;
  }
  friend constexpr qboolean operator<<(qboolean lhs,
                                       const qboolean &rhs) noexcept {
    return lhs <<= rhs;
  }
  friend constexpr qboolean operator>>(qboolean lhs,
                                       const qboolean &rhs) noexcept {
    return lhs >>= rhs;
  }

  friend constexpr bool operator==(const qboolean &lhs,
                                   const qboolean &rhs) noexcept {
    return lhs.value == rhs.value;
  }
  friend constexpr bool operator!=(const qboolean &lhs,
                                   const qboolean &rhs) noexcept {
    return lhs.value != rhs.value;
  }
  friend constexpr bool operator<(const qboolean &lhs,
                                  const qboolean &rhs) noexcept {
    return lhs.value < rhs.value;
  }
  friend constexpr bool operator>(const qboolean &lhs,
                                  const qboolean &rhs) noexcept {
    return lhs.value > rhs.value;
  }
  friend constexpr bool operator<=(const qboolean &lhs,
                                   const qboolean &rhs) noexcept {
    return lhs.value <= rhs.value;
  }
  friend constexpr bool operator>=(const qboolean &lhs,
                                   const qboolean &rhs) noexcept {
    return lhs.value >= rhs.value;
  }
};
#pragma pack(pop)

inline constexpr qboolean qfalse = 0;
inline constexpr qboolean qtrue = 1;

static_assert(sizeof(qboolean) == 4, "qboolean size mismatch: must be 32-bit.");
static_assert(std::is_trivially_copyable<qboolean>::value,
              "qboolean must be trivially copyable for engine interop.");
} // namespace game