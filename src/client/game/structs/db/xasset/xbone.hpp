#pragma once

#include <cstdint>
#include <game/structs/macros.hpp>
#include <game/structs/quake/vec.hpp>

namespace game {

namespace db {
namespace xasset {
namespace xbone {

struct __attribute__((aligned(4))) XBoneInfo {
  vec3_t bounds[2];
  vec3_t offset;
  float radiusSquared;
  uint8_t collmap;
};
} // namespace xbone
} // namespace xasset
} // namespace db
} // namespace game