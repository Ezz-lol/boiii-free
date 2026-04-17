#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace sl {
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToStringSafe{
    0x1412D7180, 0x1401632E0};
}
} // namespace game

#endif