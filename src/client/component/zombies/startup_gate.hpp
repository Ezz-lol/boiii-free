#pragma once

#include "ending_policy.hpp"
#include <cstdint>
#include <string_view>

namespace zombies::coop {
// The saved value selects the next custom lobby. The already allocated
// native capacity is authoritative for an opted-in hosted/joined session.
// Never enlarge a pool here; stock Solo/Public retain their native capacity.
// Without opt-in, foreign >4 capacities remain outside this policy.
constexpr int resolve_session_limit(int configured, int capacity,
                                    bool expanded_enabled) {
  if (configured < 1 || configured > 8 || capacity < 1 || capacity > 18)
    return 0;
  if (capacity <= 4)
    return capacity;
  if (!expanded_enabled)
    return configured <= 4 ? configured : 0;
  return capacity <= 8 ? capacity : 0;
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
    timeout,
    stopped,
    schema_lost
  };
  void arm(int epoch, int limit) {
    epoch_ = epoch;
    active_ = limit > 4;
    ready_ = false;
    ending_ = false;
    running_since_ = -1;
    progress_at_ = 0;
    heartbeat_ = 0;
    fault_ = fault::none;
  }
  void reset() { arm(0, 4); }
  fault observe(std::int64_t now_ms, bool running, std::string_view status,
                int ready_epoch, bool same_map, int heartbeat,
                bool schema_ready = true) {
    if (!active_ || fault_ != fault::none)
      return fault_;
    if (ending_)
      return fault::none;
    if (!running) {
      // A cold load can wait indefinitely before the first running
      // observation. After that, a stop needs an explicit fresh arm;
      // old same-map dvars must never resurrect a torn-down adapter.
      if (running_since_ >= 0)
        return fail(fault::stopped);
      return fault_;
    }
    if (running_since_ < 0)
      running_since_ = now_ms;
    if (status != "initializing" && status != "disabled" &&
        status != "experimental_callbacks_installed")
      return fail(fault::adapter_declined);
    if (ready_) {
      if (!schema_ready)
        return fail(fault::schema_lost);
      if (ready_epoch != epoch_)
        return fail(fault::wrong_epoch);
      if (!same_map)
        return fail(fault::wrong_map);
      if (status != "experimental_callbacks_installed" ||
          heartbeat < heartbeat_)
        return fail(fault::heartbeat_lost);
    }
    if (schema_ready && status == "experimental_callbacks_installed" &&
        ready_epoch == epoch_ && same_map && heartbeat >= 0) {
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
  bool close_for_ending(int epoch, bool same_map, int revision) noexcept {
    if (!active_ || fault_ != fault::none || epoch < 1 || epoch != epoch_ ||
        !same_map || revision != ending_policy::receipt_revision)
      return false;
    ending_ = true;
    ready_ = false;
    return true;
  }
  bool ending() const noexcept { return ending_; }
  bool ready() const {
    return !active_ || (ready_ && !ending_ && fault_ == fault::none);
  }

private:
  fault fail(fault reason) {
    ready_ = false;
    return fault_ = reason;
  }
  int epoch_{};
  int heartbeat_{};
  bool active_{};
  bool ready_{};
  bool ending_{};
  std::int64_t running_since_{-1};
  std::int64_t progress_at_{};
  fault fault_{fault::none};
};
} // namespace zombies::coop
