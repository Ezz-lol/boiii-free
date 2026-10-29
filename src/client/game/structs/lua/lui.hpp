#pragma once

#include "game/structs/quake/vec.hpp"

#include "hks.hpp"

namespace game {
namespace gfx {
struct GfxImage;
union GfxColor;
} // namespace gfx

namespace lua {
namespace lui {

struct UIQuadCache {
  vec3_t *xyz;
  vec2_t *st;
  gfx::GfxColor *color;
  hks::LUA_MATERIAL_DATATYPE material;
  const gfx::GfxImage *image;
  int16_t numQuads;
  bool dirty;
  uint8_t fontCacheVersion;
  int next;
};

} // namespace lui
} // namespace lua
} // namespace game
