#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {
namespace draw {
namespace text {

WEAK symbol<void(const LocalClientNum_t localClientNum, const char *curText,
                 db::xasset::ttf::TTFDef *font, float x, float y,
                 gfx::GfxColor *gfxColor, float fontHeight, float wrapWidth,
                 float postScale, lua::LUIAlignment alignment,
                 lua::hks::lua_State *luaVM, lua::lui::LUIElement *element,
                 uint32_t flags)>
    UI_DrawTextPhysical{0x141F26FA0, 0x0};
}
} // namespace draw
} // namespace ui
} // namespace game
