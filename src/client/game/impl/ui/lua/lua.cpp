#include <std_include.hpp>
#include "lua.hpp"

namespace game {
namespace ui {
namespace lua {
uint32_t Mods_Lists_GetInfoEntries_Slice_Impl(ugc::ZoneType zoneType,
                                              int32_t sliceStart,
                                              int32_t sliceLen,
                                              hks::lua_State *luaState) {
  int32_t endIdx;
  ugc::ExtendedWorkshopDataPool *pool;
  switch (zoneType) {
  case ugc::ZoneType::MOD:
    endIdx = ugc::modsPool.count;
    pool = &ugc::modsPool;
    break;
  case ugc::ZoneType::USERMAP:
    endIdx = ugc::usermapsPool.count;
    pool = &ugc::usermapsPool;
    break;
  default:
    return 0;
  }

  if (endIdx == 0) {
    return 0;
  }
  endIdx = (std::min)(endIdx, sliceStart + sliceLen);

  sliceLen = endIdx - sliceStart;
  if (sliceLen <= 0) {
    return 0;
  }
  lua_createtable(luaState, sliceLen, 0);
  for (int32_t i = sliceStart, key = 0; i < endIdx; i++, key++) {
    Lua_BeginTable(key, luaState);
    ugc::WorkshopData *data = &pool->data[i];
    Lua_SetTableString("name", data->title, luaState);
    Lua_SetTableString("internalName", data->internalName, luaState);
    Lua_SetTableString("description", data->description, luaState);
    Lua_SetTableString("ugcName", data->publisherId, luaState);
    Lua_SetTableInt("ugcVersion", data->version, luaState);
    Lua_EndTable(luaState);
  }
  return static_cast<uint32_t>(sliceLen);
}
} // namespace lua
} // namespace ui
} // namespace game