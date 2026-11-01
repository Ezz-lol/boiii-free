#pragma once

#include <game/game.hpp>

namespace game {
namespace lua {
namespace lui {
namespace quad {
using namespace game::lua::hks;

alwaysinline UIQuadCache *UI_CoD_GetQuadCache_Impl(LUIElement *element,
                                                   lua_State *luaVM) {
  return !nonnull(element) || ui::UI_IsRenderingImmediately() ? nullptr
                                                              : element->cache;
}

UIQuadCache *UI_CoD_GetNextQuadCache_Safe(lua_State *luaVM,
                                          UIQuadCache *quadCache);
void UI_CoD_UnrefQuadCacheChain_Safe(lua_State *luaVM, UIQuadCache *quadCache);

alwaysinline int32_t GetSizeForQuadCache_Impl(const int16_t numQuads) {
  return 0x4 * numQuads + 0x50 * numQuads + sizeof(UIQuadCache);
}

luaReturnCount_e UI_CoD_LuaCall_QuadCache_GC_Safe(lua_State *luaVM);
} // namespace quad
} // namespace lui
} // namespace lua
} // namespace game
