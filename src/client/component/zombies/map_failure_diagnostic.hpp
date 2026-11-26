#pragma once

// Diagnostic metadata only. This does not relax routing/readiness or attest VM
// linked bodies. Snapshots/format buffers can survive an engine longjmp without
// a destructor; capture functions must destroy their owning temporaries first.
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <type_traits>

namespace zombies::coop::map_diagnostics {
enum class failure_stage : unsigned char {
  none, stock_transform, helper_lookup, stock_exception, helper_exception, helper_detour
};
struct source_observation {
  std::uint64_t bytes{};
  std::array<std::uint8_t, 32> sha256{};
  bool sha256_computed{};
};
struct failure_snapshot {
  bool available{};
  failure_stage stage{};
  int row{-1}, helper{-1}, epoch{}, capacity{};
  std::uint64_t generation{};
  source_observation source{};
  std::array<char, 48> reason{};
  std::array<char, 96> detail{};
};
static_assert(std::is_trivially_copyable_v<failure_snapshot> &&
              std::is_trivially_destructible_v<failure_snapshot>);

template <std::size_t N>
void bounded_text(std::array<char, N> &out, std::string_view text) noexcept {
  static_assert(N > 0);
  out.fill(0);
  const auto count = text.size() < N - 1 ? text.size() : N - 1;
  for (std::size_t i = 0; i < count; ++i) {
    const auto ch = static_cast<unsigned char>(text[i]);
    out[i] = ch >= 32 && ch < 127 ? static_cast<char>(ch) : '?';
  }
}
inline const char *stage_name(failure_stage stage) noexcept {
  switch (stage) {
  case failure_stage::stock_transform: return "stock_transform";
  case failure_stage::helper_lookup: return "helper_lookup";
  case failure_stage::stock_exception: return "stock_exception";
  case failure_stage::helper_exception: return "helper_exception";
  case failure_stage::helper_detour: return "helper_detour";
  default: return "none";
  }
}
template <std::size_t N>
void format_failure_error(const char *base, const failure_snapshot &value,
                          std::array<char, N> &out) noexcept {
  static_assert(N >= 1024);
  if (!value.available) {
    (void)std::snprintf(out.data(), out.size(), "%.280s [map31_failure=unavailable]", base ? base : "");
    return;
  }
  std::array<char, 65> digest{};
  constexpr char hex[] = "0123456789abcdef";
  if (value.source.sha256_computed) {
    for (std::size_t i = 0; i < value.source.sha256.size(); ++i) {
      digest[i * 2] = hex[value.source.sha256[i] >> 4];
      digest[i * 2 + 1] = hex[value.source.sha256[i] & 15];
    }
  }
  (void)std::snprintf(out.data(), out.size(),
      "%.280s [map31_failure=%s row=%d helper=%d reason=%.47s detail=%.95s bytes=%llu sha256=%s epoch=%d capacity=%d generation=%llu]",
      base ? base : "", stage_name(value.stage), value.row, value.helper,
      value.reason.data(), value.detail.data(), static_cast<unsigned long long>(value.source.bytes),
      value.source.sha256_computed ? digest.data() : "unavailable", value.epoch, value.capacity,
      static_cast<unsigned long long>(value.generation));
}
} // namespace zombies::coop::map_diagnostics
