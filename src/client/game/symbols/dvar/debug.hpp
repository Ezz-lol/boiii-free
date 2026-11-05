#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> debug_color_pallete{
    0x14AE160F0, 0x14AE95070, 0x147CBC1B8};
WEAK symbol<EngineDependentDvarMut> debug_litbrushes{0x157F09478, 0x157F883A0,
                                                     0x14A752030};
WEAK symbol<EngineDependentDvarMut> debug_physicsgunobjcount{
    0x157F09468, 0x157F88390, 0x14A752020};
WEAK symbol<EngineDependentDvarMut> debug_protocol{0x15686EDE8, 0x1568EDD68,
                                                   0x14948DD08};
WEAK symbol<EngineDependentDvarMut> debug_ragdollspawnobjcount{
    0x157F09470, 0x157F88398, 0x14A752028};
WEAK symbol<EngineDependentDvarMut> debug_reflection{0x14AE160E0, 0x14AE95060,
                                                     0x147CBC1A8};
WEAK symbol<EngineDependentDvarMut> debug_reflection_matte{
    0x14AE160E8, 0x14AE95068, 0x147CBC1B0};
WEAK symbol<EngineDependentDvarMut> debugrendercollision{
    0x157EC93F0, 0x157F48318, 0x14A711FA8};
WEAK symbol<EngineDependentDvarMut> debugrendercollisiondistance{
    0x157F094A0, 0x157F883C8, 0x14A752058};
WEAK symbol<EngineDependentDvarMut> debugrendermask{0x157F09498, 0x157F883C0,
                                                    0x14A752050};
WEAK symbol<EngineDependentDvarMut> debugRenderPlayerCollision{
    0x159BAB9F0, 0x159C2A970, 0x14B349F28};
WEAK symbol<EngineDependentDvarMut> debug_show_viewpos{0x15686EDC8, 0x1568EDD48,
                                                       0x14948DCE8};
WEAK symbol<EngineDependentDvarMut> debug_trace{0x157F09460, 0x157F88388,
                                                0x14A752018};
} // namespace game
