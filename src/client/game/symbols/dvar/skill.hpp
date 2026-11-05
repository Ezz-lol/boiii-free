#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> showvisionsetdebuginfo{
    0x144CB3388, 0x144D32388, 0x142240DD0};
WEAK symbol<EngineDependentDvarMut> skill_onboardingAdjustOnExit{
    0x151399948, 0x1514188B8, 0x148378178};
WEAK symbol<EngineDependentDvarMut> skill_onboardingEnabled{
    0x151399950, 0x1514188C0, 0x148378180};
WEAK symbol<EngineDependentDvarMut> skill_onboardingEnter{
    0x151399920, 0x151418890, 0x148378150};
WEAK symbol<EngineDependentDvarMut> skill_onboardingExit{
    0x151399928, 0x151418898, 0x148378158};
WEAK symbol<EngineDependentDvarMut> skill_onboardingGames{
    0x151399940, 0x1514188B0, 0x148378170};
WEAK symbol<EngineDependentDvarMut> skill_onboardingThreshold{
    0x151399930, 0x1514188A0, 0x148378160};
WEAK symbol<EngineDependentDvarMut> skill_scoreBeta{0x151399900, 0x151418870,
                                                    0x148378130};
WEAK symbol<EngineDependentDvarMut> skill_scoreRange{0x151399908, 0x151418878,
                                                     0x148378138};
WEAK symbol<EngineDependentDvarMut> skill_teamBeta{0x151399910, 0x151418880,
                                                   0x148378140};
WEAK symbol<EngineDependentDvarMut> skill_teamRange{0x151399918, 0x151418888,
                                                    0x148378148};
} // namespace game
