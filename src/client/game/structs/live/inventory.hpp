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
} // namespace inventory
} // namespace live
} // namespace game
