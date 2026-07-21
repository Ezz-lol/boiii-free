#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace stats {
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Core_Ready{0x141EA9A30};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    LiveStats_Loadouts_Ready{0x141EAF490};
} // namespace stats
} // namespace live
} // namespace game