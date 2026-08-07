#pragma once

#include "quake/vec.hpp"

namespace game {
namespace math {

typedef vec4_t Float4;
typedef Float4 float4;

typedef Float4 Dir3;
typedef Float4 Position3;

typedef Dir3 UnitDir3;

struct Mat43 {
  UnitDir3 x;
  UnitDir3 y;
  UnitDir3 z;
  Position3 w;

  inline constexpr void setPosition(const vec3_t pos) noexcept {
    w = {.x = pos.x, .y = pos.y, .z = pos.z, .w = 1.0f};
  }

  inline constexpr void setPosition(const Position3 pos) noexcept { w = pos; }

  /*
    Constructs a Mat43 directly from its component vectors.
    Used in place of a constructor to maintain C++03 POD constraints.
  */
  static inline constexpr Mat43 from(UnitDir3 x, UnitDir3 y, UnitDir3 z,
                                     Position3 w) {
    Mat43 m;
    m.x = x;
    m.y = y;
    m.z = z;
    m.w = w;
    return m;
  }

  /*
      Constructs a Mat43 from 3D vectors, expanding them into 4D homogeneous
     vectors. Direction axes (x, y, z) receive a homogeneous w-component of
     0.0f. The position vector (w) receives a homogeneous w-component of 1.0f.
    */
  static inline constexpr Mat43 from(const vec3_t x, const vec3_t y,
                                     const vec3_t z, const vec3_t w) {
    Mat43 m;

    // X axis (Direction)
    m.x.x = x.x;
    m.x.y = x.y;
    m.x.z = x.z;
    m.x.w = 0.0f;

    // Y axis (Direction)
    m.y.x = y.x;
    m.y.y = y.y;
    m.y.z = y.z;
    m.y.w = 0.0f;

    // Z axis (Direction)
    m.z.x = z.x;
    m.z.y = z.y;
    m.z.z = z.z;
    m.z.w = 0.0f;

    // W axis (Position/Translation)
    m.w.x = w.x;
    m.w.y = w.y;
    m.w.z = w.z;
    m.w.w = 1.0f;

    return m;
  }

  /*
    Computes a rotation matrix from Euler angles (pitch, yaw, roll) in
    degrees. Uses the default identity translation (0, 0, 0, 1) for the
    position vector.
  */
  static inline constexpr Mat43 from_euler(const vec3_t euler) {
    const float p = to_radians<float>(euler.x); // Pitch
    const float y = to_radians<float>(euler.y); // Yaw
    const float r = to_radians<float>(euler.z); // Roll

    const float cp = std::cos(p), sp = std::sin(p);
    const float cy = std::cos(y), sy = std::sin(y);
    const float cr = std::cos(r), sr = std::sin(r);

    UnitDir3 vx;
    vx.x = cp * cy;
    vx.y = cp * sy;
    vx.z = -sp;
    vx.w = 0.0f; // Homogeneous direction

    UnitDir3 vy;
    vy.x = sr * sp * cy - cr * sy;
    vy.y = sr * sp * sy + cr * cy;
    vy.z = sr * cp;
    vy.w = 0.0f; // Homogeneous direction

    UnitDir3 vz;
    vz.x = cr * sp * cy + sr * sy;
    vz.y = cr * sp * sy - sr * cy;
    vz.z = cr * cp;
    vz.w = 0.0f; // Homogeneous direction

    Position3 vw;
    vw.x = 0.0f;
    vw.y = 0.0f;
    vw.z = 0.0f;
    vw.w = 1.0f; // Homogeneous position (translation applies)

    return from(vx, vy, vz, vw);
  }
  static inline constexpr Mat43 from_euler(const vec4_t euler) {
    return from_euler(euler.xyz);
  }
};

ASSERT_CPP03_POD(Mat43);

typedef Mat43 RotTranMat43;
} // namespace math
} // namespace game