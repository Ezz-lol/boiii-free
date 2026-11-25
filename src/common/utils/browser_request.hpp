#pragma once

#include <cstddef>
#include <cstdint>

namespace utils {
// Kept under the browser owner's mutex. A token belongs to one discovery/query
// batch, so a late UDP response cannot complete or overwrite a newer request.
class browser_request_state {
public:
  using token = std::uint64_t;

  token begin_discovery() noexcept {
    advance();
    discovering_ = true;
    pending_ = 0;
    refreshing_ = true;
    return generation_;
  }
  token begin_queries(std::size_t count) noexcept {
    advance();
    discovering_ = false;
    pending_ = count;
    refreshing_ = count != 0;
    return generation_;
  }
  bool discovered(token generation, std::size_t count) noexcept {
    if (!current(generation) || !discovering_)
      return false;
    discovering_ = false;
    pending_ = count;
    refreshing_ = count != 0;
    return true;
  }
  bool complete_one(token generation) noexcept {
    if (!current(generation) || discovering_ || pending_ == 0)
      return false;
    --pending_;
    refreshing_ = pending_ != 0;
    return !refreshing_;
  }
  void cancel() noexcept {
    advance();
    discovering_ = false;
    pending_ = 0;
    refreshing_ = false;
  }
  bool current(token generation) const noexcept {
    return generation_ == generation;
  }
  bool refreshing() const noexcept { return refreshing_; }

private:
  void advance() noexcept {
    if (++generation_ == 0)
      ++generation_;
  }
  token generation_{};
  std::size_t pending_{};
  bool discovering_{};
  bool refreshing_{};
};
} // namespace utils
