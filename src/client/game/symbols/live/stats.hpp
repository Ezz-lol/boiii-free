#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace stats {
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Core_Ready{0x141E9CFA0, 0x141EA9A30, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Loadouts_Ready{0x141EA2A00, 0x141EAF490, 0x0};
} // namespace stats
} // namespace live
} // namespace game
