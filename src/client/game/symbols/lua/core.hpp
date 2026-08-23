#pragma once

#include <game/symbols/sym_include.hpp>
#include <game/symbols/lua/hks.hpp>

namespace game {
namespace lua {
WEAK symbol<void(const char *key, int32_t value, lua::hks::lua_State *luaVM)>
    Lua_SetTableInt{0x141F066E0};
WEAK symbol<void(const char *key, bool value, lua::hks::lua_State *luaVM)>
    Lua_SetTableBool{0x141F064E0};
WEAK symbol<void(const char *key, const char *value, hks::lua_State *luaVM)>
    Lua_SetTableString{0x141F06800};
WEAK symbol<void(hks::lua_Integer key, hks::lua_State *luaVM)> Lua_BeginTable{
    0x141F04D30};
WEAK symbol<void(hks::lua_State *luaVM)> Lua_EndTable{0x141F04F10};

WEAK symbol<hks::lua_CFunction *(hks::lua_State *s, hks::lua_CFunction panicf)>
    lua_atpanic{0x141D535D0, 0x1403FC0A0};
WEAK symbol<void(hks::lua_State *s)> lua_close{0x141D53600, 0x1403FC0C0};
WEAK symbol<void(hks::lua_State *s, int32_t narr, int32_t nrec)>
    lua_createtable{0x141D53680, 0x1403FC140};
WEAK symbol<void(hks::lua_State *s, int32_t index)> lua_insert{0x1414292D0,
                                                               0x1401DE3F0};
WEAK symbol<hks::HksInteger(hks::lua_State *s, int32_t narg)> luaL_checkinteger{
    0x141D52C70};
WEAK symbol<void *(hks::lua_State *s, int32_t narg, const char *tname)>
    luaL_checkudata{0x141D52EC0};
WEAK symbol<int32_t(hks::lua_State *s, const char *fmt, ...)> luaL_error{
    0x141D52FC0};
WEAK symbol<void(hks::lua_State *s, const char *tname)> luaL_getmetatable{
    0x0, 0x1403FBD30};
WEAK symbol<int32_t(hks::lua_State *s, const char *filename)> luaL_loadfile{
    0x141D532D0};
WEAK symbol<int32_t(hks::lua_State *s, const char *tname)> luaL_newmetatable{
    0x141D532F0};
WEAK symbol<void(hks::lua_State *s)> luaL_openlibs{0x141D53420, 0x1403FBF10};
WEAK symbol<int32_t(hks::lua_State *s, int32_t t)> luaL_ref{0x141D4D1A0};
WEAK
    symbol<void(hks::lua_State *s, const char *libname, const hks::luaL_Reg *l)>
        luaL_register{0x141D53560, 0x1403FC040};
WEAK symbol<void(hks::lua_State *s, int32_t t, int32_t ref)> luaL_unref{
    0x141D4D320};
WEAK symbol<void *(void *userData, void *ptr, size_t osize, size_t nsize)>
    lua_mem_alloc{0x141F12A50, 0x1404BF0F0};
WEAK symbol<void *(hks::lua_State *s, size_t size)> lua_newuserdata{
    0x141D53D50};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_base{0x141D32F30, 0x1403DBDA0};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_debug{0x141D34190, 0x1403DD000};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_hks{0x141D34B10, 0x1403DD990};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_io{0x141D33510, 0x1403DC380};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_math{0x141D34070, 0x1403DCEE0};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_os{0x141D33D20, 0x1403DCB90};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_package{0x141D34390,
                                                        0x1403DD200};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_profile{0x141D341C0,
                                                        0x1403DD030};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_serialize{0x141D34B40,
                                                          0x1403DD9C0};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_string{0x141D33D50,
                                                       0x1403DCBC0};
WEAK symbol<int32_t(hks::lua_State *s)> luaopen_table{0x141D334E0, 0x1403DC350};
WEAK symbol<const char *(hks::lua_State *s, const char *fmt)> lua_pushfstring{
    0x141D53DC0};
WEAK symbol<void(hks::lua_State *s, const char *str)> lua_pushstring{
    0x140A186B0, 0x1401DEE30};
WEAK symbol<void(hks::lua_State *s, int32_t index, int32_t n)> lua_rawgeti{
    0x141D4B510};
WEAK symbol<void(hks::lua_State *s, int32_t index)> lua_remove{0x141D53DF0};
WEAK symbol<void(hks::lua_State *s, int32_t index, const char *k)> lua_setfield{
    0x141429680, 0x1401DEA20};
WEAK symbol<void *(hks::lua_State *s, int32_t index)> lua_touserdata{
    0x141D4F140};
WEAK symbol<int32_t(hks::lua_State *s)> luaWrapper_finalizeUserdata{
    0x141D53590};

WEAK symbol<uint32_t(ZoneType zoneType, int32_t sliceStart, int32_t sliceLen,
                     hks::lua_State *luaState)>
    Mods_Lists_GetInfoEntries_Slice{0x1420D61E0};

inline hks::HksObject *getObjectForIndex(hks::lua_State *s, int32_t index) {
  hks::HksObject *object = nullptr;

  if (index <= LUA_REGISTRYINDEX) {
    switch (index) {
    case LUA_REGISTRYINDEX:
      object = &s->m_global->m_registry;
      break;
    case LUA_GLOBALSINDEX:
      object = &s->globals;
      break;
    case LUA_ENVIRONINDEX:
      s->m_cEnv.v.cClosure = reinterpret_cast<hks::cclosure *>(
          s->m_apistack.base[-1].v.cClosure->m_env);
      s->m_cEnv.t = hks::HksObjectType::TTABLE;
      object = &s->m_cEnv;
      break;
    default:
      object = reinterpret_cast<hks::HksObject *>(
          &s->m_apistack.base[-1].v.cClosure->m_numUpvalues +
          8 * (LUA_GLOBALSINDEX - index));
      break;
    }
  } else if (index < 0) {
    if (&s->m_apistack.top[index] >= s->m_apistack.base) {
      object = &s->m_apistack.top[index];
    }
  } else if (&s->m_apistack.base[index - 1] < s->m_apistack.top) {
    object = &s->m_apistack.base[index - 1];
  }
  // TODO: Handle failures
  return object;
}

inline hks::HksNumber lua_tonumber(hks::lua_State *s, int32_t index) {
  const hks::HksObject *object = getObjectForIndex(s, index);
  return object->v.number;
}

inline const char *lua_tostring(hks::lua_State *s, int32_t index) {
  hks::HksObject *object = getObjectForIndex(s, index);
  return hks::hks_obj_tolstring(s, object, nullptr);
}

inline void lua_pushboolean(hks::lua_State *s, hks::hksBool b) {
  hks::HksObject *top = s->m_apistack.top;
  top->v.boolean = b;
  top->t = hks::HksObjectType::TBOOLEAN;
  s->m_apistack.top = top + 1;
}
} // namespace lua
} // namespace game