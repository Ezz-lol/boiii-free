#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace snd {

// SNDL functions act as more of the internal functionality of the sound system,
// whereas SND functions are generally the public API, with most major SND
// functionality being handled by passing command messages to the async queue,
// where SNDL functions are called to actually handle the command
namespace sndl {
WEAK symbol<bool(const snd::SndBank *bank)> SNDL_TryRemoveBank{0x0,
                                                               0x14064A5A0};
WEAK symbol<snd::SndPlaybackId(const snd::SndPlayState *state,
                               snd::SndPlayback *playback)>
    SNDL_Play{0x0, 0x14064DB90};
WEAK
    symbol<void(bool isMature, bool isPaused, float timescale, uint32_t cg_time,
                uint32_t seed, float voiceScale, float musicScale,
                float sfxScale, float masterScale, float cinematicScale,
                int masterPatch, bool hearingImpaired, snd::SndGameMode mode)>
        SNDL_SetGameState{0x0, 0x14064E430};
WEAK symbol<void(const char *zoneName)> SNDL_BankUpdateZone{0x0, 0x14064D500};
WEAK symbol<void(const snd::SndBank *bank, int priority, bool patchZone,
                 int32_t unknown)>
    SNDL_AddBank{0x0, 0x14064A030};
WEAK symbol<void(const snd::SndBank *bank)> SNDL_RemoveBank{0x0, 0x14064A3A0};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SNDL_AddGlobals{
    0x0, 0x1406432B0};
WEAK symbol<void(const snd::SndDriverGlobals *globals)> SNDL_RemoveGlobals{
    0x0, 0x1406432C0};
WEAK symbol<void(const snd::SndPatch *patch)> SNDL_AddPatch{0x0, 0x1405472C0};
WEAK symbol<void(const snd::SndPatch *patch)> SNDL_RemovePatch{0x0,
                                                               0x140547550};
WEAK symbol<void(float value)> SNDL_SetScriptTimescale{0x0, 0x14064ECB0};
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
WEAK symbol<void(snd::SndDuckCategoryType category, snd::SndStringHash duckId,
                 float amount)>
    SNDL_SetDuck{0x0, 0x14064E390};
WEAK symbol<void(snd::SndEntHandle entHandle, LocalClientNum_t localClientNum,
                 ClientNum_t clientNum, ControllerIndex_t controllerNum,
                 team_t team, const vec3_t *origin, const vec3_t *inAxis)>
    SNDL_SetListener{0x0, 0x14064E5D0};
WEAK symbol<void(snd::SndEntHandle handle, const vec3_t *origin,
                 const vec3_t *velocity, const vec3_t *orientation)>
    SNDL_SetEntState{0x0, 0x1405459E0};
WEAK symbol<void(snd::SndEntHandle handle, snd::SndStringHash type,
                 snd::SndStringHash value)>
    SNDL_SetEntContext{0x0, 0x14064E3E0};
WEAK symbol<void(snd::SndEntHandle sndEnt)> SNDL_StopSoundsOnEnt{0x0,
                                                                 0x14064F360};
WEAK symbol<void(snd::SndEntHandle sndEnt, snd::SndStringHash name)>
    SNDL_StopSoundAliasOnEnt{0x0, 0x14064F0B0};
WEAK symbol<void(snd::SndEntHandle sndEnt, uint32_t alias, float attenuation,
                 float attenuationRate, float pitch, float pitchRate)>
    SNDL_SetLoopState{0x0, 0x14064E900};
WEAK symbol<void()> SNDL_ApplyPatches{0x0, 0x140547400};
WEAK symbol<void()> SNDL_Checkpoint{0x0, 0x14064D770};
WEAK symbol<void()> SNDL_GameReset{0x0, 0x14064D880};
WEAK symbol<void()> SNDL_ResetAliases{0x0, 0x14064A3D0};
WEAK symbol<void()> SNDL_ResetEntState{0x0, 0x140546840};
WEAK symbol<void()> SNDL_Shutdown{0x0, 0x1406432D0};
WEAK symbol<void()> SNDL_Update{0x0, 0x140643340};
WEAK symbol<void()> SNDL_UpdateLoopingSounds{0x0, 0x14064F560};
WEAK symbol<void()> SNDL_UpdateStaticSounds{0x0, 0x14064F690};
WEAK symbol<void(snd::SndStopSoundFlags which)> SNDL_StopSounds{0x0,
                                                                0x14064F0C0};
WEAK symbol<void(SndStringHash type, SndStringHash value)> SNDL_SetContext{
    0x0, 0x14064E360};
WEAK symbol<void(snd::SndStringHash id, snd::SndGfutzLocation callLocation)>
    SNDL_SetGlobalFutz{0x0, 0x14064E5C0};
WEAK symbol<void(snd::SndStringHash id)> SNDL_SetMusicState{0x0, 0x1405468C0};
WEAK symbol<void(vec3_t *origin, int minDist, int maxDist)> SNDL_Rattle{
    0x0, 0x14064E000};
WEAK symbol<void(int count, const snd::SndEntLoop *loops)> SNDL_PlayLoops{
    0x0, 0x14064DEA0};
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
WEAK symbol<void(LocalClientNum_t localClientNum)> SNDL_DisconnectListener{
    0x0, 0x14064D7D0};
WEAK symbol<void(qboolean force)> SNDL_ForceAmbientRoom{0x0, 0x140545290};

} // namespace sndl
} // namespace snd
} // namespace game

#endif
