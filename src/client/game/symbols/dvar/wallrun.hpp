#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> wallrun_avoidanceDot{
    0x159BAB8C8, 0x159C2A848, 0x14B349E98};
WEAK symbol<EngineDependentDvarMut> wallRun_avoidanceEnabled{
    0x159BAB8A0, 0x159C2A820, 0x14B349E80};
WEAK symbol<EngineDependentDvarMut> wallRun_avoidanceMinSpeedScale{
    0x159BAB8C0, 0x159C2A840, 0x14B349E90};
WEAK symbol<EngineDependentDvarMut> wallRun_avoidanceProbeDistance{
    0x159BAB8A8, 0x159C2A828, 0x14B349E88};
WEAK symbol<EngineDependentDvarMut> wallrun_enabled{0x159BAB8D0, 0x159C2A850,
                                                    0x14B34A398};
WEAK symbol<EngineDependentDvarMut> wallrun_jumpLateralLimit{
    0x159BAB8E0, 0x159C2A860, 0x14B349EA0};
WEAK symbol<EngineDependentDvarMut> wallRun_peakTest_zm{
    0x159BAB8F8, 0x159C2A878, 0x14B349EB0};

} // namespace game
