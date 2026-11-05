#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> equipmentAltModeEnabled{
    0x159BABE30, 0x159C2ADB0, 0x14B34A7D0};
WEAK symbol<EngineDependentDvarMut> equipmentAsGadgets{0x159BABE20, 0x159C2ADA0,
                                                       0x14B34A7C0};
WEAK symbol<EngineDependentDvarMut> equipmentMods{0x159BABE28, 0x159C2ADA8,
                                                  0x14B34A7C8};

} // namespace game
