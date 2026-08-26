#pragma once

#include "game/structs/db/xasset/core.hpp"
#include <cstdint>
#include <game/structs/gfx/core.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace db {
namespace xasset {
namespace font {

struct __declspec(align(8)) FontIconName {
  const char *string;
  int32_t hash;
};

struct __declspec(align(8)) FontIconEntry {
  FontIconName fontIconName;
  gfx::GfxImagePtr fontIconImage;
  int32_t fontIconSize;
  float xScale;
  float yScale;
};

struct FontIconAlias {
  int32_t aliasHash;
  int32_t buttonHash;
  int32_t buttonHashAlt;
};

// Verified
struct FontIcon {
  const char *name;
  int32_t numEntries;
  int32_t numAliasEntries;
  FontIconEntry *fontIconEntry;
  FontIconAlias *fontIconAlias;
};
ASSERT_SIZE(FontIcon, 0x20);

struct Glyph {
  uint16_t letter;
  char x0;
  char y0;
  uint8_t dx;
  uint8_t pixelWidth;
  uint8_t pixelHeight;
  float s0;
  float t0;
  float s1;
  float t1;
};

struct KerningPairs {
  uint16_t wFirst;
  uint16_t wSecond;
  int32_t iKernAmount;
};

// Verified
struct Font_s {
  const char *fontName;
  int32_t pixelHeight;
  int32_t isScalingAllowed;
  int32_t glyphCount;
  int32_t kerningPairsCount;
  MaterialHandle material;
  MaterialHandle glowMaterial;
  Glyph *glyphs;
  KerningPairs *kerningPairs;
};

ASSERT_SIZE(Font_s, 0x38);
typedef Font_s Font;

} // namespace font
} // namespace xasset
} // namespace db
} // namespace game