#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

namespace zombies::coop {
inline constexpr int stock_limit = 4;
inline constexpr int player_limit = 8;
inline constexpr int machine_stock = 16;

constexpr bool valid_limit(const int value) {
  return value >= 1 && value <= player_limit;
}

constexpr int configured_limit(const int value) {
  return valid_limit(value) ? value : stock_limit;
}

// Expanded adapters are deliberately limited until each map's quest/character
// ownership, HUD and special rounds have been audited.
constexpr bool supported_map(const std::string_view map) {
  return map == "zm_zod" || map == "zm_tomb";
}

// Use the current target, never a historical high-water player count.
// Already spawned AI complete naturally when the reduced target is exceeded.
constexpr int remaining_spawns(const int target, const int spawned) {
  return target <= 0 ? 0 : target - (std::min)(target, (std::max)(0, spawned));
}

// Keep consumed spawns separately: a target reduction may clamp pending spawns
// to zero, but a later join must not forget zombies already created this round.
class round_budget {
public:
  bool begin(const int target, const int remaining, const int respawns) {
    active_ = valid_counters(remaining, respawns) && target >= 0 &&
              remaining - respawns <= target;
    if (!active_)
      return false;
    consumed_ = target - (remaining - respawns);
    last_pending_ = remaining - respawns;
    return true;
  }

  std::optional<int> update(const int remaining, const int respawns,
                            const int target) {
    if (!active_ || !valid_counters(remaining, respawns) || target < 0)
      return invalidate();
    const auto pending = remaining - respawns;
    // Unexpected additions belong to a map/mod controller. Stop adapting it.
    if (pending > last_pending_)
      return invalidate();
    consumed_ += static_cast<std::int64_t>(last_pending_) - pending;
    const auto next = (std::max)(std::int64_t{0},
                                 static_cast<std::int64_t>(target) - consumed_);
    if (next + respawns > (std::numeric_limits<int>::max)())
      return invalidate();
    last_pending_ = static_cast<int>(next);
    return last_pending_ + respawns;
  }

  bool active() const { return active_; }
  std::int64_t consumed() const { return consumed_; }

private:
  static bool valid_counters(const int remaining, const int respawns) {
    return remaining >= 0 && respawns >= 0 && respawns <= remaining;
  }
  std::optional<int> invalidate() {
    active_ = false;
    return std::nullopt;
  }
  bool active_{};
  int last_pending_{};
  std::int64_t consumed_{};
};

struct player_token {
  std::uint64_t map{};
  std::uint64_t generation{};
  std::uint64_t identity{};
  int slot{-1};
  bool operator==(const player_token &) const = default;
};

// Fixed capacity matches the Zombies feature, not the native MP wire layout.
class roster {
public:
  void reset(const int limit) {
    ++map_;
    active_ = true;
    limit_ = configured_limit(limit);
    players_.fill({});
  }

  void shutdown() {
    ++map_;
    active_ = false;
    players_.fill({});
  }

  std::optional<player_token> join(const int slot,
                                   const std::uint64_t identity) {
    if (!active_ || slot < 0 || slot >= limit_ || identity == 0) {
      return std::nullopt;
    }
    // A reconnect may claim a new slot before the old disconnect callback.
    for (auto &previous : players_) {
      if (previous.identity == identity)
        previous = {};
    }
    auto &player = players_[slot];
    player = {map_, ++generation_, identity, slot};
    return player;
  }

  bool contains(const player_token &token) const {
    return active_ && token.identity && token.map == map_ && token.slot >= 0 &&
           token.slot < limit_ && players_[token.slot] == token;
  }

  bool leave(const player_token &token) {
    if (!contains(token))
      return false;
    players_[token.slot] = {};
    return true;
  }

  int count() const {
    return static_cast<int>(std::count_if(
        players_.begin(), players_.end(),
        [](const player_token &player) { return player.identity != 0; }));
  }

  std::uint64_t map() const { return map_; }
  bool active() const { return active_; }

private:
  int limit_{stock_limit};
  bool active_{};
  std::uint64_t map_{};
  std::uint64_t generation_{};
  std::array<player_token, player_limit> players_{};
};

// A per-machine shared stock. Sequence numbers belong to confirmed purchases,
// not incoming requests. Map adapters call this on the authoritative GSC
// thread.
class finite_stock {
public:
  void reset(const roster &players) {
    if (!players.active() || (initialized_ && map_ == players.map()))
      return;
    initialized_ = true;
    map_ = players.map();
    remaining_ = machine_stock;
    receipts_.fill({});
  }

  bool commit(const roster &players, const player_token &buyer,
              const std::uint64_t sequence) {
    if (!initialized_ || remaining_ == 0 || sequence == 0 ||
        map_ != players.map() || !players.contains(buyer)) {
      return false;
    }
    auto &receipt = receipts_[buyer.slot];
    if (receipt.generation == buyer.generation &&
        sequence <= receipt.sequence) {
      return false;
    }
    receipt = {buyer.generation, sequence};
    --remaining_;
    return true;
  }

  int remaining() const { return remaining_; }

private:
  struct receipt {
    std::uint64_t generation{};
    std::uint64_t sequence{};
  };
  std::uint64_t map_{};
  bool initialized_{};
  int remaining_{machine_stock};
  std::array<receipt, player_limit> receipts_{};
};
} // namespace zombies::coop
