#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> bullet_enterWater{0x159BAB6F0, 0x159C2A670,
                                                      0x14B34A2B0};
WEAK symbol<EngineDependentDvarMut> bullet_penetrationEnabled{
    0x14A0377D8, 0x14A3D14F8, 0x1478368C0};
WEAK symbol<EngineDependentDvarMut> bullet_penetrationMinFxDist{
    0x159BAB6D8, 0x159C2A658, 0x14B34A2A0};
WEAK symbol<EngineDependentDvarMut> bulletrange{0x0, 0x0, 0x14A1797D8};
WEAK symbol<EngineDependentDvarMut> bullet_ricochetBaseChance{
    0x159BAB6E8, 0x159C2A668, 0x14B34A2A8};
} // namespace game
