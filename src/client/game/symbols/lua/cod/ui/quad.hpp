#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace cod {
namespace api {
namespace ui {
namespace quad {

WEAK symbol<void(hks::lua_State *luaVM, lui::UIQuadCache *quadCache)>
    UI_CoD_UnrefQuadCacheChain{0x141F268F0, 0x0, 0x0};

WEAK symbol<hks::lua_CFunction> UI_CoD_LuaCall_QuadCache_GC{0x141F1F6C0, 0x0,
                                                            0x0};
WEAK symbol<lui::UIQuadCache *(hks::lua_State *luaVM,
                               lui::UIQuadCache *quadCache)>
    UI_CoD_GetNextQuadCache{0x141F1C080, 0x0, 0x0};
} // namespace quad
} // namespace ui
} // namespace api
} // namespace cod
} // namespace lua
} // namespace game
