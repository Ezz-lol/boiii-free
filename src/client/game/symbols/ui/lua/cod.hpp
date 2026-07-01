#pragma once
#ifdef GAME_HPP

#include "../../macros.hpp"

namespace game {
namespace ui {
namespace lua {
namespace cod {
WEAK symbol<void(hks::lua_State *, const char *)> Lua_CoD_LoadLuaFile{
    0x141F11A20, 0x1404BE1C0};

WEAK symbol<game::db::xasset::XAssetHeader(const char *filename)>
    Lua_CoD_GetRawFile{0x141F0EFE0, 0x1404BCB70};
WEAK symbol<hks::luaReturnCount_e(hks::lua_State *luaVM)>
    Lua_CoD_LuaCall_ForceHUDRefresh{0x141F45740};
WEAK symbol<bool> s_luaInitialized{0x159C76D90};

// If generating a `lua_State` in boiii, call this function.
WEAK symbol<hks::lua_State *(hks::luaState_e luaState, void *memoryLocation,
                             int32_t memSize, hks::lua_CFunction panicFn)>
    Lua_CoD_GenerateLuaState{0x141F117B0, 0x1404BE010};
// Also named `Lua_CoD_GenerateLuaState` in the engine. Override called by the
// other function of same name.
WEAK symbol<hks::lua_State *(hks::luaState_e luaState,
                             hks::HksStateSettings *settings,
                             hks::lua_CFunction panicFn)>
    Lua_CoD_GenerateLuaState_Internal{0x141F115D0, 0x1404BDE40};
WEAK symbol<CodLuaStates> s_codLuaStates{0x1434100A0, 0x141122FB0};
} // namespace cod
} // namespace lua
} // namespace ui
} // namespace game

#endif