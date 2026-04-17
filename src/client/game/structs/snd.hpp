#ifndef STRUCTS_SND_HPP
#define STRUCTS_SND_HPP

#include <cstdint>
#include "core.hpp"
#include "quake.hpp"
#include "db/xasset.hpp"
#include "scr.hpp"
#include "io.hpp"

namespace game {
namespace snd {

typedef uint32_t SndStringHash;
typedef int32_t SndPlaybackId;
typedef uint8_t sd_byte;
typedef uint32_t SndAliasId;

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
static_assert(sizeof(SndAssetBankHeader) == 0x800,
              "SndAssetBankHeader size must be 0x800 bytes");
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
static_assert(sizeof(SndAssetBankEntry) == 0x24,
              "SndAssetBankEntry size must be 32 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x918
struct SndAssetBankLoad {
  SndAssetBankHeader header;
  char filename[256];
  SndAssetBankEntry *entries;
  uint32_t entryCount;
  io::stream_fileid fileHandle;
  qboolean indicesLoaded;
  qboolean indicesAllocated;
};
static_assert(sizeof(SndAssetBankLoad) == 0x918,
              "SndAssetBankLoad size must be 0x918 bytes");
#pragma pack(pop)

#define G_SND_INITIALIZED_MAGIC 0x23459876

#pragma pack(push, 1)
struct SndVolumeGroup {
  char name[32];
  char parentName[32];
  uint32_t id;
  int32_t parentIndex;
  SndMenuCategory category;
  uint16_t attenuationSp;
  uint16_t attenuationMp;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndCurve {
  char name[32];
  uint32_t id;
  game::vec2_t points[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SndDuckGroup {
  char name[32];
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
  char name[32];
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
  char name[32];
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

// sizeof=0x18
class tlAtomicMutex {
public:
  uint64_t ThreadId;
  int LockCount;
  uint8_t _padding0C[4];
  tlAtomicMutex *ThisPtr;
};
static_assert(sizeof(tlAtomicMutex) == 0x18,
              "tlAtomicMutex size must be 24 bytes");

#pragma pack(pop)
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
static_assert(sizeof(SndCommandBuffer) == 0x8020,
              "SndCommandBuffer size must be 32768 bytes plus header");
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x80220
struct SndQueueBuffers {
  tlAtomicMutex mutex;
  SndCommandBuffer buffers[16];
  SndCommandBuffer *freeList;
};
static_assert(sizeof(SndQueueBuffers) == 0x80220,
              "SndQueueBuffers size must be 16 times the size of "
              "SndCommandBuffer plus header");
#pragma pack(pop)

struct SndQueue;
typedef void (*SND_QueueCallback)(SndQueue *);

#pragma pack(push, 1)
// sizeof=0x38
struct SndQueue {
  tlAtomicMutex mutex;
  SndCommandBuffer *active;
  SndCommandBuffer *submitted;
  SndQueueBuffers *buffers;
  SND_QueueCallback processNotify;
};
static_assert(sizeof(SndQueue) == 0x38, "SndQueue size must be 56 bytes");

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
  - `SndAssetBankHeader` matches that both that used in BO3 and BO4 alpha.
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
static_assert(sizeof(SndLocal) == SND_LOCAL_SIZE);
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
static_assert(sizeof(SndAliasList) == 0x28);

#pragma pack(push, 1)
// Correct
struct SndPatch : db::xasset::NamedXAsset {
  uint64_t elementCount;
  uint32_t *elements;
};
static_assert(sizeof(SndPatch) == 0x18);
#pragma pack(pop)

#pragma pack(push, 1)
struct SndAliasLookupNode {
  SndStringHash id;
  uint8_t _padding04[4];
  SndAliasList *list;
};
static_assert(sizeof(SndAliasLookupNode) == 0x10);
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
static_assert(sizeof(SndAliasLookupCache) == 0xFE20);
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

// Completely unknown if correct at this time.
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
  qboolean continueLoop;
  uint32_t magic;
  uint32_t fireTimeMs;
  SndCallLocation location;
  int32_t gpadFlags;
  uint8_t _padding[4];
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
static_assert(sizeof(SndPlayback) == 0x28);
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
  to strongly type this 'nopless' command ID handling.
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

#pragma pack(push, 1)

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
static_assert(sizeof(SndMusicAsset) == 0x8C, "SndMusicAsset size is incorrect");

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
static_assert(sizeof(SndMusicState) == 0x178,
              "SndMusicState size is incorrect");

// Unverified.
struct SndMusicSet {
  char name[64];
  SndStringHash id;
  uint32_t stateCount;
  SndMusicState *states;
};
static_assert(sizeof(SndMusicSet) == 0x50, "SndMusicSet size is incorrect");

// Unverified.
struct SndAmbientBspVolume {
  uint32_t id;
  int32_t planeIndex;
  int32_t planeCount;
};
static_assert(sizeof(SndAmbientBspVolume) == 0xC,
              "SndAmbientBspVolume size is incorrect");

// Unverified.
struct SndAmbientBspTrigger {
  uint32_t id;
  int32_t priority;
  SndStringHash roomId;
  char roomName[64];
};
static_assert(sizeof(SndAmbientBspTrigger) == 0x4C,
              "SndAmbientBspTrigger size is incorrect");

// Unverified.
// sizeof=0x20
struct SndAmbientBspNode {
  vec4_t split;
  int32_t frontIndex;
  int32_t backIndex;
  int32_t frontCount;
  int32_t backCount;
};
static_assert(sizeof(SndAmbientBspNode) == 0x20,
              "SndAmbientBspNode size is incorrect");

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
static_assert(sizeof(SndAmbientBsp) == 0x22020,
              "SndAmbientBsp size is incorrect");

// Unverified.
struct SndReverbEarly {
  float preDelayMs;
  float inputLpf;
  float feedback;
  float smear;
  float baseDelayMs;
  float returnDB;
};
static_assert(sizeof(SndReverbEarly) == 0x18,
              "SndReverbEarly size is incorrect");

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
static_assert(sizeof(SndReverbLate) == 0x20, "SndReverbLate size is incorrect");

// Unverified.
struct SndReverb {
  char name[64];
  SndStringHash id;
  float masterReturn;
  SndReverbEarly earlyVerb;
  SndReverbLate nearVerb;
  SndReverbLate farVerb;
};
static_assert(sizeof(SndReverb) == 0xA0, "SndReverb size is incorrect");

// Unverified. Most likely unchanged.
struct SndDialogScriptIdLookup {
  SndStringHash scriptId;
  SndStringHash aliasId;
};
static_assert(sizeof(SndDialogScriptIdLookup) == 0x8,
              "SndDialogScriptIdLookup size is incorrect");

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
static_assert(sizeof(SndBank) == 0x220B0, "SndBank size is incorrect");
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
  io::stream_id streamRequestId;
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

static_assert(sizeof(SndBankLoad) == 0x1278, "SndBankLoad size is incorrect");
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
static_assert(sizeof(SndBankGlobals) == 0x39594,
              "SndBankGlobals size is incorrect");
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
static_assert(sizeof(snd_fire_manager) == 0x18,
              "snd_fire_manager size is incorrect");

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
static_assert(sizeof(snd_weapon_shot) == 0x40,
              "snd_weapon_shot size is incorrect");

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
static_assert(sizeof(snd_autosim) == 0x58, "snd_autosim size is incorrect");

struct snd_autosim_play {
  uint32_t frame;
  uint8_t _padding04[4];
  snd_weapon_shot shot;
};
static_assert(sizeof(snd_autosim_play) == 0x48,
              "snd_autosim_play size is incorrect");

// sizeof=0x18
struct EntityImpactPositions {
  uint32_t entityImpactPositions[6];
};
static_assert(sizeof(EntityImpactPositions) == 0x18,
              "EntityImpactPositions size must be 24 bytes");

// sizeof=0x110
struct EntitySoundImpacts {
  const char *name;
  EntityImpactPositions entityImpacts[11];
};
static_assert(sizeof(EntitySoundImpacts) == 0x110,
              "EntitySoundImpacts size must be 272 bytes");

// sizeof=0xA8
struct SurfaceSoundDef {
  const char *name;
  snd::SndAliasId surfaceSound[40];
};
static_assert(sizeof(SurfaceSoundDef) == 0xA8,
              "SurfaceSoundDef size must be 168 bytes");

typedef SurfaceSoundDef *SurfaceSoundDefPtr;
typedef EntitySoundImpacts *EntitySoundImpactsPtr;
// sizeof=0x20
struct SoundsImpactTable {
  const char *name;
  SurfaceSoundDefPtr surfaceSoundTable;
  EntitySoundImpactsPtr entitySoundImpacts;
  EntitySoundImpactsPtr victimSoundImpacts;
};
static_assert(sizeof(SoundsImpactTable) == 0x20,
              "SoundsImpactTable size must be 32 bytes");

typedef SoundsImpactTable *SoundsImpactTablePtr;

// sizeof=0x20
struct SndEntLoop {
  SndAliasId id;
  uint8_t _padding04[4];
  SndEntHandle handle;
  vec3_t origin;
  int32_t fade;
};
static_assert(sizeof(SndEntLoop) == 0x20, "SndEntLoop size must be 32 bytes");

// sizeof=0x14
struct SndOcclusionStartCache {
  vec3_t position;
  float value;
  bool valid;
  uint8_t _padding11[3];
};
static_assert(sizeof(SndOcclusionStartCache) == 0x14,
              "SndOcclusionStartCache size must be 20 bytes");

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
static_assert(sizeof(SndMusicAssetInstance) == 0x28,
              "SndMusicAssetInstance size must be 40 bytes");
#pragma pack(pop)

} // namespace snd
} // namespace game
#endif