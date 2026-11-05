#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> gpad_button_deadzone{0x157DEF350,
                                                         0x157E6E280, 0x0};
WEAK symbol<EngineDependentDvarMut> gpad_debug{0x157DEF348, 0x157E6E278, 0x0};
WEAK symbol<EngineDependentDvarMut> gpad_enabled{0x157DEF378, 0x157E6E2A8, 0x0};
WEAK symbol<EngineDependentDvarMut> gpad_stick_deadzone_max{0x157DEF360,
                                                            0x157E6E290, 0x0};
WEAK symbol<EngineDependentDvarMut> gpad_stick_deadzone_min{0x157DEF358,
                                                            0x157E6E288, 0x0};
WEAK symbol<EngineDependentDvarMut> gpad_stick_pressed{0x157DEF368, 0x157E6E298,
                                                       0x0};
WEAK symbol<EngineDependentDvarMut> gpad_stick_pressed_hysteresis{
    0x157DEF370, 0x157E6E2A0, 0x0};
} // namespace game
