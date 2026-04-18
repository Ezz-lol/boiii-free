#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {

WEAK symbol<void(const char *comErrorString, int32_t code)>
    Live_DelayedComError{0x141E0C140};

// Live
WEAK symbol<bool(uint64_t, int32_t *, bool)> Live_GetConnectivityInformation{
    0x141E0C380};

// LiveStats
WEAK symbol<const char *(int32_t controllerIndex)> LiveStats_GetClanTagText{
    0x141E9CE20};

} // namespace live
} // namespace game

#endif