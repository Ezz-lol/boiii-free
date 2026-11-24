#pragma once

#include <cstdint>
#include <string_view>

namespace zombies::coop {
// The configured value selects the next custom lobby, not Solo/Public.
// Never raise an already allocated host capacity. Zero rejects an invalid
// configuration or an opted-in expansion whose native capacity mismatches.
// With expansion off, foreign capacities remain outside this policy.
constexpr int resolve_session_limit(int configured, int capacity) {
  if (configured < 1 || configured > 8 || capacity < 1)
    return 0;
  if (capacity <= 4)
    return capacity;
  if (configured <= 4)
    return configured;
  return capacity == configured ? configured : 0;
}

// Main-thread state only. Network handlers consume a separately published
// atomic.
class startup_gate {
public:
  enum class fault {
    none,
    adapter_declined,
    wrong_epoch,
    wrong_map,
    heartbeat_lost,
    timeout
  };
  void arm(int epoch, int limit) {
    epoch_ = epoch;
    active_ = limit > 4;
    ready_ = false;
    running_since_ = -1;
    progress_at_ = 0;
    heartbeat_ = 0;
    fault_ = fault::none;
  }
  void reset() { arm(0, 4); }
  fault observe(std::int64_t now_ms, bool running, std::string_view status,
                int ready_epoch, bool same_map, int heartbeat) {
    if (!active_ || fault_ != fault::none)
      return fault_;
    if (!running) {
      ready_ = false;
      running_since_ = -1;
      return fault_;
    }
    if (running_since_ < 0)
      running_since_ = now_ms;
    if (status != "initializing" && status != "disabled" &&
        status != "experimental_callbacks_installed")
      return fail(fault::adapter_declined);
    if (ready_) {
      if (ready_epoch != epoch_)
        return fail(fault::wrong_epoch);
      if (!same_map)
        return fail(fault::wrong_map);
      if (status != "experimental_callbacks_installed" ||
          heartbeat < heartbeat_)
        return fail(fault::heartbeat_lost);
    }
    if (status == "experimental_callbacks_installed" && ready_epoch == epoch_ &&
        same_map && heartbeat >= 0) {
      if (!ready_ || heartbeat > heartbeat_)
        progress_at_ = now_ms;
      heartbeat_ = heartbeat;
      ready_ = true;
      // GSC time legitimately stops during pause/timescale changes.
      // A stationary counter is not a failure; epochs/status revoke readiness.
    }
    if (!ready_ && now_ms - running_since_ > 40000)
      return fail(fault::timeout);
    return fault_;
  }
  bool ready() const { return !active_ || (ready_ && fault_ == fault::none); }

private:
  fault fail(fault reason) {
    ready_ = false;
    return fault_ = reason;
  }
  int epoch_{};
  int heartbeat_{};
  bool active_{};
  bool ready_{};
  std::int64_t running_since_{-1};
  std::int64_t progress_at_{};
  fault fault_{fault::none};
};
} // namespace zombies::coop
