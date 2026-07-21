#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace steam {
WEAK symbol<char *(char *username, uint32_t length, bool asciionly)>
    LiveSteam_GetUserName{0x141EB0D30};
WEAK symbol<bool()> LiveSteam_NotVacBanned{0x141EB1350};
} // namespace steam
} // namespace live
} // namespace game