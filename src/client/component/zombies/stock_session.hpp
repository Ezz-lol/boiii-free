#pragma once

#include <string_view>

namespace zombies::coop {
// The engine can select its builtin usermaps bootstrap for an otherwise
// stock dedicated lobby. Selection alone does not identify Workshop content.
constexpr bool stock_session_allowed(std::string_view selected_mod,
                                     bool mod_loaded,
                                     std::string_view loaded_mod_publisher,
                                     bool usermap_loaded) noexcept {
  if ((!selected_mod.empty() && selected_mod != "usermaps") || usermap_loaded)
    return false;
  // A loaded mod without a publisher is unknown, rather than stock.
  return !mod_loaded || loaded_mod_publisher == "usermaps";
}
} // namespace zombies::coop
