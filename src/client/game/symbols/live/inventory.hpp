#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace inventory {
WEAK symbol<bool()> LiveInventory_ShouldWaitOnInventory{0x141DFDF00,
                                                        0x141E0A990, 0x0};
WEAK symbol<EngineDependentDvar> inventory_blocking{0x15114E3E8, 0x1511CD368,
                                                    0x0};
} // namespace inventory
} // namespace live
} // namespace game
