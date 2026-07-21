#ifndef STRUCTS_SND_HPP
#define STRUCTS_SND_HPP

#include <cstdint>
#include "../core.hpp"
#include "../quake/core.hpp"
#include "../db/xasset/core.hpp"
#include "../scr/scr.hpp"
#include "../stream.hpp"

namespace game {
namespace snd {

namespace cmd {
struct SndCommandBuffer;
struct SndQueueBuffers;
}; // namespace cmd

typedef uint32_t SndStringHash;
typedef int32_t SndPlaybackId;
typedef uint8_t sd_byte;
typedef uint32_t SndAliasId;
typedef int64_t SndPlaybackHandle;

enum class SndGameMode : int32_t {
  SND_GAME_MODE_MULTIPLAYER = 0x0,
  SND_GAME_MODE_ZOMBIES = 0x1,
  SND_GAME_MODE_CAMPAIGN = 0x2,
  SND_GAME_MODE_CAMPAIGN_ZOMBIES = 0x3,
};

enum class SndStopSoundFlags : int32_t {
  SND_STOP_ALL = 0x0,
  SND_STOP_PAUSED = 0x1,
  SND_STOP_DEATH = 0x2,
};

enum class SndGfutzLocation : int32_t {
  SND_GFUTZ_LOCATION_SCRIPT = 0x0,
  SND_GFUTZ_LOCATION_GDT = 0x1,
  SND_GFUTZ_LOCATION_COUNT = 0x2,
};

enum class SndLimitType : int32_t {
  SND_LIMIT_NONE = 0x0,
  SND_LIMIT_OLDEST = 0x1,
  SND_LIMIT_REJECT = 0x2,
  SND_LIMIT_PRIORITY = 0x3,
  SND_LIMIT_COUNT = 0x4,
};

/**
 * @brief Defines the lifecycle of a Sound Bank loading process.
 * Managed primarily within SND_BankLoadUpdateState.
 */
enum class SndBankState : int32_t {
  /**
   * Initial state for a new load request.
   * Resets load indices and file handles.
   * Triggers the initial asynchronous read of the Stream Asset Bank header.
   */
  SND_BANK_STATE_NEW = 0x0,

  /**
   * Waiting for the Stream Bank header IO to complete.
   * Performs magic/version validation.
   * On success, triggers the asynchronous read of the Stream Table of
   * Contents (ToC).
   */
  SND_BANK_STATE_STREAM_HEADER = 0x1,

  /**
   * Waiting for the Stream Bank ToC IO to complete.
   * Validates the ToC structure.
   * On success, begins the asynchronous read for the Load (Resident) Asset
   * Bank header.
   */
  SND_BANK_STATE_STREAM_TOC = 0x2,

  /**
   * Waiting for the Load Bank header IO to complete.
   * Performs validation and checks for bank metadata consistency
   * (filesize/entry counts). On success, triggers the asynchronous read for
   * the Load Bank ToC.
   */
  SND_BANK_STATE_LOADED_HEADER = 0x3,

  /**
   * Waiting for the Load Bank ToC IO to complete.
   * Final validation of the resident bank structure.
   * On success, releases the sequential read lock
   * (g_sb.sequentialReadInProgress).
   */
  SND_BANK_STATE_LOADED_TOC = 0x4,

  /**
   * Asset Enqueueing Phase.
   * Assets are moved into the global load queue (SND_EnqueueLoadedAssets) and
   * the queue is sorted by priority/address (qsort).
   */
  SND_BANK_STATE_LOADED_ASSET_WAIT = 0x5,

  /**
   * Asset Processing Phase.
   * The state machine polls here until 'loadedAssetCount' matches
   * 'loadedAssetTotal'. Effectively waits for the background worker threads
   * to finish loading actual wave data.
   */
  SND_BANK_STATE_LOADED_ASSETS = 0x6,

  /**
   * Load sequence finalized.
   * SND_BankLoadedNotify is triggered to alert the system the bank is
   * resident in memory. State is checked via (state - 7 >= 2) to bypass the
   * update logic.
   */
  SND_BANK_STATE_READY_TO_USE = 0x7,

  /**
   * Terminal error state.
   * Set when IO errors occur or header/ToC validation fails.
   * Bypasses standard update logic to prevent further IO attempts.
   */
  SND_BANK_STATE_ERROR = 0x8,
};

enum class SndDuckCategoryType : uint32_t {
  SND_DUCK_CATEGORY_ALIAS = 0x0,
  SND_DUCK_CATEGORY_AMBIENT = 0x1,
  SND_DUCK_CATEGORY_SCRIPT = 0x2,
  SND_DUCK_CATEGORY_SHELLSHOCK = 0x3,
  SND_DUCK_CATEGORY_BREATH = 0x4,
  SND_DUCK_CATEGORY_ADS = 0x5,
  SND_DUCK_CATEGORY_MENU = 0x6,
  SND_DUCK_CATEGORY_ENEMIES = 0x7,
  SND_DUCK_CATEGORY_HEALTH = 0x8,
  SND_DUCK_CATEGORY_DEBUG = 0x9,
  SND_DUCK_CATEGORY_LEVELFADE = 0xA,
  SND_DUCK_CATEGORY_MOVIE = 0xB,
  SND_DUCK_CATEGORY_HIPFIRE = 0xC,
  SND_DUCK_CATEGORY_XCAM = 0xD,
  SND_DUCK_CATEGORY_COUNT = 0xE,
};

enum class SndLengthType : uint32_t {
  SND_LENGTH_NOTIFY_NONE = 0x0,
  SND_LENGTH_NOTIFY_SCRIPT = 0x1,
  SND_LENGTH_NOTIFY_SUBTITLE = 0x2,
  SND_LENGTH_NOTIFY_COUNT = 0x3,
};

enum class SndCallLocation : int32_t {
  SND_CALL_LOCATION_CLIENTSCRIPT = 0x0,
  SND_CALL_LOCATION_SERVER = 0x1,
  SND_CALL_LOCATION_GDT = 0x2,
  SND_CALL_LOCATION_FX = 0x3,
  SND_CALL_LOCATION_CODE = 0x4,
  SND_CALL_LOCATION_UNKNOWN = 0x5,
  SND_CALL_LOCATION_COUNT = 0x6,
};

enum class SndEntityUpdate : uint32_t {
  SND_ENTITY_UPDATE_ALWAYS = 0x0,
  SND_ENTITY_UPDATE_NEVER = 0x1,
};

enum class SndMenuCategory : uint32_t {
  SND_CATEGORY_SFX = 0x0,
  SND_CATEGORY_MUSIC = 0x1,
  SND_CATEGORY_VOICE = 0x2,
  SND_CATEGORY_UI = 0x3,
  SND_CATEGORY_CINEMATIC = 0x4,
  SND_CATEGORY_COUNT = 0x5,
};

#pragma pack(push, 1)
// sizeof==0x800
struct SndAssetBankHeader {
  uint32_t magic;
  uint32_t version;
  uint32_t entrySize;
  uint32_t checksumSize;
  uint32_t dependencySize;
  uint32_t entryCount;
  uint32_t dependencyCount;
  uint32_t pad32;
  int64_t fileSize;
  int64_t entryOffset;
  int64_t checksumOffset;
  uint8_t checksumChecksum[16];
  /*
    This may be a struct in its own right, but the type is unknown and
    unlabelled in engine, only stored as a 512-byte buffer. It does not seem
    to be used in BO3 alpha, at least directly with automatically decompiled
    struct field indexing, so it may simply just be a field leftover from
    earlier implementations of the engine's sound architecture.
  */
  uint8_t dependencies[512];
  int64_t SourceChecksumOffset;
  int64_t AssetNameOffset;
  char zoneName[64];
  char platform[8];
  char language[2];
  uint32_t convertedAssetVersion;
  /*
    These padding fields are used and labelled explicitly in both BO3
    and BO4 engines. These are not our additions - this is, verbatim, what is
    used by Treyarch.
  */
  uint32_t padding0;
  uint8_t padding[1366];
};
ASSERT_SIZE(SndAssetBankHeader, 0x800);
#pragma pack(pop)

/*
  This struct is not 100% verified to be correct yet, but the fields
  marked as correct have been verified, and the rest are carry-overs from that
  used in BO4 alpha, which generally has engine SND architecture and data
  structure implementations most closely matching the latest versions seen in
  BO3 client and dedicated server.
*/
#pragma pack(push, 1)
struct SndAssetBankEntry {
  uint32_t id; // correct
  uint32_t size;
  uint32_t frameCount; // correct
  uint32_t order;
  uint64_t offset;
  uint8_t frameRateIndex; // correct
  uint8_t channelCount;
  uint8_t looping;
  uint8_t format;
  uint8_t EnvelopeLoudness0;
  uint8_t EnvelopeLoudness1;
  uint8_t EnvelopeLoudness2;
  uint8_t EnvelopeLoudness3;
  uint16_t EnvelopeTime1;
  uint16_t EnvelopeTime2;
};
ASSERT_SIZE(SndAssetBankEntry, 0x24);
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x918
struct SndAssetBankLoad {
  SndAssetBankHeader header;
  char filename[256];
  SndAssetBankEntry *entries;
  uint32_t entryCount;
  stream::stream_fileid fileHandle;
  qboolean indicesLoaded;
  qboolean indicesAllocated;
};
ASSERT_SIZE(SndAssetBankLoad, 0x918);
#pragma pack(pop)

#define G_SND_INITIALIZED_MAGIC 0x23459876

#pragma pack(push, 1)
struct SndVolumeGroup {
  name_t name;
  name_t parentName;
  uint32_t id;
  int32_t parentIndex;
  SndMenuCategory category;
  uint16_t attenuationSp;
  uint16_t attenuationMp;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCurve {
  name_t name;
  uint32_t id;
  game::vec2_t points[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndDuckGroup {
  name_t name;
  uint32_t id;
};
#pragma pack(pop)

// Unverified
#pragma pack(push, 1)
struct SndMaster {
  char name[64];
  SndStringHash id;
  int32_t lowE;
  float lowG;
  float lowF;
  float lowQ;
  int32_t peak1E;
  float peak1G;
  float peak1F;
  float peak1Q;
  int32_t peak2E;
  float peak2G;
  float peak2F;
  float peak2Q;
  int32_t hiE;
  float hiG;
  float hiF;
  float hiQ;
  float eqG;
  int32_t compE;
  float compPG;
  float compMG;
  float compT;
  float compR;
  float compTA;
  float compTR;
  int32_t compITU1770;
  int32_t limitE;
  float limitPG;
  float limitMG;
  float limitT;
  float limitR;
  float limitTA;
  float limitTR;
  int32_t limitITU1770;
  float busNearVerbG;
  float busFarVerbG;
  float busEarlyVerbG;
  float busNearverbG;
  float busFarverbG;
  float busFxG;
  float busVoiceG;
  float busPfutzG;
  float busHdrfxG;
  float busUiG;
  float busMusicG;
  float busMovieG;
  float busVcsG;
  float busMasterG;
  int32_t busNearVerbE;
  int32_t busFarVerbE;
  int32_t busEarlyVerbE;
  int32_t busNearverbE;
  int32_t busFarverbE;
  int32_t busFxE;
  int32_t busVoiceE;
  int32_t busPfutzE;
  int32_t busHdrfxE;
  int32_t busUiE;
  int32_t busMusicE;
  int32_t busMovieE;
  int32_t hdrfxCompE;
  int32_t voiceEqE;
  int32_t voiceCompE;
};

#pragma pack(pop)

#pragma pack(push, 1)
struct SndContext {
  uint32_t type;
  uint32_t values[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndPan {
  name_t name;
  uint32_t id;
  float front;
  float back;
  float center;
  float lfe;
  float left;
  float right;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndFutz {
  name_t name;
  uint32_t id;
  float bpfF;
  float bpfQ;
  float lsG;
  float lsF;
  float lsQ;
  float dist;
  float preG;
  float postG;
  float th;
  float tg;
  float clippre;
  float clippost;
  float blend;
  uint32_t startAliasId;
  uint32_t stopAliasId;
  uint32_t loopAliasId;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndDriverGlobals {
  const char *name;
  uint32_t groupCount;
  uint8_t _padding0C[4];
  SndVolumeGroup *groups;
  uint32_t curveCount;
  uint8_t _padding1C[4];
  SndCurve *curves;
  uint32_t panCount;
  uint8_t _padding2C[4];
  SndPan *pans;
  uint32_t duckGroupCount;
  uint8_t _padding3C[4];
  SndDuckGroup *duckGroups;
  uint32_t contextCount;
  uint8_t _padding4C[4];
  SndContext *contexts;
  uint32_t masterCount;
  uint8_t _padding5C[4];
  SndMaster *masters;
  uint32_t futzCount;
  uint8_t _padding6C[4];
  SndFutz *futzes;
};
#pragma pack(pop)

#pragma pack(push, 1)

struct SndOcclusionTrace {
  SndPlaybackId id;
  int32_t voiceIndex;
  vec3_t listener;
  vec3_t playback;
  int32_t cache[5];
  float occlusion;
  uint8_t padding[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndQueue;
typedef void (*SND_QueueCallback)(SndQueue *);
struct SndQueue {
  tlAtomicMutex mutex;
  cmd::SndCommandBuffer *active;
  cmd::SndCommandBuffer *submitted;
  cmd::SndQueueBuffers *buffers;
  SND_QueueCallback processNotify;
};
ASSERT_SIZE(SndQueue, 0x38);

#pragma pack(pop)

/*
  The SndLocal struct used in the latest BO3 client and dedicated server engines
  deviates significantly from the BO2 dedicated server, BO3 client, and BO4
  client alphas - in field ordering and count, total size, and that of quite a
  few of its fields which represent non-primitive data structures.

  We have not yet required a full reverse engineering of the struct, so it is
  left incomplete here, for now.

  Note that this is the only struct encountered in the BO3 client and dedicated
  server engines, thus far, that does not at least nearly match that used in
  either the BO4 alpha or BO3 alpha. All other data structures foundational to
  the engine's sound functionality closely match either the BO3 alpha or BO4
  alpha (most often the latter). For example:
   - `SndBankGlobals` - stored under g_sb, in engine - closely matches that used
  in BO4 alpha
   - `SndBankLoad` exactly matches that used in the BO4 alpha, with only one
  unknown, additional field, and additional padding added along with it for
  alignment.
  - `SndAssetBankHeader` matches that used in both BO3 and BO4 alpha.
   - `SndCommandType` matches the BO3 alpha exactly.


*/

/*
  Known to be 0x1CD100 bytes in size, but the fields and their order are
  not yet completely verified

  Progress:
  - EntState offset and length are verified to be correct.
  - In entState, offset of contexts is verified.
  - `pfutzLoops` offset is correct. Length of array (SndPlaybackId[8]) is also
  - verified.
  - Voice offset and length are verified.
  - Offset of playbackId in voice verified.
  - voiceAliasHash offset is verified.
*/
#define SND_LOCAL_SIZE 0x1CD168
static inline const size_t SND_LOCAL_CGQ_OFFSET =
    SND_LOCAL_SIZE - 2 * sizeof(SndQueue);
#pragma pack(push, 1)
partial_def(SND_LOCAL_SIZE, struct, SndLocal, {
  inline_partial_def(0, SND_LOCAL_CGQ_OFFSET, struct, {
    int32_t magic;
    qboolean init;
    qboolean paused;
    float timescale;
    float scriptTimescale;
    int32_t time;
    int32_t looptime;
    int32_t pausetime;
    uint32_t frame;
    uint8_t padding[4];
    const SndDriverGlobals *global_constants;
    int32_t cinematicVoicesPlaying;
    int32_t cinematicTimestamp;
    qboolean cinematicUpdate;
    qboolean forcePause;
    SndPlaybackId playbackIdCounter;
    SndStringHash defaultHash;
    const SndCurve *defaultCurve;
    uint32_t defaultPanIndex;
    uint32_t activeListenerCount;
    qboolean curveMagnitudeIsZero[32];
  });
  SndQueue CGQ;
  SndQueue SNDQ;
  /*
   Confirmed last fields:
   g_snd_cgq: len 56 == 0x38
   g_snd_sndq: len 56 == 0x38
   Struct in BO3 and BO4 alpha has 16 bytes of padding here, but
   there is no padding in latest version of engine - there is an unrelated
   global (function-specific, static variable) that is allocated just after
   SNDQ. Either an alignment change, or size of non-pointer field(s) change.
  */
});
static_assert(offsetof(SndLocal, CGQ) == SND_LOCAL_CGQ_OFFSET);
static_assert(offsetof(SndLocal, SNDQ) ==
              SND_LOCAL_CGQ_OFFSET + sizeof(SndQueue));
ASSERT_SIZE(SndLocal, SND_LOCAL_SIZE);
/*
  SndLocal has total size (0x1413568F0 - 0x141189800) == 0x1CD168.
  Confirmed by:
    - Struct allocation size
    - Real memory addresses of definite-last field in g_snd and definite-first
field in g_snd
*/
#pragma pack(pop)

// Not verified to be correct.
#pragma pack(push, 1)
struct SndAlias {
  const char *name;
  SndStringHash id;
  uint8_t _padding0C[4];
  const char *subtitle;
  const char *secondaryname;
  SndStringHash secondaryid;
  uint8_t _padding24[4];
  const char *stopAliasName;
  SndStringHash stopAliasID;
  scr::ScrString_t facialAnimation;
  SndStringHash assetId; // correct
  uint8_t _padding3C[4];
  const char *assetFileName;
  SndStringHash assetIdSustain;
  uint8_t _padding4C[4];
  const char *assetFileNameSustain;
  SndStringHash assetIdRelease;
  uint8_t _padding5C[4];
  const char *assetFileNameRelease;
  uint32_t flags0;
  uint32_t flags1;
  SndStringHash duck;
  SndStringHash contextType;
  SndStringHash contextValue;
  SndStringHash contextType1;
  SndStringHash contextValue1;
  SndStringHash contextType2;
  SndStringHash contextValue2;
  SndStringHash contextType3;
  SndStringHash contextValue3;
  SndStringHash stopOnPlay;
  SndStringHash futzPatch;
  float reverbSend;
  float centerSend;
  float volMin;
  float volMax;
  float envelopPercentage;
  uint16_t fluxTime;
  uint16_t startDelay;
  uint16_t pitchMin;
  uint16_t pitchMax;
  uint16_t distMin;
  uint16_t distMax;
  uint16_t distReverbMax;
  uint16_t envelopMin;
  uint16_t envelopMax;
  uint16_t distanceLpfMin;
  uint16_t distanceLpfMax;
  int16_t fadeIn;
  int16_t fadeOut;
  int16_t dopplerScale;
  uint8_t minPriorityThreshold;
  uint8_t maxPriorityThreshold;
  uint8_t probability;
  uint8_t occlusionLevel;
  uint8_t minPriority;
  uint8_t maxPriority;
  uint8_t pan;
  uint8_t limitCount;
  uint8_t entityLimitCount;
  uint8_t duckGroup;
  uint8_t bus;
  uint8_t _paddingD7[1];
};
#pragma pack(pop)

// correct
#pragma pack(push, 1)
struct SndIndexEntry {
  uint16_t value;
  uint16_t next;
};
#pragma pack(pop)

// correct
// sizeof=0x28
#pragma pack(push, 1)
struct SndAliasList : db::xasset::NamedXAsset {
  SndAliasId id;
  uint8_t _padding0C[4];
  SndAlias *head; // correct
  uint32_t count; // correct
  uint32_t sequence;
  float cullDistance;
  qboolean spatial;
};
#pragma pack(pop)
ASSERT_SIZE(SndAliasList, 0x28);

#pragma pack(push, 1)
// Correct
struct SndPatch : db::xasset::NamedXAsset {
  uint64_t elementCount;
  uint32_t *elements;
};
ASSERT_SIZE(SndPatch, 0x18);
#pragma pack(pop)

#pragma pack(push, 1)
struct SndAliasLookupNode {
  SndStringHash id;
  uint8_t _padding04[4];
  SndAliasList *list;
};
ASSERT_SIZE(SndAliasLookupNode, 0x10);
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0xFE20
struct SndAliasLookupCache {
  uint32_t hit;
  uint32_t miss;
  uint32_t collision;
  uint8_t _padding0C[4];
  SndAliasLookupNode cache[4065];
};
ASSERT_SIZE(SndAliasLookupCache, 0xFE20);
#pragma pack(pop)

enum class SndFileLoadingState : int32_t {
  SFLS_UNLOADED = 0x0,
  SFLS_LOADING = 0x1,
  SFLS_LOADED = 0x2,
};

#pragma pack(push, 1)
union SndEntHandle {
  struct {
    uint64_t entIndex : 12;
    uint64_t fxIndex : 16;
    uint64_t fxInUse : 1;
    uint64_t useCount : 5;
    uint64_t localClientNum : 3;
    uint64_t isStationary : 1;
    uint64_t isInKillCam : 1;
    uint64_t tag : 7;
    uint64_t pad : 15;
  } field;
  uint64_t handle;
};
#pragma pack(pop)

// Not yet known to be correct.
#pragma pack(push, 1)
struct SndAmbient {
  char name[64];
  SndStringHash id;
  qboolean defaultRoom;
  SndStringHash reverb;
  char reverbName[64];
  SndStringHash nearVerb;
  char nearVerbName[64];
  SndStringHash farVerb;
  char farVerbName[64];
  float reverbDryLevel;
  float reverbWetLevel;
  SndStringHash loop;
  char duck[64];
  SndStringHash duckId;
  SndStringHash entityContextType0;
  SndStringHash entityContextValue0;
  SndStringHash entityContextType1;
  SndStringHash entityContextValue1;
  SndStringHash entityContextType2;
  SndStringHash entityContextValue2;
  SndStringHash globalContextType;
  SndStringHash globalContextValue;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndEntState;
// Known correct.
struct SndEntState {
  SndStringHash contexts[64];
  SndEntHandle handle;
  SndEntState *next;
  const SndAmbient *ambientRoom;
  SndStringHash ambientRoomId;
  vec3_t orientation[3];
  vec3_t origin;
  vec3_t velocity;
  uint32_t lastUsed;
  uint32_t hasPosition;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndListener {
  SndEntHandle entHandle;
  orientation_t orient;
  ClientNum_t clientNum;
  ControllerIndex_t controllerNum;
  qboolean active;
  team_t team;
  float distanceTraveled;
  uint8_t _padding4C[4];
};
#pragma pack(pop)

// sizeof=0x88
#pragma pack(push, 1)
struct SndDuck {
  char name[64]; // Correct
  SndStringHash id;
  float fadeIn;
  float fadeOut;
  float startDelay;
  float distance;
  float length;
  SndStringHash fadeInCurve;
  SndStringHash fadeOutCurve;
  float *attenuation;
  float *filter;
  qboolean updateWhilePaused; // correct
  qboolean trackAmplitude;
  qboolean disableInSplitscreen; // correct
  float aliasAttenuation;
  float aliasLpf;
  SndStringHash duckAlias;
};
#pragma pack(pop)

struct SndVoice;

#pragma pack(push, 1)
struct SndDuckActive {
  const SndDuck *duck; // correct
  SndVoice *voice;     // correct
  SndDuckCategoryType category;
  vec3_t position;
  qboolean inUse; // correct
  int32_t startTime;
  int32_t endTime;
  int32_t pingTime;
  SndStringHash id;
  float amount;
  float effect;
  float amplitude;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndPlayState {
  SndEntHandle entHandle;
  vec3_t position;
  vec3_t direction;
  SndAliasId aliasId;
  int32_t fadeTimeMs;
  float startAttenuation;
  qboolean notify;
  qboolean startPaused;
  qboolean setScriptValues;
  float pitch;
  float pitchRate;
  float attenuation;
  float attenuationRate;
  uint32_t _unknown48;
  qboolean continueLoop;
  uint32_t magic;
  uint32_t fireTimeMs;
  SndCallLocation location;
  int32_t gpadFlags;
};
#pragma pack(pop)

typedef void *SndLengthNotifyData;
#pragma pack(push, 1)
struct SndLengthNotifyInfo {
  SndLengthType id[4];
  SndLengthNotifyData data[4];
  int32_t count;
  uint8_t _padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndFader {
  float value;
  float goal;
  float rate;
};
#pragma pack(pop)

#pragma pack(push, 1)

struct SndSpeakerMap {
  int32_t inputChannelCount;
  int32_t outputChannelCount;
  float volumes[16];
};

#pragma pack(pop)

// Correct.
#pragma pack(push, 1)
struct SndPlayback;
struct SndPlayback {
  SndPlaybackId id;
  float attenuation;
  int32_t lengthMs;
  int32_t playedMs;
  SndAliasId aliasId;
  int32_t use;
  SndEntHandle entHandle;
  SndPlayback *next;
};
ASSERT_SIZE(SndPlayback, 0x28);
#pragma pack(pop)

#pragma pack(push, 1)
// Length known correct, and all but the SndFade fields are verified to be
// correct, though not necessarily the fields contained within structs
// included by pointers here; see those structs for verification progress.
struct SndVoice {
  SndPlayState state;
  SndFileLoadingState loadingState;
  int32_t assetChannelCount;
  int32_t assetFrameRate;
  int32_t assetEndTime;
  int32_t assetTotalMsec;
  SndEntHandle sndEnt;
  SndEntityUpdate entity_update;
  int32_t group;
  int32_t startDelay;
  int32_t startTime;
  uint32_t startFrame;
  int32_t looptime;
  int32_t totalMsec;
  SndPlayback *playback;
  SndPlaybackId playbackId;
  SndPlaybackId firstPlaybackId;
  SndPlaybackId secondaryId;
  SndPlaybackId primaryId;
  SndLengthNotifyInfo lengthNotifyInfo;
  qboolean isDiscontinuous;
  const SndAlias *alias;
  vec3_t offset;
  vec3_t position;
  vec3_t velocity;
  vec3_t orientation[3];
  vec3_t direction;
  vec3_t fluxVelocity;
  float baseDistance;
  float distanceAttenuation;
  float reverbAttenuation;
  float earlyAttenuation;
  float volModSeed;
  float volModStart;
  float pitchModSeed;
  float globalPriority;
  float groupScalar;
  float nearFar;
  SndFader doppler;
  SndFader fade;
  SndFader script_fade;
  SndFader script_pitch;
  SndFader losOcclusion;
  SndFader duckAttenuation;
  SndFader duckLpf;
  int32_t traceCache[5];
  qboolean needsToStop;
  SndMenuCategory category;
  int32_t closestListenerIndex;
  int32_t voiceStartTime;
  float dryLevel;
  float nearLevel;
  float farLevel;
  float earlyLevel;
  float earlyTimes[4];
  float futzBlend;
  SndSpeakerMap pan;
  int64_t played;
  int64_t length;
  float amplitude;
  const SndDuck *duck;
  SndDuckActive *activeDuck;
  bool paused;
  bool startPaused;
  bool timescale;
  bool positionUpdated;
  bool pitchShift;
  bool aliasFadeOut;
  bool continueLoop;
  bool softStop;
  bool isSilent;
};

// Correct
enum class SndMusicStateStatus : uint32_t {
  SND_MUSIC_STATE_INACTIVE = 0x0,
  SND_MUSIC_STATE_ACTIVE = 0x1,
};

// Unverified.
// Size: 0x8C
struct SndMusicAsset {
  char alias[64];
  SndStringHash aliasId;
  qboolean looping;
  qboolean completeLoop;
  qboolean removeAfterPlay;
  qboolean firstRandom;
  qboolean startSync;
  qboolean stopSync;
  qboolean completeOnStop;
  uint32_t loopStartOffset;
  int32_t bpm;
  int32_t assetType;
  int32_t loopNumber;
  int32_t order;
  int32_t startDelayBeats;
  int32_t startFadeBeats;
  int32_t stopDelayBeats;
  int32_t stopFadeBeats;
  int32_t startOffsetFrames;
  int32_t meter;
};
ASSERT_SIZE(SndMusicAsset, 0x8C);

/*
  Mostly unverified.
  `name`, `id`, `loopCount`, `order`,
  `isSequential`, and `skipPreviousExit` are known to be correct.
*/
// sizeof=0x178
struct SndMusicState {
  char name[64];
  SndStringHash id;
  SndMusicAsset intro;
  SndMusicAsset exit;
  uint32_t loopCount;
  SndMusicAsset *loops;
  int32_t order;
  bool isRandom;
  bool isSequential;
  bool skipPreviousExit;
  uint8_t _padding16F[1];
  SndMusicStateStatus status;
  uint8_t _padding174[4];
};
ASSERT_SIZE(SndMusicState, 0x178);

// Unverified.
struct SndMusicSet {
  char name[64];
  SndStringHash id;
  uint32_t stateCount;
  SndMusicState *states;
};
ASSERT_SIZE(SndMusicSet, 0x50);

// Unverified.
struct SndAmbientBspVolume {
  uint32_t id;
  int32_t planeIndex;
  int32_t planeCount;
};
ASSERT_SIZE(SndAmbientBspVolume, 0xC);

// Unverified.
struct SndAmbientBspTrigger {
  uint32_t id;
  int32_t priority;
  SndStringHash roomId;
  char roomName[64];
};
ASSERT_SIZE(SndAmbientBspTrigger, 0x4C);

// Unverified.
// sizeof=0x20
struct SndAmbientBspNode {
  vec4_t split;
  int32_t frontIndex;
  int32_t backIndex;
  int32_t frontCount;
  int32_t backCount;
};
ASSERT_SIZE(SndAmbientBspNode, 0x20);

// sizeof=0x22020
struct SndAmbientBsp {
  SndAmbientBspNode nodes[512];
  int32_t numNodes;
  uint8_t _padding4004[12];
  vec4_t planes[2048];
  int32_t numPlanes;
  SndAmbientBspVolume volumes[1024];
  int32_t numVolumes;
  SndAmbientBspTrigger triggers[1024];
  int32_t numTriggers;
  uint8_t __padding2201C[4];
};
ASSERT_SIZE(SndAmbientBsp, 0x22020);

// Unverified.
struct SndReverbEarly {
  float preDelayMs;
  float inputLpf;
  float feedback;
  float smear;
  float baseDelayMs;
  float returnDB;
};
ASSERT_SIZE(SndReverbEarly, 0x18);

// Unverified.
struct SndReverbLate {
  float inputLpf;
  float feedback;
  float returnDB;
  float lowDamp;
  float highDamp;
  float decayTime;
  float smear;
  float preDelayMs;
};
ASSERT_SIZE(SndReverbLate, 0x20);

// Unverified.
struct SndReverb {
  char name[64];
  SndStringHash id;
  float masterReturn;
  SndReverbEarly earlyVerb;
  SndReverbLate nearVerb;
  SndReverbLate farVerb;
};
ASSERT_SIZE(SndReverb, 0xA0);

// Unverified. Most likely unchanged.
struct SndDialogScriptIdLookup {
  SndStringHash scriptId;
  SndStringHash aliasId;
};
ASSERT_SIZE(SndDialogScriptIdLookup, 0x8);

// Correct! Must be length 0x220B0.
struct SndBank : db::xasset::NamedXAsset {
  const char *zone;
  const char *gameLanguage;
  const char *soundLanguage;
  uint32_t aliasCount;
  uint8_t _padding24[4];
  SndAliasList *alias;
  SndIndexEntry *aliasIndex;
  uint32_t reverbCount;
  uint8_t _padding3C[4];
  SndReverb *reverbs;
  uint32_t duckCount;
  uint8_t _padding4C[4];
  SndDuck *ducks;
  uint32_t ambientCount;
  uint8_t _padding5C[4];
  SndAmbient *ambients;
  uint32_t scriptIdLookupCount;
  uint8_t _padding6C[4];
  SndDialogScriptIdLookup *scriptIdLookups;
  uint8_t _padding78[8];
  SndAmbientBsp ambientBsp;
  uint32_t musicSetCount;
  uint8_t _padding220A4[4];
  SndMusicSet *musicSets;
};
ASSERT_SIZE(SndBank, 0x220B0);
typedef SndBank *SndBankPtr;

#pragma pack(pop)

// Correct.

#pragma pack(push, 1)

// sizeof=x01278, verified
struct SndBankLoad {
  const SndBank *bank;
  SndAssetBankLoad streamAssetBank;
  SndAssetBankLoad loadAssetBank;
  SndAssetBankEntry *loadedEntries;
  sd_byte *loadedData;
  uint32_t loadedAssetCount;
  uint32_t loadedAssetTotal;
  uint32_t loadedEntryCount;
  uint32_t loadedDataSize;
  uint32_t priority;
  qboolean patchZone;
  uint8_t _unknown[4];
  stream::stream_id streamRequestId;
  qboolean pendingIo;
  qboolean ioError;
  SndBankState state;
  /*
     Added to allow multiple concurrent readers; previously read call back
     toggled pendingIO off on read completion even if there were other reads in
     progress.
  */
  uint32_t pendingIoCount;
};

ASSERT_SIZE(SndBankLoad, 0x1278);
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x39594
struct SndBankGlobals {
  uint32_t bankMagic;
  uint32_t bankCount;
  uint32_t patchCount;
  const SndBank *banks[16];
  const SndPatch *patches[8];
  SndAliasLookupCache aliasCache;
  SndStringHash missingAliasCache[4065];
  SndBankLoad loads[32];
  qboolean loadGate; // Verified correct <= here
  uint32_t assetLoadIndex;
  uint32_t assetLoadId;
  SndBankLoad *sequentialReadInProgress;
  SndAssetBankHeader bankHeader;
  uint32_t headerMagic;
  float assetLoadPercent;
  uint8_t _padding317F8[8];
};
ASSERT_SIZE(SndBankGlobals, 0x39594);
#pragma pack(pop)

// Not yet verified to be correct.
#pragma pack(push, 1)
struct snd_fire_manager {
  int active;
  vec3_t location;
  float level;
  int id;
};
#pragma pack(pop)
ASSERT_SIZE(snd_fire_manager, 0x18);

#pragma pack(push, 1)
struct snd_weapon_shot {
  LocalClientNum_t localClientNum;
  uint8_t _padding04[4];
  SndEntHandle shooter;
  weapon::Weapon weapon;
  vec3_t origin;
  vec3_t direction;
  uint32_t tagName;
  uint32_t burstCount;
  bool shooterIsPlayer;
  bool shooterIsLocalPlayer;
  bool leftGun;
  bool isLastShotInClip;
  bool isTurret;
  bool doubleTap;
  bool fakeFire;
  bool firstShotOfBurst;
};
ASSERT_SIZE(snd_weapon_shot, 0x40);

struct snd_autosim {
  snd_weapon_shot shot;
  uint32_t lastPing;
  uint32_t lastShot;
  uint32_t fireTime;
  uint16_t shotCount;
  uint16_t eventCount;
  int32_t used;
  int32_t isNew;
};
ASSERT_SIZE(snd_autosim, 0x58);

struct snd_autosim_play {
  uint32_t frame;
  uint8_t _padding04[4];
  snd_weapon_shot shot;
};
ASSERT_SIZE(snd_autosim_play, 0x48);

// sizeof=0x18
struct EntityImpactPositions {
  uint32_t entityImpactPositions[6];
};
ASSERT_SIZE(EntityImpactPositions, 0x18);

// sizeof=0x110
struct EntitySoundImpacts {
  const char *name;
  EntityImpactPositions entityImpacts[11];
};
ASSERT_SIZE(EntitySoundImpacts, 0x110);

// sizeof=0xA8
struct SurfaceSoundDef {
  const char *name;
  snd::SndAliasId surfaceSound[40];
};
ASSERT_SIZE(SurfaceSoundDef, 0xA8);

typedef SurfaceSoundDef *SurfaceSoundDefPtr;
typedef EntitySoundImpacts *EntitySoundImpactsPtr;
// sizeof=0x20
struct SoundsImpactTable {
  const char *name;
  SurfaceSoundDefPtr surfaceSoundTable;
  EntitySoundImpactsPtr entitySoundImpacts;
  EntitySoundImpactsPtr victimSoundImpacts;
};
ASSERT_SIZE(SoundsImpactTable, 0x20);

typedef SoundsImpactTable *SoundsImpactTablePtr;

// sizeof=0x20
struct SndEntLoop {
  SndAliasId id;
  uint8_t _padding04[4];
  SndEntHandle handle;
  vec3_t origin;
  int32_t fade;
};
ASSERT_SIZE(SndEntLoop, 0x20);

// sizeof=0x14
struct SndOcclusionStartCache {
  vec3_t position;
  float value;
  bool valid;
  uint8_t _padding11[3];
};
ASSERT_SIZE(SndOcclusionStartCache, 0x14);

enum class SndMusicAssetPlaybackState : int32_t {
  SND_MUSIC_PLAYBACK_STOPPING = 0x0,
  SND_MUSIC_PLAYBACK_STOPPED = 0x1,
  SND_MUSIC_PLAYBACK_STARTED = 0x2,
  SND_MUSIC_PLAYBACK_COUNT = 0x3,
};

// sizeof=0x28
struct SndMusicAssetInstance {
  const SndMusicAsset *asset;
  const SndMusicState *state;
  SndPlaybackId id;
  SndMusicAssetPlaybackState playbackState;
  int startFrame;
  int loopNumber;
  int queuedNextLoop;

  uint8_t _padding24[4];
};
ASSERT_SIZE(SndMusicAssetInstance, 0x28);
#pragma pack(pop)
#pragma pack(push, 1)

struct LoopSoundInfo {
  SndAliasId id;
  int16_t fade;
  uint8_t _padding06[2];
};
ASSERT_SIZE(LoopSoundInfo, 0x8);
#pragma pack(pop)

} // namespace snd
} // namespace game
#endif