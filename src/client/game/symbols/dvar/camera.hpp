#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> camerashakeremotehelo_angles{0x144CADAB8,
                                                                 0x14223E108};
WEAK symbol<EngineDependentDvarMut> camerashakeremotehelo_freqs{0x144CADAC0,
                                                                0x14223E110};
WEAK symbol<EngineDependentDvarMut> camerashakeremotehelo_speedrange{
    0x144CADAC8, 0x14223E118};
WEAK symbol<EngineDependentDvarMut> camerashakeremotemissile_angles{
    0x144CADAA0, 0x14223E0F0};
WEAK symbol<EngineDependentDvarMut> camerashakeremotemissile_freqs{0x144CADAA8,
                                                                   0x14223E0F8};
WEAK symbol<EngineDependentDvarMut> camerashakeremotemissile_speedrange{
    0x144CADAB0, 0x14223E100};
} // namespace game
