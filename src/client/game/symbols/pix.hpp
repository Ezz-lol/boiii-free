#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace pix {
WEAK symbol<void(int64_t, const char *event)> PIXBeginNamedEvent{0x0,
                                                                 0x14050BAE0};
WEAK symbol<void()> PIXEndNamedEvent{0x0, 0x14050C280};
} // namespace pix
} // namespace game

