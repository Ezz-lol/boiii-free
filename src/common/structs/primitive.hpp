#pragma once

#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

// Feature detection for platform native 128-bit floating-point support
#if defined(__FLOAT128__) || defined(__SIZEOF_FLOAT128__) ||                   \
    defined(__HAVE_FLOAT128)
#define HAS_NATIVE_FLOAT128 1
using native_float128_t = __float128;
#elif defined(__FLT128_MANT_DIG__)
#define HAS_NATIVE_FLOAT128 1
using native_float128_t = _Float128;
#else
#define HAS_NATIVE_FLOAT128 0
#endif

class float128;

namespace detail {

// Compile-time & runtime string parser for float128 literals
inline constexpr float128 parse_float128(std::string_view sv) noexcept;

struct DoubleDouble {
  double hi{0.0};
  double lo{0.0};

  inline constexpr bool operator==(const DoubleDouble &rhs) const noexcept {
    return hi == rhs.hi && lo == rhs.lo;
  }
};

} // namespace detail

class float128 {
private:
#if HAS_NATIVE_FLOAT128
  native_float128_t value_{0.0};
#else
  detail::DoubleDouble value_{0.0, 0.0};
#endif

public:
  // Constructors
  inline constexpr float128() noexcept = default;

  // High and Low double initializer
  inline constexpr float128(double hi, double lo) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ =
        static_cast<native_float128_t>(hi) + static_cast<native_float128_t>(lo);
#else
    value_.hi = hi;
    value_.lo = lo;
#endif
  }

  template <std::integral T> constexpr float128(T val) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ = static_cast<native_float128_t>(val);
#else
    value_.hi = static_cast<double>(val);
    value_.lo = 0.0;
#endif
  }

  template <std::floating_point T> constexpr float128(T val) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ = static_cast<native_float128_t>(val);
#else
    value_.hi = static_cast<double>(val);
    value_.lo = 0.0;
#endif
  }

  inline constexpr explicit float128(std::string_view sv) noexcept {
    *this = detail::parse_float128(sv);
  }

  explicit float128(const std::string &s) noexcept {
    *this = detail::parse_float128(std::string_view(s));
  }

  explicit float128(const char *s) noexcept {
    *this = detail::parse_float128(std::string_view(s));
  }

  // Underlying Value Accessors
#if HAS_NATIVE_FLOAT128
  inline constexpr native_float128_t native_value() const noexcept {
    return value_;
  }
#else
  inline constexpr double hi() const noexcept { return value_.hi; }
  inline constexpr double lo() const noexcept { return value_.lo; }
#endif

  // Explicit Conversions
  explicit constexpr operator float() const noexcept {
#if HAS_NATIVE_FLOAT128
    return static_cast<float>(value_);
#else
    return static_cast<float>(value_.hi);
#endif
  }

  explicit constexpr operator double() const noexcept {
#if HAS_NATIVE_FLOAT128
    return static_cast<double>(value_);
#else
    return value_.hi;
#endif
  }

  explicit constexpr operator long double() const noexcept {
#if HAS_NATIVE_FLOAT128
    return static_cast<long double>(value_);
#else
    return static_cast<long double>(value_.hi);
#endif
  }

  // Unary Operators
  inline constexpr float128 operator+() const noexcept { return *this; }

  inline constexpr float128 operator-() const noexcept {
#if HAS_NATIVE_FLOAT128
    float128 res;
    res.value_ = -value_;
    return res;
#else
    return float128(-value_.hi, -value_.lo);
#endif
  }

  // Compound Assignment Operators
  inline constexpr float128 &operator+=(const float128 &rhs) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ += rhs.value_;
#else
    double s1 = value_.hi + rhs.value_.hi;
    double v = s1 - value_.hi;
    double e1 = (value_.hi - (s1 - v)) + (rhs.value_.hi - v);
    double e2 = e1 + value_.lo + rhs.value_.lo;
    value_.hi = s1 + e2;
    value_.lo = e2 - (value_.hi - s1);
#endif
    return *this;
  }

  inline constexpr float128 &operator-=(const float128 &rhs) noexcept {
    return *this += (-rhs);
  }

  inline constexpr float128 &operator*=(const float128 &rhs) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ *= rhs.value_;
#else
    double p1 = value_.hi * rhs.value_.hi;
    double p2 = value_.hi * rhs.value_.lo + value_.lo * rhs.value_.hi;
    value_.hi = p1 + p2;
    value_.lo = p2 - (value_.hi - p1);
#endif
    return *this;
  }

  inline constexpr float128 &operator/=(const float128 &rhs) noexcept {
#if HAS_NATIVE_FLOAT128
    value_ /= rhs.value_;
#else
    double q1 = value_.hi / rhs.value_.hi;
    double r = value_.hi - q1 * rhs.value_.hi;
    double q2 = (r + value_.lo - q1 * rhs.value_.lo) / rhs.value_.hi;
    value_.hi = q1 + q2;
    value_.lo = q2 - (value_.hi - q1);
#endif
    return *this;
  }

  // Binary Arithmetic Operators
  inline friend constexpr float128 operator+(float128 lhs,
                                             const float128 &rhs) noexcept {
    lhs += rhs;
    return lhs;
  }
  inline friend constexpr float128 operator-(float128 lhs,
                                             const float128 &rhs) noexcept {
    lhs -= rhs;
    return lhs;
  }
  inline friend constexpr float128 operator*(float128 lhs,
                                             const float128 &rhs) noexcept {
    lhs *= rhs;
    return lhs;
  }
  inline friend constexpr float128 operator/(float128 lhs,
                                             const float128 &rhs) noexcept {
    lhs /= rhs;
    return lhs;
  }

  // Increment / Decrement
  inline constexpr float128 &operator++() noexcept {
    *this += float128(1.0);
    return *this;
  }
  inline constexpr float128 operator++(int32_t) noexcept {
    float128 tmp = *this;
    ++(*this);
    return tmp;
  }
  inline constexpr float128 &operator--() noexcept {
    *this -= float128(1.0);
    return *this;
  }
  inline constexpr float128 operator--(int32_t) noexcept {
    float128 tmp = *this;
    --(*this);
    return tmp;
  }

  // C++20 Comparisons
  inline constexpr bool operator==(const float128 &rhs) const noexcept {
#if HAS_NATIVE_FLOAT128
    return value_ == rhs.value_;
#else
    return value_ == rhs.value_;
#endif
  }

  inline constexpr std::partial_ordering
  operator<=>(const float128 &rhs) const noexcept {
#if HAS_NATIVE_FLOAT128
    if (value_ < rhs.value_)
      return std::partial_ordering::less;
    if (value_ > rhs.value_)
      return std::partial_ordering::greater;
    if (value_ == rhs.value_)
      return std::partial_ordering::equivalent;
    return std::partial_ordering::unordered;
#else
    if (value_.hi != value_.hi || rhs.value_.hi != rhs.value_.hi)
      return std::partial_ordering::unordered;
    if (value_.hi < rhs.value_.hi)
      return std::partial_ordering::less;
    if (value_.hi > rhs.value_.hi)
      return std::partial_ordering::greater;
    if (value_.lo < rhs.value_.lo)
      return std::partial_ordering::less;
    if (value_.lo > rhs.value_.lo)
      return std::partial_ordering::greater;
    return std::partial_ordering::equivalent;
#endif
  }

  // Stream I/O
  friend std::ostream &operator<<(std::ostream &os, const float128 &f) {
    os << static_cast<long double>(f);
    return os;
  }

  friend std::istream &operator>>(std::istream &is, float128 &f) {
    std::string s;
    if (is >> s) {
      f = float128(s);
    }
    return is;
  }
};

namespace detail {

// Compile-time string parser implementation
inline constexpr float128 parse_float128(std::string_view sv) noexcept {
  if (sv.empty())
    return float128(0.0);

  std::size_t idx = 0;
  while (idx < sv.size() && (sv[idx] == ' ' || sv[idx] == '\t' ||
                             sv[idx] == '\n' || sv[idx] == '\r')) {
    ++idx;
  }
  if (idx >= sv.size())
    return float128(0.0);

  bool negative = false;
  if (sv[idx] == '-') {
    negative = true;
    ++idx;
  } else if (sv[idx] == '+') {
    ++idx;
  }

  // Handle Inf / NaN
  if (idx + 3 <= sv.size()) {
    if ((sv[idx] == 'i' || sv[idx] == 'I') &&
        (sv[idx + 1] == 'n' || sv[idx + 1] == 'N') &&
        (sv[idx + 2] == 'f' || sv[idx + 2] == 'F')) {
      float128 inf = float128(std::numeric_limits<double>::infinity());
      return negative ? -inf : inf;
    }
    if ((sv[idx] == 'n' || sv[idx] == 'N') &&
        (sv[idx + 1] == 'a' || sv[idx + 1] == 'A') &&
        (sv[idx + 2] == 'n' || sv[idx + 2] == 'N')) {
      return float128(std::numeric_limits<double>::quiet_NaN());
    }
  }

  float128 mantissa(0.0);
  int32_t dec_places = 0;
  bool has_dec = false;

  while (idx < sv.size()) {
    char c = sv[idx];
    if (c >= '0' && c <= '9') {
      mantissa = mantissa * float128(10.0) + float128(c - '0');
      if (has_dec)
        ++dec_places;
    } else if (c == '.') {
      if (has_dec)
        break;
      has_dec = true;
    } else {
      break;
    }
    ++idx;
  }

  int32_t exp = 0;
  if (idx < sv.size() && (sv[idx] == 'e' || sv[idx] == 'E')) {
    ++idx;
    bool exp_neg = false;
    if (idx < sv.size() && sv[idx] == '-') {
      exp_neg = true;
      ++idx;
    } else if (idx < sv.size() && sv[idx] == '+') {
      ++idx;
    }
    while (idx < sv.size() && sv[idx] >= '0' && sv[idx] <= '9') {
      exp = exp * 10 + (sv[idx] - '0');
      ++idx;
    }
    if (exp_neg)
      exp = -exp;
  }

  exp -= dec_places;

  float128 result = mantissa;
  if (exp > 0) {
    float128 base(10.0);
    int32_t e = exp;
    while (e > 0) {
      if (e & 1)
        result *= base;
      base *= base;
      e >>= 1;
    }
  } else if (exp < 0) {
    float128 base(10.0);
    int32_t e = -exp;
    while (e > 0) {
      if (e & 1)
        result /= base;
      base *= base;
      e >>= 1;
    }
  }

  return negative ? -result : result;
}

} // namespace detail

// User-Defined Literals
inline constexpr float128 operator""_f128(long double val) noexcept {
  return float128(static_cast<double>(val));
}

inline constexpr float128 operator""_f128(unsigned long long val) noexcept {
  return float128(val);
}

inline constexpr float128 operator""_f128(const char *str,
                                          std::size_t len) noexcept {
  return float128(std::string_view(str, len));
}

// Raw Literal Operator: Handles unquoted numeric literals that exceed long
// double bounds (e.g. 1e4932_f128)
inline constexpr float128 operator""_f128(const char *str) noexcept {
  return float128(std::string_view(str));
}

// Auxiliary Math Functions
inline constexpr float128 abs(const float128 &f) noexcept {
  return (f < float128(0)) ? -f : f;
}

inline constexpr float128 sqrt(const float128 &f) noexcept {
  if (f <= float128(0))
    return float128(0);
#if HAS_NATIVE_FLOAT128 &&                                                     \
    (defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 7))
  return float128(__builtin_sqrtf128(f.native_value()));
#else
  // Newton-Raphson precision doubling iteration
  double hi_approx = std::sqrt(static_cast<double>(f));
  float128 g(hi_approx);
  g = float128(0.5) * (g + f / g);
  g = float128(0.5) * (g + f / g);
  return g;
#endif
}

inline constexpr bool isnan(const float128 &f) noexcept {
#if HAS_NATIVE_FLOAT128
  return f.native_value() != f.native_value();
#else
  return std::isnan(f.hi()) || std::isnan(f.lo());
#endif
}

inline constexpr bool isinf(const float128 &f) noexcept {
#if HAS_NATIVE_FLOAT128
  return static_cast<double>(f.native_value()) ==
             std::numeric_limits<double>::infinity() ||
         static_cast<double>(f.native_value()) ==
             -std::numeric_limits<double>::infinity();
#else
  return std::isinf(f.hi());
#endif
}

inline constexpr bool isfinite(const float128 &f) noexcept {
  return !isnan(f) && !isinf(f);
}

// Specialization of std::numeric_limits
template <> class std::numeric_limits<float128> {
public:
  static inline constexpr bool is_specialized = true;
  static inline constexpr bool is_signed = true;
  static inline constexpr bool is_integer = false;
  static inline constexpr bool is_exact = false;
  static inline constexpr int32_t radix = 2;
  static inline constexpr bool has_infinity = true;
  static inline constexpr bool has_quiet_NaN = true;
  static inline constexpr bool has_signaling_NaN = true;
  static inline constexpr std::float_denorm_style has_denorm =
      std::denorm_present;
  static inline constexpr bool has_denorm_loss = true;
  static inline constexpr std::float_round_style round_style =
      std::round_to_nearest;
  static inline constexpr bool is_bounded = true;
  static inline constexpr bool is_modulo = false;
  static inline constexpr bool traps = false;
  static inline constexpr bool tinyness_before = false;

#if HAS_NATIVE_FLOAT128
  static inline constexpr bool is_iec559 = true;
  static inline constexpr int32_t digits = 113;
  static inline constexpr int32_t digits10 = 34;
  static inline constexpr int32_t max_digits10 = 36;
  static inline constexpr int32_t min_exponent = -16381;
  static inline constexpr int32_t min_exponent10 = -4931;
  static inline constexpr int32_t max_exponent = 16384;
  static inline constexpr int32_t max_exponent10 = 4932;

  static inline constexpr float128 min() noexcept {
#if defined(__FLT128_MIN__)
    return float128(static_cast<native_float128_t>(__FLT128_MIN__));
#else
    return float128(3.36210314311209350626267781732175260e-4932_f128);
#endif
  }

  static inline constexpr float128 max() noexcept {
#if defined(__FLT128_MAX__)
    return float128(static_cast<native_float128_t>(__FLT128_MAX__));
#else
    return float128(1.18973149535723176508575932662800702e+4932_f128);
#endif
  }

  static inline constexpr float128 lowest() noexcept { return -max(); }

  static inline constexpr float128 epsilon() noexcept {
#if defined(__FLT128_EPSILON__)
    return float128(static_cast<native_float128_t>(__FLT128_EPSILON__));
#else
    return float128(1.92592994438723585305597794258492732e-34_f128);
#endif
  }

  static inline constexpr float128 denorm_min() noexcept {
#if defined(__FLT128_DENORM_MIN__)
    return float128(static_cast<native_float128_t>(__FLT128_DENORM_MIN__));
#else
    return float128(6.47517511943802511092443895822764655e-4966_f128);
#endif
  }

  static inline constexpr float128 infinity() noexcept {
    return float128(static_cast<native_float128_t>(
        std::numeric_limits<double>::infinity()));
  }

  static inline constexpr float128 quiet_NaN() noexcept {
    return float128(static_cast<native_float128_t>(
        std::numeric_limits<double>::quiet_NaN()));
  }

  static inline constexpr float128 signaling_NaN() noexcept {
    return float128(static_cast<native_float128_t>(
        std::numeric_limits<double>::signaling_NaN()));
  }

#else // Double-Double Portable Fallback
  static inline constexpr bool is_iec559 = false;
  static inline constexpr int32_t digits = 106;
  static inline constexpr int32_t digits10 = 31;
  static inline constexpr int32_t max_digits10 = 33;
  static inline constexpr int32_t min_exponent =
      std::numeric_limits<double>::min_exponent;
  static inline constexpr int32_t min_exponent10 =
      std::numeric_limits<double>::min_exponent10;
  static inline constexpr int32_t max_exponent =
      std::numeric_limits<double>::max_exponent;
  static inline constexpr int32_t max_exponent10 =
      std::numeric_limits<double>::max_exponent10;

  static inline constexpr float128 min() noexcept {
    return float128(std::numeric_limits<double>::min(), 0.0);
  }

  static inline constexpr float128 max() noexcept {
    return float128(1.79769313486231570815e+308, 1.99584030953471981165e+292);
  }

  static inline constexpr float128 lowest() noexcept {
    return float128(-1.79769313486231570815e+308, -1.99584030953471981165e+292);
  }

  static inline constexpr float128 epsilon() noexcept {
    return float128(2.46519032881566189191e-32, 0.0);
  }

  static inline constexpr float128 denorm_min() noexcept {
    return float128(std::numeric_limits<double>::denorm_min(), 0.0);
  }

  static inline constexpr float128 infinity() noexcept {
    return float128(std::numeric_limits<double>::infinity(), 0.0);
  }

  static inline constexpr float128 quiet_NaN() noexcept {
    return float128(std::numeric_limits<double>::quiet_NaN(), 0.0);
  }

  static inline constexpr float128 signaling_NaN() noexcept {
    return float128(std::numeric_limits<double>::signaling_NaN(), 0.0);
  }
#endif

  static inline constexpr float128 round_error() noexcept {
    return float128(0.5);
  }
};

typedef float128 float128_t;
