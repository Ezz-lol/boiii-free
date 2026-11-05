#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> curveBallBackFactor{
    0x14A5746C8, 0x14A5F36B8, 0x147A5C2B8};
WEAK symbol<EngineDependentDvarMut> curveBallDownFactor{
    0x14A5746B8, 0x14A5F36A8, 0x147A5C2A8};
WEAK symbol<EngineDependentDvarMut> curveBallForwardFactor{
    0x14A5746C0, 0x14A5F36B0, 0x147A5C2B0};
WEAK symbol<EngineDependentDvarMut> curveBallUpFactor{0x14A5746B0, 0x14A5F36A0,
                                                      0x147A5C2A0};
} // namespace game
