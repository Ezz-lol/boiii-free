#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> splitscreen{0x1453559F0, 0x0, 0x1428AAEC8};
WEAK symbol<EngineDependentDvarMut> splitscreen_horizontal{0x14535D138, 0x0,
                                                           0x1428B0B90};
WEAK symbol<EngineDependentDvarMut> splitscreen_playercount{
    0x145355A00, 0x1453D4A00, 0x1428AAED8};
WEAK symbol<EngineDependentDvarMut> doublescreen{0x0, 0x0, 0x1428AAED0};

} // namespace game
