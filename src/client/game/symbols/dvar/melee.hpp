#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> melee_debug{0x14A037918, 0x147102FF8};
WEAK symbol<EngineDependentDvarMut> melee_weapon_no_dupes_crate_dwid{
    0x151271418, 0x148360688};
WEAK symbol<EngineDependentDvarMut> melee_weapon_no_dupes_drop_id{0x1512713D8,
                                                                  0x148360648};
} // namespace game
