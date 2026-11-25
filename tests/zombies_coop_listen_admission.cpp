#include "../src/client/component/zombies/listen_admission_policy.hpp"
#include <cassert>
#include <array>
#include <initializer_list>
using namespace zombies::coop;
int main() {
  for (const bool client : {false, true})
    for (const bool expanded : {false, true})
      for (const bool loopback : {false, true})
        assert(requires_game_lobby_membership(client, expanded, loopback) ==
               (client && expanded && !loopback));
  assert(requires_game_lobby_membership(true, true, false)); // includes RAWIP 127/8
  assert(!requires_game_lobby_membership(false, true, false)); // dedicated unchanged
  assert(!requires_game_lobby_membership(true, false, false)); // stock four-player unchanged
  assert(authenticated_member_matches(42, 42, true));
  assert(!authenticated_member_matches(42, 42, false));
  assert(!authenticated_member_matches(42, 43, true));
  assert(!authenticated_member_matches(0, 0, true));

  closed_custom_join_policy closed{};
  closed.prior_success = closed.listen_client = closed.feature = closed.zombies = true;
  closed.game_host = closed.live = closed.custom = closed.closed = true;
  closed.capacity = 8;
  assert(deny_closed_custom_join(closed));
  for (auto field : {&closed_custom_join_policy::prior_success,
      &closed_custom_join_policy::listen_client, &closed_custom_join_policy::feature,
      &closed_custom_join_policy::zombies, &closed_custom_join_policy::game_host,
      &closed_custom_join_policy::live, &closed_custom_join_policy::custom,
      &closed_custom_join_policy::closed}) {
    closed.*field = false;
    assert(!deny_closed_custom_join(closed) && !deny_opaque_closed_custom_join(closed));
    closed.*field = true;
  }
  closed.known_or_local = true; // active GAME/PRIVATE, local XUID or native loopback
  assert(!deny_closed_custom_join(closed));
  assert(deny_opaque_closed_custom_join(closed)); // CB cannot inspect a carried batch
  closed.known_or_local = false;
  for (int capacity = -1; capacity <= 19; ++capacity) {
    closed.capacity = capacity;
    assert(deny_closed_custom_join(closed) == (capacity >= 5 && capacity <= 8));
  }

  const std::array<std::uint64_t, 3> party{42, 43, 0};
  const auto known = [](std::uint64_t id) { return id == 42 || id == 43; };
  assert(closed_join_members_known(party, 2, known));
  assert(!closed_join_members_known(party, 3, known)); // zero is not an active member
  const std::array<std::uint64_t, 2> injected{42, 99};
  assert(!closed_join_members_known(injected, 2, known)); // known sender cannot carry unknown
  assert(closed_join_members_known(injected, 1, known));
  for (const int count : {-1, 0, 4, 19})
    assert(!closed_join_members_known(party, count, known));
  const std::span<const std::uint64_t> empty;
  assert(!closed_join_members_known(empty, 1, known));
}
