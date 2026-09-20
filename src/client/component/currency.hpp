#pragma once

#include <std_include.hpp>

#include <game/game.hpp>

namespace currency {
std::optional<uint32_t> item_quantity(game::ControllerIndex_t controller,
                                      uint32_t inventory_id);
std::optional<uint32_t> add_cod_points(game::ControllerIndex_t controller,
                                       uint32_t amount);
bool purchase_vials(game::ControllerIndex_t controller, uint32_t cost,
                    uint32_t amount);
bool purchase_distills(game::ControllerIndex_t controller,
                       std::string_view kind, uint32_t currency);
bool cook_recipe(game::ControllerIndex_t controller, uint32_t recipe,
                 bool free_distills);
uint32_t distill_balance(bool free);
uint32_t free_distill_cooldown();
bool reset_gobblegums(game::ControllerIndex_t controller);
bool set_currencies_maxed(game::ControllerIndex_t controller, bool maxed);

namespace accounting {
/*
   FIXME: a `9999999` value is not related to the HavokScript VM's exclusive
   `HksNumber` floating point numeric representation. Why was `9999999`
   selected? What is the true limit?
*/
// HKS numbers are floats; keep displayed balances exactly representable.
constexpr uint32_t max_balance = 9999999;

inline uint32_t match_reward(int64_t seconds, uint32_t per_minute,
                             uint32_t cap) {
  if (seconds <= 0 || !per_minute) {
    return 0;
  }
  const uint64_t minutes = static_cast<uint64_t>(seconds / 60);
  return minutes > cap / per_minute
             ? cap
             : static_cast<uint32_t>(minutes * per_minute);
}

inline uint32_t remaining(uint32_t gained, uint32_t used) {
  return gained > used ? std::min(gained - used, max_balance) : 0;
}

inline std::optional<uint32_t> debit(uint32_t gained, uint32_t used,
                                     uint32_t amount) {
  if (amount > remaining(gained, used)) {
    return std::nullopt;
  }
  return used + amount;
}

inline std::optional<uint32_t> credit(uint32_t gained, uint32_t amount) {
  if (amount > std::numeric_limits<uint32_t>::max() - gained) {
    return std::nullopt;
  }
  return gained + amount;
}

inline std::optional<uint32_t> parse_amount(const std::string_view &text) {
  uint32_t amount = 0;
  const auto [end, error] =
      std::from_chars(text.data(), text.data() + text.size(), amount);
  if (error != std::errc{} || end != text.data() + text.size() ||
      amount > max_balance) {
    return std::nullopt;
  }
  return amount;
}
} // namespace accounting
} // namespace currency