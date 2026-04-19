#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {

// SND
namespace snd {

WEAK symbol<SndBankGlobals> g_sb{0x1580E0E00, 0x14A8AAA80};

// snd globals
WEAK symbol<volatile int32_t> snd_update_fence{0x1579072B0, 0x14A2CEF30};
WEAK symbol<volatile int32_t> snd_update_start{0x1579072B4, 0x14A2CEF34};

// g_snd globals
WEAK symbol<SndLocal> g_snd{0x14347EE00, 0x141189800};
// one snd_fire_manager per local client, each with 8 fire slots
WEAK symbol<matrix2d<snd_fire_manager, 2, 8>> cl_g_snd_fire{0x144D2A13C, 0x0};
WEAK symbol<matrix2d<snd_fire_manager, 1, 8>> sv_g_snd_fires{0x0, 0x14223B840};
// Total size 0x1600
WEAK symbol<snd_autosim[64]> g_snd_autosims{0x144D3FC20, 0x142243B80};
// Total size 0x1200
WEAK symbol<snd_autosim_play[64]> g_snd_autosim_history{0x144D3FC20,
                                                        0x142242980};
WEAK symbol<uint32_t> g_snd_autosim_time{0x144D4B640, 0x14224E040};
WEAK symbol<uint32_t> g_snd_autosim_frame{0x144D42AEC, 0x1422454EC};
/*
   True if "nosnd" given as CLI arg.
   Probably does not exist on client, and is not seen used
   anywhere where it is used in dedicated server.
   See note above G_SNDEnabled.
*/
WEAK symbol<qboolean> g_pc_nosnd{0x0, 0x14A63D4EC};

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
WEAK symbol<bool(snd::SndPlaybackId playbackId,
                 snd::SndLengthNotifyData lengthNotifyData,
                 snd::SndLengthType id)>
    SND_AddLengthNotify{0x0, 0x140643680};
WEAK symbol<bool()> SND_ShouldInit{0x0, 0x140647FB0};

WEAK symbol<bool(uint32_t channel, const vec3_t *position, float *value)>
    SND_LosOcclusionCache{0x0, 0x140645C70};
WEAK symbol<void(int msec, int lengthNotifyData)> CG_ScriptSndLengthNotify{
    0x0, 0x14011F2D0};
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
WEAK symbol<int64_t(const vec3_t *p1, const vec3_t *p2, const vec3_t *p3,
                    float r, vec3_t *start, vec3_t *end)>
    SND_WhizbyPath{0x0, 0x140651D80};
WEAK symbol<qboolean(SndVoice *voice)> SND_CheckContextLoops{0x0, 0x14064FB50};
WEAK symbol<int32_t(jq::jqBatch *batch, void *data)>
    snd_occlusionCallback_Implementation{0x0, 0x1406510E0};
WEAK symbol<bool(SndEntHandle sndEnt, const vec3_t *missilePosition,
                 const vec3_t *missileDirection, const vec3_t *position,
                 const float whizbyRadius, const char *whizbySound)>
    SND_MissileWhizby{0x0, 0x1406515A0};
WEAK symbol<SndPlaybackId(SndStringHash aliasId, SndPlayback *playback,
                          const SndPlayState *state)>
    SND_ContinueLoopingSound{0x0, 0x140643800};
WEAK symbol<SndMusicState *(SndStringHash id)> SND_GetMusicState{0x0,
                                                                 0x14064C060};
WEAK symbol<int64_t(jq::jqBatch *batch)> Snd_Occlusion{0x0, 0x140651080};
WEAK symbol<void(SndMusicAssetInstance *instance, bool forceQuickFade)>
    SND_MusicAssetStop{0x0, 0x140546DB0};
WEAK symbol<bool(snd::SndPlaybackId id)> SND_IsPlaying{0x0, 0x140546670};
WEAK symbol<void(int msec, const char *lengthNotifyData)>
    CG_SubtitleSndLengthNotify{0x0, 0x140120200};
WEAK symbol<void(SndEntHandle handle)> SND_SetEntState{0x0, 0x140548DC0};
WEAK symbol<bool(SndContext *ctx)> SND_IsValidContext{0x0, 0x140B29490};
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
WEAK symbol<void(const char *type, const char *value)> SND_SetContext{
    0x0, 0x140548C00};
WEAK symbol<void(const char *futzName, SndGfutzLocation callLocation)>
    SND_SetGlobalFutz{0x0, 0x140548EC0};
WEAK symbol<void(const char *name)> SND_ForceAmbientRoom{0x0, 0x140547F20};
WEAK symbol<void(const char *stateName)> SND_SetMusicState2{
    0x0, 0x140549180}; // named `SND_SetMusicState`; overload
WEAK symbol<void(const char *name)> SND_SetShockAmbientRoom{0x0, 0x1405493B0};
WEAK symbol<void(const snd::SndBank *bank, int priority, bool patchZone,
                 int unknown)>
    SND_AddBank{0x0, 0x1405479E0};
WEAK symbol<void(const snd::SndBank *bank)> SND_RemoveBank{0x0, 0x140548B00};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SND_AddGlobals{
    0x0, 0x140547A80};
WEAK symbol<void(const snd::SndPatch *patch)> SND_AddPatch{0x0, 0x140547AE0};
WEAK symbol<void(const snd::SndPatch *patch)> SND_PatchApply{0x0, 0x140547680};
WEAK symbol<void(const snd::SndPatch *patch)> SND_RemovePatch{0x0, 0x140548B60};
WEAK symbol<void(const snd::SndVoice *voice, float *wet, float *dry)>
    SND_GetLevels{0x0, 0x1406511C0};
WEAK symbol<void(const vec3_t *P, const vec3_t *segmentA,
                 const vec3_t *segmentB, vec3_t *nearPoint)>
    SND_GetNearestPointOnSegment{0x0, 0x140651310};
WEAK symbol<void(float value)> SND_SetScriptTimescale{0x0, 0x140549350};
WEAK symbol<void(float fadetime, bool setScriptValues, float scriptPitch,
                 float scriptPitchRate, float scriptAttenuation,
                 float scriptAttenuationRate, snd::SndVoice *voice)>
    SND_SetVoiceStartFades{0x0, 0x140646F00};
WEAK symbol<void(float *priority, int *channel, uint32_t start, uint32_t count)>
    Snd_GetLowestPriority{0x0, 0x140649B20};
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
WEAK symbol<void(const char *subtitle, uint32_t lengthMs)> SND_SubtitleNotify{
    0x0, 0x140549710};
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
WEAK symbol<void(snd::SndBankLoad *bank, io::stream_fileid fileHandle,
                 int64_t offset, size_t size, void *data)>
    SND_StreamRead{0x14258C820, 0x14064CC50};
WEAK symbol<void()> SND_BankLoadedNotify{0x0, 0x140547BE0};
WEAK symbol<void(snd::SndBankLoad *load)> SND_EnqueueLoadedAssets{0x14258C3B0,
                                                                  0x14064C900};
WEAK symbol<void(snd::SndBankLoad *load)> SND_BankLoadError{0x14258AF40,
                                                            0x14064B460};
WEAK symbol<void(snd::SndBankLoad *load)> SND_BankLoadUpdateState{0x0,
                                                                  0x14064B550};
WEAK symbol<void(snd::SndBankPtr *sound)> Load_SndBankAsset{0x0, 0x1401DB2D0};
WEAK symbol<void()> SND_CheckpointRestore{0x0, 0x140547D60};
WEAK symbol<void(snd::SndCommandBuffer *buffer)> SND_CommandCG{0x0,
                                                               0x140545B60};
WEAK symbol<void(snd::SndCommandType command, int64_t cmdSize, void *cmdData)>
    SND_CommandSND{0x0, 0x140545CB0};
WEAK symbol<void()> SND_DebugFini{0x0, 0x14064CE90};
WEAK symbol<void(snd::SndDuckActive *duck)> SND_StopDuck{0x0, 0x1406480D0};
WEAK symbol<void(snd::SndDuckCategoryType category, uint32_t duckId,
                 const snd::SndDuck *duck, float amount)>
    SND_SetDuckByCategory{0x0, 0x140646A50};
WEAK symbol<void()> SND_DuckReset{0x0, 0x140643C20};
WEAK symbol<void()> SND_EndFrame{0x0, 0x140547DF0};
WEAK symbol<void(snd::SndEntHandle handle, scr::ScrString_t animation)>
    SND_FacialAnimationNotify{0x0, 0x140547EA0};
WEAK symbol<void(snd::SndEntHandle handle)> SND_EntStateRequest{0x0,
                                                                0x140547E40};
WEAK symbol<void(snd::SndEntHandle sndEnt, snd::SndStringHash name)>
    StopSoundAliasesOnEnt{0x0, 0x14064FE30};
WEAK symbol<void()> SND_EntStateFrame{0x0, 0x140546200};
WEAK symbol<void()> SND_ErrorIfSoundGlobalsTrashed{0x142583EB0, 0x140644DE0};
WEAK symbol<void()> SND_Frame{0x0, 0x140547F80};
WEAK symbol<void()> SND_GameReset{0x0, 0x140548060};
WEAK symbol<void()> SND_InitDvar{0x0, 0x14064CEA0};
WEAK symbol<void()> SND_ListenerDiscontinuity{0x0, 0x14064FD90};

WEAK symbol<void()> SND_LosOcclusionFini{0x0, 0x140650220};
WEAK symbol<void()> SND_LosOcclusionSync{0x0, 0x140650300};
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
WEAK symbol<void(bool isMature, bool isPaused, float timescale, uint32_t cgTime,
                 uint32_t seed, float voiceScale, float musicScale,
                 float sfxScale, float masterScale, float cinematicScale,
                 int masterPatch, bool hearingImpaired, SndGameMode mode,
                 uint64_t unk1, uint64_t _unk2,
                 uint64_t unk3)> // 3 new SndCommandSetGameState fields
    SND_BeginFrame{0x0, 0x140547C20};
WEAK symbol<void(snd_weapon_shot *shot)> CG_SndWeaponFire{0x0, 0x140148F50};

WEAK symbol<void(uint32_t table, char *asset, uint32_t fieldIndex,
                 uint32_t value)>
    SND_PatchValue{0x0, 0x1405478D0};
WEAK symbol<void(uint32_t table, snd::SndDuck *duck, uint32_t fieldIndex,
                 snd::SndStringHash groupId, uint32_t value)>
    SND_PatchDuckValue{0x0, 0x140547810};
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
WEAK symbol<void(uint32_t ent, uint32_t lengthMs)> SND_LengthNotify{
    0x0, 0x1405480C0};
WEAK symbol<void(LocalClientNum_t listener)> SND_DisconnectListener{
    0x0, 0x140547DA0};
WEAK symbol<void(SndStringHash stateid, const char *stateName)>
    SND_SetMusicState{0x0, 0x140549130};
WEAK symbol<void(SndStopSoundFlags flags)> SND_StopSounds{0x0, 0x140549660};
WEAK symbol<void(uint32_t id, const vec3_t origin)> SND_PlayLoopAt{0x0,
                                                                   0x1405486F0};
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
WEAK symbol<void(int id)> SND_ContinueLoopingPlayback{0x0, 0x1406437A0};
WEAK symbol<void(int index)> SND_ResetVoiceInfo{0x0, 0x140646910};
WEAK symbol<void(int minUpdateMs)> SND_PossiblyUpdate{0x0, 0x140548990};
WEAK symbol<void(int playbackId)> SND_StopPlayback{0x0, 0x140549590};
WEAK symbol<void(int voiceindex)> SND_Stop{0x0, 0x140647FC0};
WEAK symbol<void(int voiceIndex)> SND_StopVoice{0x0, 0x140649C60};
WEAK symbol<void(int voiceIndex)> SND_StopVoicePFutz{0x0, 0x140648190};
WEAK symbol<bool()> SND_HasLoadingBanks{0x14258C090, 0x14064C5E0};
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

WEAK symbol<int(const void *loadA, const void *loadB)> SND_CompareAssetLoads{
    0x142589AA0, 0x14064A010};
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
WEAK symbol<void()> SND_Init{0x142584E20, 0x1406459B0};
/*
   `return g_pc_nosnd != 0;`
   Most likely does not exist on client.
   The `nosnd` CLI arg handling seems to, in general,
   be server-specific in older CoD engines.
*/
WEAK symbol<bool()> G_SNDEnabled{0x0, 0x140584DB0};
WEAK symbol<void(game::level::gentity_s *ent, SndAliasId index,
                 game::scr::ScrString_t notifyString)>
    G_RegisterSoundWait{0x141B80F40, 0x140308090};

} // namespace snd
} // namespace game

#endif