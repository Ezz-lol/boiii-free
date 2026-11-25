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

// Use the current target, never a historical high-water player count.
// Already spawned AI complete naturally when the reduced target is exceeded.
constexpr int remaining_spawns(const int target, const int spawned) {
  return target <= 0 ? 0 : target - (std::min)(target, (std::max)(0, spawned));
}

// Keep consumed spawns separately: a target reduction may clamp pending spawns
// to zero, but a later join must not forget zombies already created this round.
class round_budget {
public:
  bool begin(const int target, const int remaining, const int respawns,
             const int recycled = 0) {
    active_ = valid_counters(remaining, respawns) && target >= 0 &&
              recycled >= 0 && remaining - respawns <= target;
    if (!active_)
      return false;
    consumed_ = target - (remaining - respawns);
    last_pending_ = remaining - respawns;
    last_recycled_ = recycled;
    recycle_credits_ = 0;
    return true;
  }

  std::optional<int> update(const int remaining, const int respawns,
                            const int target, const int recycled = 0) {
    if (!active_ || !valid_counters(remaining, respawns) || target < 0 ||
        recycled < last_recycled_)
      return invalidate();
    const auto pending = remaining - respawns;
    const auto recycle_delta = recycled - last_recycled_;
    // Stock recycling adds total++ and cumulative total_subtract++; explicit
    // map cleanup replacements instead add total++ and respawns++.
    const auto available =
        static_cast<std::int64_t>(last_pending_) + recycle_delta;
    const auto credits =
        static_cast<std::int64_t>(recycle_credits_) + recycle_delta;
    const auto maximum = (std::numeric_limits<int>::max)();
    if (available > maximum || credits > maximum || pending > available)
      return invalidate();
    const auto spawns = available - pending;
    // Polling cannot order a spawn and a new recycle in the same sample.
    // Retain the maximum queued credits consistent with the observed queue.
    const auto next_credits =
        (std::min)(static_cast<std::int64_t>(pending), credits);
    const auto recycled_spawns = credits - next_credits;
    const auto consumed = consumed_ + spawns - recycled_spawns;
    if (next_credits > pending || consumed > maximum)
      return invalidate();
    const auto next = (std::max)(std::int64_t{0},
                                 static_cast<std::int64_t>(target) - consumed) +
                      next_credits;
    if (next + respawns > maximum)
      return invalidate();
    consumed_ = consumed;
    recycle_credits_ = static_cast<int>(next_credits);
    last_recycled_ = recycled;
    last_pending_ = static_cast<int>(next); // includes queued stock recycling
    return last_pending_ + respawns;
  }

  bool active() const { return active_; }
  std::int64_t consumed() const { return consumed_; }
  int recycle_pending() const { return recycle_credits_; }

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
  int last_recycled_{};
  int recycle_credits_{};
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
  roster() = default;
  roster(const roster &) = delete;
  roster &operator=(const roster &) = delete;

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
    // Duplicate authoritative join notifications are not a reconnect. Keeping
    // the generation also keeps purchase receipts from being reset/replayed.
    if (players_[slot].identity == identity)
      return players_[slot];
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
    if (!players.active() ||
        (initialized_ && owner_ == &players && map_ == players.map()))
      return;
    initialized_ = true;
    owner_ = &players;
    map_ = players.map();
    remaining_ = machine_stock;
    receipts_.fill({});
  }

  bool commit(const roster &players, const player_token &buyer,
              const std::uint64_t sequence) {
    if (!initialized_ || owner_ != &players || remaining_ == 0 ||
        sequence == 0 || map_ != players.map() || !players.contains(buyer)) {
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
  const roster *owner_{};
  std::uint64_t map_{};
  bool initialized_{};
  int remaining_{machine_stock};
  std::array<receipt, player_limit> receipts_{};
};
} // namespace zombies::coop
