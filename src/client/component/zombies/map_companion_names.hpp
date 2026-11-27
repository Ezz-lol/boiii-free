#pragma once
#include <array>
#include <optional>
#include <string_view>

namespace zombies::coop::t7 {
// Server-only code dependencies. These are not network field-schema objects.
inline constexpr std::array<std::string_view, 8> map_companion_names{
    "custom_scripts/zm/eroded_coop_map_readiness.gsc",
    "custom_scripts/zm/eroded_coop_temple_readiness_entry.gsc",
    "custom_scripts/zm/eroded_coop_castle_readiness_entry.gsc",
    "custom_scripts/zm/eroded_coop_factory_teleport_entry.gsc",
    "custom_scripts/zm/eroded_coop_theater_teleport_entry.gsc",
    "custom_scripts/zm/eroded_coop_stalingrad_transport_entry.gsc",
    "custom_scripts/zm/eroded_coop_bgb_transport_entry.gsc",
    "custom_scripts/zm/eroded_coop_stalingrad_challenges_entry.gsc"};
inline std::optional<unsigned> map_companion_identity(std::string_view path) noexcept {
  for (unsigned index = 0; index < map_companion_names.size(); ++index) {
    const auto expected = map_companion_names[index];
    if (path.size() != expected.size()) continue;
    bool same = true;
    for (std::size_t i = 0; i < path.size(); ++i) {
      char c = path[i];
      if (c == '\\') c = '/';
      if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
      if (c != expected[i]) { same = false; break; }
    }
    if (same) return index;
  }
  return std::nullopt;
}
} // namespace zombies::coop::t7
