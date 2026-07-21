#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace savegame {

WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 eNetworkModes networkMode)>
    SaveGame_IsDataReady{0x1415E2C90};

} // namespace savegame
} // namespace game