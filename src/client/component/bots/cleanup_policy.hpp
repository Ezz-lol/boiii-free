#pragma once
#include "observation_policy.hpp"

namespace bots::cleanup {
// A terminal, current native identity grants only removal of native test
// clients. It is never convertible to allocation readiness or the strict drop
// snapshot.
struct snapshot {
  observation::snapshot identity;
  bool ending_verified{};
};
inline bool valid(const snapshot &value) noexcept {
  return value.ending_verified && observation::valid(value.identity) &&
         value.identity.mode == 0 && value.identity.capacity >= 5 &&
         value.identity.capacity <= 8 &&
         value.identity.frozen_capacity == value.identity.capacity &&
         value.identity.native_epoch > 0 && value.identity.source_current;
}
inline bool same_session(const snapshot &requested,
                         const snapshot &current) noexcept {
  return valid(requested) && valid(current) &&
         observation::same_session(requested.identity, current.identity);
}
inline bool may_cleanup_test_client(const snapshot &requested,
                                    const snapshot &current,
                                    std::uint64_t generation, int slot,
                                    bool connected,
                                    bool native_is_test_client) noexcept {
  return generation && generation == requested.identity.epoch &&
         same_session(requested, current) && slot >= 0 &&
         slot < current.identity.capacity && connected && native_is_test_client;
}
} // namespace bots::cleanup
