#pragma once
#include <cstdint>

#include "../../../../game/game.hpp"

namespace game {
namespace ui {
namespace lua {
uint32_t Mods_Lists_GetInfoEntries_Slice_Impl(ZoneType zoneType,
                                              int32_t sliceStart,
                                              int32_t sliceLen,
                                              hks::lua_State *luaState);

}
} // namespace ui
} // namespace game