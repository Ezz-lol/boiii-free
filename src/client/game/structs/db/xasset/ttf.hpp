#pragma once

#include <cstdint>
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
PACKED(struct TTFDef {
  const char *name;
  int32_t fileLen;
  uint8_t _padding0C[4];
  const char *file;
  void *ftFace;
  TTFKerningEntry kerningCache[16384];
});
ASSERT_SIZE(TTFDef, 0x20020);
} // namespace ttf
} // namespace xasset
} // namespace db
} // namespace game