#pragma once

#include <structs/str.hpp>

#include <game/structs/macros.hpp>

#include <cstdint>
namespace game {
namespace loot {

typedef str256_t itemName_t;

struct LootResultItem {
  uint32_t itemId;
  uint32_t itemQuantity;
  itemName_t itemName;
};

enum class LootResultType : uint32_t {
  INVALID = 0,
  SUCCESS = 1,
  FAILURE_INSUFFICIENT_FUNDS = 2,
  FAILURE_BAD_RESPONE = 3,
  FAILURE = 4,
  COUNT = 5,
};

PACKED(struct LootResult {
  bool isValid;
  uint8_t _padding01[3];
  LootResultType result;
  LootResultItem granted[4];
  LootResultItem all[4];
  int32_t bonusCryptoKeys;
});

} // namespace loot
} // namespace game
