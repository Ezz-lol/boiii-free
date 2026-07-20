#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace connect {
WEAK symbol<bool(ControllerIndex_t localControllerIndex)>
    LiveConnect_WasPlayerQueueSuccessful{0x141DEB850};
WEAK symbol<uint32_t(ControllerIndex_t localControllerIndex)>
    LiveConnect_GetPlayerQueuePosition{0x141DEB0E0};
WEAK symbol<uint32_t(ControllerIndex_t localControllerIndex)>
    LiveConnect_GetPlayerQueueTimeEstimate{0x141DEB220};
WEAK symbol<bool(ControllerIndex_t localControllerIndex)>
    LiveConnect_IsPlayerQueued{0x141DEB3F0};

} // namespace connect

} // namespace live
} // namespace game