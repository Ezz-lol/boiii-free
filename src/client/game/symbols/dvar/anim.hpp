#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> asm_animDeltaParamsThresh{0x0, 0x143684110,
                                                              0x1413861C0};
WEAK symbol<EngineDependentDvarMut> asm_debugAimLayer{0x1436050F0, 0x1436840F0,
                                                      0x1413861A0};
WEAK symbol<EngineDependentDvarMut> asm_debugLevel{0x1436050E8, 0x1436840E8,
                                                   0x141386198};
WEAK symbol<EngineDependentDvarMut> asm_debugPrimaryDeltaLayer{
    0x1436050F8, 0x1436840F8, 0x1413861A8};
WEAK symbol<EngineDependentDvarMut> asm_debugShootLayer{
    0x143605100, 0x143684100, 0x1413861B0};
WEAK symbol<EngineDependentDvarMut> asm_disableTransitions{
    0x1436050E0, 0x1436840E0, 0x141386190};
WEAK symbol<EngineDependentDvarMut> asm_useNetworkFrameDelta{
    0x143605108, 0x143684108, 0x1413861B8};
} // namespace game
