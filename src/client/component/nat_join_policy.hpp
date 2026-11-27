#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace nat::join_policy {
struct request_scope {
  std::uint64_t generation{};
  std::function<bool()> still_current;
  bool current(std::uint64_t active) const {
    return generation != 0 && generation == active &&
           (!still_current || still_current());
  }
};
inline std::uint64_t advance_generation(std::uint64_t &generation) noexcept {
  if (++generation == 0) ++generation;
  return generation;
}
constexpr bool strict_match_token(std::string_view token) noexcept {
  constexpr std::string_view prefix = "match1_";
  if (token.size() != prefix.size() + 32 || !token.starts_with(prefix)) return false;
  for (const char c : token.substr(prefix.size()))
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
      return false;
  return true;
}
constexpr bool reply_matches(bool active, bool same_endpoint,
    std::string_view expected, std::string_view actual) noexcept {
  return active && same_endpoint && !expected.empty() && expected == actual;
}
} // namespace nat::join_policy
