#pragma once

#include "core.hpp" // IWYU pragma: export

namespace game {
// Registered by `Loot_Init`.
WEAK symbol<EngineDependentDvarMut> loot_cryptokeyCost{0x1512712F0, 0x1512F0260,
                                                       0x0};
WEAK symbol<EngineDependentDvarMut> loot_earnTime{0x151271300, 0x1512F0270,
                                                  0x0};
WEAK symbol<EngineDependentDvarMut> loot_commonCrate_cpCost{0x151271338,
                                                            0x1512F02A8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_commonCrate_cryptoCost{
    0x151271348, 0x1512F02B8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_rareCrate_cpCost{0x151271378,
                                                          0x1512F02E8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_rareCrate_cryptoCost{0x151271388,
                                                              0x1512F02F8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_cryptoCost{0x151271500,
                                                            0x1512F0470, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_final_count{0x151271510,
                                                             0x1512F0480, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_crate_dwid{0x151271518,
                                                            0x1512F0488, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_consumable_id{
    0x151271520, 0x1512F0490, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_drop_id{0x151271528,
                                                         0x1512F0498, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_cooloffSeconds{
    0x151271530, 0x1512F04A0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_cryptoCost{
    0x151271540, 0x1512F04B0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_final_count{
    0x151271550, 0x1512F04C0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_dwid{0x151271558,
                                                          0x1512F04C8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_consumable_id{
    0x151271560, 0x1512F04D0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_drop_id{0x151271568,
                                                             0x1512F04D8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_rare_crate_dwid{
    0x151271570, 0x1512F04E0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_cooloffSeconds{
    0x151271578, 0x1512F04E8, 0x0};
} // namespace game
