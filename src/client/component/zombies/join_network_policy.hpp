#pragma once

namespace zombies::coop {
// The local engine's selected mode is authoritative for Zombies. A peer's
// message must not switch a LAN host into the online friend-join path.
template <typename NetworkMode>
constexpr NetworkMode join_network_mode(bool client, bool zombies,
                                       NetworkMode native_mode) noexcept {
  return client && !zombies ? NetworkMode::LIVE : native_mode;
}

template <typename NetworkMode>
constexpr bool network_join_allowed(bool mod_loaded, bool local_player,
                                    NetworkMode host_mode) noexcept {
  return mod_loaded || local_player || host_mode == NetworkMode::LIVE ||
         host_mode == NetworkMode::LAN;
}

template <typename NetworkMode, typename JoinType>
constexpr bool lan_join_request_allowed(NetworkMode request_mode,
                                        JoinType join_type) noexcept {
  return request_mode == NetworkMode::LAN &&
         (join_type == JoinType::PARTY || join_type == JoinType::PLAYLIST);
}
} // namespace zombies::coop
