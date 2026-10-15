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
WEAK symbol<bool(ControllerIndex_t controllerIndex)> LiveInventory_IsValid{
    0x141DFD920, 0x141E0A3B0, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex)> LiveInventory_Update{
    0x141DFDF10, 0x141E0A9A0, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t currency,
                 int32_t newValue)>
    LiveInventory_UpdatePlayerBalance{0x141DFE0C0, 0x141E0AB50, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, uint32_t itemId,
                 uint32_t newAmount, uint32_t modDateTime,
                 uint32_t collisionField, uint16_t)>
    LiveInventory_UpdateItemQuantity{0x141DFDFA0, 0x141E0AA30, 0x0};
WEAK symbol<void *(ControllerIndex_t controllerIndex, uint32_t *itemIds,
                   int32_t count, uint32_t *amounts)>
    LiveInventory_ConsumeItem{0x141DFC180, 0x141E08C10, 0x0};
WEAK symbol<EngineDependentDvar> inventory_blocking{0x15114E3E8, 0x1511CD368,
                                                    0x0};
} // namespace inventory
} // namespace live
} // namespace game
