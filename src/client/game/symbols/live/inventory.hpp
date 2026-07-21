#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace inventory {
WEAK symbol<bool()> LiveInventory_ShouldWaitOnInventory{0x141E0A990};
WEAK symbol<EngineDependentDvar> inventory_blocking{0x1511CD368};
} // namespace inventory
} // namespace live
} // namespace game