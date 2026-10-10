#pragma once

#include <std_include.hpp>

// Local SQLite store for data DW's servers would normally keep, such as
// inventory items and currency balances.
namespace local_db {
struct item {
  uint32_t quantity;
  uint32_t expire_time;
};

uint32_t balance(int32_t currency);
bool set_balance(int32_t currency, uint32_t value);

std::optional<item> get_item(uint32_t item_id);
bool set_item(uint32_t item_id, uint32_t quantity, uint32_t expire_time = 0);

// Runs `body` in one transaction, which is rolled back if it returns false.
bool transaction(const std::function<bool()> &body);
} // namespace local_db
