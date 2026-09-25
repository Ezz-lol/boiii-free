#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace render {
// Rendering
WEAK symbol<void(const char *text, int, const char *font, float x, float y,
                 float xScale, float yScale, float, const vec4_t *color,
                 itemTextStyle textStyle)>
    R_AddCmdDrawText{0x141CCD500, 0x141CD98D0, 0x0};
} // namespace render
} // namespace game
