#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace zombies::coop {
// Phase names must be fixed source-defined labels, never user/network strings.
// Recording is bounded and does not read dvars, acquire game locks, or
// allocate.
void diagnostic_event(std::string_view phase, std::int64_t a = 0,
                      std::int64_t b = 0) noexcept;
// Best-effort snapshot of cached diagnostics only; safe from game/cache locks.
std::string crash_diagnostics();
} // namespace zombies::coop
