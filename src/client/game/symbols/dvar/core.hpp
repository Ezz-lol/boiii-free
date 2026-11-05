#pragma once

#include "game/symbols/sl.hpp"
#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {

// Also named sv_cheats, but global is named `dvar_cheats` in the engine.
// Used to enable/disable cheat dvars - controls whether cheat
// protection on a dvar to be modified is checked and respected in internal
// setters.
WEAK symbol<EngineDependentDvarMut> dvar_cheats{0x157A47310, 0x157AC6210,
                                                0x14A3CB610};
WEAK symbol<EngineDependentDvarMut> mapname{0x157736848, 0x1577C57C8,
                                            0x14A179858};

WEAK symbol<EngineDependentDvarMut> motd_enabled{0x15118D800, 0x15120C780,
                                                 0x1482DED80};
WEAK symbol<EngineDependentDvarMut> motddelay{0x15118D7F8, 0x15120C778,
                                              0x1482DED78};

WEAK symbol<EngineDependentDvarMut> testmissingcontentpacks{
    0x15686EE70, 0x1568EDDF0, 0x14948DD90};

WEAK symbol<EngineDependentDvarMut> selectedfriendindex{0x157960B20, 0x0,
                                                        0x14A30C4A8};
WEAK symbol<EngineDependentDvarMut> selectedfriendname{0x157960B18, 0x1579DFA18,
                                                       0x14A30C4A0};
WEAK symbol<EngineDependentDvarMut> selectedplayerxuid{0x157960B28, 0x1579DFA20,
                                                       0x14A30C4B0};
WEAK symbol<EngineDependentDvarMut> mapcrc{0x0, 0x0, 0x14A179868};
WEAK symbol<EngineDependentDvarMut> matchmaking_debug{0x151395898, 0x151414808,
                                                      0x148374620};
WEAK symbol<EngineDependentDvarMut> maxDailyZMDoubleXP{0x1511A68F0, 0x151225870,
                                                       0x0};
WEAK symbol<EngineDependentDvarMut> maxLossesValue{0x151399960, 0x1514188D0,
                                                   0x148378190};
WEAK symbol<EngineDependentDvarMut> menuMapMaxRange{0x144785520, 0x144804520,
                                                    0x14201EF50};
WEAK symbol<EngineDependentDvarMut> long_blocking_call{0x15686E8A0, 0x1568ED820,
                                                       0x14948EE40};
WEAK symbol<EngineDependentDvarMut> local_media_tier{0x151272790, 0x1512F1700,
                                                     0x0};
WEAK symbol<EngineDependentDvarMut> disableLookAtEntityLogic{
    0x159BAB4D0, 0x159C2A450, 0x14B349C90};
WEAK symbol<EngineDependentDvarMut> disable_rope{0x159BAB3F0, 0x159C2A370,
                                                 0x14B349C20};
WEAK symbol<EngineDependentDvarMut> disconnected_ctrls{0x15686EB70, 0x1568EDAF0,
                                                       0x14948F148};
WEAK symbol<EngineDependentDvarMut> beta_special{0x15686EB78, 0x1568EDAF8,
                                                 0x14948F150};
WEAK symbol<EngineDependentDvarMut> version{0x0, 0x0, 0x14948EE90};
WEAK symbol<EngineDependentDvarMut> zombie_devgui{0x15686ED58, 0x1568EDCD8,
                                                  0x14948DC78};
WEAK symbol<EngineDependentDvarMut> Zm_Private_Rankedmatch{
    0x15686ED70, 0x1568EDCF0, 0x14948DC90};
WEAK symbol<EngineDependentDvarMut> SystemLink{0x0, 0x0, 0x1428AAEC0};
WEAK symbol<EngineDependentDvarMut> systemUnderMaintenance{
    0x15118D898, 0x15120C818, 0x1482DEE18};
WEAK symbol<EngineDependentDvarMut> teamsplitter_verbose{
    0x1512508B8, 0x1512CF7F8, 0x148340248};
WEAK symbol<EngineDependentDvarMut> testintervaljitter{0x1556CE7E8, 0x15574D758,
                                                       0x148581288};
WEAK symbol<EngineDependentDvarMut> throwback_enabled{0x15686ED00, 0x1568EDC80,
                                                      0x14948DC00};
WEAK symbol<EngineDependentDvarMut> timescale{0x15686E8B8, 0x1568ED838,
                                              0x14948EE58};
WEAK symbol<EngineDependentDvarMut> useClassSets{0x151642E30, 0x1516C1DA0,
                                                 0x1483C2A40};
WEAK symbol<EngineDependentDvarMut> useLargeRadius{0x14A037938, 0x14A3D1658,
                                                   0x147103018};
WEAK symbol<EngineDependentDvarMut> useMapPreloading{0x15686EEC0, 0x0, 0x0};
WEAK symbol<EngineDependentDvarMut> useRadius{0x14A037930, 0x14A3D1650,
                                              0x147103010};
WEAK symbol<EngineDependentDvarMut> waitforinitial{0x15686EEB0, 0x1568EDE38,
                                                   0x14948DDD8};
WEAK symbol<EngineDependentDvarMut> waitforstreamer{0x15686EEA8, 0x1568EDE30,
                                                    0x14948DDD0};
WEAK symbol<EngineDependentDvarMut> waterbrush_entity{0x159BABA50, 0x159C2A9D0,
                                                      0x14B349F78};
WEAK symbol<EngineDependentDvarMut> actors_walkable{0x159BABA90, 0x159C2AA10,
                                                    0x14B349FB8};
WEAK symbol<EngineDependentDvarMut> addmappackflagsuserinfo{
    0x15686EE78, 0x1568EDDF8, 0x14948DD98};
WEAK symbol<EngineDependentDvarMut> adsZeroSpread{0x159BAB820, 0x159C2A7A0,
                                                  0x14B34A340};
WEAK symbol<EngineDependentDvarMut> allowtaskmanagerrsu{
    0x15686EB68, 0x1568EDAE8, 0x14948F140};
WEAK symbol<EngineDependentDvarMut> all_players_are_connected{
    0x15686EDA8, 0x1568EDD28, 0x14948DCC8};
WEAK symbol<EngineDependentDvarMut> amd_useshaderextension{
    0x14AE16CF8, 0x14AE95C78, 0x147CBCFA0};
WEAK symbol<EngineDependentDvarMut> autoexecdemoconfig{0x15686EB90, 0x1568EDB10,
                                                       0x14948F168};
WEAK symbol<EngineDependentDvarMut> bandwidth_retry_interval{
    0x15138F230, 0x15140E1A0, 0x14836DFC8};
WEAK symbol<EngineDependentDvarMut> behaviorTrackerVersionNumber{
    0x151399818, 0x151418788, 0x148378048};
WEAK symbol<EngineDependentDvarMut> checkEmblemForRank{0x159B80488, 0x159BFF408,
                                                       0x0};
WEAK symbol<EngineDependentDvarMut> content_trialcontentpackbits{
    0x0, 0x156CF41C8, 0x149758CF8};
WEAK symbol<EngineDependentDvarMut> con_typewriterColorBase{0x0, 0x0,
                                                            0x142872C38};
WEAK symbol<EngineDependentDvarMut> cp_queued_level{0x157736858, 0x0,
                                                    0x14A179870};
WEAK symbol<EngineDependentDvarMut> cybercom_enabled{0x1441D5C50, 0x144254C50,
                                                     0x141E56C08};
WEAK symbol<EngineDependentDvarMut> daily_contract_cryptokey_reward_count{
    0x15114E488, 0x1511CD408, 0x0};
WEAK symbol<EngineDependentDvarMut> db_keyserver1{0x1493890E0, 0x1494080E0,
                                                  0x1468F91C0};
WEAK symbol<EngineDependentDvarMut> defaultClassSetCount{
    0x151642E40, 0x1516C1DB0, 0x1483C2A50};
WEAK symbol<EngineDependentDvarMut> def_unlink_nodes_for{0x0, 0x1568EDB58,
                                                         0x14948F1B0};
WEAK symbol<EngineDependentDvarMut> developer{0x14AE169A0, 0x14AE95920,
                                              0x147CBCC48};
WEAK symbol<EngineDependentDvarMut> doublejump_enabled{0x159BAB898, 0x159C2A818,
                                                       0x14B34A380};
WEAK symbol<EngineDependentDvarMut> doublesided_raycasts{
    0x15686ECF0, 0x1568EDC70, 0x14948DBF0};
WEAK symbol<EngineDependentDvarMut> excellentping{0x1556CE748, 0x15574D6B8,
                                                  0x1485811E8};
WEAK symbol<EngineDependentDvarMut> exp_objective_setgamemodeflags{0x0, 0x0,
                                                                   0x14A179880};
WEAK symbol<EngineDependentDvarMut> filterdedicatedserverresults{
    0x151395900, 0x151414870, 0x148374688};
WEAK symbol<EngineDependentDvarMut> fixedtime{0x15686ECB8, 0x1568EDC38,
                                              0x14948DBB8};
WEAK symbol<EngineDependentDvarMut> fixedtime_float{0x15686ECC0, 0x1568EDC40,
                                                    0x14948DBC0};
WEAK symbol<EngineDependentDvarMut> fixNegativeLosses{0x151399958, 0x1514188C8,
                                                      0x148378188};
WEAK symbol<EngineDependentDvarMut> flaredisableeffects{
    0x144CB33A0, 0x144D323A0, 0x142240DE8};
WEAK symbol<EngineDependentDvarMut> footstep_sounds_cutoff{
    0x159BAB410, 0x159C2A390, 0x14B349C30};
WEAK symbol<EngineDependentDvarMut> friction{0x159BAB388, 0x159C2A308,
                                             0x14B34A0E0};
WEAK symbol<EngineDependentDvarMut> friendlyContentOutlines{
    0x159BAB970, 0x159C2A8F0, 0x14B34A3E8};
WEAK symbol<EngineDependentDvarMut> hackMultiLauncherVerticleOffset{
    0x14A5746F8, 0x14A5F36E8, 0x147A5C2E8};
WEAK symbol<EngineDependentDvarMut> hidef{0x15686E908, 0x1568ED888,
                                          0x14948EEA8};
WEAK symbol<EngineDependentDvarMut> highlightedplayername{
    0x157960B30, 0x1579DFA30, 0x14A30C4B8};
WEAK symbol<EngineDependentDvarMut> hopper_playlist{0x0, 0x0, 0x1482936A8};
WEAK symbol<EngineDependentDvarMut> juke_enabled{0x159BAB8B0, 0x159C2A830,
                                                 0x14B34A388};
WEAK symbol<EngineDependentDvarMut> keyarchivewritedelay{0x0, 0x15163FB58,
                                                         0x148382868};
WEAK symbol<EngineDependentDvarMut> killonlytrailfx{0x1456BC1D0, 0x14573B1D0,
                                                    0x142C41D88};
WEAK symbol<EngineDependentDvarMut> Lagometer_Pos{0x1449B2B18, 0x144A31B18,
                                                  0x14222C050};
WEAK symbol<EngineDependentDvarMut> lbtaskdelay{0x1511A83E8, 0x151227368,
                                                0x1482F9810};
WEAK symbol<EngineDependentDvarMut> minDelayForOtherPlayerStatsFetch{
    0x1515C0C60, 0x15163FBD0, 0x1483828E8};
WEAK symbol<EngineDependentDvarMut> miniscoreboardhide{0x15686EE18, 0x1568EDD98,
                                                       0x14948DD38};
WEAK symbol<EngineDependentDvarMut> missingcommandwillerror{
    0x15686EE20, 0x1568EDDA0, 0x14948DD40};
WEAK symbol<EngineDependentDvarMut> mods_enabled{0x15670DD00, 0x15678CC80,
                                                 0x14933E5F0};
WEAK symbol<EngineDependentDvarMut> mortarstrikesleft{0x0, 0x0, 0x1428AAEB8};
WEAK symbol<EngineDependentDvarMut> mp_blackjack_consumable_wait{
    0x159BAB7B0, 0x159C2A730, 0x14B349E08};
WEAK symbol<EngineDependentDvarMut> msg_logpredictionpositionerrors{
    0x156DCAB30, 0x0, 0x1498C1840};
WEAK symbol<EngineDependentDvarMut> ms_netapop{0x0, 0x14A56D800, 0x1477FDD80};
WEAK symbol<EngineDependentDvarMut> nextarena{0x15686EDF0, 0x1568EDD70,
                                              0x14948DD10};
WEAK symbol<EngineDependentDvarMut> nextmap{0x15686E900, 0x1568ED880,
                                            0x14948EEA0};
WEAK symbol<EngineDependentDvarMut> onlinegame{0x15686ED68, 0x1568EDCE8,
                                               0x14948DC88};
WEAK symbol<EngineDependentDvarMut> paintjobVersion{0x151274F48, 0x1512F3EB8,
                                                    0x1483613E8};
WEAK symbol<EngineDependentDvarMut> pclive_updateFriends{0x1512708D8, 0x0,
                                                         0x148360268};
WEAK symbol<EngineDependentDvarMut> pc_newversionavailable{
    0x1512508A0, 0x1512CF7E0, 0x148340230};
WEAK symbol<EngineDependentDvarMut> pickupprints{0x14A037970, 0x14A3D1690,
                                                 0x147103050};
WEAK symbol<EngineDependentDvarMut> popInLetterFXTime{0x1447854F8, 0x1448044F8,
                                                      0x14201F090};
WEAK symbol<EngineDependentDvarMut> prone_bipod_enable{0x159BAB1E8, 0x159C2A168,
                                                       0x14B349B78};
WEAK symbol<EngineDependentDvarMut> protocol{0x0, 0x0, 0x14A179898};
WEAK symbol<EngineDependentDvarMut> purchasedClassSetCount{
    0x151642E38, 0x1516C1DA8, 0x1483C2A48};
WEAK symbol<EngineDependentDvarMut> quit_on_error{0x15686EDC0, 0x1568EDD40,
                                                  0x14948DCE0};
WEAK symbol<EngineDependentDvarMut> ragdoll_activationVelocitySq{
    0x159BAB708, 0x159C2A688, 0x14B34A2B8};
WEAK symbol<EngineDependentDvarMut> saved_gameskill{0x15686EE10, 0x1568EDD90,
                                                    0x14948DD30};
WEAK symbol<EngineDependentDvarMut> saveLocalMatchRecordBinaryFile{
    0x149B358E0, 0x149BB48E0, 0x0};
WEAK symbol<EngineDependentDvarMut> Settings_AllowGibbing{
    0x14A00B5B0, 0x14A08A5B0, 0x1470D6DD0};
WEAK symbol<EngineDependentDvarMut> Settings_DefaultSubtitles{0x0, 0x14A08A5B8,
                                                              0x1470D6DD8};
WEAK symbol<EngineDependentDvarMut> shortversion{0x0, 0x0, 0x14948EE98};
WEAK symbol<EngineDependentDvarMut> shoutcastHighlightedClient{
    0x1447854F0, 0x1448044F0, 0x14201F088};
WEAK symbol<EngineDependentDvarMut> shoutcastSelectedClient{
    0x1447854E8, 0x1448044E8, 0x14201F080};
WEAK symbol<EngineDependentDvarMut> showmaperrors{0x15686EBD0, 0x1568EDB50,
                                                  0x14948F1A8};
WEAK symbol<EngineDependentDvarMut> stopspeed{0x159BAB390, 0x159C2A310,
                                              0x14B34A0E8};
WEAK symbol<EngineDependentDvarMut> streammbpersecond{0x0, 0x157500AF0,
                                                      0x149F78950};
WEAK symbol<EngineDependentDvarMut> total_paintjobs{0x151274F40, 0x1512F3EB0,
                                                    0x1483613E0};
WEAK symbol<EngineDependentDvarMut> tracer_debugdraw{0x14495A408, 0x1449D9408,
                                                     0x1421E3768};
WEAK symbol<EngineDependentDvarMut> turretPlayerAvoidScale{
    0x14A037890, 0x14A3D15B0, 0x147836978};
WEAK symbol<EngineDependentDvarMut> typeWriterCod7LetterFXTime{
    0x1447854C0, 0x1448044C0, 0x14201F058};
WEAK symbol<EngineDependentDvarMut> unsubscriptionCooloffTimer{
    0x1511D1CB0, 0x151250C30, 0x14831F150};
WEAK symbol<EngineDependentDvarMut> weaponrest_enabled{0x159BAB900, 0x159C2A880,
                                                       0x14B34A3B0};
WEAK symbol<EngineDependentDvarMut> xpGroups{0x1511A68D0, 0x151225850, 0x0};
/*
  Name of dvar is unknown.

  Causes game to error with `EXE_LUI_OUT_OF_MEMORY` reason if UI model
  allocation fails and value is 1 or 2 (initialised as 0).

  Never set in the engine, and none of the set of permutation of words in Lua,
  GSC scripts, nor engine strings has hash matching this dvar.
  This somehow still is set, somewhere, to `1` or `2`, as the aforementioned
  error _does_ occur on model allocation failure.

  TODO: what is the name of this dvar?
  TODO: how, where, and why is this dvar value changed?
*/
WEAK symbol<EngineDependentDvarMut> lui_oom{0x157960BB0, 0x0, 0x0};

WEAK symbol<EngineDependentDvarMut> scr_xpscalecp{0x15686EBF8, 0x1568EDB78,
                                                  0x14948F1D8};
WEAK symbol<EngineDependentDvarMut> scr_xpscalemp{0x15686EC00, 0x1568EDB80,
                                                  0x14948F1E0};
WEAK symbol<EngineDependentDvarMut> scr_xpscalezm{0x15686EC08, 0x1568EDB88,
                                                  0x14948F1E8};
WEAK symbol<EngineDependentDvarMut> scr_gunxpscalecp{0x15686EC18, 0x1568EDB98,
                                                     0x14948DB18};
WEAK symbol<EngineDependentDvarMut> scr_gunxpscalemp{0x15686EC20, 0x1568EDBA0,
                                                     0x14948DB20};
WEAK symbol<EngineDependentDvarMut> scr_gunxpscalezm{0x15686EC28, 0x1568EDBA8,
                                                     0x14948DB28};

WEAK symbol<int32_t> g_dvarCount{0x157A472CC, 0x157AC61CC, 0x14A3CB5FC};
WEAK symbol<DvarPool> s_dvarPool{0x157A47320, 0x0, 0x14A3CB620};
WEAK symbol<EncryptedDvarPool> s_dvarPool_cl{0x157A47320, 0x157AC6220, 0x0};
inline EngineDependentDvarPool dvar_pool() {
  return is_server() ? EngineDependentDvarPool(s_dvarPool.get())
                     : EngineDependentDvarPool(s_dvarPool_cl.get());
}

WEAK symbol<int32_t> s_nextFreeCallback{0x157A4730C, 0x157AC620C, 0x14A3CB60C};
WEAK symbol<DvarCallbackPool> s_dvarCallbackPool{0x157B87320, 0x157C06220,
                                                 0x14A4DB620};

WEAK symbol<DvarHashTable> s_dvarHashTable{0x157B88B20, 0x157C07A20,
                                           0x14A4DCE20};

WEAK symbol<bool> s_canSetConfigDvars{0x157A472E8, 0x0, 0x14A3CB5D8};

// Used for e.g. changing in-game client FOV when `cg_fov` is changed,
// queueing mod load when `fs_game` is changed.
WEAK symbol<void(EngineDependentDvar dvar, modifiedCallback callback)>
    Dvar_SetModifiedCallback{0x14226BB30, 0x1422C8650, 0x140579990};

WEAK symbol<void(EngineDependentDvar dvar, DvarFlags flags)> Dvar_AddFlags{
    0x14225B740, 0x1422B8260, 0x140574880};

WEAK symbol<bool(EngineDependentDvar dvar)> Dvar_IsSessionModeBaseDvar{
    0x142265880, 0x1422C23A0, 0x140576890};

WEAK symbol<EngineDependentDvarMut(const char *dvarName)> Dvar_FindVar{
    0x1422601B0, 0x1422BCCD0, 0x140575540};

WEAK symbol<game::CanonHash_t(const char *str)> Dvar_GenerateHash{
    0x14133DC10, 0x14133DBF0, 0x140185800};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash)> Dvar_FindMalleableVar{
    0x142260120, 0x1422BCC40, 0x1405754B0};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_GetDebugName{
    0x157B8CB30, 0x1422BD250, 0x0};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_DisplayableValue{
    0x14225F560, 0x1422BC080, 0x140574F40};

WEAK symbol<EngineDependentDvarMut(
    dvarStrHash_t hash, const char *dvarName, dvarType_t type, DvarFlags flags,
    DvarValue *value, DvarLimits *domain, const char *description,
    bool isSessionModeDvar)>
    Dvar_RegisterVariant{0x142268380, 0x1422C4EA0, 0x140577A90};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   bool value, DvarFlags flags,
                                   const char *description)>
    Dvar_RegisterBool{0x142273DE0, 0x1422D0900, 0x14057B500};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   int32_t value, int32_t min, int32_t max,
                                   DvarFlags flags, const char *description)>
    Dvar_RegisterInt{0x142273FC0, 0x1422D0AE0, 0x14057B7B0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   float value, float min, float max,
                                   DvarFlags flags, const char *description)>
    Dvar_RegisterFloat{0x142273F40, 0x1422D0A60, 0x14057B6B0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   bool value, DvarFlags flags,
                                   const char *description)>
    Dvar_SessionModeRegisterBool{0x142274220, 0x1422D0D40, 0x14057BAA0};
WEAK symbol<void(dvarStrHash_t hash, bool value, eModes mode)>
    Dvar_SessionModeSetDefaultBool{0x142274370, 0x1422D0E90, 0x14057BCE0};
WEAK symbol<EngineDependentDvarMut(dvarStrHash_t hash, const char *dvarName,
                                   const char *value, DvarFlags flags,
                                   const char *description)>
    Dvar_RegisterString{0x142274050, 0x1422D0B70, 0x14057B890};

WEAK symbol<void(forEachCallback callback, void *userData)> Dvar_ForEach{
    0x1422601E0, 0x1422BCD00, 0x140575570};
WEAK
    symbol<void(const char *dvarName, const char *string, bool createIfMissing)>
        Dvar_SetFromStringByName{0x14226A9E0, 0x1422C7500, 0x140579290};
WEAK symbol<EngineDependentDvarMut(EngineDependentDvarMut dvar, eModes mode)>
    Dvar_GetSessionModeSpecificDvar{0x1422629E0, 0x1422BF500, 0x140575D90};
WEAK symbol<void(EngineDependentDvar dvar, DvarValue *value,
                 DvarSetSource source)>
    Dvar_SetVariant{0x14226C510, 0x1422C9030, 0x0};
WEAK symbol<void(void (*callback)(const char *debugName))> Dvar_ForEachName{
    0x142260310, 0x1422BCE30, 0x0};
// Actual name is unknown - Dvar_ForEachName with a local client number
WEAK symbol<void(
    LocalClientNum_t localClientNum,
    void (*callback)(LocalClientNum_t localClientNum, const char *debugName))>
    Dvar_ForEachNameClientNum{0x142260260, 0x1422BCD80, 0x0};
WEAK symbol<bool(EngineDependentDvar dvar)> Dvar_CanSetConfigDvar{
    0x14225BD70, 0x1422B8890, 0x140574C00};
WEAK symbol<bool(EngineDependentDvar dvar)> Dvar_GetBool{
    0x1422603B0, 0x1422BCED0, 0x140575740};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainEnumStringCount{
    0x142260820, 0x1422BD340, 0x140575900};
WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetDomainFloatMax{
    0x1422608A0, 0x1422BD3C0, 0x140575940};
WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetDomainFloatMin{
    0x1422608E0, 0x1422BD400, 0x140575980};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetDomainInt64Max{
    0x142260920, 0x1422BD440, 0x140575A90};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetDomainInt64Min{
    0x142260960, 0x1422BD480, 0x1405759C0};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainIntMax{
    0x1422609A0, 0x1422BD4C0, 0x140575A00};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetDomainIntMin{
    0x1422609F0, 0x1422BD510, 0x140575A50};

WEAK symbol<float(EngineDependentDvar dvar)> Dvar_GetFloat{
    0x142260B00, 0x1422BD620, 0x140575B20};
WEAK symbol<int64_t(EngineDependentDvar dvar)> Dvar_GetInt64{
    0x142261130, 0x1422BDC50, 0x140575B90};
WEAK symbol<int32_t(EngineDependentDvar dvar)> Dvar_GetInt{
    0x142261D40, 0x1422BE860, 0x140575C20};
WEAK symbol<dvarType_t(EngineDependentDvar dvar)> Dvar_GetType{
    0x142262AC0, 0x1422BF5E0, 0x140575E90};
WEAK symbol<uint64_t(EngineDependentDvar dvar)> Dvar_GetUInt64{
    0x142262B00, 0x1422BF620, 0x140575ED0};
WEAK symbol<const char *(EngineDependentDvar dvar)> Dvar_GetString{
    0x142262A70, 0x1422BF590, 0x140575E30};

WEAK symbol<void(EngineDependentDvar dvar, bool val, DvarSetSource source)>
    _Dvar_SetBoolFromSource{0x142269770, 0x1422C6290, 0x140578BA0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetBoolFromSource(EngineDependentDvar dvar, bool val,
                                   DvarSetSource source) {
  return _Dvar_SetBoolFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, float val, DvarSetSource source)>
    _Dvar_SetFloatFromSource{0x14226A2B0, 0x1422C6DD0, 0x140579190};
// To allow forward declaration for use in struct methods
inline void Dvar_SetFloatFromSource(EngineDependentDvar dvar, float val,
                                    DvarSetSource source) {
  return _Dvar_SetFloatFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, int64_t val, DvarSetSource source)>
    _Dvar_SetInt64FromSource{0x14226B050, 0x1422C7B70, 0x140579660};
// To allow forward declaration for use in struct methods
inline void Dvar_SetInt64FromSource(EngineDependentDvar dvar, int64_t val,
                                    DvarSetSource source) {
  return _Dvar_SetInt64FromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, int32_t val, DvarSetSource source)>
    _Dvar_SetIntFromSource{0x14226B3B0, 0x1422C7ED0, 0x140579740};
// To allow forward declaration for use in struct methods
inline void Dvar_SetIntFromSource(EngineDependentDvar dvar, int32_t val,
                                  DvarSetSource source) {
  return _Dvar_SetIntFromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, uint64_t val, DvarSetSource source)>
    _Dvar_SetUInt64FromSource{0x14226BFD0, 0x1422C8AF0, 0x140579B60};
// To allow forward declaration for use in struct methods
inline void Dvar_SetUInt64FromSource(EngineDependentDvar dvar, uint64_t val,
                                     DvarSetSource source) {
  return _Dvar_SetUInt64FromSource(dvar, val, source);
}
WEAK symbol<void(EngineDependentDvar dvar, const char *string,
                 DvarSetSource source)>
    _Dvar_SetStringFromSource{0x14226BC50, 0x1422C8770, 0x140579AB0};
// To allow forward declaration for use in struct methods
inline void Dvar_SetStringFromSource(EngineDependentDvar dvar, const char *val,
                                     DvarSetSource source) {
  return _Dvar_SetStringFromSource(dvar, val, source);
}

WEAK symbol<void(EngineDependentDvar dvar, const char *val)> _Dvar_SetString{
    0x14226BC00, 0x1422C8720, 0x140579A60};
// To allow forward declaration for use in struct methods
inline void Dvar_SetString(EngineDependentDvar dvar, const char *val) {
  return _Dvar_SetString(dvar, val);
}
WEAK symbol<void(EngineDependentDvar dvar, int32_t value)> _Dvar_SetInt{
    0x14226B3A0, 0x1422C7EC0, 0x140579730};
// To allow forward declaration for use in struct methods
inline void Dvar_SetInt(EngineDependentDvar dvar, int32_t value) {
  return _Dvar_SetInt(dvar, value);
}

WEAK symbol<EngineDependentDvar(EngineDependentDvar base, eModes mode)>
    Dvar_GetSessionModeSpecificDvarInternal{0x1422629E0, 0x1422BF500,
                                            0x140575D90};

WEAK symbol<DvarFlags> g_dvar_modifiedFlags{0x157A472C8, 0x157AC61C8,
                                            0x14A3CB5F8};

template <typename T_DvarValue>
inline dvarCallBack_t *dvar<T_DvarValue>::modifiedCallback() noexcept {
  if (*s_nextFreeCallback > 0) {
    for (int32_t i = 0; i < *s_nextFreeCallback; ++i) {
      dvarCallBack_t *cb = &s_dvarCallbackPool->pool[i];
      if (cb->dvar.name() == name()) {
        return cb;
      }
    }
  }

  return nullptr;
}

template <typename T_DvarValue>
template <typename T>
  requires(!std::same_as<T, const char *> &&
           !std::same_as<T, const std::string_view &> &&
           !std::same_as<T, const std::string &>)
inline std::optional<T>
dvar<T_DvarValue>::set(T val, bool execModifiedCallback) noexcept {
  dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
  if (sessionModeSpecificDvar) {
    return sessionModeSpecificDvar->set(val);
  }

  if (domain().contains(val)) {
    T prev;
    if (flags().latch) {
      prev = latched().set(val);
    } else {
      *g_dvar_modifiedFlags |= flags();
      prev = current().set(val);
      (void)latched().set(val);
      modified() = true;

      if (execModifiedCallback && flags().modifiedCallback) {
        dvarCallBack_t *cb = modifiedCallback();
        if (cb) {
          cb->needsCallback = true;
        }
      }
    }

    return prev;
  }

  return std::nullopt;
}

template <typename T_DvarValue>
inline std::optional<std::string>
dvar<T_DvarValue>::set(const char *val, bool execModifiedCallback) noexcept {
  dvar<T_DvarValue> *sessionModeSpecificDvar = sessionModeSpecific();
  if (sessionModeSpecificDvar) {
    return sessionModeSpecificDvar->set(val);
  }

  switch (type()) {
  case dvarType_t::ENUM: {
    if (domain().contains(val)) {
      const char *prev = nullptr;
      if (flags().latch) {
        prev = latched().set(val);
      } else {
        (void)latched().set(val);
        prev = current().set(val);
        *g_dvar_modifiedFlags |= flags();
        modified() = true;

        if (execModifiedCallback && flags().modifiedCallback) {
          dvarCallBack_t *cb = modifiedCallback();
          if (cb) {
            cb->needsCallback = true;
          }
        }
      }

      return prev;
    }
    break;
  }
  case dvarType_t::STRING: {
    std::string prev;
    if (flags().latch) {
      if (val != latched().string()) {
        const char *new_alloc =
            val == reset().string() || val == current().string()
                ? val
                : sl::CopyString(val);
        const char *prev_alloc = latched().set(new_alloc, false);
        prev = prev_alloc;
        if (prev_alloc != current().string() &&
            prev_alloc != reset().string()) {
          sl::FreeString(prev_alloc);
        }
      } else {
        break;
      }
    } else if (val != current().string()) {
      *g_dvar_modifiedFlags |= flags();
      const bool new_alloc_eq_latched = val == latched().string();
      const char *new_alloc = val == reset().string() || new_alloc_eq_latched
                                  ? val
                                  : sl::CopyString(val);
      const char *prev_alloc = current().set(new_alloc, false);
      prev = prev_alloc;
      if (prev_alloc != reset().string()) {
        sl::FreeString(prev_alloc);
      }
      if (!new_alloc_eq_latched) {
        latched().string() = new_alloc;
      }
      modified() = true;

      if (execModifiedCallback && flags().modifiedCallback) {
        dvarCallBack_t *cb = modifiedCallback();
        if (cb) {
          cb->needsCallback = true;
        }
      }
    } else {
      break;
    }

    return prev;
  }
  default: {
    break;
  }
  }

  return std::nullopt;
}
template <typename T_DvarValue>
alwaysinline std::optional<std::string>
dvar<T_DvarValue>::set(const std::string_view &val,
                       bool execModifiedCallback) noexcept {
  return set(val.data(), execModifiedCallback);
}

template <typename T_DvarValue>
alwaysinline std::optional<std::string>
dvar<T_DvarValue>::set(const std::string &val,
                       bool execModifiedCallback) noexcept {
  return set(val.c_str(), execModifiedCallback);
}
} // namespace game
