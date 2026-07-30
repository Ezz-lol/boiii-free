#pragma once
#include <macros.hpp>
#include <bit>
/*
  `enum`s can't have templated underlying type,
  so we re-create an enum-like interface here with templated `Underlying` type.

  User has to define the type's discretely enumerated values as
  `static inline constexpr`s for equivalency. E.g.:
  ```cpp
  struct NumericValuesFromOneToTwoInclusive: public Enum<uint32_t> {
    static inline constexpr Enum<uint32_t> ONE = {1};
    static inline constexpr Enum<uint32_t> TWO = {2};
  }
  ```
*/
template <IntegralLike Underlying> struct Enum {
#pragma pack(push, 1)
  Underlying __value;
#pragma pack(pop)
private:
  inline constexpr const Underlying &value() const noexcept { return __value; }
  inline constexpr Underlying &value() noexcept { return __value; }

public:
  /*
  If we define any constructors here whatsoever,
  both LLVM's clang and clang-cl, as well as Microsoft's cl.exe will
  implicitly pass this type as a pointer when we compile for the x86-64
  Windows MSVC target - in arguments, and via a return struct in function
  returns.

  See the "Return values" section of Microsoft's x86-64 MSVC calling
  convention documentation [1] for more information.

  Implicit conversion via assignment is the best we can do.

  [1] https://learn.microsoft.com/cpp/build/x64-calling-convention#return-values
  */
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator=(const Rhs &rhs) noexcept {
    value() = static_cast<Underlying>(rhs);
    return value();
  }

  inline constexpr Enum<Underlying> &operator=(const Underlying &rhs) noexcept {
    value() = static_cast<Underlying>(rhs);
    return value();
  }

  // Implicit conversion to any integral type
  template <IntegralLike<Underlying> Out> inline operator Out() const noexcept {
    return std::bit_cast<Out>(__value);
  }

  // Explicit conversion to bool (truthiness)
  inline constexpr explicit operator bool() const noexcept {
    return value() != 0;
  }

  // ==========================================
  // Unary Operators
  // ==========================================
  inline constexpr Enum operator+() const noexcept { return Enum(+value()); }
  inline constexpr Enum operator-() const noexcept { return Enum(-value()); }
  inline constexpr Enum operator~() const noexcept { return Enum(~value()); }
  inline constexpr bool operator!() const noexcept { return !value(); }

  // ==========================================
  // Increment & Decrement
  // ==========================================

  // Pre-increment / Pre-decrement
  inline constexpr Enum &operator++() noexcept {
    ++value();
    return *this;
  }
  inline constexpr Enum &operator--() noexcept {
    --value();
    return *this;
  }

  // Post-increment / Post-decrement
  inline constexpr Enum operator++(int) noexcept {
    Enum temp = *this;
    ++value();
    return temp;
  }
  inline constexpr Enum operator--(int) noexcept {
    Enum temp = *this;
    --value();
    return temp;
  }

  // ==========================================
  // Compound Assignment Operators
  // ==========================================

  // -- Interoperability with IntegralLike<Underlying> types --
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator+=(const Rhs &v) noexcept {
    value() += static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator-=(const Rhs &v) noexcept {
    value() -= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator*=(const Rhs &v) noexcept {
    value() *= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator/=(const Rhs &v) noexcept {
    value() /= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator%=(const Rhs &v) noexcept {
    value() %= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator|=(const Rhs &v) noexcept {
    value() |= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator^=(const Rhs &v) noexcept {
    value() ^= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator&=(const Rhs &v) noexcept {
    value() &= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator<<=(const Rhs &v) noexcept {
    value() <<= static_cast<Underlying>(v);
    return *this;
  }
  template <IntegralLike<Underlying> Rhs>
  inline constexpr Enum &operator>>=(const Rhs &v) noexcept {
    value() >>= static_cast<Underlying>(v);
    return *this;
  }

  // -- Interoperability with other Enums --
  inline constexpr Enum &operator+=(const Enum &v) noexcept {
    value() += v.value();
    return *this;
  }
  inline constexpr Enum &operator-=(const Enum &v) noexcept {
    value() -= v.value();
    return *this;
  }
  inline constexpr Enum &operator*=(const Enum &v) noexcept {
    value() *= v.value();
    return *this;
  }
  inline constexpr Enum &operator/=(const Enum &v) noexcept {
    value() /= v.value();
    return *this;
  }
  inline constexpr Enum &operator%=(const Enum &v) noexcept {
    value() %= v.value();
    return *this;
  }
  inline constexpr Enum &operator|=(const Enum &v) noexcept {
    value() |= v.value();
    return *this;
  }
  inline constexpr Enum &operator^=(const Enum &v) noexcept {
    value() ^= v.value();
    return *this;
  }
  inline constexpr Enum &operator&=(const Enum &v) noexcept {
    value() &= v.value();
    return *this;
  }
  inline constexpr Enum &operator<<=(const Enum &v) noexcept {
    value() <<= v.value();
    return *this;
  }
  inline constexpr Enum &operator>>=(const Enum &v) noexcept {
    value() >>= v.value();
    return *this;
  }

  // Binary Arithmetic & Bitwise Operators

  // Binary +
  friend inline constexpr Enum operator+(Enum lhs, const Enum &rhs) noexcept {
    lhs += rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator+(Enum lhs, const Rhs &rhs) noexcept {
    lhs += rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator+(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res += rhs;
    return res;
  }

  // Binary -
  friend inline constexpr Enum operator-(Enum lhs, const Enum &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator-(Enum lhs, const Rhs &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator-(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res -= rhs;
    return res;
  }

  // Binary *
  friend inline constexpr Enum operator*(Enum lhs, const Enum &rhs) noexcept {
    lhs *= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator*(Enum lhs, const Rhs &rhs) noexcept {
    lhs *= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator*(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res *= rhs;
    return res;
  }

  // Binary /
  friend inline constexpr Enum operator/(Enum lhs, const Enum &rhs) noexcept {
    lhs /= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator/(Enum lhs, const Rhs &rhs) noexcept {
    lhs /= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator/(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res /= rhs;
    return res;
  }

  // Binary %
  friend inline constexpr Enum operator%(Enum lhs, const Enum &rhs) noexcept {
    lhs %= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator%(Enum lhs, const Rhs &rhs) noexcept {
    lhs %= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator%(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res %= rhs;
    return res;
  }

  // Bitwise |
  friend inline constexpr Enum operator|(Enum lhs, const Enum &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator|(Enum lhs, const Rhs &rhs) noexcept {
    lhs |= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator|(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res |= rhs;
    return res;
  }

  // Bitwise &
  friend inline constexpr Enum operator&(Enum lhs, const Enum &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator&(Enum lhs, const Rhs &rhs) noexcept {
    lhs &= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator&(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res &= rhs;
    return res;
  }

  // Bitwise ^
  friend inline constexpr Enum operator^(Enum lhs, const Enum &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator^(Enum lhs, const Rhs &rhs) noexcept {
    lhs ^= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator^(const Lhs &lhs,
                                         const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res ^= rhs;
    return res;
  }

  // Bitwise Shift Left <<
  friend inline constexpr Enum operator<<(Enum lhs, const Enum &rhs) noexcept {
    lhs <<= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator<<(Enum lhs, const Rhs &rhs) noexcept {
    lhs <<= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator<<(const Lhs &lhs,
                                          const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res <<= rhs;
    return res;
  }

  // Bitwise Shift Right >>
  friend inline constexpr Enum operator>>(Enum lhs, const Enum &rhs) noexcept {
    lhs >>= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr Enum operator>>(Enum lhs, const Rhs &rhs) noexcept {
    lhs >>= rhs;
    return lhs;
  }
  template <IntegralLike<Underlying> Lhs>
  friend inline constexpr Enum operator>>(const Lhs &lhs,
                                          const Enum &rhs) noexcept {
    Enum res(static_cast<Underlying>(lhs));
    res >>= rhs;
    return res;
  }

  // Comparisons

  // C++20 default spaceship automatically creates all 6 operators (==, !=, <,
  // >, <=, >=) for Enum vs Enum
  friend inline constexpr auto operator<=>(const Enum &,
                                           const Enum &) noexcept = default;

  // Manual fallback handling to allow symmetric evaluation vs standard integers
  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr bool operator==(const Enum &lhs,
                                          const Rhs &rhs) noexcept {
    return lhs.value() == static_cast<Underlying>(rhs);
  }

  template <IntegralLike<Underlying> Rhs>
  friend inline constexpr auto operator<=>(const Enum &lhs,
                                           const Rhs &rhs) noexcept {
    return lhs.value() <=> static_cast<Underlying>(rhs);
  }
};