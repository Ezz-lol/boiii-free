#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> enable_camo_materials_tab{
    0x151274F58, 0x1512F3EC8, 0x1483613F8};
WEAK symbol<EngineDependentDvarMut> enableDLCWeapons{0x159BABE18, 0x159C2AD98,
                                                     0x14B34A7B8};
WEAK symbol<EngineDependentDvarMut> enable_global_wind{0x159BF75C0, 0x159C76540,
                                                       0x14B395F40};
WEAK symbol<EngineDependentDvarMut> enable_grass_wind{0x159BF75C8, 0x159C76548,
                                                      0x14B395F48};
WEAK symbol<EngineDependentDvarMut> enable_moving_paths{
    0x157F09488, 0x157F883B0, 0x14A752040};
WEAK symbol<EngineDependentDvarMut> enable_new_prone_check{
    0x157F09490, 0x157F883B8, 0x14A752048};
WEAK symbol<EngineDependentDvarMut> enable_sp_exploit_check{0x15114E480,
                                                            0x1511CD400, 0x0};
WEAK symbol<EngineDependentDvarMut> enable_weapon_contract{0x15114E478,
                                                           0x1511CD3F8, 0x0};

WEAK symbol<EngineDependentDvarMut> tu10_cleanupRagdollsOnExit{
    0x157F294D0, 0x157FA8400, 0x14A772090};
WEAK symbol<EngineDependentDvarMut>
    tu10_clearweapnextheldinputonswitchweaponcmd{0x15686EE98, 0x1568EDE18,
                                                 0x14948DDB8};
WEAK symbol<EngineDependentDvarMut> tu10_enableCyclicalDailyChallenges{
    0x1515C0C98, 0x15163FC08, 0x148382920};
WEAK symbol<EngineDependentDvarMut>
    tu10_enablehighestavailabledlcentitlementcheck{0x156C75260, 0x156CF41D0,
                                                   0x149758D00};
WEAK symbol<EngineDependentDvarMut> tu10_metPlayerClearOnUserSignedOut{
    0x1511D1CC8, 0x151250C48, 0x14831F168};
WEAK symbol<EngineDependentDvarMut> tu10_mpSwitchSpecialistWeaponExploitFix{
    0x159BAB6A0, 0x159C2A620, 0x14B349D80};
WEAK symbol<EngineDependentDvarMut> tu10_validationFatal{
    0x1515C0C88, 0x15163FBF8, 0x148382910};
WEAK symbol<EngineDependentDvarMut>
    tu11_AllowIgnoreDisabledWeaponCyclingInActionSlotUsageAllowed{
        0x159BAB720, 0x159C2A6A0, 0x14B349DC0};
WEAK symbol<EngineDependentDvarMut> tu11_blackjack_contract_time{
    0x159BAB6C8, 0x159C2A648, 0x14B349D90};
WEAK symbol<EngineDependentDvarMut> tu11_cleanupRagdollsOnDemoJump{
    0x157F294D8, 0x157FA8408, 0x14A772098};
WEAK symbol<EngineDependentDvarMut> tu11_clearMpVehicleWaterMaterial{
    0x0, 0x150C1D350, 0x14826DE30};
WEAK symbol<EngineDependentDvarMut> tu11_contracts_weapon_vo_time_limit{
    0x15114E4B8, 0x1511CD438, 0x0};
WEAK symbol<EngineDependentDvarMut> Tu11_DontUpdateWeaponPosFracWhenLinked{
    0x144C98B00, 0x144D17B00, 0x14222BC58};
WEAK symbol<EngineDependentDvarMut> tu11_enableClassicMode{
    0x159BAB6D0, 0x159C2A650, 0x14B349D98};
WEAK symbol<EngineDependentDvarMut> tu11_enableVehicleMode{
    0x159BAB6E0, 0x159C2A660, 0x14B349DA0};
WEAK symbol<EngineDependentDvarMut>
    tu11_IgnoreItemsWithPLevel100InIsItemLockedForChallenge{
        0x159BAB718, 0x159C2A698, 0x14B349DB8};
WEAK symbol<EngineDependentDvarMut> tu11_lootCryptoAutoRetry{0x1512727C0,
                                                             0x1512F1730, 0x0};
WEAK symbol<EngineDependentDvarMut> tu11_matchmakingprioritydoa{
    0x1556CE410, 0x15574D380, 0x148580EB0};
WEAK symbol<EngineDependentDvarMut> tu11_matchmakingprioritydoaignoretime{
    0x1556CE418, 0x15574D388, 0x148580EB8};
WEAK symbol<EngineDependentDvarMut> tu11_matchmakingprioritymp{
    0x1556CE400, 0x15574D370, 0x148580EA0};
WEAK symbol<EngineDependentDvarMut> tu11_matchmakingprioritympignoretime{
    0x1556CE408, 0x15574D378, 0x148580EA8};
WEAK symbol<EngineDependentDvarMut> tu11_maxquadcachealloc{
    0x157960AB8, 0x1579DF9A8, 0x14A30C440};
WEAK symbol<EngineDependentDvarMut> tu11_mpNewCodCasterKeyline{
    0x159BAB570, 0x159C2A4F0, 0x14B349CE0};
WEAK symbol<EngineDependentDvarMut>
    tu11_mpReconstructTrailsForStationaryEntities{0x159BAB558, 0x159C2A4D8,
                                                  0x14B349CD8};
WEAK symbol<EngineDependentDvarMut> tu11_programaticallyColoredGameFX{
    0x159BAB730, 0x159C2A6B0, 0x14B349DC8};
WEAK symbol<EngineDependentDvarMut> tu11_uploadstreamfailureflag{
    0x15118D870, 0x15120C7F0, 0x1482DEDF0};
WEAK symbol<EngineDependentDvarMut> tu11_useLootXPDueStat{0x1512727C8,
                                                          0x1512F1738, 0x0};
WEAK symbol<EngineDependentDvarMut> tu12_delayFFOTD{0x1515C0C90, 0x15163FC00,
                                                    0x148382918};
WEAK symbol<EngineDependentDvarMut> tu12_mpMaintainMissileSpeed{
    0x159BAB748, 0x159C2A6C8, 0x14B349DD0};
WEAK symbol<EngineDependentDvarMut> tu12_switchToWeaponImmediateFix{
    0x159BAB660, 0x159C2A5E0, 0x14B349D60};
WEAK symbol<EngineDependentDvarMut> tu12_zmHideLegsWithViewmodel{
    0x159BAB9D0, 0x159C2A950, 0x14B34A418};
WEAK symbol<EngineDependentDvarMut> tu13_recordcontentavailable{
    0x15118D878, 0x15120C7F8, 0x1482DEDF8};
WEAK symbol<EngineDependentDvarMut> tu16_lobbyMonitor{0x1556CE420, 0x15574D390,
                                                      0x148580EC0};
WEAK symbol<EngineDependentDvarMut> tu16_physicsPushOutThreshold{
    0x159BAB768, 0x159C2A6E8, 0x14B349DE0};
WEAK symbol<EngineDependentDvarMut> tu16_SLFreeStringFix{
    0x145145FC8, 0x1451C4FC8, 0x1426DF920};
WEAK symbol<EngineDependentDvarMut> tu1_aiPathableMaterials{
    0x14A1D41F0, 0x14A56DFA0, 0x147836680};
WEAK symbol<EngineDependentDvarMut> tu1_build{0x15118D998, 0x15120C918,
                                              0x1482DEEA8};
WEAK symbol<EngineDependentDvarMut> tu1_pickupParentlessGrenade{
    0x14A1D41E8, 0x14A56DF98, 0x147836678};
WEAK symbol<EngineDependentDvarMut> tu1_queuedEventInterval{0x0, 0x1499B1ED0,
                                                            0x146EA2E80};
WEAK symbol<EngineDependentDvarMut> tu1_vehicleCollisionCacheSetup{
    0x14A1D41F8, 0x14A56DFA8, 0x147836688};
WEAK symbol<EngineDependentDvarMut> tu1_zombiePaintJobsEnabled{
    0x14A1D4200, 0x14A56DFB0, 0x147836690};
WEAK symbol<EngineDependentDvarMut> tu2_catchmissingrbendframe{
    0x14AE16D00, 0x14AE95C80, 0x147CBCFA8};
WEAK symbol<EngineDependentDvarMut> tu2_cscgetdstatcp{0x15118D9A0, 0x15120C920,
                                                      0x1482DEEB0};
WEAK symbol<EngineDependentDvarMut> tu2_disableChallengesForLockedItemsInArena{
    0x159BABE10, 0x159C2AD90, 0x14B34A7B0};
WEAK symbol<EngineDependentDvarMut> tu2_forceconnectionstatetransitionspinneron{
    0x1456BC1F8, 0x14573B1F8, 0x142C41DB0};
WEAK symbol<EngineDependentDvarMut> tu2_logLootToKVS{0x151271640, 0x1512F05B0,
                                                     0x0};
WEAK symbol<EngineDependentDvarMut> tu3_lobby_droprejoiningclients{
    0x1556CE3D0, 0x15574D340, 0x148580E70};
WEAK symbol<EngineDependentDvarMut> tu3_onlinesavegameerrorfatal{
    0x149F19328, 0x149F98328, 0x146FE4B48};
WEAK symbol<EngineDependentDvarMut> tu4_burnDuplicates{0x151271658, 0x1512F05C8,
                                                       0x0};
WEAK symbol<EngineDependentDvarMut> tu4_enableBonusCryptokeysHint{
    0x151271660, 0x1512F05D0, 0x0};
WEAK symbol<EngineDependentDvarMut> tu4_enableCodPoints{0x151271648,
                                                        0x1512F05B8, 0x0};
WEAK symbol<EngineDependentDvarMut> tu4_enableMeleeWeaponFlourish{
    0x159BAB4F0, 0x159C2A470, 0x14B349CA0};
WEAK symbol<EngineDependentDvarMut> tu4_firstValidCurrencyID{0x15114E510,
                                                             0x1511CD490, 0x0};
WEAK symbol<EngineDependentDvarMut> tu4_newSysInfo{0x15118D9B0, 0x15120C930,
                                                   0x1482DEEC0};
WEAK symbol<EngineDependentDvarMut> tu4_newsysinfoupdaterate{
    0x15118D9B8, 0x15120C938, 0x1482DEEC8};
WEAK symbol<EngineDependentDvarMut> tu4_partyprivacyluacheck{
    0x1556CE480, 0x15574D3F0, 0x148580F20};
WEAK symbol<EngineDependentDvarMut> tu4_partyprivacyuseglobal{
    0x1556CE478, 0x15574D3E8, 0x148580F18};
WEAK symbol<EngineDependentDvarMut> tu4_refetchBalanceOnUpdate{
    0x15114E518, 0x1511CD498, 0x0};
WEAK symbol<EngineDependentDvarMut> tu4_skipStatStatusCache{
    0x1513A4680, 0x1514235F0, 0x14837E370};
WEAK symbol<EngineDependentDvarMut> tu4_storageClearTargetQueue{
    0x0, 0x15790D9B0, 0x14A2D25E0};
WEAK symbol<EngineDependentDvarMut> tu4_storageControllerSwapProtection{
    0x15788EAA8, 0x15790D9A8, 0x14A2D25D8};
WEAK symbol<EngineDependentDvarMut> tu4_zmChangePlayerStandOnSentients{
    0x159BAB978, 0x159C2A8F8, 0x14B34A3F0};
WEAK symbol<EngineDependentDvarMut> tu5_defaultMaxHeroAttachmentsToZero{
    0x159BAB518, 0x159C2A498, 0x14B349CB8};
WEAK symbol<EngineDependentDvarMut> tu5_dwNetFatalErrors{0x0, 0x1499B0F48,
                                                         0x146EA06F8};
WEAK symbol<EngineDependentDvarMut> tu5_enableVialsOwed{0x151399800,
                                                        0x151418770, 0x0};
WEAK symbol<EngineDependentDvarMut> tu5_gadgetThiefSupport{
    0x1441D5DF8, 0x144254DF8, 0x141E56DB8};
WEAK symbol<EngineDependentDvarMut> tu5_live_friends_signedin_fetch{
    0x150F7D6B0, 0x150FFC630, 0x1482B7F78};
WEAK symbol<EngineDependentDvarMut> tu5_recordXuidMismatch{0x0, 0x1514235F8,
                                                           0x14837E378};
WEAK symbol<EngineDependentDvarMut> tu5_vialsOwedCheckDelay{0x151399808,
                                                            0x151418778, 0x0};
WEAK symbol<EngineDependentDvarMut> tu5_zmChangeOffhandGearWeaponChange{
    0x159BAB9A0, 0x159C2A920, 0x14B34A400};
WEAK symbol<EngineDependentDvarMut> tu5_zmChangeSlamUseWalkable{
    0x159BAB988, 0x159C2A908, 0x14B34A3F8};
WEAK symbol<EngineDependentDvarMut> tu5_zmEnableClipOnlyWeaponFlourish{
    0x159BAB500, 0x159C2A480, 0x14B349CA8};
WEAK symbol<EngineDependentDvarMut> tu5_zmPathDistanceCheckTolarance{
    0x159BAB508, 0x159C2A488, 0x14B349CB0};
WEAK symbol<EngineDependentDvarMut> tu6_doublePistolSpreadAmmo{
    0x15686EE88, 0x1568EDE08, 0x0};
WEAK symbol<EngineDependentDvarMut> tu6_mpMeleeAttachmentObitIcon{
    0x159BAB578, 0x159C2A4F8, 0x14B349CE8};
WEAK symbol<EngineDependentDvarMut> tu6_mpReconstructTrailForOtherPlayers{
    0x159BAB548, 0x159C2A4C8, 0x14B349CD0};
WEAK symbol<EngineDependentDvarMut> tu6_mpReconstructTrailForTriboltCrossbow{
    0x159BAB538, 0x159C2A4B8, 0x14B349CC8};
WEAK symbol<EngineDependentDvarMut> tu6_mpStackFirePitchChange{
    0x159BAB530, 0x159C2A4B0, 0x14B349CC0};
WEAK symbol<EngineDependentDvarMut> tu7_enableBundles{0x15114E508, 0x1511CD488,
                                                      0x0};
WEAK symbol<EngineDependentDvarMut> tu7_mpDoNotSetBallisticAnglesWhenLinked{
    0x159BAB5B0, 0x159C2A530, 0x14B349D00};
WEAK symbol<EngineDependentDvarMut> tu7_mpNoVehicleNeutralTeamTargetHighlight{
    0x159BAB5A0, 0x159C2A520, 0x14B349CF8};
WEAK symbol<EngineDependentDvarMut> tu7_mpUseImprovedMissileStickAngles{
    0x159BAB5C8, 0x159C2A548, 0x14B349D10};
WEAK symbol<EngineDependentDvarMut> tu7_mpUseNewBounceMissileMayStopFleshLogic{
    0x159BAB5C0, 0x159C2A540, 0x14B349D08};
WEAK symbol<EngineDependentDvarMut> tu7_zmInAirDisabledTimeoutTime{
    0x159BAB9A8, 0x159C2A928, 0x14B34A408};
WEAK symbol<EngineDependentDvarMut> tu8_ai_enableBTInterruptPriority{
    0x14A1D4328, 0x14A56E0D8, 0x1478367B8};
WEAK symbol<EngineDependentDvarMut> tu8_animTimeResetInterval{
    0x159BAB5E0, 0x159C2A560, 0x14B349D20};
WEAK symbol<EngineDependentDvarMut> tu8_enableDurableProductsExchange{
    0x15188CE48, 0x15190BDB8, 0x0};
WEAK symbol<EngineDependentDvarMut> tu8_mpChallengesGroupPrereqFix{
    0x159BAB608, 0x159C2A588, 0x14B349D30};
WEAK symbol<EngineDependentDvarMut> tu8_mpLootWeaponPurchasedCheckEnabled{
    0x159BAB5D8, 0x159C2A558, 0x14B349D18};
WEAK symbol<EngineDependentDvarMut> tu8_mtx_enabled{0x156C75248, 0x156CF41B8,
                                                    0x149758CE8};
WEAK symbol<EngineDependentDvarMut> tu8_trmRestrictionLevel{
    0x159BAB5F0, 0x159C2A570, 0x14B349D28};
WEAK symbol<EngineDependentDvarMut> tu8_yySlideExploitFix{
    0x159BAB610, 0x159C2A590, 0x14B349D38};
WEAK symbol<EngineDependentDvarMut> tu9_backupBadDecompress{
    0x1515C0C78, 0x15163FBE8, 0x148382900};
WEAK symbol<EngineDependentDvarMut> tu9_backupBadSpace{0x1515C0C80, 0x15163FBF0,
                                                       0x148382908};
WEAK symbol<EngineDependentDvarMut> tu9_cansplitinprogressunbalancedteams{
    0x1556CE3F0, 0x15574D360, 0x148580E90};
WEAK symbol<EngineDependentDvarMut> tu9_entryExitImpactSurfaceNormalFix{
    0x159BAB630, 0x159C2A5B0, 0x14B349D48};
WEAK symbol<EngineDependentDvarMut>
    tu9_fileshareCommunityIncludeEmptyDescriptors{0x150BCF078, 0x150C4DFF8,
                                                  0x148294528};
WEAK symbol<EngineDependentDvarMut> tu9_hideMasterCallingCards{
    0x151271650, 0x1512F05C0, 0x0};
WEAK symbol<EngineDependentDvarMut> tu9_highestavailabledlc{
    0x156C75250, 0x156CF41C0, 0x149758CF0};
WEAK symbol<EngineDependentDvarMut> tu9_hostlaunchcheckminplayer{
    0x1556CE3E0, 0x15574D350, 0x148580E80};
WEAK symbol<EngineDependentDvarMut> tu9_hostlaunchcheckunbalancedteams{
    0x1556CE3E8, 0x15574D358, 0x148580E88};
WEAK symbol<EngineDependentDvarMut> tu9_hostprivatesessions{
    0x1556CE3F8, 0x15574D368, 0x148580E98};
WEAK symbol<EngineDependentDvarMut> tu9_jitterModExploitFix{
    0x159BAB648, 0x159C2A5C8, 0x14B349D50};
WEAK symbol<EngineDependentDvarMut> tu9_skipperlocalclientallocation{
    0x15686EBE8, 0x1568EDB68, 0x14948F1C8};
WEAK symbol<EngineDependentDvarMut> tu9_ui_zmlobbyeeenabled{
    0x1556CE3D8, 0x15574D348, 0x148580E78};
WEAK symbol<EngineDependentDvarMut> tu9_yySlideExploitFix{
    0x159BAB628, 0x159C2A5A8, 0x14B349D40};
WEAK symbol<EngineDependentDvarMut> tu9_zmDisableMeleeChargeJump{
    0x159BAB9C0, 0x159C2A940, 0x14B34A410};
} // namespace game
