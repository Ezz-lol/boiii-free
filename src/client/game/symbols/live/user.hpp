#pragma once

#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {
namespace user {
WEAK symbol<game::XUID(game::ControllerIndex_t controllerIndex)>
    LiveUser_GetXuid{0x141EC7310};
WEAK symbol<bool(game::ControllerIndex_t controllerIndex)> LiveUser_IsSignedIn{
    0x141EC7030};
WEAK symbol<bool(game::ControllerIndex_t controllerIndex)>
    LiveUser_IsSignedInToLive{0x141EC7050};
WEAK symbol<const char *(ControllerIndex_t controllerIndex)>
    LiveUser_GetXuidString{0x141EC6E90};
WEAK symbol<const char *(ControllerIndex_t controllerIndex)>
    LiveUser_GetClientName{0x141EC72E0};
// Initial XUID generation. Value is later fetched by LiveUser_GetXuid
WEAK symbol<bool(int64_t controllerIndex, XUID *xuid)> LiveUser_UserGetXuid{
    0x141EC7650};
WEAK symbol<UserDataForControllerMap> s_UserDataForControllerMap{0x14340F180};
} // namespace user

} // namespace live
} // namespace game

#endif