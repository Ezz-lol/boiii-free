#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace pix {
WEAK symbol<void(int64_t, char *event)> PIXBeginNamedEvent{0x0, 0x14050BAE0};
WEAK symbol<void()> PIXEndNamedEvent{0x0, 0x14050C280};
} // namespace pix
} // namespace game

#endif