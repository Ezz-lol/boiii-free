#pragma once

#include <cstdint>
#include "game.hpp"
#include "structs/structs.hpp"

#ifdef _MSC_VER
#define WEAK __declspec(selectany)
#else
#define WEAK __attribute__((weak))
#endif

namespace game {
#define Com_Error(code, fmt, ...)                                              \
  Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

// CG
WEAK symbol<void(int localClientNum, float *fov_x,
                 float *dxDzAtDefaultAspectRatio, float *dxDz, float *dyDz)>
    CG_CalcFOVfromLens{0x1404D6230};

// CL
WEAK symbol<void(int controllerIndex, net::XSESSION_INFO *hostInfo,
                 const net::netadr_t *addr, int numPublicSlots,
                 int numPrivateSlots, const char *mapname, const char *gametype,
                 const char *somethingWithUserMaps)>
    CL_ConnectFromLobby{0x14134C570};
WEAK symbol<bool(int localClientNum, int index, char *buf, int size,
                 bool addClanName)>
    CL_GetClientName{0x1413E3140};
WEAK symbol<bool(int localClientNum)> CL_LocalClient_IsActive{0x14283AA50};
WEAK symbol<float(void *key)> CL_KeyState{0x1412FF860};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *pszMapName,
                 const char *pszGametype)>
    CL_SetupForNewServerMap{0x14135CD20};

// Game
WEAK symbol<void(level::gentity_s *ent, level::gentity_s *target, int mode,
                 const char *chatText)>
    G_Say{0x0, 0x140299170};
WEAK symbol<void(const char *fmt, ...)> G_LogPrintf{0x0, 0x1402A7BB0};

WEAK symbol<void(int32_t levelTime, int32_t randomSeed, game::qboolean restart,
                 qboolean registerDvars,

                 qboolean savegame)>
    G_InitGame{0x1419CA420, 0x1402ABB80};

WEAK symbol<level::gentity_s *()> G_Spawn{0x141B81420, 0x140308570};
WEAK symbol<void(level::gentity_s *ed)> G_FreeEntity{0x141B77950, 0x1403067A0};
WEAK symbol<void()> G_PrintEntities{0x141B7BE40, 0x140307800};

// Com
WEAK symbol<void()> Com_Init_Try_Block_Function{0x1421123B0, 0x140504170};
WEAK symbol<void(int channel, unsigned int label, const char *fmt, ...)>
    Com_Printf{0x142148F60, 0x140505630};
WEAK symbol<void(const char *file, int line, int code, const char *fmt, ...)>
    Com_Error_{0x1420F8170, 0x140501470};
WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7370};
WEAK symbol<int()> Com_SessionMode_GetMode{0x1420F6D30, 0x1405002D0};
WEAK symbol<int()> Com_SessionMode_GetGameMode{0x1420F68B0, 0x1404FFE50};
WEAK symbol<void(eNetworkModes networkMode)> Com_SessionMode_SetNetworkMode{
    0x1420F75B0, 0x140500B80};
WEAK symbol<eGameModes(eGameModes gameMode)> Com_SessionMode_SetGameMode{
    0x1420F7570, 0x140500B40};
WEAK symbol<eModes(eModes mode)> Com_SessionMode_SetMode{0x1420F7570};
WEAK symbol<void(const char *gametype, bool loadDefaultSettings)>
    Com_GametypeSettings_SetGametype{0x1420F5980};
WEAK symbol<unsigned int(const char *settingName, bool getDefault)>
    Com_GametypeSettings_GetUInt{0x1420F4E00, 0x1404FE5C0};
WEAK symbol<bool()> Com_IsRunningUILevel{0x142148350, 0x140504BD0};
WEAK symbol<bool()> Com_IsInGame{0x1421482C0, 0x140504B90};
WEAK symbol<void(int localClientNum, eModes fromMode, eModes toMode,
                 uint32_t flags)>
    Com_SwitchMode{0x14214A4D0};
WEAK symbol<const char *(const char *fullpath)> Com_LoadRawTextFile{
    0x1420F61B0};

WEAK symbol<void(int localClientNum, const char *text)> Cbuf_AddText{
    0x1420EC010, 0x1404F75B0};
WEAK symbol<void(int localClientNum, ControllerIndex_t controllerIndex,
                 const char *buffer)>
    Cbuf_ExecuteBuffer{0x14133BE10, 0x1404F78D0};
WEAK symbol<void(const char *cmdName, xcommand_t function,
                 cmd_function_s *allocedCmd)>
    Cmd_AddCommandInternal{0x1420ECC90, 0x1404F8210};
// Reused _a lot_ in the engine for feature-gated function calls.
// There are a few dozen of these.
WEAK symbol<void()> Stub{0x0, 0x1407DB4C0};
WEAK symbol<void(const char *cmdName, xcommand_t function,
                 cmd_function_s *allocedCmd)>
    Cmd_AddServerCommandInternal{0x0, 0x1404F8280};
WEAK symbol<void(int localClientNum, ControllerIndex_t controllerIndex,
                 const char *text, bool fromRemoteConsole)>
    Cmd_ExecuteSingleCommand{0x1420ED380, 0x1404F8890};
WEAK symbol<void(int localClientNum, ControllerIndex_t localControllerIndex,
                 const char *text_in, int max_tokens, bool evalExpressions,
                 CmdArgs *args)>
    Cmd_TokenizeStringKernel{0x1420EED60, 0x1404FA300};
WEAK symbol<void()> Cmd_EndTokenizedString{0x1420ECED0, 0x1404F8420};
WEAK symbol<void(char *text, int maxSize)> Con_GetTextCopy{0x14133A7D0,
                                                           0x140182C40};
namespace db {

WEAK symbol<bool(const char *zoneName, int source)> DB_FileExists{0x141420B40};

namespace xzone {

WEAK symbol<bool> g_zoneInited{0x0, 0x14690202C};
WEAK symbol<uint32_t> g_zoneCount{0x14941097C, 0x14699D21C};
WEAK symbol<uint32_t> g_zoneIndex{0x0, 0x1469BB268};
WEAK symbol<ZonePool<XZoneName>> g_zoneNames{0x14998FB80, 0x146E83270};
WEAK symbol<ZonePool<XZone>> g_zones{0x0, 0x146E84AD0};
WEAK symbol<char[4160]> g_zoneNameList{0x0, 0x146E876D0};
WEAK symbol<ZonePool<XZoneInfoInternal>> g_zoneInfo{0x0, 0x146E88770};
WEAK symbol<uint32_t> g_zoneInfoCount{0x0, 0x146E88714};

} // namespace xzone

namespace xasset {

// Asset pool (client only)
WEAK symbol<XAssetPool> DB_XAssetPool{0x1494093F0};

WEAK symbol<void(XAssetType type, XAssetEnum *func, void *inData,
                 bool includeOverride)>
    DB_EnumXAssets{0x141420970, 0x1401D5A50};
WEAK symbol<XAssetHeader(XAssetType type, const char *name, bool errorIfMissing,
                         int32_t waitTime)>
    DB_FindXAssetHeader{0x141420ED0, 0x1401D5FB0};
WEAK symbol<const char *(const XAsset *asset)> DB_GetXAssetName{0x1413E9DA0,
                                                                0x14019F080};
WEAK symbol<int(XAssetType type)> DB_GetXAssetTypeSize{0x1413E9DD0};

} // namespace xasset
namespace load {
WEAK symbol<DB_LoadData> g_load{0x14940C3B0, 0x1468FD4A0};
WEAK symbol<void(xzone::XZoneInfo *zoneInfo, uint32_t zoneCount, bool sync,
                 bool suppressSync)>
    DB_LoadXAssets{0x1414236A0, 0x1401D8740};

WEAK symbol<void(const char *path, DBFile f, xzone::XZoneBuffer *fileBuffer,
                 const char *filename, xasset::XAssetList *assetList,
                 XBlock *blocks, DB_Interrupt *interrupt, uint8_t *buf,
                 PMemStack side, int flags, int32_t desiredReadBytes)>
    DB_LoadXFile{0x1413EF6D0, 0x1401A4920};
WEAK symbol<void(int zoneIndex, bool createDefault, qboolean suppressSync)>
    DB_UnloadXZone{0x141425A70, 0x1401DA6C0};
WEAK symbol<void()> DB_ReleaseXAssets{0x1414247C0};
} // namespace load
} // namespace db

// G
WEAK symbol<void()> G_ClearVehicleInputs{0x1423812E0, 0x1405C1200};
WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, BoneIndex bone)>
    G_PlaySoundAlias{0x0, 0x140307480};
WEAK symbol<level::gentity_s *(const vec3_t *origin,
                               game::snd::SndAliasId alias)>
    G_PlaySoundAliasAtPoint{0x0, 0x1403075C0};
WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, uint32_t tag)>
    G_PlaySoundAliasWithNotify{0x0, 0x1403076E0};

WEAK symbol<void(scr::scriptInstance_t inst)> CScr_SetExposureActiveBank{
    0x0, 0x140092F80};
WEAK symbol<void(scr::scriptInstance_t inst)> CScr_SetLitFogBank{0x0,
                                                                 0x140093FE0};
WEAK symbol<void(scr::scriptInstance_t inst)> CScr_SetLutVolumeActiveBank{
    0x0, 0x140094120};
WEAK symbol<void(scr::scriptInstance_t inst)> CScr_SetPBGActiveBank{
    0x0, 0x140094360};
WEAK symbol<void(scr::scriptInstance_t inst)> CScr_SetWorldFogActiveBank{
    0x0, 0x140094960};
WEAK symbol<void(scr::scriptInstance_t inst, scr::scr_entref_t entref)>
    GScr_SetWorldFogActiveBank{0x0, 0x1402DF580};

// SND
namespace snd {

WEAK symbol<SndBankGlobals> g_sb{0x1580E0DAC, 0x14A8AAA80};
WEAK symbol<qboolean> g_sb_loadGate{0x0, 0x14A8E39E0};

WEAK symbol<bool(const char *filename, const snd::SndAssetBankHeader *header,
                 int64_t size, const uint8_t *checksum)>
    SND_AssetBankValidateHeader{0x0, 0x140649E70};
WEAK symbol<bool(const char *name, int aliasHash, snd::SndLimitType limitType,
                 int64_t limitCount, snd::SndEntHandle ent,
                 const vec3_t *useEnt, float priority)>
    SND_Limit{0x0, 0x140645A00};
WEAK symbol<bool(const snd::SndBank *bank, snd::SndAssetBankLoad *assetBank,
                 bool stream)>
    SND_HeaderCheck{0x0, 0x14064C610};
WEAK symbol<bool(const snd::SndBank *bank, snd::SndAssetBankLoad *assetBank)>
    SND_TocCheck{0x0, 0x14064CD80};
WEAK symbol<bool(const snd::SndBank *bank)> SNDL_TryRemoveBank{0x0,
                                                               0x14064A5A0};
WEAK symbol<bool()> G_SndEnabled{0x0, 0x140584DB0};
WEAK symbol<bool(int voiceIndex, const snd::SndAlias *alias)>
    SND_SetVoiceStartInfo{0x0, 0x140647570};
WEAK symbol<bool(snd::SndBankLoad *load, snd::SndAssetBankLoad *assetBank,
                 bool streamed)>
    SND_StartTocRead{0x0, 0x14064CBB0};
WEAK symbol<bool()> SND_BankUpdate{0x0, 0x14064BA30};
WEAK symbol<bool(snd::SndEntHandle handle, vec3_t *origin, vec3_t *velocity,
                 vec3_t *orientation)>
    SND_GetEntPosition{0x0, 0x140546500};
WEAK symbol<bool()> SND_GetProcessCommandFlag{0x0, 0x1405480B0};
WEAK symbol<bool()> SND_HasLoadingBanks{0x0, 0x14064C5E0};
WEAK symbol<bool(snd::SndPlaybackId playbackId,
                 snd::SndLengthNotifyData lengthNotifyData,
                 snd::SndLengthType id)>
    SND_AddLengthNotify{0x0, 0x140643680};
WEAK symbol<bool()> SND_ShouldInit{0x0, 0x140647FB0};

WEAK symbol<bool(uint32_t channel, const vec3_t *position, float *value)>
    SND_LosOcclusionCache{0x0, 0x140645C70};
WEAK symbol<char(int a1, int a2)> CG_ScriptSndLengthNotify{0x0, 0x14011F2D0};
WEAK symbol<float(bool fancy, int *cache, const vec3_t *listener,
                  const vec3_t *playback)>
    SND_LosOcclusionTrace2{0x0, 0x1406505F0};
WEAK symbol<float(const snd::SndAlias *alias, const vec3_t *org)>
    Snd_GetGlobalPriorityVolume{0x0, 0x1406499E0};
WEAK symbol<float(const snd::SndAlias *alias, float volume, float amplitude)>
    Snd_GetGlobalPriority{0x0, 0x140652260};
WEAK symbol<float(const snd::SndVoice *voice)> SND_GetBaseLevel{0x0,
                                                                0x140645120};
WEAK symbol<float()> SND_GetPriorityB{0x0, 0x14056A850};
WEAK symbol<int64_t()> CG_SndAutoSimReset{0x0, 0x1401481D0};
WEAK symbol<int64_t(float *a1, float *a2, float *a3, float a4, float *a5,
                    float *a6)>
    SND_WhizbyPath{0x0, 0x140651D80};
WEAK symbol<int64_t(int64_t a1)> SND_ResetEnt{0x0, 0x14064FB50};
WEAK symbol<int64_t(int64_t a1, uint32_t *a2)>
    snd_occlusionCallback_Implementation{0x0, 0x1406510E0};
WEAK symbol<int64_t(int a1, float *a2, int64_t a3, float *a4, float a5,
                    int64_t a6)>
    SND_MissileWhizby{0x0, 0x1406515A0};
WEAK symbol<int64_t(int a1, int64_t a2, int64_t a3)> SND_ContinueLoopingSound{
    0x0, 0x140643800};
WEAK symbol<int64_t(int a1)> SND_GetMusicState{0x0, 0x14064C060};
WEAK symbol<int64_t(jq::jqBatch *batch)> Snd_Occlusion{0x0, 0x140651080};
WEAK symbol<int64_t(snd::SndMusicAssetInstance *a1, int a2)> SND_MusicAssetStop{
    0x0, 0x140546DB0};
WEAK symbol<int64_t(snd::SndPlaybackId a1)> SND_IsPlaying{0x0, 0x140546670};
WEAK symbol<int64_t(uint64_t a1, uint64_t a2)> CG_SubtitleSndLengthNotify{
    0x0, 0x140120200};
WEAK symbol<int64_t(uint64_t a1)> SND_SetEntState{0x0, 0x140548DC0};
WEAK symbol<int64_t(uint32_t a1, uint32_t a2)> SND_IsValidContext{0x0,
                                                                  0x140B29490};
WEAK symbol<int(const vec3_t *origin, float *minDistanceSq)>
    SND_GetListenerIndexNearestToOrigin{0x0, 0x1406454A0};

WEAK symbol<int(snd::SndStringHash type)> SND_FindContextIndex{0x0,
                                                               0x14064CF20};
WEAK symbol<qboolean()> SND_ActiveListenerCount{0x0, 0x140643670};
WEAK symbol<snd::SndMenuCategory(uint32_t group)> SND_GroupCategory{
    0x0, 0x14064D4E0};
WEAK symbol<snd::SndPlaybackId(
    const snd::SndAliasList *aliasList, const snd::SndPlayState *state,
    snd::SndPlayback *playback, snd::SndOcclusionStartCache *ocache,
    int playCount)>
    SND_PlayList{0x0, 0x140646790};
WEAK symbol<snd::SndPlaybackId(const snd::SndPlayState *state,
                               snd::SndPlayback *playback)>
    SNDL_Play{0x0, 0x14064DB90};
WEAK symbol<snd::SndPlaybackId(LocalClientNum_t localClientNum, int entitynum,
                               const vec3_t *origin, int fadeMs, bool doNotify,
                               float volume, snd::SndAliasId id)>
    CG_PlaySoundWithHandle{0x0, 0x14011EEF0};
WEAK symbol<snd::SndStringHash(snd::SndStringHash type)> SND_GetCurrentContext{
    0x0, 0x140645260};
WEAK symbol<snd::SndVoice *(int playbackId)> SND_GetPlaybackVoice{0x0,
                                                                  0x140645740};
WEAK symbol<uint64_t()> SND_LosOcclusionUpdate{0x0, 0x140643D40};
WEAK symbol<void(bool atStreamStart)> Load_SndBank{0x0, 0x1401BBBE0};
WEAK
    symbol<void(bool isMature, bool isPaused, float timescale, uint32_t cg_time,
                uint32_t seed, float voiceScale, float musicScale,
                float sfxScale, float masterScale, float cinematicScale,
                int masterPatch, bool hearingImpaired, snd::SndGameMode mode)>
        SNDL_SetGameState{0x0, 0x14064E430};
WEAK symbol<void(bool stream, const char *zone, const char *language,
                 int pathBufferLength, char *path, qboolean isPatch,
                 io::stream_id streamRequestId)>
    SND_GetRuntimeAssetBankFileName{0x0, 0x14064C470};
WEAK symbol<void(bool streamed, snd::SndAssetBankLoad *assetBank,
                 snd::SndBankLoad *load)>
    SND_StartHeaderRead{0x0, 0x14064C720};
WEAK symbol<void()> CG_SndFireReset{0x0, 0x14011F720};
WEAK symbol<void()> CG_SndGameReset{0x0, 0x14011F870};
WEAK symbol<void()> CL_Snd_Restart_f{0x0, 0x140191B30};
WEAK symbol<void(const char *a1, const char *a2)> SND_SetContext{0x0,
                                                                 0x140548C00};
WEAK symbol<void(const char *a1, int a2)> SND_SetGlobalFutz{0x0, 0x140548EC0};
WEAK symbol<void(const char *a1)> SND_ForceAmbientRoom{0x0, 0x140547F20};
WEAK symbol<void(const char *a1)> SND_SetMusicState_0{0x0, 0x140549180};
WEAK symbol<void(const char *a1)> SND_SetShockAmbientRoom{0x0, 0x1405493B0};
WEAK symbol<void(const char *alias, int fadeTimeMs, float attenuation,
                 snd::SndEntHandle entHandle, const vec3_t *position,
                 const vec3_t *direction, bool notify, double a8, bool a9)>
    SND_PlaySoundAlias{0x0, 0x140646490};
WEAK symbol<void(const char *zoneName)> SNDL_BankUpdateZone{0x0, 0x14064D500};
WEAK symbol<void(const snd::SndBank *bank, int priority, bool patchZone,
                 int unknown)>
    SND_AddBank{0x0, 0x1405479E0};
WEAK symbol<void(const snd::SndBank *bank, int priority, bool patchZone)>
    SNDL_AddBank{0x0, 0x14064A030};
WEAK symbol<void(const snd::SndBank *bank)> SNDL_RemoveBank{0x0, 0x14064A3A0};
WEAK symbol<void(const snd::SndBank *bank)> SND_RemoveBank{0x0, 0x140548B00};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SND_AddGlobals{
    0x0, 0x140547A80};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SNDL_AddGlobals{
    0x0, 0x1406432B0};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SNDL_RemoveGlobals{
    0x0, 0x1406432C0};
WEAK symbol<void(const snd::SndPatch *patch)> SND_AddPatch{0x0, 0x140547AE0};
WEAK symbol<void(const snd::SndPatch *patch)> SNDL_AddPatch{0x0, 0x1405472C0};
WEAK symbol<void(const snd::SndPatch *patch)> SNDL_RemovePatch{0x0,
                                                               0x140547550};
WEAK symbol<void(const snd::SndPatch *patch)> SND_PatchApply{0x0, 0x140547680};
WEAK symbol<void(const snd::SndPatch *patch)> SND_RemovePatch{0x0, 0x140548B60};
WEAK symbol<void(const snd::SndVoice *voice, float *wet, float *dry)>
    SND_GetLevels{0x0, 0x1406511C0};
WEAK symbol<void(const vec3_t *P, const vec3_t *segmentA,
                 const vec3_t *segmentB, vec3_t *nearPoint)>
    SND_GetNearestPointOnSegment{0x0, 0x140651310};
WEAK symbol<void(float a1)> SND_SetScriptTimescale{0x0, 0x140549350};
WEAK symbol<void(float fadetime, bool setScriptValues, float scriptPitch,
                 float scriptPitchRate, float scriptAttenuation,
                 float scriptAttenuationRate, snd::SndVoice *voice)>
    SND_SetVoiceStartFades{0x0, 0x140646F00};
WEAK symbol<void(float *priority, int *channel, uint32_t start, uint32_t count)>
    Snd_GetLowestPriority{0x0, 0x140649B20};
WEAK symbol<void(float value)> SNDL_SetScriptTimescale{0x0, 0x14064ECB0};
WEAK symbol<void(level::gentity_s *ent, snd::SndAliasId index,
                 scr::ScrString_t notifyString)>
    G_RegisterSoundWait{0x0, 0x140308090};
WEAK symbol<void(SndEntHandle handle, const char *type, const char *value)>
    SND_SetEntContext{0x0, 0x140548D20};
WEAK symbol<void(SndEntHandle sndEnt, const vec3_t *shotPosition,
                 const vec3_t *shotDirection, const vec3_t *position,
                 const vec3_t *center, const char *whizbySound,
                 const bool isUnderwater)>
    SND_Whizby{0x0, 0x140651C10};
WEAK symbol<void(SndEntHandle ent, SndAliasId alias, float attenuation,
                 float attenuationRate, float pitch, float pitchRate)>
    SND_SetLoopState{0x0, 0x140549070};
WEAK symbol<void(SndEntHandle handle, LocalClientNum_t listener,
                 ClientNum_t clientNum, team_t team, const vec3_t *origin,
                 const vec3_t *axis)>
    SND_SetListener{0x0, 0x140548F30};
WEAK symbol<void(SndEntHandle ent, SndStringHash alias_name)>
    SND_StopSoundAliasOnEnt{0x0, 0x1405495E0};
WEAK symbol<void(const char *subtitle, unsigned int lengthMs)>
    SND_SubtitleNotify{0x0, 0x140549710};
WEAK symbol<void(SndPlayback *p)> SND_FreePlayback{0x0, 0x140546490};

WEAK symbol<void(SndQueue *queue)> SND_QueueFlush{0x0, 0x140549C20};
WEAK symbol<void(SndEntHandle ent)> SND_StopSoundsOnEnt{0x0, 0x1405496B0};
WEAK symbol<void(const SndAmbientBsp **bsps, int *numBsps)>
    SND_BankGetAmbientBsps{0x0, 0x14064B320};
WEAK symbol<void(SndVoice *voice, const vec3_t *startPosition)>
    SND_UpdateVoicePosition{0x0, 0x140649300};
WEAK symbol<void(SndDuckCategoryType type, const char *name, int64_t length,
                 float amount)>
    SND_SetDuck{0x0, 0x140548C70};
WEAK symbol<void(int count, const SndEntLoop *loops)> SND_PlayLoops{
    0x0, 0x140548790};
WEAK symbol<void(int32_t id, float attenuation)> SND_SetPlaybackAttenuation{
    0x0, 0x1405491D0};
WEAK symbol<void(int32_t id, float attenuationRate)>
    SND_SetPlaybackAttenuationRate{0x0, 0x140549230};
WEAK symbol<void(int32_t id, float pitch)> SND_SetPlaybackPitch{0x0,
                                                                0x140549290};
WEAK symbol<void(int32_t id, float pitchRate)> SND_SetPlaybackPitchRate{
    0x0, 0x1405492F0};

WEAK symbol<void(snd::SndAliasId id, const vec3_t *origin)> SNDL_PlayLoopAt{
    0x0, 0x14064DDE0};
WEAK symbol<void(snd::SndAliasId alias, vec3_t *origin)> SNDL_RattleSetup{
    0x0, 0x14064E240};
WEAK symbol<void(snd::SndAliasId id, const vec3_t *origin0,
                 const vec3_t *origin1)>
    SNDL_PlayLineAt{0x0, 0x14064DC80};
WEAK symbol<void(snd::SndAliasId id, const vec3_t *origin0,
                 const vec3_t *origin1)>
    SNDL_StopLineAt{0x0, 0x14064ED10};
WEAK symbol<void(snd::SndAliasId id, const vec3_t *origin)> SNDL_StopLoopAt{
    0x0, 0x14064EEC0};
WEAK symbol<void(snd::SndAliasId id, const vec3_t *previousOrigin0,
                 const vec3_t *previousOrigin1, const vec3_t *origin0,
                 const vec3_t *origin1)>
    SNDL_UpdateLineAt{0x0, 0x14064F420};
WEAK symbol<void()> SND_AmbientReset{0x0, 0x1405452A0};
WEAK symbol<void()> SND_AmbientUpdate{0x0, 0x1405454A0};
WEAK symbol<void(snd::SndBank *bank, io::stream_fileid fileHandle,
                 int64_t offset, size_t size, void *data)>
    SND_StreamRead{0x0, 0x14064CC50};
WEAK symbol<void()> SND_BankLoadedNotify{0x0, 0x140547BE0};
WEAK symbol<void(snd::SndBankLoad *load, snd::SndAssetBankLoad *assetBank)>
    SND_EnqueueAssetLoads{0x0, 0x14064C900};
WEAK symbol<void(snd::SndBankLoad *load)> SND_BankLoadError{0x0, 0x14064B460};
WEAK symbol<void(snd::SndBankLoad *load)> SND_BankLoadUpdateState{0x0,
                                                                  0x14064B550};
WEAK symbol<void(snd::SndBankPtr *sound)> Load_SndBankAsset{0x0, 0x1401DB2D0};
WEAK symbol<void()> SND_CheckpointRestore{0x0, 0x140547D60};
WEAK symbol<void(snd::SndCommandBuffer *buffer)> SND_CommandCG{0x0,
                                                               0x140545B60};
WEAK symbol<void(snd::SndCommandType command, int64_t cmdSize, void *cmdData,
                 double _unk1, float _unk2, double _unk3, double _unk4,
                 float _unk5)>
    SND_CommandSND{0x0, 0x140545CB0};
WEAK symbol<void()> SND_DebugFini{0x0, 0x14064CE90};
WEAK symbol<void(snd::SndDuckActive *duck)> SND_StopDuck{0x0, 0x1406480D0};
WEAK symbol<void(snd::SndDuckCategoryType category, snd::SndStringHash duckId,
                 float amount)>
    SNDL_SetDuck{0x0, 0x14064E390};
WEAK symbol<void(snd::SndDuckCategoryType category, uint32_t duckId,
                 const snd::SndDuck *duck, float amount)>
    SND_SetDuckByCategory{0x0, 0x140646A50};
WEAK symbol<void()> SND_DuckReset{0x0, 0x140643C20};
WEAK symbol<void()> SND_EndFrame{0x0, 0x140547DF0};
WEAK symbol<void(snd::SndEntHandle entHandle, LocalClientNum_t localClientNum,
                 ClientNum_t clientNum, ControllerIndex_t controllerNum,
                 team_t team, const vec3_t *origin, const vec3_t *inAxis)>
    SNDL_SetListener{0x0, 0x14064E5D0};
WEAK symbol<void(snd::SndEntHandle handle, const vec3_t *origin,
                 const vec3_t *velocity, const vec3_t *orientation)>
    SNDL_SetEntState{0x0, 0x1405459E0};
WEAK symbol<void(snd::SndEntHandle handle, scr::ScrString_t animation)>
    SND_FacialAnimationNotify{0x0, 0x140547EA0};
WEAK symbol<void(snd::SndEntHandle handle)> SND_EntStateRequest{0x0,
                                                                0x140547E40};
WEAK symbol<void(snd::SndEntHandle handle, snd::SndStringHash type,
                 snd::SndStringHash value)>
    SNDL_SetEntContext{0x0, 0x14064E3E0};
WEAK symbol<void(snd::SndEntHandle sndEnt)> SNDL_StopSoundsOnEnt{0x0,
                                                                 0x14064F360};
WEAK symbol<void(snd::SndEntHandle sndEnt, snd::SndStringHash name)>
    SNDL_StopSoundAliasOnEnt{0x0, 0x14064F0B0};
WEAK symbol<void(snd::SndEntHandle sndEnt, snd::SndStringHash name)>
    StopSoundAliasesOnEnt{0x0, 0x14064FE30};
WEAK symbol<void(snd::SndEntHandle sndEnt, uint32_t alias, float attenuation,
                 float attenuationRate, float pitch, float pitchRate)>
    SNDL_SetLoopState{0x0, 0x14064E900};
WEAK symbol<void()> SND_EntStateFrame{0x0, 0x140546200};
WEAK symbol<void()> SND_ErrorIfSoundGlobalsTrashed{0x0, 0x140644DE0};
WEAK symbol<void()> SND_Frame{0x0, 0x140547F80};
WEAK symbol<void()> SND_GameReset{0x0, 0x140548060};
WEAK symbol<void()> SND_InitDvar{0x0, 0x14064CEA0};
WEAK symbol<void()> SNDL_ApplyPatches{0x0, 0x140547400};
WEAK symbol<void()> SNDL_Checkpoint{0x0, 0x14064D770};
WEAK symbol<void()> SNDL_GameReset{0x0, 0x14064D880};
WEAK symbol<void()> SND_ListenerDiscontinuity{0x0, 0x14064FD90};

WEAK symbol<void()> SND_LosOcclusionFini{0x0, 0x140650220};
WEAK symbol<void()> SND_LosOcclusionSync{0x0, 0x140650300};
WEAK symbol<void()> SNDL_ResetAliases{0x0, 0x14064A3D0};
WEAK symbol<void()> SNDL_ResetEntState{0x0, 0x140546840};
WEAK symbol<void()> SNDL_Shutdown{0x0, 0x1406432D0};
WEAK symbol<void()> SNDL_Update{0x0, 0x140643340};
WEAK symbol<void()> SNDL_UpdateLoopingSounds{0x0, 0x14064F560};
WEAK symbol<void()> SNDL_UpdateStaticSounds{0x0, 0x14064F690};
WEAK symbol<void()> SND_MusicUpdate{0x0, 0x140650650};
WEAK symbol<void(snd::SndPlayback *playback)> SND_FreePlaybackNotify{
    0x0, 0x140548010};
WEAK symbol<void()> SND_ProcessCLQueue{0x0, 0x1405466B0};
WEAK symbol<void()> SND_ProcessSNDQueue{0x0, 0x140546720};
WEAK symbol<void(snd::SndQueue *queue, snd::SndCommandType cmd, int size,
                 const void *data)>
    SND_QueueAdd{0x0, 0x140549A20};
WEAK symbol<void()> SND_RestartDriver{0x0, 0x140548BC0};
WEAK symbol<void(snd::SndSpeakerMap *map)> Snd_SpeakerMapZero{0x0, 0x140652D60};
WEAK symbol<void(snd::SndStopSoundFlags which)> SNDL_StopSounds{0x0,
                                                                0x14064F0C0};
WEAK symbol<void(SndStringHash type, SndStringHash valu)> SNDL_SetContext{
    0x0, 0x14064E360};
WEAK symbol<void(snd::SndStringHash id, snd::SndGfutzLocation callLocation)>
    SNDL_SetGlobalFutz{0x0, 0x14064E5C0};
WEAK symbol<void(snd::SndStringHash id)> SNDL_SetMusicState{0x0, 0x1405468C0};
WEAK symbol<void(snd::SndStringHash room)> SND_AmbientShockRoom{0x0,
                                                                0x140545490};
WEAK symbol<void()> SND_UpdateVoice_0{0x0, 0x1406496E0};
WEAK symbol<void()> SND_UpdateWait{0x0, 0x1405498A0};
WEAK symbol<void(SndVoice *voice, float dt)> SND_UpdateVoice{0x0, 0x1406484C0};
WEAK symbol<void(snd::SndVoice *voice, const snd::SndAlias *alias)>
    SND_SetVoiceStartSeeds{0x0, 0x140645780};
WEAK symbol<void(snd::SndVoice *voice, vec3_t *player)> SND_SetVoiceStartFlux{
    0x0, 0x140647280};
WEAK symbol<void(io::stream_id id, io::stream_status result,
                 uint32_t numBytesRead, uint8_t *b)>
    SND_BankReadCallback{0x0, 0x14064B970};
WEAK symbol<void(const SndPlayState *state, SndPlayback *playback)>
    SND_PlayInternal{0x0, 0x140548470};
WEAK symbol<void(const SndPlayState *state)> SND_Play2{0x0, 0x1405482D0};
WEAK symbol<void(bool isMature, bool isPaused, float timescale,
                 unsigned int cgTime, unsigned int seed, float voiceScale,
                 float musicScale, float sfxScale, float masterScale,
                 float cinematicScale, int masterPatch, bool hearingImpaired,
                 SndGameMode mode, uint64_t unk1, uint64_t _unk2,
                 uint64_t unk3)> // 3 new SndCommandSetGameState fields
    SND_BeginFrame{0x0, 0x140547C20};
WEAK symbol<void(snd_weapon_shot *shot)> CG_SndWeaponFire{0x0, 0x140148F50};

WEAK symbol<void(uint32_t table, char *asset, uint32_t fieldIndex,
                 uint32_t value)>
    SND_PatchValue{0x0, 0x1405478D0};
WEAK symbol<void(uint32_t table, snd::SndDuck *duck, uint32_t fieldIndex,
                 snd::SndStringHash groupId, uint32_t value)>
    SND_PatchDuckValue{0x0, 0x140547810};
WEAK symbol<void(vec3_t *origin, int minDist, int maxDist)> SNDL_Rattle{
    0x0, 0x14064E000};

WEAK symbol<void(const char *alias, int fadeTimeMs, float attenuation,
                 SndEntHandle entHandle, const vec3_t *position,
                 const vec3_t *direction, bool notify)>
    SND_Play{0x0, 0x140548140};
WEAK symbol<void(SndAliasId aliasId, int fadeTimeMs, float attenuation,
                 SndEntHandle entHandle, const vec3_t *position,
                 const vec3_t *direction, bool notify, float scriptPitch,
                 float scriptPitchRate, float scriptAttenuation,
                 float scriptAttenuationRate, bool skipEntStart)>
    SND_PlayBundle{0x0, 0x140548310};
WEAK symbol<void(const char *roomName, const int entnum)>
    SND_AmbientStateNotify{0x0, 0x140547B40};
WEAK symbol<void(unsigned int ent, unsigned int lengthMs)> SND_LengthNotify{
    0x0, 0x1405480C0};
WEAK symbol<void(LocalClientNum_t listener)> SND_DisconnectListener{
    0x0, 0x140547DA0};
WEAK symbol<void(SndStringHash stateid, const char *stateName)>
    SND_SetMusicState{0x0, 0x140549130};
WEAK symbol<void(SndStopSoundFlags flags)> SND_StopSounds{0x0, 0x140549660};
WEAK symbol<void(int a1, uint32_t *a2)> SND_PlayLoopAt{0x0, 0x1405486F0};
WEAK symbol<void(SndAliasId id, const vec3_t *origin)> SND_StopLoopAt{
    0x0, 0x1405494F0};
WEAK symbol<void(SndAliasId id, const vec3_t *origin0, const vec3_t *origin1)>
    SND_PlayLineAt{0x0, 0x140548620};
WEAK symbol<void(SndAliasId id, const vec3_t *origin0, const vec3_t *origin1)>
    SND_StopLineAt{0x0, 0x140549420};
WEAK symbol<void(SndAliasId id, const vec3_t *previousOrigin0,
                 const vec3_t *previousOrigin1, const vec3_t *origin0,
                 const vec3_t *origin1)>
    SND_UpdateLineAt{0x0, 0x140549780};
WEAK symbol<void(int count, const snd::SndEntLoop *loops)> SNDL_PlayLoops{
    0x0, 0x14064DEA0};
WEAK symbol<void(int id)> SND_ContinueLoopingPlayback{0x0, 0x1406437A0};
WEAK symbol<void(int index)> SND_ResetVoiceInfo{0x0, 0x140646910};
WEAK symbol<void(int minUpdateMs)> SND_PossiblyUpdate{0x0, 0x140548990};
WEAK symbol<void(int playbackId, bool pause)> SNDL_SetStartPaused{0x0,
                                                                  0x14064ECC0};
WEAK symbol<void(int playbackId, float attenuation)>
    SNDL_SetPlaybackAttenuation{0x0, 0x14064EB30};
WEAK symbol<void(int playbackId, float pitch)> SNDL_SetPlaybackPitch{
    0x0, 0x14064EBF0};
WEAK symbol<void(int playbackId, float rate)> SNDL_SetPlaybackAttenuationRate{
    0x0, 0x14064EBC0};
WEAK symbol<void(int playbackId, float rate)> SNDL_SetPlaybackPitchRate{
    0x0, 0x14064EC80};
WEAK symbol<void(int playbackId)> SNDL_StopPlayback{0x0, 0x14064F050};
WEAK symbol<void(int playbackId)> SND_StopPlayback{0x0, 0x140549590};
WEAK symbol<void(int voiceindex)> SND_Stop{0x0, 0x140647FC0};
WEAK symbol<void(int voiceIndex)> SND_StopVoice{0x0, 0x140649C60};
WEAK symbol<void(int voiceIndex)> SND_StopVoicePFutz{0x0, 0x140648190};
WEAK symbol<void(LocalClientNum_t localClientNum)> SNDL_DisconnectListener{
    0x0, 0x14064D7D0};
WEAK symbol<void(qboolean force)> SNDL_ForceAmbientRoom{0x0, 0x140545290};

WEAK symbol<void()> SND_LoadSoundsWait{0x14258C190, 0x14064C6E0};
// Does not exist on server
WEAK symbol<void()> SND_InitAfterGlobals{0x142584FE0};
// Internally, just returning `SND_HashName` if `SND_Active`
WEAK symbol<SndAliasId(const char *aliasName)> SND_FindAliasId{0x14258B860,
                                                               0x14064BD90};
WEAK symbol<int32_t(const char *name)> SND_GetPlaybackTime{
    0x14258BCD0,
    0x14064C220}; // Gscr - get length of sound by name, in milliseconds
WEAK symbol<SndStringHash(const char *name)> SND_HashName{0x142595ED0,
                                                          0x140651520};
WEAK symbol<bool(game::snd::SndPlaybackId playbackId, int32_t *msec)>
    SND_GetKnownLength{0x142271340, 0x1405465C0};
/*
 Used only in cscr - get _current time_ in playback of sound by playbackId, in
 milliseconds.

 Note that this is not the same as the length of the sound, and
 will be less than or equal to the value returned by SND_GetPlaybackTime for
 the same sound, or -1 if playback or sound not found.

 Interestingly, when you call `soundgetplaybacktime` in CSC scripts, this
 function is used.

 When the same function - `soundgetplaybacktime` - is called in GSC scripts, the
 above SND_GetPlaybackTime function is used.

 Simply: in CSC scripts, `soundgetplaybacktime` returns current playback
 progress, and in GSC scripts, `soundgetplaybacktime` returns total length of
 the sound.
*/
WEAK symbol<int32_t(game::snd::SndPlaybackId playbackId)> SND_GetPlaybackTime2{
    0x142271390,
    0x140546620}; // Name in engine is also SND_GetPlaybackTime - override.

/* Called by the other SND_AssetBankFindEntry.

   Name in engine is also `SND_AssetBankFindEntry`, overriding the other
   SND_AssetBankEntry with a different signature.

   Overrides will not work when defining symbols here, to my knowledge, so
   providing it with a more specific name based on usage.
*/
WEAK symbol<bool(game::snd::SndStringHash id,
                 game::snd::SndAssetBankEntry *entries, uint32_t entryCount,
                 game::snd::SndAssetBankEntry **entry)>
    SND_AssetBankFindEntryIn{0x1425897A0, 0x140649D40};
WEAK symbol<uint32_t(const SndAssetBankEntry *entry)> SND_AssetBankGetFrameRate{
    0x142589810, 0x140649DA0};
WEAK symbol<uint32_t(const SndAssetBankEntry *entry)> SND_AssetBankGetLengthMs{
    0x142589890, 0x140649E20};
WEAK symbol<bool(game::snd::SndStringHash id,
                 game::snd::SndAssetBankEntry **entry, io::stream_fileid *fid,
                 bool streamed)>
    SND_AssetBankFindEntry{0x14258A6F0, 0x14064AC10};
WEAK symbol<bool(game::snd::SndStringHash id,
                 game::snd::SndAssetBankEntry **entry, void **data)>
    SND_AssetBankFindLoaded{0x14258A8C0, 0x14064ADF0};
WEAK symbol<SndAliasList *(game::snd::SndStringHash key)> SND_BankAliasLookup{
    0x14258AA70, 0x14064AFA0};

/*
   Client has two equivalent functions for this. The main, used-as-usual
   SND_Active at 0x142272820, and an exact copy at 0x1422F4880, used only for:
   1. `SND_ShouldInit`'s return value - return true if sound not yet initialised
   2. An inlined `SND_ShouldInit` call in `SND_InitAfterGlobals`
*/
WEAK symbol<qboolean()> SND_Active{0x142272820, 0x1405479D0};

/*
 Very heavily obfuscated by arxan on client. Control flow is hard to follow,
 but this is indeed SND_Init
*/
WEAK symbol<bool()> SND_Init{0x142584E20, 0x1406459B0};
/*
   `return g_pc_nosnd != 0;`
   Most likely does not exist on client.
   The `nosnd` CLI arg handling seems to, in general,
   be server-specific in older CoD engines.
*/
WEAK symbol<bool()> G_SNDEnabled{0x0, 0x140584DB0};

// snd globals
WEAK symbol<volatile int32_t> snd_update_fence{0x1579072B0, 0x14A2CEF30};
WEAK symbol<volatile int32_t> snd_update_start{0x1579072B4, 0x14A2CEF34};

// g_snd globals
WEAK symbol<SndLocal> g_snd{0x14347EE00, 0x141189800};
// one snd_fire_manager per local client, each with 8 fire slots
WEAK symbol<matrix2d<snd_fire_manager, 2, 8>> cl_g_snd_fire{0x144D2A13C, 0x0};
WEAK symbol<matrix2d<snd_fire_manager, 1, 8>> sv_g_snd_fires{0x0, 0x14223B840};
// Total size 0x1600
WEAK symbol<std::array<snd_autosim, 64>> g_snd_autosims{0x144D3FC20,
                                                        0x142243B80};
// Total size 0x1200
WEAK symbol<std::array<snd_autosim_play, 64>> g_snd_autosim_history{
    0x144D3FC20, 0x142242980};
WEAK symbol<unsigned int> g_snd_autosim_time{0x144D4B640, 0x14224E040};
WEAK symbol<unsigned int> g_snd_autosim_frame{0x144D42AEC, 0x1422454EC};
/*
   True if "nosnd" given as CLI arg.
   Probably does not exist on client, and is not seen used
   anywhere where it is used in dedicated server.
   See note above G_SNDEnabled.
*/
WEAK symbol<qboolean> g_pc_nosnd{0x0, 0x14A63D4EC};
} // namespace snd

// misc globals
WEAK symbol<clientplatform_t> clientplatform{0x0, 0x14A63D4E8};
WEAK symbol<qboolean(void *ent)> StuckInClient{0x1415A8360, 0x14023BFE0};

// Live
WEAK symbol<bool(uint64_t, int *, bool)> Live_GetConnectivityInformation{
    0x141E0C380};

// LiveStats
WEAK symbol<const char *(int controllerIndex)> LiveStats_GetClanTagText{
    0x141E9CE20};

// Info
WEAK symbol<const char *(const char *, const char *key)> Info_ValueForKey{
    0x1422E87B0};
WEAK symbol<void(char *s, const char *key, const char *value)>
    Info_SetValueForKey{0x1422E8410, 0x1405802D0};

namespace net {

// MSG
WEAK symbol<uint8_t(msg_t *msg)> MSG_ReadByte{0x142155450, 0x14050D1B0};

// NET
WEAK symbol<bool(netsrc_t sock, netadr_t *adr, const void *data, int len)>
    NET_OutOfBandData{0x142173600};
WEAK symbol<bool(netsrc_t sock, int length, const void *data,
                 const netadr_t *to)>
    NET_SendPacket{0x1423323B0, 0x140596E40};
WEAK symbol<bool(const char *, netadr_t *)> NET_StringToAdr{0x142172780,
                                                            0x140515110};
} // namespace net

// Sys
WEAK symbol<int()> Sys_Milliseconds{0x142332870, 0x1405972F0};
WEAK symbol<void()> Sys_ShowConsole{0x1423333C0, 0x140597E40};
WEAK symbol<TLSData *()> Sys_GetTLS{0x1421837B0, 0x140525EB0};
WEAK symbol<TLSData *()> Sys_IsDatabaseReady{0x142183A60};

// Rope
WEAK symbol<void()> Rope_InitRopes{0x1420D8D00, 0x1404E4300};

// Glass
WEAK symbol<void()> GlassSv_Shutdown{0x1425AA7A0, 0x14065BCC0};

// Mantle
WEAK symbol<void()> Mantle_ShutdownAnims{0x142678C80, 0x1406A1B50};

// PMem
WEAK symbol<void(const char *name, int allocDir)> PMem_Free{0x141439190,
                                                            0x1405D8B30};

// Unnamed
WEAK symbol<const char *(const char *name)> CopyString{0x1422AC220,
                                                       0x14056BD70};

WEAK symbol<bool()> isModLoaded{0x1420D5020};
WEAK symbol<const char *()> getPublisherIdFromLoadedMod{0x1420D7680,
                                                        0x1404E3230};
WEAK symbol<void(int localClientNum, const char *mod, bool reloadFS)> loadMod{
    0x1420D6930};
WEAK symbol<bool(int localClientNum)> isModLoading{0x1420D6DE0};
WEAK symbol<char> usermap_publisher_id{0x1567D9A24};
WEAK symbol<char> usermap_title{0x1567D99A0};
// This should correspond to folder name, with value something like
// cp_* for a campaign map, zm_* for zombies map, or mp_* for multiplayer map
WEAK symbol<char> internal_usermap_id{0x1567D9A04};
WEAK symbol<void()> reloadUserContent{0x1420D66C0, 0x1404E25C0};

// Dvar
// Used for e.g. changing in-game client FOV when `cg_fov` is changed,
// queueing mod load when `fs_game` is changed.
WEAK symbol<void(const dvar_t *dvar, modifiedCallback callback)>
    Dvar_SetModifiedCallback{0x1422C8650, 0x140579990};
WEAK symbol<void(const dvar_t *dvar, int flags)> Dvar_AddFlags{0x1422B8260,
                                                               0x140574880};
WEAK symbol<bool(const dvar_t *dvar)> Dvar_IsSessionModeBaseDvar{0x1422C23A0,
                                                                 0x140576890};
WEAK symbol<dvar_t *(const char *dvarName)> Dvar_FindVar{0x1422BCCD0,
                                                         0x140575540};
WEAK symbol<unsigned int(const char *str)> Dvar_GenerateHash{0x14133DBF0,
                                                             0x140185800};
WEAK symbol<dvar_t *(unsigned int hash)> Dvar_FindMalleableVar{0x1422BCC40,
                                                               0x1405754B0};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_GetDebugName{0x1422BD250};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_DisplayableValue{
    0x1422BC080};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     int flags, const char *description)>
    Dvar_RegisterBool{0x1422D0900, 0x14057B500};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, int value,
                     int min, int max, unsigned int flags,
                     const char *description)>
    Dvar_RegisterInt{0x1422D0AE0, 0x14057B7B0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, float value,
                     float min, float max, unsigned int flags,
                     const char *description)>
    Dvar_RegisterFloat{0x1422D0A60, 0x14057B6B0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName, bool value,
                     int flags, const char *description)>
    Dvar_SessionModeRegisterBool{0x1422D0D40, 0x14057BAA0};
WEAK symbol<void(dvarStrHash_t hash, bool value, eModes mode)>
    Dvar_SessionModeSetDefaultBool{0x1422D0E90, 0x14057BCE0};
WEAK symbol<dvar_t *(dvarStrHash_t hash, const char *dvarName,
                     const char *value, int flags, const char *description)>
    Dvar_RegisterString{0x1422D0B70, 0x14057B890};
WEAK symbol<void(void (*callback)(const dvar_t *, void *), void *userData)>
    Dvar_ForEach{0x1422BCD00};
WEAK
    symbol<void(const char *dvarName, const char *string, bool createIfMissing)>
        Dvar_SetFromStringByName{0x1422C7500, 0x140579290};
WEAK symbol<dvar_t *(dvar_t *dvar, eModes mode)>
    Dvar_GetSessionModeSpecificDvar{0x1422BF500, 0x140575D90};

WEAK symbol<bool()> Dvar_GetBool{0x1422BCED0, 0x140575740};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainEnumStringCount{
    0x1422BD340, 0x140575900};
WEAK symbol<float(const dvar_t *dvar)> Dvar_GetDomainFloatMax{0x1422BD3C0,
                                                              0x140575940};
WEAK symbol<float(const dvar_t *dvar)> Dvar_GetDomainFloatMin{0x1422BD400,
                                                              0x140575980};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetDomainInt64Max{0x1422BD440,
                                                                0x140575A90};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetDomainInt64Min{0x1422BD480,
                                                                0x1405759C0};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainIntMax{0x1422BD4C0,
                                                              0x140575A00};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetDomainIntMin{0x1422BD510,
                                                              0x140575A50};

WEAK symbol<float(const dvar_t *dvar)> Dvar_GetFloat{0x1422BD620, 0x140575B20};
WEAK symbol<int64_t(const dvar_t *dvar)> Dvar_GetInt64{0x1422BDC50,
                                                       0x140575B90};
WEAK symbol<int32_t(const dvar_t *dvar)> Dvar_GetInt{0x1422BE860, 0x140575C20};
WEAK symbol<dvarType_t(const dvar_t *dvar)> Dvar_GetType{0x1422BF5E0,
                                                         0x140575E90};
WEAK symbol<uint64_t(const dvar_t *dvar)> Dvar_GetUInt64{0x1422BF620,
                                                         0x140575ED0};
WEAK symbol<const char *(const dvar_t *dvar)> Dvar_GetString{0x1422BF590,
                                                             0x140575E30};

WEAK symbol<void(const dvar_t *dvar, bool val, DvarSetSource source)>
    Dvar_SetBoolFromSource{0x1422C6290, 0x140578BA0};
WEAK symbol<void(const dvar_t *dvar, float val, DvarSetSource source)>
    Dvar_SetFloatFromSource{0x1422C6DD0, 0x140579190};
WEAK symbol<void(const dvar_t *dvar, int64_t val, DvarSetSource source)>
    Dvar_SetInt64FromSource{0x1422C7B70, 0x140579660};
WEAK symbol<void(const dvar_t *dvar, int32_t val, DvarSetSource source)>
    Dvar_SetIntFromSource{0x1422C7ED0, 0x140579740};
WEAK symbol<void(const dvar_t *dvar, uint64_t val, DvarSetSource source)>
    Dvar_SetUInt64FromSource{0x1422C8AF0, 0x140579B60};

// UI
WEAK
    symbol<void(int localClientNumber, int errorcode, const char *errorMessage)>
        UI_OpenErrorPopupWithMessage{0x14228DEE0};
WEAK symbol<void(bool frontend)> UI_CoD_Init{0x141F29010, 0x1404A0A50};
WEAK symbol<void()> UI_CoD_LobbyUI_Init{0x141F2BD80, 0x1404A1F50};
WEAK symbol<void()> UI_CoD_Shutdown{0x141F32E10, 0x0};
WEAK symbol<void(const char *, const char *, int, hks::lua_State *)> UI_AddMenu{
    0x1427018F0, 0x0};
WEAK symbol<const char *(int)> UI_CoD_GetRootNameForController{0x141F28940,
                                                               0x0};
WEAK symbol<void(hks::lua_State *, const char *)> Lua_CoD_LoadLuaFile{
    0x141F11A20, 0x0};
WEAK symbol<void(int localClientNum)> CG_LUIHUDRestart{0x140F7E970};
WEAK symbol<void(int localClientNum)> CL_CheckKeepDrawingConnectScreen{
    0x1413CCAE0};
WEAK symbol<void(const char *key, int value, hks::lua_State *luaVM)>
    Lua_SetTableInt{0x141F066E0};
WEAK symbol<const char *(const char *reference)> SEH_SafeTranslateString{
    0x142279510};

// Scr
WEAK symbol<void(scr::scriptInstance_t inst, int value)> Scr_AddInt{
    0x1412E9870, 0x14016F160};
WEAK symbol<void(scr::scriptInstance_t inst, const char *value)> Scr_AddString{
    0x0, 0x14016F320};
WEAK symbol<const char *(scr::scriptInstance_t inst, unsigned int index)>
    Scr_GetString{0x0, 0x140171490};
WEAK symbol<void(level::gentity_s *ent, ScrVarCanonicalName_t stringValue,
                 unsigned int paramcount)>
    Scr_Notify_Canon{0x0, 0x1402F5FF0};
WEAK symbol<unsigned int(scr::scriptInstance_t inst)> Scr_GetNumParam{
    0x0, 0x140171320};
WEAK symbol<unsigned int(scr::scriptInstance_t inst, const char *filename)>
    Scr_LoadScript{0x1412C83F0, 0x140156610};
WEAK symbol<void(scr::scriptInstance_t inst, int user)> Scr_BeginLoadScripts{
    0x1412C7DF0, 0x140156010};

WEAK symbol<void(const char *name, const char *key, unsigned int playbackFlags,
                 float volume, void *callbackInfo, int id)>
    Cinematic_StartPlayback{0x1412BE3A0};
WEAK symbol<void(uint64_t id, bool cancelAll)> Cinematic_StopPlayback{
    0x1412BEA70};

// Rendering
WEAK symbol<void(const char *, int, const void *, float, float, float, float,
                 float, const float *, int)>
    R_AddCmdDrawText{0x141CD98D0};

// PCache
WEAK symbol<void(ControllerIndex_t controllerIndex)> PCache_DeleteEntries{
    0x141E8D710};

// SV
WEAK symbol<bool()> SV_Loaded{0x142252250, 0x140535460};
WEAK symbol<void *()> SV_AddTestClient{0x142248F40, 0x14052E3E0};
WEAK symbol<void(net::netadr_t from)> SV_DirectConnect{0x142249880,
                                                       0x14052EC60};
WEAK symbol<void(int clientNum, net::svscmd_type type, const char *text)>
    SV_GameSendServerCommand{0x14224F580, 0x140532CA0};
WEAK symbol<void(net::client_s *cl_0, net::svscmd_type type, const char *fmt,
                 ...)>
    SV_SendServerCommand{0x142254D30, 0x140537F10};
WEAK symbol<bool(int clientNum)> SV_IsTestClient{0x14224AB60, 0x14052FF40};
WEAK symbol<void(int controllerIndex, const char *server, MapPreload preload,
                 bool savegame)>
    SV_SpawnServer{0x1422528C0, 0x140535B20};
WEAK symbol<void(const char *text_in)> SV_Cmd_TokenizeString{0x1420EF130,
                                                             0x1404FA6C0};
WEAK symbol<void()> SV_Cmd_EndTokenizedString{0x1420EF0E0, 0x1404FA670};
WEAK symbol<void(void *client, net::msg_t *msg)> SV_ExecuteClientMessage{
    0x14224A460, 0x14052F840};

WEAK symbol<void(net::client_s *drop, const char *reason, bool tellThem,
                 bool removeFromLobby)>
    SV_DropClient{0x14224A050, 0x14052F430};
WEAK symbol<void(net::client_s *cl_0, const char *reason)> SV_Live_RemoveClient{
    0x142242510, 0x140527530};
/*
  Server only. Function exists on client but requires accessing areas of memory
  which are only initialized when running as dedicated server. Also might not
  (?) take the second `reason` argument, but it is entirely unused anyway, so
  this is of no consequence.
*/
WEAK symbol<void(net::client_s *cl_0, const char *reason)>
    SV_Live_RemoveAllClientsFromAddress{0x142254630, 0x1405379E0};
WEAK symbol<void(net::client_s *client, net::svscmd_type type, const char *cmd)>
    SV_AddServerCommand{0x142253460, 0x140536660};

// FS
WEAK symbol<char *(int bytes)> FS_AllocMem{0x1422AC9F0, 0x14056C340};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileWriteToDir{0x1422A35D0};
WEAK symbol<fileHandle_t(const char *filename, const char *dir,
                         const char *osbasepath)>
    FS_FOpenFileReadFromDir{0x1422A3510};

// Lobby
namespace lobby {
WEAK symbol<int(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ED8AC0, 0x14048A360};

WEAK symbol<int(int64_t lobbySession, LobbyClientType clientType)>
    LobbySession_GetClientCount{0x141ED8B30, 0x0};
} // namespace lobby

// Utils
WEAK symbol<const char *(char *str)> I_CleanStr{0x1422E9050, 0x140580E80};
WEAK symbol<int(const char *s0, const char *s1)> I_stricmp{0x1422E9530,
                                                           0x140581300};
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcpy{
    0x1422E9410, 0x1405811E0};

// Variables
WEAK symbol<cmd_function_s> cmd_functions{0x15689DF58, 0x14946F860};
WEAK symbol<CmdArgs> sv_cmd_args{0x15689AE30, 0x14944C740};

WEAK symbol<level::gentity_s> g_entities{0x0, 0x1471031B0};

WEAK symbol<int> level_time{0x0, 0x1474FDC94};
WEAK symbol<int> level_rounds_played{0x14A55BDEC, 0x1475097BC};

WEAK symbol<SOCKET> ip_socket{0x157E75818, 0x14A640988};

WEAK symbol<lobby::Join> s_join{0x15574A640};
WEAK symbol<char> s_dvarPool{0x157AC6220, 0x14A3CB620};
WEAK symbol<bool> s_canSetConfigDvars{0x0, 0x14A3CB5D8};

WEAK symbol<int> g_dvarCount{0x157AC61CC, 0x14A3CB5FC};

WEAK symbol<unsigned int> modsCount{0x15678D170, 0x14933EAE0};
WEAK symbol<workshop_data> modsPool{0x15678D178, 0x14933EAE8};

WEAK symbol<unsigned int> usermapsCount{0x1567B3580, 0x149364EE8};
WEAK symbol<workshop_data> usermapsPool{0x1567B3588, 0x149364EF0};

WEAK symbol<int> fs_loadStack{0x157A65310, 0x14A39C650};

// Client and dedi struct size differs :(
WEAK symbol<net::client_s_cl *> svs_clients_cl{0x1576F9318, 0};
WEAK symbol<net::client_s *> svs_clients{0x0, 0x14A178E98};
WEAK symbol<uint32_t> svs_time{0x0, 0x14A178E84};

// Dvar variables
WEAK symbol<dvar_t *> com_maxclients{0x0, 0x14948EE70};

WEAK symbol<clientUIActive_t> clientUIActives{0x1453D8BC0};

// SL
WEAK symbol<const char *(scr::ScrString_t stringValue)> SL_ConvertToStringSafe{
    0x1412D7180, 0x1401632E0};

// GScr
WEAK symbol<void(scr::scriptInstance_t inst)> GScr_PIXBeginEvent{0x0,
                                                                 0x1402DA730};
WEAK symbol<void(scr::scriptInstance_t inst)> GScr_PIXEndEvent{0x0,
                                                               0x140515B60};
WEAK symbol<void()> GScr_LoadConsts{0x141C353B0, 0x14032E640};

// PIX
WEAK symbol<void(int64_t, char *event)> PIXBeginNamedEvent{0x0, 0x14050BAE0};
WEAK symbol<void()> PIXEndNamedEvent{0x0, 0x14050C280};

namespace jq {
// jq
WEAK symbol<void(jqBatch *batch, char *event)> jqCallbackPre{0x0, 0x1405711A0};
WEAK symbol<void(jqBatch *batch, void *data, bool finished)> jqCallbackPost{
    0x0, 0x140571140};
WEAK symbol<void(void *data)> jqFreeBatchData{
    0x0, 0x14000A350}; // arg usually passed as batch->p3x_info
} // namespace jq

namespace bg {
// BG_ASM_

} // namespace bg

namespace s_wcd {
WEAK symbol<HWND> codLogo{0x157E75A50, 0x14A640BC0};
WEAK symbol<HFONT> hfBufferFont{0x157E75A58, 0x14A640BC8};
WEAK symbol<HWND> hWnd{0x157E75A40, 0x14A640BB0};
WEAK symbol<HWND> hwndBuffer{0x157E75A48, 0x14A640BB8};
WEAK symbol<HWND> hwndInputLine{0x157E75A60, 0x14A640BD0};
WEAK symbol<int> windowHeight{0x157E7606C, 0x14A6411DC};
WEAK symbol<int> windowWidth{0x157E76068, 0x14A6411D8};
WEAK symbol<WNDPROC> SysInputLineWndProc{0x157E76070, 0x14A6411E0};
} // namespace s_wcd

// Global game definitions
constexpr auto CMD_MAX_NESTING = 8;

// Re-implementations
bool I_islower(int c);
bool I_isupper(int c);

namespace scr {
WEAK symbol<scr::scr_const_t> scr_const{0x14A7290F0, 0x147B91CE0};

unsigned int Scr_CanonHash(const char *str);
} // namespace scr

namespace hks {
WEAK symbol<lua_State *> lua_state{0x159C76D88, 0x14858C408};
WEAK symbol<void(lua_State *s, const char *str, unsigned int l)>
    hksi_lua_pushlstring{0x140A18430, 0x1401DE6F0};

WEAK symbol<void(lua_State *s, const HksObject *tbl, const HksObject *key,
                 const HksObject *val)>
    hks_obj_settable{0x141D4B660, 0x1403F41B0};
WEAK symbol<HksObject *(HksObject *result, lua_State *s, const HksObject *table,
                        const HksObject *key)>
    hks_obj_gettable{0x141D4ABF0, 0x1403F3750};
WEAK symbol<void(lua_State *s, int nargs, int nresults, const unsigned int *pc)>
    vm_call_internal{0x141D70FE0, 0x140418E40};
WEAK symbol<HashTable *(lua_State *s, unsigned int arraySize,
                        unsigned int hashSize)>
    Hashtable_Create{0x141D3B5F0, 0x1403E46D0};
WEAK symbol<cclosure *(lua_State *s, lua_function function, int num_upvalues,
                       int internal_, int profilerTreatClosureAsFunc)>
    cclosure_Create{0x141D3B7E0, 0x1403E48C0};
WEAK symbol<int(lua_State *s, int t)> hksi_luaL_ref{0x141D4D1A0, 0x1403F5CF0};
WEAK symbol<void(lua_State *s, int t, int ref)> hksi_luaL_unref{0x141D4D320,
                                                                0x1403F5E70};

WEAK symbol<int(lua_State *s, const HksCompilerSettings *options,
                const char *buff, uint64_t sz, const char *name)>
    hksi_hksL_loadbuffer{0x141D4BD80, 0x1403F48D0};
WEAK symbol<int(lua_State *s, const char *what, lua_Debug *ar)>
    hksi_lua_getinfo{0x141D4D8D0, 0x1403F64B0};
WEAK symbol<int(lua_State *s, int level, lua_Debug *ar)> hksi_lua_getstack{
    0x141D4DB90, 0x1403F6770};
WEAK symbol<void(lua_State *s, const char *fmt, ...)> hksi_luaL_error{
    0x141D4D050, 0x1403F5BA0};
WEAK symbol<const char *(lua_State *s, int index, size_t *len)>
    hksi_lua_tolstring{0x141D4B6C0, 0x1403F42B0};
WEAK symbol<const char *> s_compilerTypeName{0x140A18430};
} // namespace hks

namespace ui {
namespace hud {
WEAK symbol<std::array<game_hudelem_t, 1116>> g_hudelems{0x14A08B560,
                                                         0x1470D7D70};
}
} // namespace ui

} // namespace game