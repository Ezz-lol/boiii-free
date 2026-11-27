#pragma once
// Local lifecycle receipt, never peer authentication or gameplay state.
#include <atomic>
#include <cstdint>
#include <string_view>
namespace zombies::coop::ending_policy {
inline constexpr int receipt_revision = 1;
constexpr bool valid_map(std::string_view map) noexcept {
  if (map.empty() || map.size() > 64 || map == "core_frontend")
    return false;
  for (char c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-'))
      return false;
  return true;
}
constexpr bool matches(bool zombies, int capacity, int native_epoch,
                       std::string_view actual_map, std::string_view frozen_map,
                       std::string_view status, int revision, int receipt_epoch,
                       std::string_view receipt_map) noexcept {
  return zombies && capacity >= 5 && capacity <= 8 && native_epoch > 0 &&
         valid_map(actual_map) && actual_map == frozen_map &&
         status == "ending" && revision == receipt_revision &&
         receipt_epoch == native_epoch && receipt_map == actual_map;
}
// A single atomic tuple prevents stale epochs from closing a successor.
class admission_lifecycle {
  std::atomic<std::uint64_t> state_{};

public:
  void arm(int epoch, bool expanded = true) noexcept {
    state_.store(expanded && epoch > 0 ? static_cast<std::uint64_t>(epoch) << 1
                                       : 0,
                 std::memory_order_release);
  }
  void reset() noexcept { state_.store(0, std::memory_order_release); }
  int epoch() const noexcept {
    return static_cast<int>(state_.load(std::memory_order_acquire) >> 1);
  }
  bool ending() const noexcept {
    return (state_.load(std::memory_order_acquire) & 1u) != 0;
  }
  bool finish(int epoch) noexcept {
    if (epoch < 1)
      return false;
    auto expected = static_cast<std::uint64_t>(epoch) << 1;
    const auto ended = expected | 1u;
    return state_.compare_exchange_strong(expected, ended,
                                          std::memory_order_acq_rel,
                                          std::memory_order_acquire) ||
           expected == ended;
  }
};
} // namespace zombies::coop::ending_policy
