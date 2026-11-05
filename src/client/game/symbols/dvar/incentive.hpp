#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> incentive_rare_crate_dwid{0x15114E4D0,
                                                              0x1511CD450, 0x0};
WEAK symbol<EngineDependentDvarMut> incentive_rare_drop_id{0x15114E4C0,
                                                           0x1511CD440, 0x0};
WEAK symbol<EngineDependentDvarMut> incentive_weapon_crate_dwid{
    0x15114E4D8, 0x1511CD458, 0x0};
WEAK symbol<EngineDependentDvarMut> incentive_weapon_drop_id{0x15114E4C8,
                                                             0x1511CD448, 0x0};
} // namespace game
