#pragma once

namespace game {
namespace math {
struct Float4 {
  float x;
  float y;
  float z;
  float w;
};

struct Dir3 {
  Float4 v;
};

typedef Dir3 UnitDir3;

struct Position3 {
  Float4 v;
};

struct Mat43 {
  UnitDir3 x;
  UnitDir3 y;
  UnitDir3 z;
  Position3 w;
};

typedef Mat43 RotTranMat43;

} // namespace math
} // namespace game