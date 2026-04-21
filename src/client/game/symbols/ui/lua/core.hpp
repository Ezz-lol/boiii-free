#pragma once
#ifdef GAME_HPP

#include "../../macros.hpp"

namespace game {
namespace ui {
namespace lua {
WEAK symbol<void(const char *key, int value, lua::hks::lua_State *luaVM)>
    Lua_SetTableInt{0x141F066E0};
WEAK symbol<void(hks::lua_State *, const char *)> Lua_CoD_LoadLuaFile{
    0x141F11A20, 0x0};

WEAK symbol<game::db::xasset::XAssetHeader(const char *filename)>
    Lua_CoD_GetRawFile{0x141F0EFE0, 0x1404BCB70};
} // namespace lua
} // namespace ui
} // namespace game

#endif