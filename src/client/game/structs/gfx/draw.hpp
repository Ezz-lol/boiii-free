#pragma once

#include <cstdint>

namespace game {
namespace gfx {
namespace draw {

struct __declspec(align(4)) GfxCmdHeader {
  uint32_t byteCount;
  uint8_t id;
  uint8_t ui3d;
};

struct GfxCmdStartTable {
  GfxCmdHeader header;
  uint32_t offsets[127];
};

struct __declspec(align(8)) GfxCmdArray {
  uint8_t *cmds;
  int32_t usedTotal;
  int32_t usedCritical;
  GfxCmdHeader *lastCmd;
  int32_t byteSize;
  int32_t warnSize;
  int32_t criticalSize;
  int32_t criticalWarnSize;
  int32_t surfCount;
  GfxCmdStartTable *offsetTable;
  uint32_t offsets[127];
};

} // namespace draw
} // namespace gfx
} // namespace game
