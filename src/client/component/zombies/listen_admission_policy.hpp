#pragma once

#include <cstdint>
#include <span>

namespace zombies::coop {
constexpr bool requires_game_lobby_membership(bool listen_client, bool expanded,
    bool native_loopback) noexcept {
  return listen_client && expanded && !native_loopback;
}
constexpr bool authenticated_member_matches(std::uint64_t authenticated,
    std::uint64_t member, bool active_client) noexcept {
  return authenticated != 0 && authenticated == member && active_client;
}

struct closed_custom_join_policy {
  bool prior_success{}, listen_client{}, feature{}, zombies{}, game_host{};
  bool live{}, custom{}, closed{}, known_or_local{};
  int capacity{};
};
constexpr bool deny_closed_custom_join(const closed_custom_join_policy &in) noexcept {
  return in.prior_success && in.listen_client && in.feature && in.zombies &&
      in.game_host && in.live && in.custom && in.closed &&
      in.capacity >= 5 && in.capacity <= 8 && !in.known_or_local;
}


// The CB callback exposes no verified party-member batch. A local-looking or
// already-known sender cannot authorize uninspected carried members.
constexpr bool deny_opaque_closed_custom_join(closed_custom_join_policy in) noexcept {
  in.known_or_local = false;
  return deny_closed_custom_join(in);
}

template <typename Known>
bool closed_join_members_known(std::span<const std::uint64_t> members,
    int declared_count, const Known &known) {
  if (declared_count <= 0 || static_cast<std::size_t>(declared_count) > members.size())
    return false;
  for (int i = 0; i < declared_count; ++i)
    if (!members[static_cast<std::size_t>(i)] || !known(members[static_cast<std::size_t>(i)]))
      return false;
  return true;
}
} // namespace zombies::coop
