#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> dynent_bulletforce{0x149B775E8, 0x149BF65E8,
                                                       0x146EE5118};
WEAK symbol<EngineDependentDvarMut> dynEnt_damageRadiusScale{
    0x149B8C3A0, 0x149C0B3A0, 0x146EF9ED0};
WEAK symbol<EngineDependentDvarMut> dynent_delete_expensive{
    0x149B77618, 0x149BF6618, 0x146EE5148};
WEAK symbol<EngineDependentDvarMut> dynent_disable_rb_collision{
    0x149B77600, 0x149BF6600, 0x146EE5130};
WEAK symbol<EngineDependentDvarMut> dynent_explodeforce{
    0x149B775F0, 0x149BF65F0, 0x146EE5120};
WEAK symbol<EngineDependentDvarMut> dynent_explodemaxents{
    0x149B775F8, 0x149BF65F8, 0x146EE5128};
WEAK symbol<EngineDependentDvarMut> dynent_explodeminforce{
    0x149B78230, 0x149BF7230, 0x146EE5D60};
WEAK symbol<EngineDependentDvarMut> dynent_explodespinscale{
    0x149B78228, 0x149BF7228, 0x146EE5D58};
WEAK symbol<EngineDependentDvarMut> dynent_explodeupbias{
    0x149B78220, 0x149BF7220, 0x146EE5D50};
WEAK symbol<EngineDependentDvarMut> dynEnt_shouldCullEntitiesForSplitscreen{
    0x149B8C3A8, 0x149C0B3A8, 0x146EF9ED8};
WEAK symbol<EngineDependentDvarMut> dynent_small_cylinder_dimension{
    0x149B77608, 0x149BF6608, 0x146EE5138};
WEAK symbol<EngineDependentDvarMut> dynent_small_cylinder_max_avel{
    0x149B77610, 0x149BF6610, 0x146EE5140};
WEAK symbol<EngineDependentDvarMut> dynent_spawnedlimit{
    0x149B78238, 0x149BF7238, 0x146EE5D68};
WEAK symbol<EngineDependentDvarMut> dynsnaps_debugspew{0x0, 0x0, 0x14A19DC88};
} // namespace game
