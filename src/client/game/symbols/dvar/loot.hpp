#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// Registered by `Loot_Init`.
WEAK symbol<EngineDependentDvarMut> loot_commonCrate_cpCost{0x151271338, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_commonCrate_cryptoCost{0x151271348, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_rareCrate_cpCost{0x151271378, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_rareCrate_cryptoCost{0x151271388, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_cryptoCost{0x151271500, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_consumable_id{0x151271520, 0x148360450};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_cryptoCost{0x151271540, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_cooloffSeconds{0x151271578, 0x0};

// Registered by `LootConfig_Init`. Cryptokeys refunded per burned duplicate.
WEAK symbol<EngineDependentDvarMut> loot_burnCommonRefund{0x1512727E8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnRareRefund{0x1512727F0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnLegendaryRefund{0x1512727F8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnEpicRefund{0x151272800, 0x0};

// Registered by `LiveInventory_Init`.
WEAK symbol<EngineDependentDvarMut> blackjack_contract_dwid{0x15114E460, 0x0};
WEAK symbol<EngineDependentDvarMut> blackjack_contract_ck_cost{0x15114E468, 0x0};

WEAK symbol<EngineDependentDvarMut> loot_3pack_bundle_id{0x1512715B0, 0x1483604E0};
WEAK symbol<EngineDependentDvarMut> loot_3pack_bundle_sku{0x1512715B8, 0x1483604E8};
WEAK symbol<EngineDependentDvarMut> loot_3pack_dwid{0x1512715A8, 0x1483604D8};
WEAK symbol<EngineDependentDvarMut> loot_3pack_final_count{0x151271598, 0x1483604C8};
WEAK symbol<EngineDependentDvarMut> loot_3pack_final_count_string_ref{0x1512715A0, 0x1483604D0};
WEAK symbol<EngineDependentDvarMut> loot_bribeCrate_dwid{0x151271360, 0x1483605D0};
WEAK symbol<EngineDependentDvarMut> loot_bundleActive{0x1512713C0, 0x148360630};
WEAK symbol<EngineDependentDvarMut> loot_bundle_cpCost{0x1512713A8, 0x148360618};
WEAK symbol<EngineDependentDvarMut> loot_bundle_final_count{0x1512713B0, 0x148360620};
WEAK symbol<EngineDependentDvarMut> loot_burnBatchSize{0x1512727D0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnCooloff{0x1512727D8, 0x0};

WEAK symbol<EngineDependentDvarMut> loot_burnMinMegaRequired{0x151272808, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnMinRareRequired{0x151272810, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnMinUltraRequired{0x151272818, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_burnRefetchOnSuccess{0x1512727E0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_commonCrate_dwid{0x151271358, 0x1483605C8};
WEAK symbol<EngineDependentDvarMut> loot_cryptoCheckDelay{0x1512727B8, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_cryptokeyCost{0x1512712F0, 0x148360560};
WEAK symbol<EngineDependentDvarMut> loot_cryptokeySku{0x1512712F8, 0x148360568};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_consumable_id{0x151271560, 0x148360490};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_drop_id{0x151271568, 0x148360498};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_dwid{0x151271558, 0x148360488};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_final_count{0x151271550, 0x148360480};
WEAK symbol<EngineDependentDvarMut> loot_dailyDouble_rare_crate_dwid{0x151271570, 0x1483604A0};
WEAK symbol<EngineDependentDvarMut> loot_distill_free_sku{0x151271460, 0x1483606D0};
WEAK symbol<EngineDependentDvarMut> loot_earnMax{0x151271310, 0x148360580};
WEAK symbol<EngineDependentDvarMut> loot_earnMin{0x151271318, 0x148360588};
WEAK symbol<EngineDependentDvarMut> loot_earnPlayThreshold{0x151271308, 0x148360578};
WEAK symbol<EngineDependentDvarMut> loot_earnTime{0x151271300, 0x148360570};
WEAK symbol<EngineDependentDvarMut> loot_emblems_max_count{0x150BC5BA0, 0x148293850};
WEAK symbol<EngineDependentDvarMut> loot_enabled{0x1512727B0, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_experimentpromo_active{0x151271610, 0x148360540};
WEAK symbol<EngineDependentDvarMut> loot_ld_salePercentOff{0x151271450, 0x1483606C0};
WEAK symbol<EngineDependentDvarMut> loot_limitedTimeItemVersions{0x0, 0x148360558};
WEAK symbol<EngineDependentDvarMut> loot_mpItemCurrentDropStringRef{0x151271618, 0x148360548};
WEAK symbol<EngineDependentDvarMut> loot_mpItemVersions{0x1512713C8, 0x148360638};
WEAK symbol<EngineDependentDvarMut> loot_noDupeRare20Bundle_drop_id{0x151271600, 0x148360530};
WEAK symbol<EngineDependentDvarMut> loot_noDupeRare20Bundle_dwid{0x1512715F0, 0x148360520};
WEAK symbol<EngineDependentDvarMut> loot_noduperare20bundle_sku_id{0x151271608, 0x148360538};
WEAK symbol<EngineDependentDvarMut> loot_noDupeRare_dwid{0x1512715D0, 0x148360500};
WEAK symbol<EngineDependentDvarMut> loot_rareCrate_dwid{0x151271398, 0x148360608};
WEAK symbol<EngineDependentDvarMut> loot_salePercentOff{0x1512714F8, 0x148360428};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_cooloffSeconds{0x151271530, 0x148360460};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_crate_dwid{0x151271518, 0x148360448};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_drop_id{0x151271528, 0x148360458};
WEAK symbol<EngineDependentDvarMut> loot_sixPack_final_count{0x151271510, 0x148360440};
WEAK symbol<EngineDependentDvarMut> loot_taskMaxAttempts{0x151271638, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_taskWindow{0x151271630, 0x0};
WEAK symbol<EngineDependentDvarMut> loot_trifectaActive{0x151271428, 0x148360698};
WEAK symbol<EngineDependentDvarMut> loot_winBonusPercent{0x151271320, 0x148360590};
WEAK symbol<EngineDependentDvarMut> lootxp_bonus_multiplier{0x151271330, 0x1483605A0};
WEAK symbol<EngineDependentDvarMut> lootxp_multiplier{0x151271328, 0x148360598};
WEAK symbol<EngineDependentDvarMut> loot_zmItemVersions{0x1512713D0, 0x148360640};

WEAK symbol<EngineDependentDvarMut> range_weapon_no_dupes_crate_dwid{0x151271420, 0x148360690};
WEAK symbol<EngineDependentDvarMut> range_weapon_no_dupes_drop_id{0x1512713E0, 0x148360650};
WEAK symbol<EngineDependentDvarMut> rare_crate_bundle_id{0x15114E500, 0x0};
WEAK symbol<EngineDependentDvarMut> reportuserinterval{0x1512043C0, 0x14833A850};
WEAK symbol<EngineDependentDvarMut> retail_incentive_id{0x15114E3F8, 0x0};
WEAK symbol<EngineDependentDvarMut> season_pass_incentive_id{0x15114E418, 0x0};
WEAK symbol<EngineDependentDvarMut> season_pass_incentive_max{0x15114E420, 0x0};

WEAK symbol<EngineDependentDvarMut> blackjack_contract_purchasable{0x15114E458, 0x0};

WEAK symbol<EngineDependentDvarMut> enable_retail_incentive{0x15114E400, 0x0};
WEAK symbol<EngineDependentDvarMut> enable_season_pass_incentive{0x15114E470, 0x0};
WEAK symbol<EngineDependentDvarMut> enable_digital_incentive{0x15114E410, 0x0};
WEAK symbol<EngineDependentDvarMut> initial_cod_points_id{0x15114E3F0, 0x0};
WEAK symbol<EngineDependentDvarMut> digital_incentive_id{0x15114E408, 0x0};
WEAK symbol<EngineDependentDvarMut> balances_retry_delay{0x15114E3D8, 0x0};
WEAK symbol<EngineDependentDvarMut> balances_retry_max{0x15114E3E0, 0x0};
WEAK symbol<EngineDependentDvarMut> allCollectiblesUnlocked{0x159BABE38, 0x14B34A7D8};
WEAK symbol<EngineDependentDvarMut> show_contracts_button{0x15114E4A0, 0x0};
WEAK symbol<EngineDependentDvarMut> skip_contract_rewards{0x15114E4A8, 0x0};
WEAK symbol<EngineDependentDvarMut> trifecta_cod_points_drop_id{0x151271400, 0x148360670};
WEAK symbol<EngineDependentDvarMut> trifecta_cod_points_sku{0x1512713F0, 0x148360660};
WEAK symbol<EngineDependentDvarMut> trifecta_cryptokeys_drop_id{0x1512713F8, 0x148360668};
WEAK symbol<EngineDependentDvarMut> trifecta_cryptokeys_sku{0x1512713E8, 0x148360658};
WEAK symbol<EngineDependentDvarMut> zm_vials_20_id{0x15114E4F8, 0x0};
WEAK symbol<EngineDependentDvarMut> zm_vials_3_id{0x15114E4E0, 0x0};
WEAK symbol<EngineDependentDvarMut> zm_vials_6_id{0x15114E4E8, 0x0};
WEAK symbol<EngineDependentDvarMut> zm_vials_9_id{0x15114E4F0, 0x0};
WEAK symbol<EngineDependentDvarMut> weapon_contract_incentive_id{0x15114E438, 0x0};
WEAK symbol<EngineDependentDvarMut> weapon_contract_max{0x15114E440, 0x0};
WEAK symbol<EngineDependentDvarMut> weapon_contract_target_value{0x15114E448, 0x0};
WEAK symbol<EngineDependentDvarMut>
    weekly_contract_blackjack_contract_reward_count{0x15114E498, 0x0};
WEAK symbol<EngineDependentDvarMut> weekly_contract_cryptokey_reward_count{0x15114E490, 0x0};

} // namespace game
