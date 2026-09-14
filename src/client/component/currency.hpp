#pragma once

#include <cstdint>

#include <game/game.hpp>

namespace currency {
// The two premium currencies the retail game backs with Demonware. Both are
// emulated locally here - see currency.cpp for why.
enum class kind {
  // "CoD Points" in the UI.
  cod_points,
  // "Liquid Divinium" in the UI, `ZMVials` in the engine.
  divinium,
};

// Master switch (`cg_local_currency`). When false every hook in this component
// falls through to the engine, restoring stock (broken) behaviour.
bool enabled();

int64_t get(kind k);
void set(kind k, int64_t amount);
void add(kind k, int64_t amount);
// Returns false and changes nothing when the balance is insufficient.
bool spend(kind k, int64_t amount);

// Gobblegums won at Dr. Monty's Factory, keyed by session mode. Stock rather
// than ownership: they are consumed by use, so the same one won twice is worth
// two. Returns 0 for anything we never handed out, which lets the caller fall
// back to the engine.
int32_t owned_quantity(game::eModes mode, int32_t item_id);
void add_owned(game::eModes mode, int32_t item_id, int32_t quantity = 1);

// Flushes the wallet to disk. Called automatically after every mutation.
void save();
} // namespace currency
