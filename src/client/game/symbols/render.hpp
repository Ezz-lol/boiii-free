#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace render {
// Rendering
WEAK symbol<void(const char *text, int, const uint32_t *font, float x, float y,
                 float xScale, float yScale, float, const vec4_t *color,
                 itemTextStyle textStyle)>
    R_AddCmdDrawText{0x141CD98D0};
} // namespace render
} // namespace game

