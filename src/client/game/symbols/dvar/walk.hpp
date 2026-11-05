#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> walkMove_avoidanceDot{
    0x159BAB938, 0x159C2A8B8, 0x14B349ED0};
WEAK symbol<EngineDependentDvarMut> walkMove_avoidanceEnabled{
    0x159BAB908, 0x159C2A888, 0x14B349EB8};
WEAK symbol<EngineDependentDvarMut> walkMove_avoidanceMinSpeedScale{
    0x159BAB930, 0x159C2A8B0, 0x14B349EC8};
WEAK symbol<EngineDependentDvarMut> walkMove_avoidanceProbeDistance{
    0x159BAB920, 0x159C2A8A0, 0x14B349EC0};
WEAK symbol<EngineDependentDvarMut>
    walkMove_avoidanceSpeedDifferentialThreshold{0x0, 0x159C2A8C8, 0x14B349ED8};

} // namespace game
