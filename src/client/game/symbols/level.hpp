#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace level {
WEAK symbol<level::gentity_s> g_entities{0x0, 0x1471031B0};

WEAK symbol<int32_t> level_time{0x14a5502c4, 0x1474FDC94};
WEAK symbol<int32_t> level_rounds_played{0x14A55BDEC, 0x1475097BC};
} // namespace level
} // namespace game

#endif