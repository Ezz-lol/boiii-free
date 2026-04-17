#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../../macros.hpp"

namespace game {
namespace ui {

namespace lua {
namespace hks {
WEAK symbol<lua_State *> lua_state{0x159C76D88, 0x14858C408};
WEAK symbol<void(lua_State *s, const char *str, unsigned int l)>
    hksi_lua_pushlstring{0x140A18430, 0x1401DE6F0};

WEAK symbol<void(lua_State *s, const HksObject *tbl, const HksObject *key,
                 const HksObject *val)>
    hks_obj_settable{0x141D4B660, 0x1403F41B0};
WEAK symbol<HksObject *(HksObject *result, lua_State *s, const HksObject *table,
                        const HksObject *key)>
    hks_obj_gettable{0x141D4ABF0, 0x1403F3750};
WEAK symbol<void(lua_State *s, int nargs, int nresults, const unsigned int *pc)>
    vm_call_internal{0x141D70FE0, 0x140418E40};
WEAK symbol<HashTable *(lua_State *s, unsigned int arraySize,
                        unsigned int hashSize)>
    Hashtable_Create{0x141D3B5F0, 0x1403E46D0};
WEAK symbol<cclosure *(lua_State *s, lua_function function, int num_upvalues,
                       int internal_, int profilerTreatClosureAsFunc)>
    cclosure_Create{0x141D3B7E0, 0x1403E48C0};
WEAK symbol<int(lua_State *s, int t)> hksi_luaL_ref{0x141D4D1A0, 0x1403F5CF0};
WEAK symbol<void(lua_State *s, int t, int ref)> hksi_luaL_unref{0x141D4D320,
                                                                0x1403F5E70};

WEAK symbol<int(lua_State *s, const HksCompilerSettings *options,
                const char *buff, uint64_t sz, const char *name)>
    hksi_hksL_loadbuffer{0x141D4BD80, 0x1403F48D0};
WEAK symbol<int(lua_State *s, const char *what, lua_Debug *ar)>
    hksi_lua_getinfo{0x141D4D8D0, 0x1403F64B0};
WEAK symbol<int(lua_State *s, int level, lua_Debug *ar)> hksi_lua_getstack{
    0x141D4DB90, 0x1403F6770};
WEAK symbol<void(lua_State *s, const char *fmt, ...)> hksi_luaL_error{
    0x141D4D050, 0x1403F5BA0};
WEAK symbol<const char *(lua_State *s, int index, size_t *len)>
    hksi_lua_tolstring{0x141D4B6C0, 0x1403F42B0};
WEAK symbol<const char *> s_compilerTypeName{0x140A18430};
} // namespace hks
} // namespace lua
} // namespace ui
} // namespace game

#endif
