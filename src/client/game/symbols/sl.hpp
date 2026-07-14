#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace sl {
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToString{
    0x1412D7160, 0x1401632C0};
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToStringSafe{
    0x1412D7180, 0x1401632E0};
WEAK symbol<scr::ScrString_t(const char *str, uint32_t user, int32_t type)>
    SL_GetString{0x1412D7B20, 0x140163D90};
} // namespace sl
} // namespace game

#endif