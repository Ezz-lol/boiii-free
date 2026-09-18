#pragma once

#include <cstdint>

#include <game/game.hpp>

namespace currency {
enum class kind {
  cod_points,
  divinium,
};

bool enabled();

int64_t get(kind k);
void set(kind k, int64_t amount);
void add(kind k, int64_t amount);
bool spend(kind k, int64_t amount);

int32_t owned_quantity(game::eModes mode, int32_t item_id);
void add_owned(game::eModes mode, int32_t item_id, int32_t quantity = 1);

void save();
} // namespace currency
