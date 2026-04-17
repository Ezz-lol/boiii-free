#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {

// Live
WEAK symbol<bool(uint64_t, int32_t *, bool)> Live_GetConnectivityInformation{
    0x141E0C380};

// LiveStats
WEAK symbol<const char *(int32_t controllerIndex)> LiveStats_GetClanTagText{
    0x141E9CE20};

} // namespace live
} // namespace game

#endif