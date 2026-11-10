#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> allowallnat{0x15118D7F0, 0x1482DED70};
WEAK symbol<EngineDependentDvarMut> allowdlcweaponsbyownership{0x157960BA0,
                                                               0x14A30C528};
WEAK symbol<EngineDependentDvarMut> allowguestsplitscreenonline{0x15118D9A8,
                                                                0x1482DEEB8};
WEAK symbol<EngineDependentDvarMut> allowHost_matchesHostedRatio{0x1513998C0,
                                                                 0x1483780F0};
WEAK symbol<EngineDependentDvarMut> allowHost_matchesHostedStreak{0x1513998D0,
                                                                  0x148378100};
WEAK symbol<EngineDependentDvarMut> allowHost_matchesPlayedRatio{0x1513998C8,
                                                                 0x1483780F8};
WEAK symbol<EngineDependentDvarMut> allowHost_matchesPlayedStreak{0x1513998D8,
                                                                  0x148378108};
WEAK symbol<EngineDependentDvarMut> disableHost_matchesHostedRatio{0x1513998E0,
                                                                   0x148378110};
WEAK symbol<EngineDependentDvarMut> disableHost_matchesHostedStreak{
    0x1513998F0, 0x148378120};
WEAK symbol<EngineDependentDvarMut> disableHost_matchesPlayedRatio{0x1513998E8,
                                                                   0x148378118};
WEAK symbol<EngineDependentDvarMut> disableHost_matchesPlayedStreak{
    0x1513998F8, 0x148378128};
} // namespace game
