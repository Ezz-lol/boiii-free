#pragma once

#include <cmath>

#include "../macros.hpp"

namespace game {
typedef float vec_t;

template <typename T = vec_t> union vec2 {
  T v[2];
  struct {
    T x;
    T y;
  };

  inline constexpr const T &operator[](size_t index) const { return v[index]; }
  inline constexpr T &operator[](size_t index) { return v[index]; }

  // Compound assignment operators (Vector)
  inline constexpr vec2 &operator+=(const vec2 &o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  inline constexpr vec2 &operator-=(const vec2 &o) {
    x -= o.x;
    y -= o.y;
    return *this;
  }
  inline constexpr vec2 &operator*=(const vec2 &o) {
    x *= o.x;
    y *= o.y;
    return *this;
  }
  inline constexpr vec2 &operator/=(const vec2 &o) {
    x /= o.x;
    y /= o.y;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec2 &operator+=(T s) {
    x += s;
    y += s;
    return *this;
  }
  inline constexpr vec2 &operator-=(T s) {
    x -= s;
    y -= s;
    return *this;
  }
  inline constexpr vec2 &operator*=(T s) {
    x *= s;
    y *= s;
    return *this;
  }
  inline constexpr vec2 &operator/=(T s) {
    x /= s;
    y /= s;
    return *this;
  }

  inline constexpr T dot(const vec2<T> &b) { return x * b.x + y * b.y; }
  inline constexpr T length_sq() { return dot(this); }
  inline constexpr T length() { return std::sqrt(length_sq()); }
  inline constexpr vec2<T> normalize() {
    T l = length();
    return (l > T(0)) ? (this / l) : this;
  }
  inline constexpr vec2<T> lerp(const vec2<T> &b, T t) {
    return this + (b - this) * t;
  }
};

typedef vec2<vec_t> vec2_t;
ASSERT_SIZE(vec2_t, sizeof(vec_t) * 2);
ASSERT_POD(vec2_t);

template <typename T = vec_t> union vec3 {
  struct {
    T x;
    T y;
    T z;
  };

  vec2<T> xy;
  T v[3];

  inline constexpr const T &operator[](size_t index) const { return v[index]; }
  inline constexpr T &operator[](size_t index) { return v[index]; }

  // Compound assignment operators (Vector)
  inline constexpr vec3 &operator+=(const vec3 &o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }
  inline constexpr vec3 &operator-=(const vec3 &o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
  }
  inline constexpr vec3 &operator*=(const vec3 &o) {
    x *= o.x;
    y *= o.y;
    z *= o.z;
    return *this;
  }
  inline constexpr vec3 &operator/=(const vec3 &o) {
    x /= o.x;
    y /= o.y;
    z /= o.z;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec3 &operator+=(T s) {
    x += s;
    y += s;
    z += s;
    return *this;
  }
  inline constexpr vec3 &operator-=(T s) {
    x -= s;
    y -= s;
    z -= s;
    return *this;
  }
  inline constexpr vec3 &operator*=(T s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
  inline constexpr vec3 &operator/=(T s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  inline constexpr T dot(const vec3<T> &b) {
    return x * b.x + y * b.y + z * b.z;
  }
  inline constexpr vec3<T> cross(const vec3<T> &b) {
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
  inline constexpr vec3<T> lerp(const vec3<T> &b, T t) {
    return this + (b - this) * t;
  }
};

typedef vec3<vec_t> vec3_t;
ASSERT_SIZE(vec3_t, sizeof(vec_t) * 3);
ASSERT_POD(vec3_t);

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
  inline constexpr vec4 &operator+=(const vec4 &o) {
    x += o.x;
    y += o.y;
    z += o.z;
    w += o.w;
    return *this;
  }
  inline constexpr vec4 &operator-=(const vec4 &o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    w -= o.w;
    return *this;
  }
  inline constexpr vec4 &operator*=(const vec4 &o) {
    x *= o.x;
    y *= o.y;
    z *= o.z;
    w *= o.w;
    return *this;
  }
  inline constexpr vec4 &operator/=(const vec4 &o) {
    x /= o.x;
    y /= o.y;
    z /= o.z;
    w /= o.w;
    return *this;
  }

  // Compound assignment operators (Scalar)
  inline constexpr vec4 &operator+=(T s) {
    x += s;
    y += s;
    z += s;
    w += s;
    return *this;
  }
  inline constexpr vec4 &operator-=(T s) {
    x -= s;
    y -= s;
    z -= s;
    w -= s;
    return *this;
  }
  inline constexpr vec4 &operator*=(T s) {
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
  }
  inline constexpr vec4 &operator/=(T s) {
    x /= s;
    y /= s;
    z /= s;
    w /= s;
    return *this;
  }

  inline constexpr T dot(const vec4<T> &b) {
    return x * b.x + y * b.y + z * b.z + w * b.w;
  }
  inline constexpr T length_sq() { return dot(this); }
  inline constexpr T length() { return std::sqrt(length_sq()); }
  inline constexpr vec4<T> normalize() {
    T l = length();
    return (l > T(0)) ? (this / l) : this;
  }
  inline constexpr vec4<T> lerp(const vec4<T> &b, T t) {
    return this + (b - this) * t;
  }
};

typedef vec4<vec_t> vec4_t;
ASSERT_SIZE(vec4_t, sizeof(vec_t) * 4);
ASSERT_POD(vec4_t);

/*
  vec2: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec2<T> operator-(const vec2<T> &v) {
  vec2<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec2<T> operator+(const vec2<T> &a, const vec2<T> &b) {
  vec2<T> r{};
  r.x = a.x + b.x;
  r.y = a.y + b.y;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator-(const vec2<T> &a, const vec2<T> &b) {
  vec2<T> r{};
  r.x = a.x - b.x;
  r.y = a.y - b.y;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator*(const vec2<T> &a, const vec2<T> &b) {
  vec2<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator/(const vec2<T> &a, const vec2<T> &b) {
  vec2<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  return r;
}

// Binary Scalar
template <typename T>
inline constexpr vec2<T> operator+(const vec2<T> &a, T s) {
  vec2<T> r{};
  r.x = a.x + s;
  r.y = a.y + s;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator-(const vec2<T> &a, T s) {
  vec2<T> r{};
  r.x = a.x - s;
  r.y = a.y - s;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator*(const vec2<T> &a, T s) {
  vec2<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator/(const vec2<T> &a, T s) {
  vec2<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  return r;
}
template <typename T>
inline constexpr vec2<T> operator*(T s, const vec2<T> &a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec2<T> &a, const vec2<T> &b) {
  return a.x == b.x && a.y == b.y;
}
template <typename T>
inline constexpr bool operator!=(const vec2<T> &a, const vec2<T> &b) {
  return !(a == b);
}

// Math utility functions
template <typename T>
inline constexpr T dot(const vec2<T> &a, const vec2<T> &b) {
  return a.dot(b);
}
template <typename T> inline constexpr T length_sq(const vec2<T> &v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec2<T> &v) {
  return v.length();
}
template <typename T> inline constexpr vec2<T> normalize(const vec2<T> &v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec2<T> lerp(const vec2<T> &a, const vec2<T> &b, T t) {
  return a.lerp(b, t);
}

/*
  vec3: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec3<T> operator-(const vec3<T> &v) {
  vec3<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  r.z = -v.z;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec3<T> operator+(const vec3<T> &a, const vec3<T> &b) {
  vec3<T> r{};
  r.x = a.x + b.x;
  r.y = a.y + b.y;
  r.z = a.z + b.z;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> &a, const vec3<T> &b) {
  vec3<T> r{};
  r.x = a.x - b.x;
  r.y = a.y - b.y;
  r.z = a.z - b.z;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator*(const vec3<T> &a, const vec3<T> &b) {
  vec3<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  r.z = a.z * b.z;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator/(const vec3<T> &a, const vec3<T> &b) {
  vec3<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  r.z = a.z / b.z;
  return r;
}

// Binary Scalar
template <typename T>
inline constexpr vec3<T> operator+(const vec3<T> &a, T s) {
  vec3<T> r{};
  r.x = a.x + s;
  r.y = a.y + s;
  r.z = a.z + s;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator-(const vec3<T> &a, T s) {
  vec3<T> r{};
  r.x = a.x - s;
  r.y = a.y - s;
  r.z = a.z - s;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator*(const vec3<T> &a, T s) {
  vec3<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  r.z = a.z * s;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator/(const vec3<T> &a, T s) {
  vec3<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  r.z = a.z / s;
  return r;
}
template <typename T>
inline constexpr vec3<T> operator*(T s, const vec3<T> &a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec3<T> &a, const vec3<T> &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}
template <typename T>
inline constexpr bool operator!=(const vec3<T> &a, const vec3<T> &b) {
  return !(a == b);
}

// Math utility functions
template <typename T>
inline constexpr T dot(const vec3<T> &a, const vec3<T> &b) {
  return a.dot(b);
}
template <typename T>
inline constexpr vec3<T> cross(const vec3<T> &a, const vec3<T> &b) {
  return a.cross(b);
}
template <typename T> inline constexpr T length_sq(const vec3<T> &v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec3<T> &v) {
  return v.length();
}
template <typename T> inline constexpr vec3<T> normalize(const vec3<T> &v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec3<T> lerp(const vec3<T> &a, const vec3<T> &b, T t) {
  return a.lerp(b, t);
}

/*
  vec4: non-member operators and functions
*/

// Unary
template <typename T> inline constexpr vec4<T> operator-(const vec4<T> &v) {
  vec4<T> r{};
  r.x = -v.x;
  r.y = -v.y;
  r.z = -v.z;
  r.w = -v.w;
  return r;
}

// Binary Vector
template <typename T>
inline constexpr vec4<T> operator+(const vec4<T> &a, const vec4<T> &b) {
  vec4<T> r{};
  r.x = a.x + b.x;
  r.y = a.y + b.y;
  r.z = a.z + b.z;
  r.w = a.w + b.w;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator-(const vec4<T> &a, const vec4<T> &b) {
  vec4<T> r{};
  r.x = a.x - b.x;
  r.y = a.y - b.y;
  r.z = a.z - b.z;
  r.w = a.w - b.w;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator*(const vec4<T> &a, const vec4<T> &b) {
  vec4<T> r{};
  r.x = a.x * b.x;
  r.y = a.y * b.y;
  r.z = a.z * b.z;
  r.w = a.w * b.w;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator/(const vec4<T> &a, const vec4<T> &b) {
  vec4<T> r{};
  r.x = a.x / b.x;
  r.y = a.y / b.y;
  r.z = a.z / b.z;
  r.w = a.w / b.w;
  return r;
}

// Binary Scalar
template <typename T>
inline constexpr vec4<T> operator+(const vec4<T> &a, T s) {
  vec4<T> r{};
  r.x = a.x + s;
  r.y = a.y + s;
  r.z = a.z + s;
  r.w = a.w + s;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator-(const vec4<T> &a, T s) {
  vec4<T> r{};
  r.x = a.x - s;
  r.y = a.y - s;
  r.z = a.z - s;
  r.w = a.w - s;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator*(const vec4<T> &a, T s) {
  vec4<T> r{};
  r.x = a.x * s;
  r.y = a.y * s;
  r.z = a.z * s;
  r.w = a.w * s;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator/(const vec4<T> &a, T s) {
  vec4<T> r{};
  r.x = a.x / s;
  r.y = a.y / s;
  r.z = a.z / s;
  r.w = a.w / s;
  return r;
}
template <typename T>
inline constexpr vec4<T> operator*(T s, const vec4<T> &a) {
  return a * s;
}

// Relational
template <typename T>
inline constexpr bool operator==(const vec4<T> &a, const vec4<T> &b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}
template <typename T>
inline constexpr bool operator!=(const vec4<T> &a, const vec4<T> &b) {
  return !(a == b);
}

// Math utility functions
template <typename T>
inline constexpr T dot(const vec4<T> &a, const vec4<T> &b) {
  return a.dot(b);
}
template <typename T> inline constexpr T length_sq(const vec4<T> &v) {
  return v.length_sq();
}
template <typename T> inline constexpr T length(const vec4<T> &v) {
  return v.length();
}
template <typename T> inline constexpr vec4<T> normalize(const vec4<T> &v) {
  return v.normalize();
}
template <typename T>
inline constexpr vec4<T> lerp(const vec4<T> &a, const vec4<T> &b, T t) {
  return a.lerp(b, t);
}
} // namespace game