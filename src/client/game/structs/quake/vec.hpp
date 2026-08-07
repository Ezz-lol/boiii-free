#pragma once

#include <cmath>
#include <numbers>

#include <game/structs/macros.hpp>
#include <macros.hpp>

namespace game {

typedef float vec_t;

namespace math {
template <typename T> struct constants {
  static inline constexpr T PI_DIV_180 =
      static_cast<T>(std::numbers::pi_v<double> / 180.0);
  static inline constexpr T ONE_DIV_360 = static_cast<T>(1.0 / 360.0);
};
template <typename T> inline constexpr T to_radians(const T deg) {
  return constants<T>::PI_DIV_180 * deg;
}

/*
   Normalizes an angle in degrees to the range [-180, 180] (with 180° mapping
   to -180°)
*/
template <typename T = vec_t>
inline constexpr T normalizeAngle(T angle) noexcept {
  return static_cast<T>(
      static_cast<float>(
          static_cast<float>(angle * constants<float>::ONE_DIV_360) -
          floor(static_cast<float>(angle * constants<float>::ONE_DIV_360) + 0.5f)) *
      360.0f);
}
} // namespace math

template <typename T = vec_t> union vec2 {
  T v[2];
  struct {
    T x;
    T y;
  };

  inline constexpr const T &operator[](size_t index) const { return v[index]; }
  inline constexpr T &operator[](size_t index) { return v[index]; }

  // Compound assignment operators (Vector)
  inline constexpr vec2<T> &operator+=(const vec2<T> o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  inline constexpr vec2<T> &operator-=(const vec2<T> o) {
    x -= o.x;
    y -= o.y;
    return *this;
  }
  inline constexpr vec2<T> &operator*=(const vec2<T> o) {
    x *= o.x;
    y *= o.y;
    return *this;
  }
  inline constexpr vec2<T> &operator/=(const vec2<T> o) {
    x /= o.x;
    y /= o.y;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec2<T> &operator+=(T s) {
    x += s;
    y += s;
    return *this;
  }
  inline constexpr vec2<T> &operator-=(T s) {
    x -= s;
    y -= s;
    return *this;
  }
  inline constexpr vec2<T> &operator*=(T s) {
    x *= s;
    y *= s;
    return *this;
  }
  inline constexpr vec2<T> &operator/=(T s) {
    x /= s;
    y /= s;
    return *this;
  }

  inline constexpr T dot(const vec2<T> b) { return x * b.x + y * b.y; }
  inline constexpr T length_sq() { return dot(this); }
  inline constexpr T length() { return std::sqrt(length_sq()); }
  inline constexpr vec2<T> normalize() {
    T l = length();
    return (l > T(0)) ? (this / l) : this;
  }
  inline constexpr vec2<T> lerp(const vec2<T> b, T t) {
    return this + (b - *this) * t;
  }

  inline constexpr vec2<T> normalizeAngle() const noexcept {
    return {
        .x = math::normalizeAngle(x),
        .y = math::normalizeAngle(y),
    };
  }

  inline constexpr vec2<T> &normalizeAngle() noexcept {
    x = math::normalizeAngle(x);
    y = math::normalizeAngle(y);
    return *this;
  }

  static inline constexpr vec2<T> normalizeAngle(vec2<T> input) noexcept {
    input.normalizeAngle();
    return input;
  }

  inline constexpr vec2<T> lerpAngle(const vec2<T> b,
                                     const T t) const noexcept {
    return *this + normalizeAngle(b - *this) * t;
  }

  /*
    Computes a normalized 2D direction vector from a single Euler angle (yaw) in
    degrees.
  */
  static inline constexpr vec2<T> from_euler(T euler) {
    const T rad = to_radians(euler);
    return {.x = std::cos(rad), .y = std::sin(rad)};
  }

  inline constexpr auto size() const noexcept { return ARRAYSIZE(v); }

  using IndexType = uint8_t;

  inline constexpr void assert_range(IndexType index) const {
    assert(index < size() && "Index to vector must not exceed its size");
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr const T &operator[](const Index index_arg) const {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr T &operator[](const Index index_arg) {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }
};

typedef vec2<vec_t> vec2_t;
ASSERT_SIZE(vec2_t, sizeof(vec_t) * 2);
ASSERT_CPP03_POD(vec2_t);

template <typename T = vec_t> union vec3 {
  T v[3];
  struct {
    T x;
    T y;
    T z;
  };

  vec2<T> xy;

  inline constexpr const T &operator[](size_t index) const { return v[index]; }
  inline constexpr T &operator[](size_t index) { return v[index]; }

  // Compound assignment operators (Vector)
  inline constexpr vec3<T> &operator+=(const vec3<T> o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }
  inline constexpr vec3<T> &operator-=(const vec3<T> o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
  }
  inline constexpr vec3<T> &operator*=(const vec3<T> o) {
    x *= o.x;
    y *= o.y;
    z *= o.z;
    return *this;
  }
  inline constexpr vec3<T> &operator/=(const vec3<T> o) {
    x /= o.x;
    y /= o.y;
    z /= o.z;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec3<T> &operator+=(T s) {
    x += s;
    y += s;
    z += s;
    return *this;
  }
  inline constexpr vec3<T> &operator-=(T s) {
    x -= s;
    y -= s;
    z -= s;
    return *this;
  }
  inline constexpr vec3<T> &operator*=(T s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
  inline constexpr vec3<T> &operator/=(T s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  inline constexpr T dot(const vec3<T> b) {
    return x * b.x + y * b.y + z * b.z;
  }
  inline constexpr vec3<T> cross(const vec3<T> b) {
    vec3<T> r{};
    r.x = y * b.z - z * b.y;
    r.y = z * b.x - x * b.z;
    r.z = x * b.y - y * b.x;
    return r;
  }
  inline constexpr T length_sq() { return dot(this); }
  inline constexpr T length() { return std::sqrt(length_sq()); }
  inline constexpr vec3<T> normalize() {
    T l = length();
    return (l > T(0)) ? (this / l) : this;
  }

  inline constexpr vec3<T> normalizeAngle() const noexcept {
    return {.x = math::normalizeAngle(x),
            .y = math::normalizeAngle(y),
            .z = math::normalizeAngle(z)};
  }

  inline constexpr vec3<T> &normalizeAngle() noexcept {
    x = math::normalizeAngle(x);
    y = math::normalizeAngle(y);
    z = math::normalizeAngle(z);
    return *this;
  }

  static inline constexpr vec3<T> fill(T val) noexcept {
    return {.x = val, .y = val, .z = val};
  }

  static inline constexpr vec3<T> normalizeAngle(vec3<T> input) noexcept {
    return input.normalizeAngle();
  }

  inline constexpr vec3<T> &lerp(const vec3<T> b, const T t) {
    *this += (b - *this) * t;
    return this;
  }

  inline constexpr vec3<T> lerp(const vec3<T> *b, const T t) const {
    return *this + (*b - *this) * t;
  }

  inline constexpr vec3<T> lerpAngle(const vec3<T> b,
                                     const T t) const noexcept {
    return *this + normalizeAngle(b - *this) * t;
  }

  /*
    Computes a normalized 3D forward direction vector from Euler angles
    (pitch, yaw, roll) in degrees. Assumes euler.x = pitch and euler.y = yaw.
    Roll (euler.z) is ignored for forward direction.
  */
  static inline constexpr vec3<T> from_euler(const vec3<T> euler) {
    const T pitch = game::math::to_radians<T>(euler.x);
    const T yaw = game::math::to_radians<T>(euler.y);

    const T cp = std::cos(pitch);
    return {.x = cp * std::cos(yaw),
            .y = cp * std::sin(yaw),
            .z = -std::sin(pitch)};
  }

  inline constexpr auto size() const noexcept { return ARRAYSIZE(v); }

  using IndexType = uint8_t;

  inline constexpr void assert_range(IndexType index) const {
    assert(index < size() && "Index to vector must not exceed its size");
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr const T &operator[](const Index index_arg) const {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr T &operator[](const Index index_arg) {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }
};

typedef vec3<vec_t> vec3_t;
ASSERT_SIZE(vec3_t, sizeof(vec_t) * 3);
ASSERT_CPP03_POD(vec3_t);

template <typename T = vec_t> union vec4 {
  T v[4];
  struct {
    T x;
    T y;
    T z;
    T w;
  };
  struct {
    T r;
    T g;
    T b;
    T a;
  };
  vec2<T> xy;
  vec3<T> xyz;

  inline constexpr const T &operator[](size_t index) const { return v[index]; }
  inline constexpr T &operator[](size_t index) { return v[index]; }

  // Compound assignment operators (Vector)
  inline constexpr vec4<T> &operator+=(const vec4<T> o) {
    x += o.x;
    y += o.y;
    z += o.z;
    w += o.w;
    return *this;
  }
  inline constexpr vec4<T> &operator-=(const vec4<T> o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    w -= o.w;
    return *this;
  }
  inline constexpr vec4<T> &operator*=(const vec4<T> o) {
    x *= o.x;
    y *= o.y;
    z *= o.z;
    w *= o.w;
    return *this;
  }
  inline constexpr vec4<T> &operator/=(const vec4<T> o) {
    x /= o.x;
    y /= o.y;
    z /= o.z;
    w /= o.w;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec4<T> &operator+=(T s) {
    x += s;
    y += s;
    z += s;
    w += s;
    return *this;
  }
  inline constexpr vec4<T> &operator-=(T s) {
    x -= s;
    y -= s;
    z -= s;
    w -= s;
    return *this;
  }
  inline constexpr vec4<T> &operator*=(T s) {
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
  }
  inline constexpr vec4<T> &operator/=(T s) {
    x /= s;
    y /= s;
    z /= s;
    w /= s;
    return *this;
  }

  inline constexpr T dot(const vec4<T> b) {
    return x * b.x + y * b.y + z * b.z + w * b.w;
  }
  inline constexpr T length_sq() { return dot(this); }
  inline constexpr T length() { return std::sqrt(length_sq()); }
  inline constexpr vec4<T> normalize() {
    T l = length();
    return (l > T(0)) ? (this / l) : this;
  }
  inline constexpr vec4<T> lerp(const vec4<T> b, T t) {
    return this + (b - *this) * t;
  }

  inline constexpr vec4<T> normalizeAngle() const noexcept {
    return {.x = math::normalizeAngle(x),
            .y = math::normalizeAngle(y),
            .z = math::normalizeAngle(z),
            .w = normalizeAngle(w)};
  }

  inline constexpr vec4<T> &normalizeAngle() noexcept {
    x = math::normalizeAngle(x);
    y = math::normalizeAngle(y);
    z = math::normalizeAngle(z);
    w = normalizeAngle(w);
    return *this;
  }

  static inline constexpr vec4<T> normalizeAngle(vec4<T> input) noexcept {
    input.normalizeAngle();
    return input;
  }

  inline constexpr vec4<T> lerpAngle(const vec4<T> b,
                                     const T t) const noexcept {
    return *this + normalizeAngle(b - *this) * t;
  }

  /*
    Computes a quaternion (x, y, z, w) representing a 3D rotation from Euler
    angles (pitch, yaw, roll) in degrees. Assumes euler.x = pitch, euler.y =
    yaw, euler.z = roll.
  */
  static inline constexpr vec4<T> from_euler(const vec3<T> euler) {
    const T p = game::math::to_radians<T>(euler.x) * static_cast<T>(0.5);
    const T y = game::math::to_radians<T>(euler.y) * static_cast<T>(0.5);
    const T r = game::math::to_radians(euler.z) * static_cast<T>(0.5);

    const T cp = std::cos(p), sp = std::sin(p);
    const T cy = std::cos(y), sy = std::sin(y);
    const T cr = std::cos(r), sr = std::sin(r);

    return {.x = sr * cp * cy - cr * sp * sy,
            .y = cr * sp * cy + sr * cp * sy,
            .z = cr * cp * sy - sr * sp * cy,
            .w = cr * cp * cy + sr * sp * sy};
  }

  inline constexpr auto size() const noexcept { return ARRAYSIZE(v); }

  using IndexType = uint8_t;

  inline constexpr void assert_range(IndexType index) const {
    assert(index < size() && "Index to vector must not exceed its size");
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr const T &operator[](const Index index_arg) const {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }

  template <IntegralLike<IndexType> Index>
  inline constexpr T &operator[](const Index index_arg) {
    const IndexType index = static_cast<IndexType>(index_arg);
    assert_range(index);
    return v[index];
  }
};

typedef vec4<vec_t> vec4_t;
ASSERT_SIZE(vec4_t, sizeof(vec_t) * 4);
ASSERT_CPP03_POD(vec4_t);

/*
  vec2: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec2<T> operator-(const vec2<T> v) {
  vec2<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec2<T> operator+(const vec2<T> a, const vec2<T> b) {
  vec2<T> r = a;
  r += b;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator-(const vec2<T> a, const vec2<T> b) {
  vec2<T> r = a;
  r -= b;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator*(const vec2<T> a, const vec2<T> b) {
  vec2<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator/(const vec2<T> a, const vec2<T> b) {
  vec2<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  return r;
}

// Binary Scalar
template <typename T> inline constexpr vec2<T> operator+(const vec2<T> a, T s) {
  vec2<T> r = a;
  a.x += s;
  a.y += s;
  return r;
}
template <typename T> inline constexpr vec2<T> operator-(const vec2<T> a, T s) {
  vec2<T> r = a;
  r.x -= s;
  r.y -= s;
  return r;
}
template <typename T> inline constexpr vec2<T> operator*(const vec2<T> a, T s) {
  vec2<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  return r;
}
template <typename T> inline constexpr vec2<T> operator/(const vec2<T> a, T s) {
  vec2<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  return r;
}
template <typename T> inline constexpr vec2<T> operator*(T s, const vec2<T> a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec2<T> a, const vec2<T> b) {
  return a.x == b.x && a.y == b.y;
}
template <typename T>
inline constexpr bool operator!=(const vec2<T> a, const vec2<T> b) {
  return !(a == b);
}

// Math utility functions
template <typename T> inline constexpr T dot(const vec2<T> a, const vec2<T> b) {
  return a.dot(b);
}
template <typename T> inline constexpr T length_sq(const vec2<T> v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec2<T> v) {
  return v.length();
}
template <typename T> inline constexpr vec2<T> normalize(const vec2<T> v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec2<T> lerp(const vec2<T> a, const vec2<T> b, T t) {
  return a.lerp(b, t);
}

/*
  vec3: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec3<T> operator-(const vec3<T> v) {
  vec3<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  r.z = -v.z;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec3<T> operator+(const vec3<T> a, const vec3<T> b) {
  vec3<T> r = a;
  r += b;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> a, const vec3<T> b) {
  vec3<T> r = a;
  r -= b;
  return r;
}

template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> *a, const vec3<T> b) {
  vec3<T> r = *a;
  r -= b;
  return r;
}

template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> a, const vec3<T> *b) {
  vec3<T> r = a;
  r -= *b;
  return r;
}

template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> *a, const vec3<T> *b) {
  vec3<T> r = *a;
  r -= *b;
  return r;
}

template <typename T>
inline constexpr vec3<T> operator*(const vec3<T> a, const vec3<T> b) {
  vec3<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  r.z = a.z * b.z;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator/(const vec3<T> a, const vec3<T> b) {
  vec3<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  r.z = a.z / b.z;
  return r;
}

// Binary Scalar
template <typename T> inline constexpr vec3<T> operator+(const vec3<T> a, T s) {
  vec3<T> r = a;
  r.x += s;
  r.y += s;
  r.z += s;
  return r;
}
template <typename T> inline constexpr vec3<T> operator-(const vec3<T> a, T s) {
  vec3<T> r = a;
  r.x -= s;
  r.y -= s;
  r.z -= s;
  return r;
}

template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> *a, T s) {
  return *a - s;
}
template <typename T> inline constexpr vec3<T> operator*(const vec3<T> a, T s) {
  vec3<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  r.z = a.z * s;
  return r;
}
template <typename T> inline constexpr vec3<T> operator/(const vec3<T> a, T s) {
  vec3<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  r.z = a.z / s;
  return r;
}
template <typename T> inline constexpr vec3<T> operator*(T s, const vec3<T> a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec3<T> a, const vec3<T> b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}
template <typename T>
inline constexpr bool operator!=(const vec3<T> a, const vec3<T> b) {
  return !(a == b);
}

// Math utility functions
template <typename T> inline constexpr T dot(const vec3<T> a, const vec3<T> b) {
  return a.dot(b);
}
template <typename T>
inline constexpr vec3<T> cross(const vec3<T> a, const vec3<T> b) {
  return a.cross(b);
}
template <typename T> inline constexpr T length_sq(const vec3<T> v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec3<T> v) {
  return v.length();
}
template <typename T> inline constexpr vec3<T> normalize(const vec3<T> v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec3<T> lerp(const vec3<T> a, const vec3<T> b, T t) {
  return a.lerp(b, t);
}

/*
  vec4: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec4<T> operator-(const vec4<T> v) {
  vec4<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  r.z = -v.z;
  r.w = -v.w;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec4<T> operator+(const vec4<T> a, const vec4<T> b) {
  vec4<T> r = a;
  r += b;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator-(const vec4<T> a, const vec4<T> b) {
  vec4<T> r = a;
  r -= b;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator*(const vec4<T> a, const vec4<T> b) {
  vec4<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  r.z = a.z * b.z;
  r.w = a.w * b.w;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator/(const vec4<T> a, const vec4<T> b) {
  vec4<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  r.z = a.z / b.z;
  r.w = a.w / b.w;
  return r;
}

// Binary Scalar
template <typename T> inline constexpr vec4<T> operator+(const vec4<T> a, T s) {
  vec4<T> r = a;
  r.x += s;
  r.y += s;
  r.z += s;
  r.w += s;
  return r;
}
template <typename T> inline constexpr vec4<T> operator-(const vec4<T> a, T s) {
  vec4<T> r = a;
  r.x -= s;
  r.y -= s;
  r.z -= s;
  r.w -= s;
  return r;
}
template <typename T> inline constexpr vec4<T> operator*(const vec4<T> a, T s) {
  vec4<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  r.z = a.z * s;
  r.w = a.w * s;
  return r;
}
template <typename T> inline constexpr vec4<T> operator/(const vec4<T> a, T s) {
  vec4<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  r.z = a.z / s;
  r.w = a.w / s;
  return r;
}
template <typename T> inline constexpr vec4<T> operator*(T s, const vec4<T> a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec4<T> a, const vec4<T> b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
template <typename T>
inline constexpr bool operator!=(const vec4<T> a, const vec4<T> b) {
  return !(a == b);
}

// Math utility functions
template <typename T> inline constexpr T dot(const vec4<T> a, const vec4<T> b) {
  return a.dot(b);
}
template <typename T> inline constexpr T length_sq(const vec4<T> v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec4<T> v) {
  return v.length();
}
template <typename T> inline constexpr vec4<T> normalize(const vec4<T> v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec4<T> lerp(const vec4<T> a, const vec4<T> b, T t) {
  return a.lerp(b, t);
}

} // namespace game