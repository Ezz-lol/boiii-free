#pragma once

#include <game/structs/core.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace inventory {
// Player inventory entry returned by `LiveInventory_GetItem`, the
// engine-side copy of `bdMarketplaceInventory` read by `GetInventoryItem`.
struct InventoryItem {
  uint32_t itemId;
  uint32_t itemQuantity;
  uint32_t modDateTime;
  uint32_t expireDateTime;
  uint16_t collisionField;
};
ASSERT_SIZE(InventoryItem, 0x14);

// Item and count passed to `LiveInventory_BurnDuplicates`, filled by
// `GetBurnableItems` from the inventory.
struct bdMarketplaceItemToPawn {
  uint8_t _padding00[0x10];
  uint32_t itemId;
  uint32_t quantity;
  uint16_t collisionField;
  uint8_t _padding1A[0x16];
};
ASSERT_SIZE(bdMarketplaceItemToPawn, 0x30);
ASSERT_OFFSET(bdMarketplaceItemToPawn, itemId, 0x10);
ASSERT_OFFSET(bdMarketplaceItemToPawn, quantity, 0x14);
ASSERT_OFFSET(bdMarketplaceItemToPawn, collisionField, 0x18);
} // namespace inventory
} // namespace live
} // namespace game
