#pragma once

#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace live {
namespace steam {
WEAK symbol<char *(char *username, uint32_t length, bool asciionly)>
    LiveSteam_GetUserName{0x141EB0D30};
} // namespace steam
} // namespace live
} // namespace game

#endif