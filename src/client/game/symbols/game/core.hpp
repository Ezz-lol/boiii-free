#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {

WEAK symbol<void(level::gentity_s *ent, level::gentity_s *target, int32_t mode,
                 const char *chatText)>
    G_Say{0x0, 0x140299170};
WEAK symbol<void(const char *fmt, ...)> G_LogPrintf{0x0, 0x1402A7BB0};

WEAK symbol<void(int32_t levelTime, int32_t randomSeed, qboolean restart,
                 qboolean registerDvars,

                 qboolean savegame)>
    G_InitGame{0x1419CA420, 0x1402ABB80};

WEAK symbol<level::gentity_s *()> G_Spawn{0x141B81420, 0x140308570};
WEAK symbol<void(level::gentity_s *ed)> G_FreeEntity{0x141B77950, 0x1403067A0};
WEAK symbol<void()> G_PrintEntities{0x141B7BE40, 0x140307800};

WEAK symbol<void()> G_ClearVehicleInputs{0x1423812E0, 0x1405C1200};

WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, BoneIndex bone)>
    G_PlaySoundAlias{0x141B7BAC0, 0x140307480};
WEAK symbol<level::gentity_s *(const vec3_t *origin,
                               game::snd::SndAliasId alias)>
    G_PlaySoundAliasAtPoint{0x141B7BC00, 0x1403075C0};
WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, uint32_t tag)>
    G_PlaySoundAliasWithNotify{0x141B7BD20, 0x1403076E0};
WEAK symbol<void(game::level::gentity_s *ent, snd::SndAliasId index,
                 game::scr::ScrString_t notifyString)>
    G_RegisterSoundWait{0x141B80F40, 0x140308090};

WEAK symbol<int32_t(const char *name)> G_TagIndex{0x141B81890, 0x1403089D0};

WEAK symbol<level::clientState_t *(ClientNum_t clientNum)> G_GetClientState{
    0x1419B0970, 0x1402A7820};
WEAK symbol<void(hunk::HunkUser *hunk, cl::clientAllocFlags_t flags)>
    Checkpoint_Init{0x1400B24C0};

WEAK symbol<level::gentity_t *(const vec3_t *origin, snd::SndAliasId alias)>
    G_TempEntity{0x141B819A0, 0x140308AF0};
WEAK symbol<str64_t> g_mapname{0x1579DF840, 0x14A30C300};
WEAK symbol<int32_t(const char *string)> G_LocalizedStringIndex{0x141B7B820,
                                                                0x1403071E0};
WEAK symbol<db::xasset::xmodel::XModel *(int32_t index)> G_GetModel{
    0x0, 0x140044390};
WEAK symbol<int64_t()> G_RelinkAllEntities{0x0, 0x14004B460};
WEAK symbol<void(mem::MemoryFile *memFile)> G_LoadMain{0x0, 0x140052B00};
WEAK symbol<void(mem::MemoryFile *memFile)> G_SaveInitConfigstrings{
    0x0, 0x140053BA0};
WEAK symbol<void(mem::MemoryFile *memFile)> G_SaveMain{0x0, 0x140053F10};
WEAK symbol<void(mem::MemoryFile **memfile, int32_t *levelTime)> G_PreLoadGame{
    0x0, 0x140056940};
WEAK symbol<void()> G_CreateClientFieldsHeap{0x0, 0x14005B0F0};
WEAK symbol<bool()> G_ClientFieldAllowVersionReRegistration{0x0, 0x1400C3380};
WEAK symbol<void(level::gentity_t *player, int32_t rank, int32_t prestige,
                 int32_t unlockTokensAdded)>
    G_ScrEvent_PlayerRankUp{0x0, 0x140170220};
WEAK symbol<bool(weapon::Weapon weapon, cm::BulletFireParams *incomingbp,
                 level::gentity_t *attacker, level::gentity_t *shieldHolder,
                 cm::BulletTraceResults *br, int32_t gameTime)>
    G_WeaponRicochetGamey{0x0, 0x140238E30};
WEAK symbol<bool(level::gclient_t *client)> G_ClientCanSpectateDead{
    0x0, 0x14023A8B0};
WEAK symbol<qboolean(level::gclient_t *client, team_t team)>
    G_ClientCanSpectateTeam{0x0, 0x14023A8C0};
WEAK symbol<void(level::gentity_t *ent)> G_DoTouchTriggers{0x0, 0x14023A8D0};
WEAK symbol<void(level::gentity_t *pEnt)> G_SetClientContents{0x0, 0x14023B030};
WEAK symbol<void(level::gentity_t *ent, anim::DObj *obj)>
    G_AnimScripted_NotifyAnimationEnd{0x0, 0x14023C370};
WEAK symbol<void(level::gentity_t *ent)> G_Animscripted_Think{0x0, 0x14023D750};
WEAK symbol<ai::AIBody *(int32_t entityNum, LocalClientNum_t localClientNum)>
    G_GetAIBody{0x0, 0x140240A40};
WEAK symbol<void(const int32_t msecTimeNow)> G_BeamUpdateAll{0x0, 0x140245990};
WEAK symbol<void(const ClientNum_t clientNum, eModes mode,
                 renderOptions_t *renderOptions)>
    G_CharacterCustomization_SetupRenderOptionsFromClientStats{0x0,
                                                               0x1402465E0};
WEAK symbol<void(const ClientNum_t clientNum, eModes mode,
                 renderOptions_t *renderOptions)>
    G_CharacterCustomization_SetupRenderOptionsFromClientStats_Internal{
        0x0, 0x140246620};
WEAK symbol<void(level::gentity_t *ent, uint32_t flags, weapon::Weapon *weapon,
                 weapon::gadgetHeldSlot_e slot)>
    G_PlayerGadget_ApplyState{0x0, 0x140247300};
WEAK symbol<const weapon::entityGadgetState_t *(
    const LocalClientNum_t localClientNum, int32_t entIndex,
    weapon::gadgetHeldSlot_e slot)>
    G_PlayerGadget_GetState{0x0, 0x1402475F0};
WEAK symbol<void(level::gentity_t *ent)> G_PlayerGadget_Init{0x0, 0x1402479B0};
WEAK symbol<void(level::gentity_t *self)> G_CancelAssassination{0x0,
                                                                0x140247AA0};
WEAK symbol<void(level::gentity_t *ent)> G_PlayerGadget_Update{0x0,
                                                               0x140247ED0};
WEAK symbol<void()> G_Gameskill_Init{0x0, 0x1402484C0};
WEAK symbol<void(level::gentity_t *ent)> G_RunZBarrier{0x0, 0x140269940};
WEAK symbol<void()> G_ZBarrier_FinishSpawningAll{0x0, 0x140269970};
WEAK symbol<void(sv::client_t *cl)> G_CreateBot{0x0, 0x140273260};
WEAK symbol<qboolean(level::gclient_t *client, level::clientState_t *cs)>
    G_ClientCanSpectateTeamOrLocalPlayer{0x0, 0x140277E10};
WEAK symbol<void(level::gentity_t *ent)> G_RunClient{0x0, 0x140278020};
WEAK symbol<void(level::gentity_t *ent)> G_UpdatePlayerNodes{0x0, 0x1402784C0};
WEAK symbol<int64_t(ClientNum_t clientNum)> G_ClientSessionInfoChanged{
    0x0, 0x140279960};
WEAK symbol<void(level::gentity_t *ent, const char *chatText)> G_Chat{
    0x0, 0x140298E70};
WEAK symbol<void(level::gentity_t *ent, level::gentity_t *other, int32_t mode,
                 int32_t color, const char *teamString, const char *cleanname,
                 const char *message)>
    G_SayTo{0x0, 0x140299370};
WEAK symbol<void(
    level::gentity_t *targ, level::gentity_t *inflictor,
    level::gentity_t *attacker, const vec3_t *dir, const vec3_t *point,
    int32_t damage, int32_t dFlags, int32_t mod, weapon::Weapon weapon,
    level::hitLocation_t hitLoc, const vec3_t *damageOrigin,
    uint32_t modelIndex, scr::ScrString_t partName, int32_t timeOffset,
    BoneIndex boneIndex, uint32_t surfaceType, const vec3_t *surfaceNormal)>
    G_Damage{0x0, 0x14029AD00};
WEAK symbol<scr::ScrString_t(level::hitLocation_t hitLoc)>
    G_GetHitLocationString{0x0, 0x14029C280};
WEAK symbol<int64_t(uint32_t index)> G_MeansOfDeathFromScriptParam{0x0,
                                                                   0x14029C7E0};
WEAK symbol<void()> G_ParseHitLocDmgTable{0x0, 0x14029C840};
WEAK symbol<void(level::gentity_t *self, int32_t damageState,
                 const vec3_t *vDir)>
    G_PlayerGibEvent{0x0, 0x14029C8F0};
WEAK symbol<void(level::gentity_t *ent)> G_RunItem{0x0, 0x1402A3BA0};
WEAK symbol<void()> G_ResetEntityParsePoint{0x0, 0x1402A6DA0};
WEAK symbol<uint16_t(anim::DObjModel *dobjModels, uint16_t numModels,
                     level::entityState_t *es, sv::clientInfo_t *ci,
                     bool hideClip)>
    G_AttachWeapon{0x0, 0x1402A7430};
WEAK symbol<int64_t()> G_GetActorSize{0x0, 0x1402A7700};
WEAK symbol<level::actorState_t *(int32_t actorNum)> G_GetActorState{
    0x0, 0x1402A7710};
WEAK symbol<int64_t()> G_GetCasterClientSize{0x0, 0x1402A77C0};
WEAK symbol<int64_t()> G_GetClientSize{0x0, 0x1402A7810};
WEAK symbol<void(int32_t handle, LocalClientNum_t localClientNum,
                 vec3_t *centroid)>
    G_GetEntityCentroid{0x0, 0x1402A7880};
WEAK symbol<bool(LocalClientNum_t localClientNum, int32_t entityNum,
                 vec3_t *origin, vec3_t *angles)>
    G_GetEntityOriginAngles{0x0, 0x1402A78A0};
WEAK symbol<const level::entityState_t *(LocalClientNum_t localClientNum,
                                         int32_t entIndex)>
    G_GetEntityState{0x0, 0x1402A7900};
WEAK symbol<level::playerState_t *(ClientNum_t clientNum)> G_GetPlayerState{
    0x0, 0x1402A7A10};
WEAK symbol<int32_t()> G_GetSavePersist{0x0, 0x1402A7A30};
WEAK symbol<bool(int32_t handle, LocalClientNum_t localClientNum,
                 scr::ScrString_t tagName, vec3_t *tagMtx,
                 int32_t cacheTimeAllowed)>
    G_GetWorldTagMatrixTimeAllowedWrapper{0x0, 0x1402A7AF0};
WEAK symbol<int32_t(ClientNum_t clientNum)> G_GetClientArchiveTime{0x0,
                                                                   0x1402A7D50};
WEAK symbol<void(qboolean savepersist)> G_SetSavePersist{0x0, 0x1402A7E50};
WEAK symbol<void(level::gentity_t *ent)> G_ClientDoPerFrameNotifies{
    0x0, 0x1402AB2C0};
WEAK symbol<void()> G_FreeAnimTreeInstances{0x0, 0x1402AB8C0};
WEAK symbol<void(bool clearTargets)> G_FreeEntities{0x0, 0x1402AB9A0};
WEAK symbol<level::CasterClientState *(ClientNum_t clientNum)>
    G_GetCasterClientState{0x0, 0x1402ABAC0};
WEAK symbol<level::CasterState *()> G_GetCasterState{0x0, 0x1402ABAF0};
WEAK symbol<int32_t(ClientNum_t clientNum)> G_GetClientPrestige{0x0,
                                                                0x1402ABB00};
WEAK symbol<int32_t(ClientNum_t clientNum)> G_GetClientRank{0x0, 0x1402ABB20};
WEAK symbol<level::MatchState *()> G_GetMatchState{0x0, 0x1402ABB60};
WEAK symbol<int64_t()> G_LoadAnimTreeInstances{0x0, 0x1402AC770};
WEAK symbol<void()> G_PopulateMatchState{0x0, 0x1402AC8B0};
WEAK symbol<void()> G_PumpCheatCode{0x0, 0x1402AC960};
WEAK symbol<void()> G_RegisterRegisterToolDvars{0x0, 0x1402ACFE0};
WEAK symbol<void(int32_t levelTime)> G_RunFrame{0x0, 0x1402AD050};
WEAK symbol<void(level::gentity_t *ent)> G_RunFrameForEntity{0x0, 0x1402AD860};
WEAK symbol<void()> G_RunFrameXanimUpdate{0x0, 0x1402ADBA0};
WEAK symbol<void(level::gentity_t *ent)> G_RunThink{0x0, 0x1402ADC90};
WEAK symbol<void(ClientNum_t clientNum, int32_t paragonRank)>
    G_SetClientParagonRank{0x0, 0x1402ADD10};
WEAK symbol<void(level::gentity_t *ent)> G_UpdateClientLinkInfo{0x0,
                                                                0x1402ADFB0};
WEAK symbol<void()> G_UpdateObjectiveToClients{0x0, 0x1402AE1D0};
WEAK symbol<void(level::gentity_t *ent)> G_UpdateTimedDamage{0x0, 0x1402AE6B0};
WEAK symbol<void(level::gentity_t *ent)> G_UpdateWeapons{0x0, 0x1402AE870};
WEAK symbol<void(level::gentity_t *ent)> G_RemoveMissileAttractors{0x0,
                                                                   0x1402B7760};
WEAK symbol<void(level::gentity_t *ent)> G_RunMissile{0x0, 0x1402B77B0};
WEAK symbol<void(level::gentity_t *ent)> G_RunMissileInternal{0x0, 0x1402B77C0};
WEAK symbol<void(level::gentity_t *ent)> G_MoverTeam_New{0x0, 0x1402C1AB0};
WEAK symbol<void(level::gentity_t *ent)> G_RunMover{0x0, 0x1402C2330};
WEAK symbol<void()> G_ClearPlayerCorpses{0x0, 0x1402C2C00};
WEAK symbol<void(level::gentity_t *ent)> G_RunCorpse{0x0, 0x1402C2DB0};
WEAK symbol<void(level::gentity_t *ent)> G_RunCorpseAnimate{0x0, 0x1402C2DF0};
WEAK symbol<void(level::gentity_t *ent)> G_RunCorpseMove{0x0, 0x1402C2EF0};
WEAK symbol<void()> G_InitObjectives{0x0, 0x1402E3410};
WEAK symbol<void()> G_ScrEvent_LevelFinalizeInit{0x0, 0x1402E9700};
WEAK symbol<void()> G_ScrEvent_GameTypeInit{0x0, 0x1402EA370};
WEAK symbol<void()> G_ScrEvent_LevelInit{0x0, 0x1402EA3D0};
WEAK symbol<void()> G_ScrEvent_LevelPreInit{0x0, 0x1402ED990};
WEAK symbol<void()> G_ScrEvent_GameTypeStart{0x0, 0x1402EECC0};
WEAK symbol<qboolean(level::gentity_t *ent)> G_CallSpawnEntity{0x0,
                                                               0x1402F3D30};
WEAK symbol<void()> G_LoadStructs{0x0, 0x1402F4110};
WEAK symbol<qboolean(const level::LinkerSpawnVar *spawnVar, const char *key,
                     const char *defaultString, float *out)>
    G_SpawnFloat{0x0, 0x1402F46C0};
WEAK symbol<qboolean(const level::LinkerSpawnVar *spawnVar, const char *key,
                     const char *defaultString, int32_t *out)>
    G_SpawnInt{0x0, 0x1402F4700};
WEAK symbol<void()> G_InitTargets{0x0, 0x1402FFEB0};
WEAK symbol<bool(const level::gentity_t *self)> G_IsTrigger{0x0, 0x1403021E0};
WEAK symbol<void(ClientNum_t client, snd::SndAliasList *aliasList)>
    G_AnimScriptSound{0x0, 0x140303D10};
WEAK symbol<bool(level::gentity_t *ent, scr::ScrString_t tagName,
                 vec3_t *tagMat, int32_t timeAllowed)>
    G_TagCache_GetCachedTag{0x0, 0x140304520};
WEAK symbol<void(level::gentity_t *ent)> G_DObjUpdate{0x0, 0x140304A50};
WEAK symbol<void(const level::gentity_t *ent, vec3_t *centroid)>
    G_EntityCentroid{0x0, 0x140305750};
WEAK symbol<void(level::gentity_t *ed)> G_FreeEntityRefs{0x0, 0x140306B90};
WEAK symbol<uint32_t *()> G_GetRandomSeed{0x0, 0x140306ED0};
WEAK symbol<void(level::entityState_t *es)> G_IncrementUseCount{0x0,
                                                                0x140307000};
WEAK symbol<void(level::gentity_t *e)> G_InitGentity{0x0, 0x140307030};
WEAK symbol<bool(level::gentity_t *ent)> G_IsEntityPaused{0x0, 0x1403070E0};
WEAK symbol<void(level::gentity_t *ent, const vec3_t *angle)> G_SetAngle{
    0x0, 0x140308270};
WEAK symbol<void(level::gentity_t *ent, const char *modelName)> G_SetModel{
    0x0, 0x1403082B0};
WEAK symbol<void(level::gentity_t *ent, const vec3_t *origin)> G_SetOrigin{
    0x0, 0x140308410};
WEAK symbol<level::gentity_t *()> G_SpawnActorCorpseClone{0x0, 0x140308690};
WEAK symbol<level::gentity_t *(int32_t start, int32_t range,
                               const char *entity_type)>
    G_SpawnStaticEntity{0x0, 0x140308700};
WEAK symbol<level::gentity_t *()> G_SpawnFake{0x0, 0x140308790};
WEAK symbol<level::gentity_t *()> G_SpawnPlayerClone{0x0, 0x140308850};
WEAK symbol<void(level::gentity_t *ent, qboolean bHasDObj)> G_UpdateTags{
    0x0, 0x140308FD0};
WEAK symbol<int32_t()> G_rand{0x0, 0x140309280};
WEAK symbol<void(uint32_t seed)> G_srand{0x0, 0x1403092D0};
WEAK symbol<void()> G_FreeScrVehicles{0x0, 0x140309930};
WEAK symbol<void(level::gentity_t *ent)> G_UpdateVehicleTags{0x0, 0x14030A780};
WEAK symbol<void()> G_FreeVehiclePaths{0x0, 0x140314820};
WEAK symbol<void()> G_FreeVehiclePathsScriptInfo{0x0, 0x140314930};
WEAK symbol<void()> G_InitVehiclePaths{0x0, 0x1403149A0};
WEAK symbol<void(bool scriptConnect)> G_SetupSplinePaths{0x0, 0x140314B60};
WEAK symbol<qboolean(const vehicle::VehicleDef *info)> G_IsVehicleAircraft{
    0x0, 0x140319A80};
WEAK symbol<qboolean(const vehicle::VehicleDef *info)> G_IsVehicleBoat{
    0x0, 0x140319AA0};
WEAK symbol<void()> G_ArchiveActorAntilagFrame{0x0, 0x140323D20};
WEAK symbol<void()> G_ArchiveVehicleAntilagFrame{0x0, 0x140324120};
WEAK symbol<void(const level::gentity_t *ent, weapon::weaponParms *wp,
                 int32_t shotCount)>
    G_CalcMuzzlePoints{0x0, 0x1403245F0};
WEAK symbol<void(level::gentity_t *ent, vec3_t *parentAxis)>
    G_CalcTagParentAxis{0x0, 0x14032A0A0};
WEAK symbol<void(viewClampState *clamp)> G_DefaultViewAngleClampGoal{
    0x0, 0x14032A230};
WEAK symbol<qboolean(level::gentity_t *ent, level::gentity_t *parent,
                     scr::ScrString_t tagName)>
    G_EntLinkTo{0x0, 0x14032A290};
WEAK symbol<void(level::gentity_t *ent)> G_GeneralLink{0x0, 0x14032A3C0};
WEAK symbol<void(level::gentity_t *ent)> G_InitPlayerLinkAngles{0x0,
                                                                0x14032A4C0};
WEAK symbol<qboolean(level::gentity_t *ent, const int32_t eAngles,
                     const contents_t contentMask, vec3_t *outTargetOrigin)>
    G_SetFixedLinkNonPlayer{0x0, 0x14032A630};
WEAK symbol<void(level::gentity_t *ent)> G_SetFixedLinkPlayer{0x0, 0x14032A730};
WEAK symbol<bool(level::gentity_t *ent)> G_ShouldClientLinkTo{0x0, 0x14032A950};
WEAK symbol<void(viewClampState *clamp)> G_SnapToViewAngleClampGoal{
    0x0, 0x14032A960};
WEAK symbol<void(level::gentity_t *from, level::gentity_t *to)>
    G_TransferLinkedEntities{0x0, 0x14032A9A0};
WEAK symbol<void(level::gentity_t *parent, qboolean bHasDObj)>
    G_UpdateTagInfoOfChildren{0x0, 0x14032AC00};
WEAK symbol<bool()> G_SVRunning{0x0, 0x140504B90};
WEAK symbol<void(mem::MemoryFile *memFile)> G_SaveDvars{0x0, 0x140578850};
WEAK symbol<void(level::gentity_t *ent)> G_PlayerGadget_ShutdownOnDeathInner{
    0x0, 0x140617EB0};
WEAK symbol<void()> G_UpdateActorCorpses{0x0, 0x140625520};
WEAK symbol<void()> G_FreePathnodesScriptInfo{0x0, 0x140637EC0};
WEAK symbol<void()> G_InitSentients{0x0, 0x1406410C0};
WEAK symbol<aligned_array_ref<uint8_t, 0x500000, 0x10>>
    G_PHYSICS_TOTAL_MEMORY_BUFFER{0x0, 0x14A7522C8};

namespace mz_unzip {
WEAK symbol<void(void *)> mz_unzip{0x0, 0x1404DE4A0};
WEAK symbol<void *(void *, const char *fileName, size_t *size)> ReadFileAlloc{
    0x0, 0x1404DE580};
WEAK symbol<bool(void *, const char *fileName)> OpenFile{0x0, 0x1404DE510};

} // namespace mz_unzip

} // namespace game