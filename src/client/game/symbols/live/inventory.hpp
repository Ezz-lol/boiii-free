#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace inventory {
WEAK symbol<bool()> LiveInventory_ShouldWaitOnInventory{0x141DFDF00,
                                                        0x141E0A990, 0x0};
WEAK symbol<void *(ControllerIndex_t controllerIndex, uint32_t *skus,
                   int32_t count, uint32_t *quantities, int32_t currency,
                   bool consume, void *onSuccess, void *onFailure)>
    LiveInventory_PurchaseSkus{0x141DFDD10, 0x141E0A7A0, 0x0};
WEAK symbol<const InventoryItem *(ControllerIndex_t controllerIndex,
                                  uint32_t itemId)>
    LiveInventory_GetItem{0x141DFC540, 0x141E08FD0, 0x0};
WEAK symbol<EngineDependentDvar> inventory_blocking{0x15114E3E8, 0x1511CD368,
                                                    0x0};
} // namespace inventory
} // namespace live
} // namespace game
