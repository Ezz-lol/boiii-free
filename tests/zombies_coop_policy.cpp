#include "../src/client/component/zombies/coop_policy.hpp"
#include <cassert>
#include <iostream>
#include <type_traits>

using namespace zombies::coop;

int main() {
  static_assert(!std::is_copy_constructible_v<roster> &&
                !std::is_copy_assignable_v<roster>);
  assert(supported_map("zm_zod") && supported_map("zm_tomb"));
  assert(!supported_map("zm_castle") && !supported_map("core_frontend") &&
         !supported_map(""));
  for (int limit = 1; limit <= 8; ++limit) {
    roster players;
    assert(!players.join(0, 1));
    players.reset(limit);
    std::array<player_token, 8> tokens{};
    for (int slot = 0; slot < limit; ++slot) {
      tokens[slot] = players.join(slot, 100 + slot).value();
    }
    assert(players.count() == limit);
    assert(!players.join(limit, 999));
    assert(!players.join(-1, 999));
    assert(!players.join(0, 0));
    assert(players.count() == limit);
    assert(players.leave(tokens[0]));
    auto next = players.join(0, 100).value();
    assert(!players.leave(tokens[0]));
    assert(players.contains(next));
    players.shutdown();
    assert(players.count() == 0 && !players.contains(next));
    assert(!players.join(0, 100));
  }

  assert(configured_limit(0) == 4 && configured_limit(9) == 4);
  assert(remaining_spawns(50, 30) == 20);
  assert(remaining_spawns(25, 30) == 0);
  assert(remaining_spawns(0, 30) == 0);
  assert(remaining_spawns(50, -1) == 50);
  // 8 -> 4 -> 8 uses each current target; already-spawned zombies aren't
  // recreated.
  assert(remaining_spawns(100, 30) == 70);
  round_budget budget;
  assert(budget.begin(
      100, 55, 5)); // 50 original spawns consumed; five replacements pending
  assert(budget.update(55, 5, 70) == 25);
  assert(budget.update(23, 3, 100) ==
         53); // two replacements spawned, no new normal spawns
  assert(budget.consumed() == 50);
  assert(budget.begin(100, 20, 0)); // 80 already spawned
  assert(budget.update(20, 0, 50) == 0);
  assert(budget.update(0, 0, 100) ==
         20); // 8 -> 4 -> 8 cannot create 50 extra zombies
  assert(budget.consumed() == 80);
  assert(budget.update(15, 0, 100) == 15);
  assert(budget.consumed() == 85);
  assert(!budget.update(16, 0, 100) &&
         !budget.active());           // custom map took ownership
  assert(!budget.update(15, 0, 100)); // remains disabled until next round
  assert(!budget.begin(100, 20, 21));
  assert(!budget.begin(100, 101, 0));
  assert(!budget.begin(-1, 20, 0));
  assert(budget.begin(100, 20, 0));
  assert(!budget.update(20, 0, -1));
  assert(budget.begin(0, 2147483647, 2147483647));
  assert(!budget.update(2147483647, 2147483647, 1));

  assert(remaining_spawns(50, 30) == 20);
  assert(remaining_spawns(100, 30) == 70);

  roster players;
  players.reset(8);
  auto buyer = players.join(0, 42).value();
  finite_stock machine;
  machine.reset(players);
  assert(machine.commit(players, buyer, 1));
  assert(!machine.commit(players, buyer, 1));
  const auto duplicate_join = players.join(0, 42).value();
  assert(duplicate_join == buyer);
  assert(!machine.commit(players, duplicate_join,
                         1)); // duplicate join cannot replay a purchase
  machine.reset(players);
  assert(machine.remaining() ==
         15); // repeated initialization cannot refill stock
  for (std::uint64_t sequence = 2; sequence <= 16; ++sequence) {
    assert(machine.commit(players, buyer, sequence)); // no two-per-player quota
  }
  assert(machine.remaining() == 0 && !machine.commit(players, buyer, 17));
  players.reset(8);
  machine.reset(players);
  assert(machine.remaining() == 16);
  assert(!machine.commit(players, buyer, 18)); // old map callback
  buyer = players.join(0, 42).value();
  assert(machine.commit(players, buyer, 1));
  auto moved = players.join(7, 42).value();
  assert(players.count() == 1 && !players.contains(buyer));
  assert(!machine.commit(players, buyer, 2));
  assert(machine.commit(players, moved, 1));
  assert(machine.remaining() == 14); // reconnect doesn't refill a machine
  roster other_map_owner;
  other_map_owner.reset(8);
  const auto other_buyer = other_map_owner.join(0, 42).value();
  // Map/generation counters are scoped to an authority, not globally unique.
  assert(!machine.commit(other_map_owner, other_buyer, 2));
  machine.reset(other_map_owner);
  assert(machine.remaining() == 16);
  assert(!machine.commit(players, moved, 2));
  assert(machine.commit(other_map_owner, other_buyer, 1));
  roster authority_a;
  roster authority_b;
  authority_a.reset(8);
  authority_b.reset(8);
  const auto token_a = authority_a.join(0, 1234).value();
  const auto token_b = authority_b.join(0, 1234).value();
  assert(token_a == token_b); // colliding local counters are deliberate
  finite_stock authority_stock;
  authority_stock.reset(authority_a);
  assert(authority_stock.commit(authority_a, token_a, 1));
  assert(!authority_stock.commit(authority_b, token_b, 2));
  assert(authority_stock.remaining() == 15);
  authority_stock.reset(authority_b);
  assert(authority_stock.remaining() == 16);
  assert(!authority_stock.commit(authority_a, token_a, 2));
  assert(authority_stock.commit(authority_b, token_b, 1));
  std::cout << "PASS: 1-8 capacity, roster lifecycle, current round budgets, "
               "finite shared stock\n";
}
