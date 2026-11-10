#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace spinner {
WEAK symbol<void(SpinnerSystem system, int32_t state)> LobbySpinner_Update{
    0x141EF68F0, 0x0};
}
} // namespace lobby
} // namespace game
