#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace connect {
WEAK symbol<bool(ControllerIndex_t localControllerIndex)>
    LiveConnect_WasPlayerQueueSuccessful{0x141DDEDC0, 0x0};
WEAK symbol<uint32_t(ControllerIndex_t localControllerIndex)>
    LiveConnect_GetPlayerQueuePosition{0x141DDE650, 0x0};
WEAK symbol<uint32_t(ControllerIndex_t localControllerIndex)>
    LiveConnect_GetPlayerQueueTimeEstimate{0x141DDE790, 0x0};
WEAK symbol<bool(ControllerIndex_t localControllerIndex)>
    LiveConnect_IsPlayerQueued{0x141DDE960, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex, const char **reason)>
    LiveConnect_DisableDemonwareConnect{0x141E72450, 0x0};

} // namespace connect

} // namespace live
} // namespace game
