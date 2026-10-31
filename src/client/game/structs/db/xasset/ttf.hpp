#pragma once

#include <cstdint>

#include <game/structs/db/xasset/core.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace db {
namespace xasset {
namespace ttf {

struct TTFKerningEntry {
  uint16_t height;
  uint16_t prevCharIndex;
  uint16_t nextCharIndex;
  int16_t kerning;
};

// Verified
PACKED(struct TTFDef : NamedXAsset {
  int32_t fileLen;
  uint8_t _padding0C[4];
  const char *file;
  void *ftFace;
  TTFKerningEntry kerningCache[16384];
});
ASSERT_SIZE(TTFDef, 0x20020);

typedef uint16_t glyphIndex_t;
struct __declspec(align(4)) CachedGlyph {
  void *font;
  float s0;
  float t0;
  float s1;
  float t1;
  glyphIndex_t next;
  uint16_t letter;
  int32_t glyphIndex;
  uint16_t fontHeight;
  uint8_t fxFlags;
  int16_t x0;
  int16_t y0;
  uint16_t pixelWidth;
  uint16_t pixelHeight;
  int16_t dx;
};

} // namespace ttf
} // namespace xasset
} // namespace db
} // namespace game
