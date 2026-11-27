#pragma once

#include <atomic>
#include <cstdint>

namespace utils {
// Connection requests cross DNS/network workers and the main thread. A fresh
// request or explicit cancellation revokes every callback from the prior one.
// This state is intentionally separate from unrelated server-browser queries.
class connection_request_state {
public:
  using token = std::uint64_t;

  token begin() noexcept {
    auto previous = generation_.load(std::memory_order_relaxed);
    token next;
    do {
      next = previous + 1;
      if (next == 0)
        next = 1;
    } while (!generation_.compare_exchange_weak(
        previous, next, std::memory_order_acq_rel, std::memory_order_relaxed));
    return next;
  }

  void cancel() noexcept { (void)begin(); }
  bool current(token value) const noexcept {
    return value != 0 && generation_.load(std::memory_order_acquire) == value;
  }

private:
  std::atomic<token> generation_{};
};
} // namespace utils
