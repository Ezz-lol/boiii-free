#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// e.g. BG_ASM_
namespace bg {
WEAK symbol<const char *(bgCacheInstance inst, int32_t index)>
    BG_Cache_GetModelNameForIndex{0x1400A7C60, 0x140044430};
WEAK symbol<bool(const vec3_t *pos, const vec3_t *mapCenter)> BG_ValidateOrigin{
    0x1426824E0, 0x1406A9BB0};
} // namespace bg
} // namespace game
