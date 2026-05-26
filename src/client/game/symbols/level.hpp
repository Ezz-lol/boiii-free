#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include <atomic>

#include "macros.hpp"

namespace game {
namespace level {
WEAK symbol<level::gentity_pool> g_entities{0x0, 0x1471031B0};
// Statically allocated on server, heap allocated with `malloc(0x27C000u);` on
// client.
WEAK symbol<level::gentity_pool *> g_entities_cl{0x14A5F25F8};
extern std::atomic<level::gentity_pool *> g_entities_cl_allocation;

WEAK symbol<int32_t> level_time{0x14A5502C4, 0x1474FDC94};
WEAK symbol<int32_t> level_rounds_played{0x14A55BDEC, 0x1475097BC};
} // namespace level
} // namespace game

#endif