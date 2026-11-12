#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace lui {
WEAK symbol<bool()> UI_IsInitialized{0x14268C590, 0x0};
WEAK symbol<db::xasset::MaterialHandle> Lua_DefaultMaterial{0x156262C88, 0x0};
WEAK symbol<LUIElement *(const char *rootName, hks::lua_State *luaVM)>
    LUI_GetRootElement{0x14268BB80, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *rootName,
                 hks::lua_State *luaVM)>
    UI_DrawRoot{0x142693AE0, 0x0};
} // namespace lui
} // namespace lua
} // namespace game
