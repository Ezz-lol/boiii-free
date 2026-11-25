#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <cstdint>
#include <limits>
#include <string_view>

namespace zombies::coop::diagnostics_detail {
static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
static_assert(std::atomic<std::int64_t>::is_always_lock_free);
static_assert(std::atomic<unsigned char>::is_always_lock_free);
static_assert(std::atomic<std::uint32_t>::is_always_lock_free);

inline constexpr std::size_t event_limit = 64;
inline constexpr std::size_t phase_limit = 32;
inline unsigned char label_char(char value) noexcept {
  return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
                 (value >= '0' && value <= '9') || value == '_' ||
                 value == '-' || value == '.'
             ? static_cast<unsigned char>(value)
             : static_cast<unsigned char>('_');
}

inline int parse_integer(std::string_view text) noexcept {
  if (text.empty() || text.size() > 32)
    return 0;
  int value = 0;
  const auto parsed =
      std::from_chars(text.data(), text.data() + text.size(), value);
  return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size()
             ? value
             : 0;
}

struct event {
  std::uint64_t sequence = 0;
  std::uint64_t tick_ms = 0;
  std::int64_t a = 0;
  std::int64_t b = 0;
  std::array<char, phase_limit> phase{};
};

// A writer claims one cell once, or drops the event. No spinning, locks,
// allocation, dvar access or waiting on a thread that may have faulted.
class event_buffer {
  struct cell {
    std::atomic<std::uint64_t> sequence{0};
    std::atomic<std::uint64_t> tick_ms{0};
    std::atomic<std::int64_t> a{0};
    std::atomic<std::int64_t> b{0};
    std::array<std::atomic<unsigned char>, phase_limit> phase{};
  };
  std::atomic<std::uint64_t> next_{0};
  std::array<cell, event_limit> cells_{};
  std::atomic<std::uint64_t> dropped_{0};

public:
  void record(std::string_view phase, std::int64_t a, std::int64_t b,
              std::uint64_t tick_ms) noexcept {
    const auto ticket = next_.fetch_add(1);
    if (ticket >= std::numeric_limits<std::uint64_t>::max() / 2 - 1)
      return;
    auto &target = cells_[ticket % event_limit];
    const auto committed = (ticket + 1) * 2;
    auto previous = target.sequence.load();
    if ((previous & 1) || previous >= committed ||
        !target.sequence.compare_exchange_strong(previous, committed - 1)) {
      dropped_.fetch_add(1);
      return;
    }
    if (phase.empty())
      phase = "unknown";
    for (std::size_t i = 0; i < phase_limit; ++i)
      target.phase[i].store(
          i < phase.size() && i + 1 < phase_limit ? label_char(phase[i]) : 0);
    target.tick_ms.store(tick_ms);
    target.a.store(a);
    target.b.store(b);
    target.sequence.store(committed);
  }

  std::size_t copy(std::array<event, event_limit> &output) const noexcept {
    std::size_t count = 0;
    for (const auto &source : cells_) {
      const auto before = source.sequence.load();
      if (!before || (before & 1))
        continue;
      event value;
      value.sequence = before / 2;
      value.tick_ms = source.tick_ms.load();
      value.a = source.a.load();
      value.b = source.b.load();
      for (std::size_t i = 0; i < phase_limit; ++i)
        value.phase[i] = static_cast<char>(source.phase[i].load());
      if (before == source.sequence.load())
        output[count++] = value;
    }
    std::sort(output.begin(), output.begin() + count,
              [](const event &left, const event &right) {
                return left.sequence < right.sequence;
              });
    return count;
  }
  std::uint64_t dropped() const noexcept { return dropped_.load(); }
};

// Main-thread publication; bounded atomic bytes keep crash readers independent
// of engine locks and of a publisher interrupted midway through a snapshot.
template <std::size_t Limit> class snapshot_text {
  static_assert(Limit > 1 &&
                Limit <= std::numeric_limits<std::uint32_t>::max());
  std::atomic<std::uint64_t> sequence_{0};
  std::atomic<std::uint32_t> length_{0};
  std::array<std::atomic<unsigned char>, Limit> bytes_{};

public:
  bool publish(std::string_view text) noexcept {
    auto before = sequence_.load();
    if ((before & 1) ||
        before > std::numeric_limits<std::uint64_t>::max() - 2 ||
        !sequence_.compare_exchange_strong(before, before + 1))
      return false;
    const auto length = std::min(text.size(), Limit - 1);
    for (std::size_t i = 0; i < length; ++i)
      bytes_[i].store(static_cast<unsigned char>(text[i]));
    bytes_[length].store(0);
    length_.store(static_cast<std::uint32_t>(length));
    sequence_.store(before + 2);
    return true;
  }
  bool copy(std::array<char, Limit> &text, std::size_t &length) const noexcept {
    length = 0;
    const auto before = sequence_.load();
    if (!before || (before & 1))
      return false;
    const auto size = length_.load();
    if (size >= Limit)
      return false;
    for (std::size_t i = 0; i <= size; ++i)
      text[i] = static_cast<char>(bytes_[i].load());
    if (before != sequence_.load())
      return false;
    length = size;
    return true;
  }
};
} // namespace zombies::coop::diagnostics_detail
