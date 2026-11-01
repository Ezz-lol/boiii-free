#include <std_include.hpp>

#include "quad.hpp"

namespace game {
namespace lua {
namespace lui {
namespace quad {
UIQuadCache *UI_CoD_GetNextQuadCache_Safe(lua_State *luaVM,
                                          UIQuadCache *quadCache) {
  if (UI_IsInitialized() && !ui::UI_IsRenderingImmediately() &&
      game::readable_ptr(quadCache) && quadCache->next != -2) {
    lua_rawgeti(luaVM, LUA_REGISTRYINDEX, quadCache->next);
    UIQuadCache *const nextCache = lua_touserdata(luaVM, -1).uiQuadCache;
    lua_pop(luaVM, 1);

    if (game::readable_ptr(nextCache)) {
      return nextCache;
    }
  }

  return nullptr;
}

void UI_CoD_UnrefQuadCacheChain_Safe(lua_State *luaVM, UIQuadCache *quadCache) {
  if (UI_IsInitialized() && !ui::UI_IsRenderingImmediately() &&
      game::readable_ptr(quadCache)) {
    for (UIQuadCache *nextCache =
             UI_CoD_GetNextQuadCache_Safe(luaVM, quadCache);
         game::readable_ptr(nextCache);
         nextCache = UI_CoD_GetNextQuadCache_Safe(luaVM, nextCache)) {
      luaL_unref(luaVM, LUA_REGISTRYINDEX, quadCache->next);
      quadCache->next = -2;
      quadCache = nextCache;
    }
  }
}

luaReturnCount_e UI_CoD_LuaCall_QuadCache_GC_Safe(lua_State *luaVM) {
  if (UI_IsInitialized() && !ui::UI_IsRenderingImmediately() &&
      lua_gettop(luaVM) == 1) {
    UIQuadCache *const cache = lua_touserdata(luaVM, 1).uiQuadCache;
    if (game::readable_ptr(cache)) {
      *quad::g_LUIAllocatedQuadCachesMemory -=
          GetSizeForQuadCache_Impl(cache->numQuads);
      UI_CoD_UnrefQuadCacheChain_Safe(luaVM, cache);
    }
  }
  return luaReturnCount_e::NONE;
}
} // namespace quad
} // namespace lui
} // namespace lua
} // namespace game
