#pragma once
#ifdef GAME_HPP

#include "../../macros.hpp"

namespace game {
namespace ui {
namespace lua {
WEAK symbol<void(const char *key, int value, lua::hks::lua_State *luaVM)>
    Lua_SetTableInt{0x141F066E0};
WEAK symbol<void(const char *key, const char *value, hks::lua_State *luaVM)>
    Lua_SetTableString{0x141F06800};
WEAK symbol<void(hks::lua_State *s, int32_t narr, int32_t nrec)>
    lua_createtable{0x141D53680};
WEAK symbol<void(hks::lua_Integer key, hks::lua_State *luaVM)> Lua_BeginTable{
    0x141F04D30};
WEAK symbol<void(hks::lua_State *luaVM)> Lua_EndTable{0x141F04F10};
WEAK symbol<void(hks::lua_State *, const char *)> Lua_CoD_LoadLuaFile{
    0x141F11A20, 0x0};

WEAK symbol<game::db::xasset::XAssetHeader(const char *filename)>
    Lua_CoD_GetRawFile{0x141F0EFE0, 0x1404BCB70};
WEAK symbol<hks::luaReturnCount_e(hks::lua_State *luaVM)>
    Lua_CoD_LuaCall_ForceHUDRefresh{0x141F45740};
WEAK symbol<uint32_t(ZoneType zoneType, int32_t sliceStart, int32_t sliceLen,
                     hks::lua_State *luaState)>
    Mods_Lists_GetInfoEntries_Slice{0x1420D61E0};
} // namespace lua
} // namespace ui
} // namespace game

#endif