#pragma once

#include <cstdint>
#include "../core.hpp"
#include "core.hpp"
#include "../quake/core.hpp"
#include "../db/xasset.hpp"
#include "../scr/scr.hpp"

namespace game {
namespace snd {
namespace cmd {
struct SndCommandAmbientState {
  int32_t entNum;
  str64_t roomName;
};

struct SndCommandPlaybackFree {
  SndPlayback *playback;
};

#pragma pack(push, 1)
struct SndCommandPlaybackUpdate {
  SndPlayback *playback;
  float attenuation;
  uint32_t lengthMs;
  uint32_t playedMs;
  uint8_t _padding14[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCommandFacialAnimation {
  scr::ScrString_t animation;
  uint8_t _padding04[4];
  SndEntHandle ent;
};
#pragma pack(pop)

struct SndCommandLength {
  uint32_t lengthMs;
  uint32_t ent;
};

#pragma pack(push, 1)
struct SndCommandSubtitle {
  uint32_t lengthMs;
  uint8_t _padding04[4];
  const char *subtitle;
};
#pragma pack(pop)

struct SndCommandEntUpdate {
  SndEntHandle handle;
};

struct SndCommandRattle {
  vec3_t origin;
  int minDist;
  int maxDist;
};

struct SndCommandRattleSetup {
  SndAliasId alias;
  vec4_t origin;
};

struct SndCommandForceAmbientRoom {
  SndStringHash room;
};

struct SndCommandBankUpdateZone {
  str64_t zoneName;
};

#pragma pack(push, 1)
struct SndCommandPlayLoops {
  SndEntLoop loops[5];
  int32_t count;
  uint8_t _paddingA4[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCommandSetLoopState {
  SndEntHandle handle;
  SndAliasId aliasId;
  float attenuation;
  float attenuationRate;
  float pitch;
  float pitchRate;
  uint8_t _padding1C[4];
};
#pragma pack(pop)

struct SndCommandSetGlobalFutz {
  SndStringHash futzId;
  SndGfutzLocation callLocation;
};

struct SndCommandPrefetchLoadedAlias {
  SndAliasId aliasId;
};

struct SndCommandSetMusicState {
  SndStringHash stateId;
};

struct SndCommandUnloadGlobals {
  const SndDriverGlobals *globals;
};

struct SndCommandLoadGlobals {
  const SndDriverGlobals *globals;
};

struct SndCommandUnloadPatch {
  const SndPatch *patch;
};

struct SndCommandLoadPatch {
  const SndPatch *patch;
};

struct SndCommandUnloadBank {
  const SndBank *bank;
};

#pragma pack(push, 1)
struct SndCommandLoadBank {
  const SndBank *bank;
  int32_t priority;
  qboolean patchZone;
  ZoneType zoneType;
  uint8_t _padding14[4];
};
#pragma pack(pop)

struct SndCommandScriptTimescale {
  float value;
};

struct SndCommandSetEntContext {
  SndEntHandle handle;
  SndStringHash type;
  SndStringHash value;
};

struct SndCommandSetContext {
  SndStringHash type;
  SndStringHash value;
};

struct SndCommandUpdateLineAt {
  SndAliasId id;
  vec3_t previousOrigin[2];
  vec3_t origin[2];
};

struct SndCommandStopLineAt {
  SndAliasId id;
  vec3_t origin[2];
};

struct SndCommandPlayLineAt {
  SndAliasId id;
  vec3_t origin[2];
};

struct SndCommandStopLoopAt {
  SndAliasId id;
  vec3_t origin;
};

struct SndCommandPlayLoopAt {
  SndAliasId id;
  vec3_t origin;
};

#pragma pack(push, 1)
struct SndCommandSetGameState {
  qboolean is_paused;
  qboolean is_mature;
  SndGameMode mode;
  float timescale;
  uint32_t cg_time;
  uint32_t seed;
  float voiceScale;
  float musicScale;
  float sfxScale;
  float masterScale;
  float cinematicScale;
  int32_t masterPatch;
  uint32_t hearingImpaired;
  qboolean muteMusic;
  qboolean muteVoice;
  qboolean gpadSounds;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCommandSetEntState {
  SndEntHandle handle;
  vec3_t origin;
  vec3_t velocity;
  vec3_t orientation[3];
  uint8_t _padding44[4];
};
#pragma pack(pop)

struct SndCommandDuck {
  SndDuckCategoryType type;
  SndStringHash id;
  float amount;
};

struct SndCommandSetStartPaused {
  SndPlaybackId id;
  qboolean pause;
};

struct SndCommandStopPlayback {
  SndPlaybackId id;
};

struct SndCommandSetPlaybackPitchRate {
  SndPlaybackId id;
  float rate;
};

struct SndCommandSetPlaybackPitch {
  SndPlaybackId id;
  float pitch;
};

struct SndCommandSetPlaybackAttenuationRate {
  SndPlaybackId id;
  float rate;
};

struct SndCommandSetPlaybackAttenuation {
  SndPlaybackId id;
  float attenuation;
};

struct SndCommandStopSounds {
  SndStopSoundFlags flags;
};

struct SndCommandSetListener {
  SndEntHandle handle;
  int64_t listener;
  int64_t clientNum;
  int64_t controllerNum;
  int64_t team;
  vec3_t origin;
  vec3_t axis[3];
};

struct SndCommandDisconnectListener {
  LocalClientNum_t listener;
};

struct SndCommandStopEnt {
  SndEntHandle ent;
};

#pragma pack(push, 1)
struct SndCommandStopAlias {
  SndEntHandle ent;
  SndStringHash alias_name;
  uint8_t _padding0C[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCommandPlay {
  SndPlaybackHandle playback;
  SndPlayState state;
  qboolean setEntState;
  vec3_t origin;
  vec3_t velocity;
  vec3_t orientation[3];
  int32_t padding;
  uint8_t _paddingAC[4];
};
#pragma pack(pop)

struct SndCommandSetShockAmbientRoom {
  SndStringHash room;
};

struct SndCommandPrimeAlias {
  SndAliasId scriptIdHash;
  int32_t timeTillPlayMs;
};

struct SndCommandCinematicStart {
  float volume;
};

struct SndCommandAliasName {
  str64_t name;
  SndAliasId id;
};

struct SndCommandHeader {
  uint16_t sequence;
  uint16_t magic;
  uint16_t command;
  uint16_t size;
};

struct SndQueueBuffers;
struct SndCommandBuffer;

typedef void (*SND_QueueBufferProcess)(SndCommandBuffer *);

#pragma pack(push, 1)
// sizeof=0x8020
struct SndCommandBuffer {
  int32_t sequence;
  int32_t used;
  SND_QueueBufferProcess *process;
  SndCommandBuffer *next;
  SndQueueBuffers *buffers;
  uint8_t data[32768];
};
ASSERT_SIZE(SndCommandBuffer, 0x8020);
#pragma pack(pop)

#pragma pack(push, 1)
struct SndQueueBuffers {
  tlAtomicMutex mutex;
  SndCommandBuffer buffers[16];
  SndCommandBuffer *freeList;
};
ASSERT_SIZE(SndQueueBuffers, 0x80220);
#pragma pack(pop)

enum class SndCommandType : uint32_t {
  SND_COMMAND_NOP = 0x0,
  SND_COMMAND_ALIAS_NAME = 0x1,
  SND_COMMAND_PLAY = 0x2,
  SND_COMMAND_STOP_ALIAS = 0x3,
  SND_COMMAND_STOP_ENT = 0x4,
  SND_COMMAND_CINEMATIC_START = 0x5,
  SND_COMMAND_CINEMATIC_END = 0x6,
  SND_COMMAND_DISCONNECT_LISTENER = 0x7,
  SND_COMMAND_SET_LISTENER = 0x8,
  SND_COMMAND_STOP_SOUNDS = 0x9,
  SND_COMMAND_FADE_IN = 0xA,
  SND_COMMAND_FADE_OUT = 0xB,
  SND_COMMAND_UNUSED1 = 0xC,
  SND_COMMAND_UNUSED2 = 0xD,
  SND_COMMAND_SET_PLAYBACK_ATTENUATION = 0xE,
  SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE = 0xF,
  SND_COMMAND_SET_PLAYBACK_PITCH = 0x10,
  SND_COMMAND_SET_PLAYBACK_PITCH_RATE = 0x11,
  SND_COMMAND_STOP_PLAYBACK = 0x12,
  SND_COMMAND_SET_START_PAUSED = 0x13,
  SND_COMMAND_DUCK = 0x14,
  SND_COMMAND_SET_ENT_STATE = 0x15,
  SND_COMMAND_SET_GAME_STATE = 0x16,
  SND_COMMAND_PLAY_LOOP_AT = 0x17,
  SND_COMMAND_STOP_LOOP_AT = 0x18,
  SND_COMMAND_PLAY_LINE_AT = 0x19,
  SND_COMMAND_STOP_LINE_AT = 0x1A,
  SND_COMMAND_UPDATE_LINE_AT = 0x1B,
  SND_COMMAND_GAME_RESET = 0x1C,
  SND_COMMAND_SET_CONTEXT = 0x1D,
  SND_COMMAND_SET_ENT_CONTEXT = 0x1E,
  SND_COMMAND_SCRIPT_TIMESCALE = 0x1F,
  SND_COMMAND_UPDATE_CG = 0x20,
  SND_COMMAND_LOAD_BANK = 0x21,
  SND_COMMAND_UNLOAD_BANK = 0x22,
  SND_COMMAND_LOAD_PATCH = 0x23,
  SND_COMMAND_UNLOAD_PATCH = 0x24,
  SND_COMMAND_LOAD_GLOBALS = 0x25,
  SND_COMMAND_UNLOAD_GLOBALS = 0x26,
  SND_COMMAND_PRIME_ALIAS = 0x27,
  SND_COMMAND_RESTART_DRIVER = 0x28,
  SND_COMMAND_SET_MUSIC_STATE = 0x29,
  SND_COMMAND_PREFETCH_LOADED_ALIAS = 0x2A,
  SND_COMMAND_SET_GLOBAL_FUTZ = 0x2B,
  SND_COMMAND_SET_FRONTEND_MUSIC = 0x2C,
  SND_COMMAND_SET_LOOP_STATE = 0x2D,
  SND_COMMAND_PLAY_LOOPS = 0x2E,
  SND_COMMAND_SHUTDOWN = 0x2F,
  SND_COMMAND_BANK_UPDATE_ZONE = 0x30,
  SND_COMMAND_FORCE_AMBIENT_ROOM = 0x31,
  SND_COMMAND_RATTLE_SETUP = 0x32,
  SND_COMMAND_RATTLE = 0x33,
  SND_COMMAND_ENT_UPDATE = 0x34,
  SND_COMMAND_SUBTITLE = 0x35,
  SND_COMMAND_LENGTH = 0x36,
  SND_COMMAND_PLAYBACK_UPDATE = 0x37,
  SND_COMMAND_PLAYBACK_FREE = 0x38,
  SND_COMMAND_AMBIENT_STATE = 0x39,
  SND_COMMAND_BANK_FREE = 0x3A,
  SND_COMMAND_SET_SHOCK_AMBIENT_ROOM = 0x3B,
  SND_COMMAND_CHECKPOINT = 0x3C,
  SND_COMMAND_FACIAL_ANIMATION = 0x3D,
  SND_COMMAND_COUNT = 0x3E,
};
/*
  The engine will often handle sound command cases by first decrementing 1
  from the command ID, then handling each case of the result. This enum serves
  to strongly type this 'nopless' command type handling.
*/
enum class NoplessSndCommandType : uint32_t {
  NOPLESS_SND_COMMAND_ALIAS_NAME = 0x0,
  NOPLESS_SND_COMMAND_PLAY = 0x1,
  NOPLESS_SND_COMMAND_STOP_ALIAS = 0x2,
  NOPLESS_SND_COMMAND_STOP_ENT = 0x3,
  NOPLESS_SND_COMMAND_CINEMATIC_START = 0x4,
  NOPLESS_SND_COMMAND_CINEMATIC_END = 0x5,
  NOPLESS_SND_COMMAND_DISCONNECT_LISTENER = 0x6,
  NOPLESS_SND_COMMAND_SET_LISTENER = 0x7,
  NOPLESS_SND_COMMAND_STOP_SOUNDS = 0x8,
  NOPLESS_SND_COMMAND_FADE_IN = 0x9,
  NOPLESS_SND_COMMAND_FADE_OUT = 0xA,
  NOPLESS_SND_COMMAND_UNUSED1 = 0xB,
  NOPLESS_SND_COMMAND_UNUSED2 = 0xC,
  NOPLESS_SND_COMMAND_SET_PLAYBACK_ATTENUATION = 0xD,
  NOPLESS_SND_COMMAND_SET_PLAYBACK_ATTENUATION_RATE = 0xE,
  NOPLESS_SND_COMMAND_SET_PLAYBACK_PITCH = 0xF,
  NOPLESS_SND_COMMAND_SET_PLAYBACK_PITCH_RATE = 0x10,
  NOPLESS_SND_COMMAND_STOP_PLAYBACK = 0x11,
  NOPLESS_SND_COMMAND_SET_START_PAUSED = 0x12,
  NOPLESS_SND_COMMAND_DUCK = 0x13,
  NOPLESS_SND_COMMAND_SET_ENT_STATE = 0x14,
  NOPLESS_SND_COMMAND_SET_GAME_STATE = 0x15,
  NOPLESS_SND_COMMAND_PLAY_LOOP_AT = 0x16,
  NOPLESS_SND_COMMAND_STOP_LOOP_AT = 0x17,
  NOPLESS_SND_COMMAND_PLAY_LINE_AT = 0x18,
  NOPLESS_SND_COMMAND_STOP_LINE_AT = 0x19,
  NOPLESS_SND_COMMAND_UPDATE_LINE_AT = 0x1A,
  NOPLESS_SND_COMMAND_GAME_RESET = 0x1B,
  NOPLESS_SND_COMMAND_SET_CONTEXT = 0x1C,
  NOPLESS_SND_COMMAND_SET_ENT_CONTEXT = 0x1D,
  NOPLESS_SND_COMMAND_SCRIPT_TIMESCALE = 0x1E,
  NOPLESS_SND_COMMAND_UPDATE_CG = 0x1F,
  NOPLESS_SND_COMMAND_LOAD_BANK = 0x20,
  NOPLESS_SND_COMMAND_UNLOAD_BANK = 0x21,
  NOPLESS_SND_COMMAND_LOAD_PATCH = 0x22,
  NOPLESS_SND_COMMAND_UNLOAD_PATCH = 0x23,
  NOPLESS_SND_COMMAND_LOAD_GLOBALS = 0x24,
  NOPLESS_SND_COMMAND_UNLOAD_GLOBALS = 0x25,
  NOPLESS_SND_COMMAND_PRIME_ALIAS = 0x26,
  NOPLESS_SND_COMMAND_RESTART_DRIVER = 0x27,
  NOPLESS_SND_COMMAND_SET_MUSIC_STATE = 0x28,
  NOPLESS_SND_COMMAND_PREFETCH_LOADED_ALIAS = 0x29,
  NOPLESS_SND_COMMAND_SET_GLOBAL_FUTZ = 0x2A,
  NOPLESS_SND_COMMAND_SET_FRONTEND_MUSIC = 0x2B,
  NOPLESS_SND_COMMAND_SET_LOOP_STATE = 0x2C,
  NOPLESS_SND_COMMAND_PLAY_LOOPS = 0x2D,
  NOPLESS_SND_COMMAND_SHUTDOWN = 0x2E,
  NOPLESS_SND_COMMAND_BANK_UPDATE_ZONE = 0x2F,
  NOPLESS_SND_COMMAND_FORCE_AMBIENT_ROOM = 0x30,
  NOPLESS_SND_COMMAND_RATTLE_SETUP = 0x31,
  NOPLESS_SND_COMMAND_RATTLE = 0x32,
  NOPLESS_SND_COMMAND_ENT_UPDATE = 0x33,
  NOPLESS_SND_COMMAND_SUBTITLE = 0x34,
  NOPLESS_SND_COMMAND_LENGTH = 0x35,
  NOPLESS_SND_COMMAND_PLAYBACK_UPDATE = 0x36,
  NOPLESS_SND_COMMAND_PLAYBACK_FREE = 0x37,
  NOPLESS_SND_COMMAND_AMBIENT_STATE = 0x38,
  NOPLESS_SND_COMMAND_BANK_FREE = 0x39,
  NOPLESS_SND_COMMAND_SET_SHOCK_AMBIENT_ROOM = 0x3A,
  NOPLESS_SND_COMMAND_CHECKPOINT = 0x3B,
  NOPLESS_SND_COMMAND_FACIAL_ANIMATION = 0x3C,
  NOPLESS_SND_COMMAND_COUNT = 0x3D,
};

union SndCommand {
  SndCommandAliasName *aliasName;
  SndCommandAmbientState *ambientState;
  SndCommandBankUpdateZone *bankUpdateZone;
  SndCommandCinematicStart *cinematicStart;
  SndCommandDisconnectListener *disconnectListener;
  SndCommandDuck *duck;
  SndCommandEntUpdate *entUpdate;
  SndCommandFacialAnimation *facialAnimation;
  SndCommandForceAmbientRoom *forceAmbientRoom;
  SndCommandLength *length;
  SndCommandLoadBank *loadBank;
  SndCommandLoadGlobals *loadGlobals;
  SndCommandLoadPatch *loadPatch;
  SndCommandPlay *play;
  SndCommandPlaybackFree *playbackFree;
  SndCommandPlaybackUpdate *playbackUpdate;
  SndCommandPlayLineAt *playLineAt;
  SndCommandPlayLoopAt *playLoopAt;
  SndCommandPlayLoops *playLoops;
  SndCommandPrefetchLoadedAlias *prefetchLoadedAlias;
  SndCommandPrimeAlias *primeAlias;
  SndCommandRattle *rattle;
  SndCommandRattleSetup *rattleSetup;
  SndCommandScriptTimescale *scriptTimescale;
  SndCommandSetContext *setContext;
  SndCommandSetEntContext *setEntContext;
  SndCommandSetEntState *setEntState;
  SndCommandSetGameState *setGameState;
  SndCommandSetGlobalFutz *setGlobalFutz;
  SndCommandSetListener *setListener;
  SndCommandSetLoopState *setLoopState;
  SndCommandSetMusicState *setMusicState;
  SndCommandSetPlaybackAttenuation *setPlaybackAttenuation;
  SndCommandSetPlaybackAttenuationRate *setPlaybackAttenuationRate;
  SndCommandSetPlaybackPitch *setPlaybackPitch;
  SndCommandSetPlaybackPitchRate *setPlaybackPitchRate;
  SndCommandSetShockAmbientRoom *setShockAmbientRoom;
  SndCommandSetStartPaused *setStartPaused;
  SndCommandStopAlias *stopAlias;
  SndCommandStopEnt *stopEnt;
  SndCommandStopLineAt *stopLineAt;
  SndCommandStopLoopAt *stopLoopAt;
  SndCommandStopPlayback *stopPlayback;
  SndCommandStopSounds *stopSounds;
  SndCommandSubtitle *subtitle;
  SndCommandUnloadBank *unloadBank;
  SndCommandUnloadGlobals *unloadGlobals;
  SndCommandUnloadPatch *unloadPatch;
  SndCommandUpdateLineAt *updateLineAt;
  // nullptr
  void *null;
};
} // namespace cmd
} // namespace snd
} // namespace game