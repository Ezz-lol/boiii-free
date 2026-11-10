#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> dog_checkObstaceInPathWhenMoving{
    0x159BABA38, 0x14B349F60};
WEAK symbol<EngineDependentDvarMut> dog_checkShouldTurnWhenMoving{0x159BABA40,
                                                                  0x14B349F68};
WEAK symbol<EngineDependentDvarMut> dog_MeleeDamage{0x159BAB9F8, 0x14B349F30};
WEAK symbol<EngineDependentDvarMut> dog_repathDistSq{0x159BABA30, 0x14B349F58};
WEAK symbol<EngineDependentDvarMut> dog_traceMask{0x159BABA28, 0x14B349F50};
WEAK symbol<EngineDependentDvarMut> dog_turn180_angle{0x159BABA18, 0x14B349F40};
WEAK symbol<EngineDependentDvarMut> dog_turn90_angle{0x159BABA10, 0x14B349F38};
WEAK symbol<EngineDependentDvarMut> dog_turn_min_goal_dist{0x159BABA20,
                                                           0x14B349F48};
} // namespace game
