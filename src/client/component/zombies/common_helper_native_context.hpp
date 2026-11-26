#pragma once
// Native session provider. No engine getters or runtime/store
// locks are called by current(). This is lifecycle/source provenance, not auth.
#include <atomic>
#include <memory>
#include <string>
#include <string_view>

namespace zombies::coop::common_helper {
struct native_identity {
  int epoch{}, capacity{};
  std::string map;
  mutable std::atomic_bool revoked{false};
};
class native_publication {
public:
  using ticket = std::shared_ptr<const native_identity>;

private:
  std::atomic<ticket> current_;
  static bool valid_map(std::string_view map) noexcept {
    if (map.empty() || map.size() > 64 || map == "core_frontend")
      return false;
    for (char c : map)
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-'))
        return false;
    return true;
  }

public:
  void invalidate() noexcept { current_.store({}, std::memory_order_release); }
  // Only native begin_startup publishes, after receipt resets and native arm.
  // Failure leaves the provider unavailable; caller keeps admission blocked.
  bool publish(int epoch, int capacity, std::string_view map) noexcept {
    invalidate();
    if (epoch < 1 || capacity < 5 || capacity > 8 || !valid_map(map))
      return false;
    try {
      current_.store(std::make_shared<const native_identity>(epoch, capacity,
                                                             std::string(map)),
                     std::memory_order_release);
      return true;
    } catch (...) {
      return false;
    }
  }
  ticket capture() const noexcept {
    return current_.load(std::memory_order_acquire);
  }
  // native_epoch/capacity/ending/failed MUST be supplied by existing
  // atomic-only APIs. No map dvar/frozen_schema_map()/state_mutex/context_mutex
  // here.
  bool owned(const ticket &saved, int native_epoch,
             int capacity) const noexcept {
    return saved && saved == capture() && saved->epoch == native_epoch &&
           saved->capacity == capacity;
  }
  bool provenance_current(const ticket &saved, int native_epoch, int capacity,
                          bool failed) const noexcept {
    return !failed && owned(saved, native_epoch, capacity) &&
           !saved->revoked.load(std::memory_order_acquire);
  }
  bool current(const ticket &saved, int native_epoch, int capacity, bool ending,
               bool failed) const noexcept {
    return !ending && provenance_current(saved, native_epoch, capacity, failed);
  }
  // The current ending epoch may still inspect/execute its owned helper. A
  // native detour on that helper withdraws proof, and joins, using this atomic.
  bool revoke(const ticket &saved) noexcept {
    if (!saved || saved != capture())
      return false;
    saved->revoked.store(true, std::memory_order_release);
    return true; // a racing successor has a distinct token and is untouched.
  }
  // This precheck runs OUTSIDE all store/registry/runtime locks. Engine strings
  // are already copied and mode/frontend already sampled by the real caller.
  bool sampled(const ticket &saved, bool zombies, bool frontend,
               std::string_view actual_map, std::string_view frozen_map,
               int native_epoch, int capacity, bool /*ending*/,
               bool failed) const noexcept {
    return zombies && !frontend && saved && valid_map(actual_map) &&
           saved->map == actual_map && saved->map == frozen_map &&
           provenance_current(saved, native_epoch, capacity, failed);
  }
};
// Concrete definitions belong beside the actual native admission atomics in
// zombies_coop.cpp beside the native admission atomics.
using native_ticket = native_publication::ticket;
native_ticket capture_native_session() noexcept;
void invalidate_native_session() noexcept;
bool native_session_current(const native_ticket &saved) noexcept;
bool native_session_owned(const native_ticket &saved) noexcept;
bool native_admission_current() noexcept;
bool revoke_native_session(const native_ticket &saved) noexcept;
} // namespace zombies::coop::common_helper
