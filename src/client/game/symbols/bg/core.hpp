#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// e.g. BG_ASM_
namespace bg {
WEAK symbol<bool(const vec3_t *pos, const vec3_t *mapCenter)> BG_ValidateOrigin{
    0x1426824E0, 0x1406A9BB0};
} // namespace bg
} // namespace game