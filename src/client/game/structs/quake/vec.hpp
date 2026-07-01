#pragma once

namespace game {
typedef float vec_t;

template <typename T = vec_t> union vec2 {
  T v[2];
  struct {
    T x;
    T y;
  };
};
typedef vec2<vec_t> vec2_t;

template <typename T = vec_t> union vec3_core {
  struct {
    T x;
    T y;
    T z;
  };

  T v[3];
};

template <typename T = vec_t> union vec3 {
  struct {
    T x;
    T y;
    T z;
  };

  vec2<T> xy;
  T v[3];
};

typedef vec3<vec_t> vec3_t;

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
  vec3_core<T> xyz;
};

typedef vec4<vec_t> vec4_t;
} // namespace game