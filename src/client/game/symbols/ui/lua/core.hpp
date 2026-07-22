#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {
namespace lua {
WEAK symbol<void(const char *key, int value, lua::hks::lua_State *luaVM)>
    Lua_SetTableInt{0x141F066E0};
WEAK symbol<void(const char *key, bool value, lua::hks::lua_State *luaVM)>
    Lua_SetTableBool{0x141F064E0};
WEAK symbol<void(const char *key, const char *value, hks::lua_State *luaVM)>
    Lua_SetTableString{0x141F06800};
WEAK symbol<void(hks::lua_State *s, int32_t narr, int32_t nrec)>
    lua_createtable{0x141D53680};
WEAK symbol<void(hks::lua_Integer key, hks::lua_State *luaVM)> Lua_BeginTable{
    0x141F04D30};
WEAK symbol<void(hks::lua_State *luaVM)> Lua_EndTable{0x141F04F10};

WEAK symbol<uint32_t(ZoneType zoneType, int32_t sliceStart, int32_t sliceLen,
                     hks::lua_State *luaState)>
    Mods_Lists_GetInfoEntries_Slice{0x1420D61E0};
} // namespace lua
} // namespace ui
} // namespace game