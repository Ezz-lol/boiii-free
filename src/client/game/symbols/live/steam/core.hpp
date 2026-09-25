#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace steam {
WEAK symbol<char *(char *username, uint32_t length, bool asciionly)>
    LiveSteam_GetUserName{0x141EA42A0, 0x141EB0D30, 0x0};
WEAK symbol<bool()> LiveSteam_NotVacBanned{0x141EA48C0, 0x141EB1350, 0x0};

} // namespace steam
} // namespace live
} // namespace game
