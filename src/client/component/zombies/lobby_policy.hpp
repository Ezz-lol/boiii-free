#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace zombies::coop {
inline constexpr std::size_t lan_host_limit = 128;
inline constexpr std::int64_t lan_query_lifetime_ms = 2000;

struct lobby_policy_input {
  bool enabled{}, zombies{}, local_game_host{}, session_ready{}, custom{};
  bool open{}, stock_content{}, lan{}, live{}, in_match{}, adapter_ready{}, adapter_owned{};
  bool advertise_opt_in{}, lan_discoverable{}, browse_screen{};
  int capacity{}, clients{};
  std::string_view map;
};
struct lobby_policy_result {
  bool active{}, can_advertise{}, advertise{}, lan_respond{};
};
constexpr bool listing_map_token(const std::string_view map) noexcept {
  if (map.size() <= 3 || map.size() > 64 || !map.starts_with("zm_")) return false;
  for (const char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return false;
  return true;
}
constexpr bool listing_official_map(const std::string_view map) noexcept {
  return map == "zm_zod" || map == "zm_factory" || map == "zm_castle" ||
      map == "zm_island" || map == "zm_stalingrad" || map == "zm_genesis" ||
      map == "zm_prototype" || map == "zm_asylum" || map == "zm_sumpf" ||
      map == "zm_theater" || map == "zm_cosmodrome" || map == "zm_temple" ||
      map == "zm_moon" || map == "zm_tomb";
}
constexpr lobby_policy_result
lobby_listing_policy(const lobby_policy_input &in) noexcept {
  // Frontend custom content has not executed its callbacks yet. Only stock
  // official profiles may list there; other maps/mods need a current in-match
  // owned adapter and paired schema, never just a map-name or saved preference.
  const bool content = (in.stock_content && listing_official_map(in.map)) ||
                       (in.in_match && in.adapter_owned);
  const bool active = in.enabled && in.zombies && in.local_game_host &&
                      in.session_ready && in.custom && in.open &&
                      content && listing_map_token(in.map) && (in.lan != in.live) &&
                      in.capacity >= 5 && in.capacity <= 8 && in.clients >= 1 &&
                      in.clients <= in.capacity &&
                      (!in.in_match || (in.adapter_ready && in.adapter_owned));
  const bool browse = in.browse_screen || (in.in_match && in.adapter_ready);
  return {active, active && in.live && browse,
          active && in.live && browse && in.advertise_opt_in,
          active && in.lan && browse && in.lan_discoverable};
}

// Host-order IPv4. Loopback is useful for two local clients; link-local and
// RFC1918 addresses stay on local networks. Public/unset/broadcast sources
// deny.
constexpr bool private_lan_ipv4(const std::uint32_t ip) noexcept {
  const auto a = ip >> 24;
  const auto b = (ip >> 16) & 255;
  return a == 10 || a == 127 || (a == 172 && b >= 16 && b <= 31) ||
         (a == 192 && b == 168) || (a == 169 && b == 254);
}
constexpr bool valid_lan_nonce(const std::string_view payload) noexcept {
  if (payload.size() != 32 && payload.size() != 64)
    return false;
  for (const char c : payload)
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F')))
      return false;
  return true;
}
constexpr bool accept_lan_reply(const std::string_view expected,
                                const std::string_view received,
                                const std::int64_t started,
                                const std::int64_t now,
                                const std::size_t count) noexcept {
  return valid_lan_nonce(expected) && valid_lan_nonce(received) &&
         expected == received && now >= started &&
         now - started < lan_query_lifetime_ms && count < lan_host_limit;
}

// Bounded state: at most 16 replies/s globally and one reply/IP/s. Full tables
// deny new peers until their entries expire instead of allocating on input.
class lan_reply_limiter {
public:
  bool allow(const std::uint32_t ip, const std::int64_t now) noexcept {
    if (!private_lan_ipv4(ip))
      return false;
    if (now < global_started_ || now - global_started_ >= 1000) {
      global_started_ = now;
      global_count_ = 0;
    }
    if (global_count_ >= 16)
      return false;
    entry *slot = nullptr;
    for (auto &peer : peers_) {
      if (peer.used && peer.ip == ip) {
        if (now >= peer.time && now - peer.time < 1000)
          return false;
        slot = &peer;
        break;
      }
      if (!slot &&
          (!peer.used || (now >= peer.time && now - peer.time >= 1000)))
        slot = &peer;
    }
    if (!slot)
      return false;
    *slot = {ip, now, true};
    ++global_count_;
    return true;
  }

private:
  struct entry {
    std::uint32_t ip{};
    std::int64_t time{};
    bool used{};
  };
  std::array<entry, 64> peers_{};
  std::int64_t global_started_{};
  unsigned global_count_{};
};
} // namespace zombies::coop
