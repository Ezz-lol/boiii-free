#pragma once

#include <game/structs/atomic.hpp>
#include <game/structs/core.hpp>
#include <game/structs/db/xasset/core.hpp>
#include <game/structs/quake/core.hpp>
#include <game/structs/scr/primitives.hpp>
#include <game/structs/stream.hpp>
#include <game/structs/weapon.hpp>

#include <cstdint>

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
  MULTIPLAYER = 0x0,
  ZOMBIES = 0x1,
  CAMPAIGN = 0x2,
  CAMPAIGN_ZOMBIES = 0x3,
};

inline constexpr const char *serialize(const SndGameMode mode) {
  switch (mode) {
  case SndGameMode::MULTIPLAYER:
    return "SND_MODE_MULTIPLAYER";
  case SndGameMode::ZOMBIES:
    return "SND_MODE_ZOMBIES";
  case SndGameMode::CAMPAIGN:
    return "SND_MODE_CAMPAIGN";
  case SndGameMode::CAMPAIGN_ZOMBIES:
    return "SND_MODE_CAMPAIGN_ZOMBIES";
  default:
    return "SND_MODE_INVALID";
  }
}

enum class SndStopSoundFlags : int32_t {
  ALL = 0x0,
  PAUSED = 0x1,
  DEATH = 0x2,
};

inline constexpr const char *serialize(const SndStopSoundFlags flags) {
  switch (flags) {
  case SndStopSoundFlags::ALL:
    return "SND_STOP_ALL";
  case SndStopSoundFlags::PAUSED:
    return "SND_STOP_PAUSED";
  case SndStopSoundFlags::DEATH:
    return "SND_STOP_DEATH";
  default:
    return "SND_STOP_INVALID";
  }
}

enum class SndGfutzLocation : int32_t {
  SCRIPT = 0x0,
  GDT = 0x1,
  COUNT = 0x2,
};

inline constexpr const char *serialize(const SndGfutzLocation loc) {
  switch (loc) {
  case SndGfutzLocation::SCRIPT:
    return "GFUTZ_SCRIPT";
  case SndGfutzLocation::GDT:
    return "GFUTZ_GDT";
  default:
    return "GFUTZ_INVALID";
  }
}

enum class SndLimitType : int32_t {
  NONE = 0x0,
  OLDEST = 0x1,
  REJECT = 0x2,
  PRIORITY = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const SndLimitType type) {
  switch (type) {
  case SndLimitType::NONE:
    return "LIMIT_NONE";
  case SndLimitType::OLDEST:
    return "LIMIT_OLDEST";
  case SndLimitType::REJECT:
    return "LIMIT_REJECT";
  case SndLimitType::PRIORITY:
    return "LIMIT_PRIORITY";
  default:
    return "LIMIT_INVALID";
  }
}

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

inline constexpr const char *serialize(const SndBankState state) {
  switch (state) {
  case SndBankState::SND_BANK_STATE_NEW:
    return "BANK_STATE_NEW";
  case SndBankState::SND_BANK_STATE_STREAM_HEADER:
    return "BANK_STATE_STREAM_HEADER";
  case SndBankState::SND_BANK_STATE_STREAM_TOC:
    return "BANK_STATE_STREAM_TOC";
  case SndBankState::SND_BANK_STATE_LOADED_HEADER:
    return "BANK_STATE_LOADED_HEADER";
  case SndBankState::SND_BANK_STATE_LOADED_TOC:
    return "BANK_STATE_LOADED_TOC";
  case SndBankState::SND_BANK_STATE_LOADED_ASSET_WAIT:
    return "BANK_STATE_LOADED_ASSET_WAIT";
  case SndBankState::SND_BANK_STATE_LOADED_ASSETS:
    return "BANK_STATE_LOADED_ASSETS";
  case SndBankState::SND_BANK_STATE_READY_TO_USE:
    return "BANK_STATE_READY_TO_USE";
  case SndBankState::SND_BANK_STATE_ERROR:
    return "BANK_STATE_ERROR";
  default:
    return "BANK_STATE_INVALID";
  }
}

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

inline constexpr const char *serialize(const SndDuckCategoryType type) {
  switch (type) {
  case SndDuckCategoryType::SND_DUCK_CATEGORY_ALIAS:
    return "DUCK_CAT_ALIAS";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_AMBIENT:
    return "DUCK_CAT_AMBIENT";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_SCRIPT:
    return "DUCK_CAT_SCRIPT";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_SHELLSHOCK:
    return "DUCK_CAT_SHELLSHOCK";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_BREATH:
    return "DUCK_CAT_BREATH";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_ADS:
    return "DUCK_CAT_ADS";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_MENU:
    return "DUCK_CAT_MENU";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_ENEMIES:
    return "DUCK_CAT_ENEMIES";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_HEALTH:
    return "DUCK_CAT_HEALTH";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_DEBUG:
    return "DUCK_CAT_DEBUG";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_LEVELFADE:
    return "DUCK_CAT_LEVELFADE";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_MOVIE:
    return "DUCK_CAT_MOVIE";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_HIPFIRE:
    return "DUCK_CAT_HIPFIRE";
  case SndDuckCategoryType::SND_DUCK_CATEGORY_XCAM:
    return "DUCK_CAT_XCAM";
  default:
    return "DUCK_CAT_INVALID";
  }
}

enum class SndLengthType : uint32_t {
  NONE = 0x0,
  SCRIPT = 0x1,
  SUBTITLE = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const SndLengthType type) {
  switch (type) {
  case SndLengthType::NONE:
    return "LENGTH_NONE";
  case SndLengthType::SCRIPT:
    return "LENGTH_SCRIPT";
  case SndLengthType::SUBTITLE:
    return "LENGTH_SUBTITLE";
  default:
    return "LENGTH_INVALID";
  }
}

enum class SndCallLocation : int32_t {
  CLIENTSCRIPT = 0x0,
  SERVER = 0x1,
  GDT = 0x2,
  FX = 0x3,
  CODE = 0x4,
  UNKNOWN = 0x5,
  COUNT = 0x6,
};

inline constexpr const char *serialize(const SndCallLocation loc) {
  switch (loc) {
  case SndCallLocation::CLIENTSCRIPT:
    return "CALL_CLIENTSCRIPT";
  case SndCallLocation::SERVER:
    return "CALL_SERVER";
  case SndCallLocation::GDT:
    return "CALL_GDT";
  case SndCallLocation::FX:
    return "CALL_FX";
  case SndCallLocation::CODE:
    return "CALL_CODE";
  case SndCallLocation::UNKNOWN:
    return "CALL_UNKNOWN";
  default:
    return "CALL_INVALID";
  }
}

enum class SndEntityUpdate : uint32_t {
  ALWAYS = 0x0,
  NEVER = 0x1,
};

inline constexpr const char *serialize(const SndEntityUpdate update) {
  switch (update) {
  case SndEntityUpdate::ALWAYS:
    return "ENTITY_UPDATE_ALWAYS";
  case SndEntityUpdate::NEVER:
    return "ENTITY_UPDATE_NEVER";
  default:
    return "ENTITY_UPDATE_INVALID";
  }
}

enum class SndMenuCategory : uint32_t {
  SFX = 0x0,
  MUSIC = 0x1,
  VOICE = 0x2,
  UI = 0x3,
  CINEMATIC = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const SndMenuCategory cat) {
  switch (cat) {
  case SndMenuCategory::SFX:
    return "MENU_CAT_SFX";
  case SndMenuCategory::MUSIC:
    return "MENU_CAT_MUSIC";
  case SndMenuCategory::VOICE:
    return "MENU_CAT_VOICE";
  case SndMenuCategory::UI:
    return "MENU_CAT_UI";
  case SndMenuCategory::CINEMATIC:
    return "MENU_CAT_CINEMATIC";
  default:
    return "MENU_CAT_INVALID";
  }
}

enum class SndAssetBankVersion : uint32_t {
  V15 = 0x15,
};

inline constexpr const char *serialize(const SndAssetBankVersion ver) {
  switch (ver) {
  case SndAssetBankVersion::V15:
    return "VERSION_V15";
  default:
    return "VERSION_INVALID";
  }
}

constexpr std::string_view SndAssetFormatNames[] = {
    "PCMS16",  "PCMS24", "PCMS32", "IEEE",      "XMA4", "MP3",
    "MSADPCM", "WMA",    "FLAC",   "WIIUADPCM", "MPC",
};

PACKED(struct SndAssetBankHeader {
  uint32_t magic;
  SndAssetBankVersion version;
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
    This may be a struct in its own right, but the type is unknown
    and unlabelled in engine, only stored as a 512-byte buffer. It does not seem
    to be used in BO3 alpha, at least directly with automatically decompiled
    struct field indexing, so it may simply just be a field leftover
    from earlier implementations of the engine's sound architecture.
  */
  uint8_t dependencies[512];
  int64_t SourceChecksumOffset;
  int64_t AssetNameOffset;
  zoneName_t zoneName;
  str<8> platform;
  localeAbbrev_t language;
  uint32_t convertedAssetVersion;
  /*
    These padding fields are used and labelled explicitly in both BO3
    and BO4 engines. These are not our additions - this is, verbatim, what is
    used by Treyarch.
  */
  uint32_t padding0;
  uint8_t padding[1366];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"magic\": 0x{:08X}, \"version\": {}, \"entrySize\": {}, "
        "\"checksumSize\": {}, "
        "\"dependencySize\": {}, \"entryCount\": {}, \"dependencyCount\": {}, "
        "\"pad32\": 0x{:08X}, "
        "\"fileSize\": {}, \"entryOffset\": {}, \"checksumOffset\": {}, "
        "\"checksumChecksum\": {}, "
        "\"dependencies\": {}, \"SourceChecksumOffset\": {}, "
        "\"AssetNameOffset\": {}, \"zoneName\": {}, "
        "\"platform\": {}, \"language\": {}, \"convertedAssetVersion\": "
        "{:08X}, \"padding0\": 0x{:08X} }}",
        magic, game::snd::serialize(version), entrySize, checksumSize,
        dependencySize, entryCount, dependencyCount, pad32, fileSize,
        entryOffset, checksumOffset, ::serialize(checksumChecksum),
        ::serialize(dependencies), SourceChecksumOffset, AssetNameOffset,
        ::serialize(zoneName), ::serialize(platform), ::serialize(language),
        convertedAssetVersion, padding0);
  }
});
ASSERT_SIZE(SndAssetBankHeader, 0x800);

/*
  This struct is not 100% verified to be correct yet, but the fields
  marked as correct have been verified, and the rest are carry-overs from that
  used in BO4 alpha, which generally has engine SND architecture and data
  structure implementations most closely matching the latest versions seen in
  BO3 client and dedicated server.
*/
PACKED(struct SndAssetBankEntry {
  uint32_t id; // Verified
  uint32_t size;
  uint32_t frameCount; // Verified
  uint32_t order;
  uint64_t offset;
  uint8_t frameRateIndex; // Verified
  uint8_t channelCount;
  uint8_t looping;
  uint8_t format;
  uint8_t EnvelopeLoudness0;
  uint8_t EnvelopeLoudness1;
  uint8_t EnvelopeLoudness2;
  uint8_t EnvelopeLoudness3;
  uint16_t EnvelopeTime1;
  uint16_t EnvelopeTime2;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"id\": 0x{:08X}, \"size\": {}, \"frameCount\": {}, \"order\": {}, "
        "\"offset\": {}, \"frameRateIndex\": {}, \"channelCount\": {}, "
        "\"looping\": {}, "
        "\"format\": {}, \"EnvelopeLoudness0\": {}, \"EnvelopeLoudness1\": {}, "
        "\"EnvelopeLoudness2\": {}, \"EnvelopeLoudness3\": {}, "
        "\"EnvelopeTime1\": {}, "
        "\"EnvelopeTime2\": {} }}",
        id, size, frameCount, order, offset, frameRateIndex, channelCount,
        looping, format, EnvelopeLoudness0, EnvelopeLoudness1,
        EnvelopeLoudness2, EnvelopeLoudness3, EnvelopeTime1, EnvelopeTime2);
  }
});
ASSERT_SIZE(SndAssetBankEntry, 0x24);

PACKED(struct SndAssetBankLoad {
  SndAssetBankHeader header;
  str<256> filename;
  SndAssetBankEntry *entries;
  uint32_t entryCount;
  stream::stream_fileid fileHandle;
  qboolean indicesLoaded;
  qboolean indicesAllocated;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"header\": {}, \"filename\": {}, \"entries\": "
                       "{}, \"entryCount\": {}, "
                       "\"fileHandle\": 0x{:08X}, \"indicesLoaded\": {}, "
                       "\"indicesAllocated\": {} }}",
                       header.serialize(), ::serialize(filename),
                       ::serialize(entries, entryCount), entryCount, fileHandle,
                       indicesLoaded.serialize(), indicesAllocated.serialize());
  }
});
ASSERT_SIZE(SndAssetBankLoad, 0x918);

#define G_SND_INITIALIZED_MAGIC 0x23459876

PACKED(struct SndVolumeGroup {
  name_t name;
  name_t parentName;
  uint32_t id;
  int32_t parentIndex;
  SndMenuCategory category;
  uint16_t attenuationSp;
  uint16_t attenuationMp;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"parentName\": {}, \"id\": 0x{:08X}, "
        "\"parentIndex\": "
        "{}, "
        "\"category\": {}, \"attenuationSp\": {}, \"attenuationMp\": {} }}",
        ::serialize(name), ::serialize(parentName), id, parentIndex,
        game::snd::serialize(category), attenuationSp, attenuationMp);
  }
});

PACKED(struct SndCurve {
  name_t name;
  uint32_t id;
  array<game::vec2_t, 8> points;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"id\": 0x{:08X}, \"points\": {} }}",
                       ::serialize(name), id, ::serialize(points));
  }
});

PACKED(struct SndDuckGroup {
  name_t name;
  uint32_t id;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"id\": 0x{:08X} }}",
                       ::serialize(name), id);
  }
});

// Unverified
PACKED(struct SndMaster {
  str64_t name;
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"lowE\": {}, \"lowG\": {}, "
        "\"lowF\": {}, \"lowQ\": {}, \"peak1E\": {}, \"peak1G\": {}, "
        "\"peak1F\": {}, \"peak1Q\": {}, \"peak2E\": {}, \"peak2G\": {}, "
        "\"peak2F\": {}, \"peak2Q\": {}, \"hiE\": {}, \"hiG\": {}, "
        "\"hiF\": {}, \"hiQ\": {}, \"eqG\": {}, \"compE\": {}, "
        "\"compPG\": {}, \"compMG\": {}, \"compT\": {}, \"compR\": {}, "
        "\"compTA\": {}, \"compTR\": {}, \"compITU1770\": {}, \"limitE\": {}, "
        "\"limitPG\": {}, \"limitMG\": {}, \"limitT\": {}, \"limitR\": {}, "
        "\"limitTA\": {}, \"limitTR\": {}, \"limitITU1770\": {}, "
        "\"busNearVerbG\": {}, "
        "\"busFarVerbG\": {}, \"busEarlyVerbG\": {}, \"busNearverbG\": {}, "
        "\"busFarverbG\": {}, "
        "\"busFxG\": {}, \"busVoiceG\": {}, \"busPfutzG\": {}, \"busHdrfxG\": "
        "{}, "
        "\"busUiG\": {}, \"busMusicG\": {}, \"busMovieG\": {}, \"busVcsG\": "
        "{}, "
        "\"busMasterG\": {}, \"busNearVerbE\": {}, \"busFarVerbE\": {}, "
        "\"busEarlyVerbE\": {}, "
        "\"busNearverbE\": {}, \"busFarverbE\": {}, \"busFxE\": {}, "
        "\"busVoiceE\": {}, "
        "\"busPfutzE\": {}, \"busHdrfxE\": {}, \"busUiE\": {}, \"busMusicE\": "
        "{}, "
        "\"busMovieE\": {}, \"hdrfxCompE\": {}, \"voiceEqE\": {}, "
        "\"voiceCompE\": {} }}",
        ::serialize(name), id, lowE, lowG, lowF, lowQ, peak1E, peak1G, peak1F,
        peak1Q, peak2E, peak2G, peak2F, peak2Q, hiE, hiG, hiF, hiQ, eqG, compE,
        compPG, compMG, compT, compR, compTA, compTR, compITU1770, limitE,
        limitPG, limitMG, limitT, limitR, limitTA, limitTR, limitITU1770,
        busNearVerbG, busFarVerbG, busEarlyVerbG, busNearverbG, busFarverbG,
        busFxG, busVoiceG, busPfutzG, busHdrfxG, busUiG, busMusicG, busMovieG,
        busVcsG, busMasterG, busNearVerbE, busFarVerbE, busEarlyVerbE,
        busNearverbE, busFarverbE, busFxE, busVoiceE, busPfutzE, busHdrfxE,
        busUiE, busMusicE, busMovieE, hdrfxCompE, voiceEqE, voiceCompE);
  }
});

PACKED(struct SndContext {
  uint32_t type;
  uint32_t values[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"type\": 0x{:08X}, \"values\": {} }}", type,
                       ::serialize(values));
  }
});

PACKED(struct SndPan {
  name_t name;
  uint32_t id;
  float front;
  float back;
  float center;
  float lfe;
  float left;
  float right;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"front\": {}, \"back\": {}, "
        "\"center\": {}, \"lfe\": {}, \"left\": {}, \"right\": {} }}",
        ::serialize(name), id, front, back, center, lfe, left, right);
  }
});

PACKED(struct SndFutz {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"bpfF\": {}, \"bpfQ\": {}, "
        "\"lsG\": {}, \"lsF\": {}, \"lsQ\": {}, \"dist\": {}, "
        "\"preG\": {}, \"postG\": {}, \"th\": {}, \"tg\": {}, "
        "\"clippre\": {}, \"clippost\": {}, \"blend\": {}, \"startAliasId\": "
        "{:08X}, "
        "\"stopAliasId\": 0x{:08X}, \"loopAliasId\": 0x{:08X} }}",
        ::serialize(name), id, bpfF, bpfQ, lsG, lsF, lsQ, dist, preG, postG, th,
        tg, clippre, clippost, blend, startAliasId, stopAliasId, loopAliasId);
  }
});

PACKED(struct SndDriverGlobals {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"groupCount\": {}, \"groups\": {}, "
        "\"curveCount\": {}, "
        "\"curves\": {}, \"panCount\": {}, \"pans\": {}, "
        "\"duckGroupCount\": {}, "
        "\"duckGroups\": {}, \"contextCount\": {}, \"contexts\": {}, "
        "\"masterCount\": {}, "
        "\"masters\": {}, \"futzCount\": {}, \"futzes\": {} }}",
        ::serialize(name), groupCount, ::serialize(groups, groupCount),
        curveCount, ::serialize(curves, curveCount), panCount,
        ::serialize(pans, panCount), duckGroupCount,
        ::serialize(duckGroups, duckGroupCount), contextCount,
        ::serialize(contexts, contextCount), masterCount,
        ::serialize(masters, masterCount), futzCount,
        ::serialize(futzes, futzCount));
  }
});
ASSERT_SIZE(SndDriverGlobals, 0x78);

PACKED(struct SndOcclusionTrace {
  SndPlaybackId id;
  int32_t voiceIndex;
  vec3_t listener;
  vec3_t playback;
  int32_t cache[5];
  float occlusion;
  uint8_t _padding38[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"id\": {}, \"voiceIndex\": {}, \"listener\": {}, "
                       "\"playback\": {}, "
                       "\"cache\": {}, \"occlusion\": {} }}",
                       id, voiceIndex, listener.serialize(),
                       playback.serialize(), ::serialize(cache), occlusion);
  }
});
ASSERT_SIZE(SndOcclusionTrace, 0x40);

struct SndQueue;
typedef fastcallPtr_t<void(SndQueue *queue)> SND_QueueCallback;
PACKED(struct SndQueue {
  tlAtomicMutex mutex;
  cmd::SndCommandBuffer *active;
  cmd::SndCommandBuffer *submitted;
  cmd::SndQueueBuffers *buffers;
  SND_QueueCallback processNotify;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"active\": {:p}, \"submitted\": {:p}, \"buffers\": "
                       "{:p}, \"processNotify\": {:p} }}",
                       reinterpret_cast<const void *>(active),
                       reinterpret_cast<const void *>(submitted),
                       reinterpret_cast<const void *>(buffers),
                       reinterpret_cast<const void *>(processNotify));
  }
});
ASSERT_SIZE(SndQueue, 0x38);

/*
  The SndLocal struct used in the latest BO3 client and dedicated
  server engines deviates significantly from the BO2 dedicated server, BO3
  client, and BO4 client alphas - in field ordering and count, total size, and
  that of quite a few of its fields which represent non-primitive data
  structures.

  We have not yet required a full reverse engineering of the struct, so it is
  left incomplete here, for now.

  Note that this is the only struct encountered in the BO3 client and
  dedicated server engines, thus far, that does not at least nearly match that
  used in either the BO4 alpha or BO3 alpha. All other data structures
  foundational to the engine's sound functionality closely match either the BO3
  alpha or BO4 alpha (most often the latter). For example:
   - `SndBankGlobals` - stored under g_sb, in engine - closely matches that used
  in BO4 alpha
   - `SndBankLoad` exactly matches that used in the BO4 alpha, with only one
     additional field - pendingIoCount - and additional alignment padding added
     correspondingly.
   - `SndAssetBankHeader` matches that used in both BO3 and BO4 alpha.
   - `SndCommandType` matches the BO3 alpha exactly.


*/
/*
  Known to be 0x1CD100 bytes in size, but the fields and their order are
  not yet completely verified.

  Progress:
  - EntState offset and length are verified.
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
PACKED(partial_def(SND_LOCAL_SIZE, struct, SndLocal, {
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
}));
ASSERT_OFFSET(SndLocal, CGQ, SND_LOCAL_CGQ_OFFSET);
ASSERT_OFFSET(SndLocal, SNDQ, SND_LOCAL_CGQ_OFFSET + sizeof(SndQueue));
/*
  SndLocal has total size (0x1413568F0 - 0x141189800) == 0x1CD168.
  Confirmed by:
    - Struct allocation size
    - Real memory addresses of definite-last field in g_snd and definite-first
      field in g_snd
*/
ASSERT_SIZE(SndLocal, SND_LOCAL_SIZE);

// Unverified
PACKED(struct SndAlias {
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
  SndStringHash assetId; // Verified
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"subtitle\": {}, "
        "\"secondaryname\": "
        "{}, "
        "\"secondaryid\": 0x{:08X}, \"stopAliasName\": {}, \"stopAliasID\": "
        "{:08X}, "
        "\"facialAnimation\": 0x{:08X}, \"assetId\": 0x{:08X}, "
        "\"assetFileName\": "
        "{}, "
        "\"assetIdSustain\": 0x{:08X}, \"assetFileNameSustain\": {}, "
        "\"assetIdRelease\": 0x{:08X}, "
        "\"assetFileNameRelease\": {}, \"flags0\": 0x{:08X}, \"flags1\": "
        "0x{:08X}, "
        "\"duck\": 0x{:08X}, \"contextType\": 0x{:08X}, \"contextValue\": "
        "0x{:08X}, "
        "\"contextType1\": 0x{:08X}, \"contextValue1\": 0x{:08X}, "
        "\"contextType2\": 0x{:08X}, "
        "\"contextValue2\": 0x{:08X}, \"contextType3\": 0x{:08X}, "
        "\"contextValue3\": 0x{:08X}, "
        "\"stopOnPlay\": 0x{:08X}, \"futzPatch\": 0x{:08X}, \"reverbSend\": "
        "{}, "
        "\"centerSend\": {}, \"volMin\": {}, \"volMax\": {}, "
        "\"envelopPercentage\": {}, "
        "\"fluxTime\": {}, \"startDelay\": {}, \"pitchMin\": {}, \"pitchMax\": "
        "{}, "
        "\"distMin\": {}, \"distMax\": {}, \"distReverbMax\": {}, "
        "\"envelopMin\": {}, "
        "\"envelopMax\": {}, \"distanceLpfMin\": {}, \"distanceLpfMax\": {}, "
        "\"fadeIn\": {}, "
        "\"fadeOut\": {}, \"dopplerScale\": {}, \"minPriorityThreshold\": {}, "
        "\"maxPriorityThreshold\": {}, "
        "\"probability\": {}, \"occlusionLevel\": {}, \"minPriority\": {}, "
        "\"maxPriority\": {}, "
        "\"pan\": {}, \"limitCount\": {}, \"entityLimitCount\": {}, "
        "\"duckGroup\": {}, \"bus\": {} }}",
        ::serialize(name), id, ::serialize(subtitle),
        ::serialize(secondaryname), secondaryid, ::serialize(stopAliasName),
        stopAliasID, facialAnimation, assetId, ::serialize(assetFileName),
        assetIdSustain, ::serialize(assetFileNameSustain), assetIdRelease,
        ::serialize(assetFileNameRelease), flags0, flags1, duck, contextType,
        contextValue, contextType1, contextValue1, contextType2, contextValue2,
        contextType3, contextValue3, stopOnPlay, futzPatch, reverbSend,
        centerSend, volMin, volMax, envelopPercentage, fluxTime, startDelay,
        pitchMin, pitchMax, distMin, distMax, distReverbMax, envelopMin,
        envelopMax, distanceLpfMin, distanceLpfMax, fadeIn, fadeOut,
        dopplerScale, minPriorityThreshold, maxPriorityThreshold, probability,
        occlusionLevel, minPriority, maxPriority, pan, limitCount,
        entityLimitCount, duckGroup, bus);
  }
});

// Verified
PACKED(struct SndIndexEntry {
  uint16_t value;
  uint16_t next;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"value\": {}, \"next\": {} }}", value, next);
  }
});

// Verified
PACKED(struct SndAliasList : public db::xasset::NamedXAsset {
  SndAliasId id;
  uint8_t _padding0C[4];
  SndAlias *head; // Verified
  uint32_t count; // Verified
  uint32_t sequence;
  float cullDistance;
  qboolean spatial;

  /*
     Bit of an odd serialization, because the fields printed are not _exactly_
     the same as the struct. Instead of printing `head`, we print the linked
     list as field named `list`. Using `head` for the field name containing the
     entire linked list seemed more confusing than using an incorrect field
     name.
  */
  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"id\": 0x{:08X}, \"list\": {}, "
                       "\"count\": {}, \"sequence\": {}, "
                       "\"cullDistance\": {}, \"spatial\": {} }}",
                       ::serialize(name), id, ::serialize(head, count), count,
                       sequence, cullDistance, spatial.serialize());
  }
});
ASSERT_SIZE(SndAliasList, 0x28);

// Verified
PACKED(struct SndPatch : public db::xasset::NamedXAsset {
  uint64_t elementCount;
  uint32_t *elements;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"elementCount\": {}, \"elements\": {} }}",
        ::serialize(name), elementCount, ::serialize(elements, elementCount));
  }
});
ASSERT_SIZE(SndPatch, 0x18);

PACKED(struct SndAliasLookupNode {
  SndStringHash id;
  uint8_t _padding04[4];
  SndAliasList *list;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"id\": 0x{:08X}, \"list\": {} }}", id,
                       ::serialize(list));
  }
});
ASSERT_SIZE(SndAliasLookupNode, 0x10);

PACKED(struct SndAliasLookupCache {
  uint32_t hit;
  uint32_t miss;
  uint32_t collision;
  uint8_t _padding0C[4];
  SndAliasLookupNode cache[4065];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"hit\": {}, \"miss\": {}, \"collision\": {}, \"cache\": {} }}",
        hit, miss, collision, ::serialize(cache));
  }
});
ASSERT_SIZE(SndAliasLookupCache, 0xFE20);

enum class SndFileLoadingState : int32_t {
  UNLOADED = 0x0,
  LOADING = 0x1,
  LOADED = 0x2,
};

inline constexpr const char *serialize(const SndFileLoadingState state) {
  switch (state) {
  case SndFileLoadingState::UNLOADED:
    return "FILE_UNLOADED";
  case SndFileLoadingState::LOADING:
    return "FILE_LOADING";
  case SndFileLoadingState::LOADED:
    return "FILE_LOADED";
  default:
    return "FILE_INVALID";
  }
}

PACKED(union SndEntHandle {
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"handle\": 0x{:016X} }}", handle);
  }
});
ASSERT_SIZE(SndEntHandle, sizeof(uint64_t));

// Unverified
PACKED(struct SndAmbient {
  str64_t name;
  SndStringHash id;
  qboolean defaultRoom;
  SndStringHash reverb;
  str64_t reverbName;
  SndStringHash nearVerb;
  str64_t nearVerbName;
  SndStringHash farVerb;
  str64_t farVerbName;
  float reverbDryLevel;
  float reverbWetLevel;
  SndStringHash loop;
  str64_t duck;
  SndStringHash duckId;
  SndStringHash entityContextType0;
  SndStringHash entityContextValue0;
  SndStringHash entityContextType1;
  SndStringHash entityContextValue1;
  SndStringHash entityContextType2;
  SndStringHash entityContextValue2;
  SndStringHash globalContextType;
  SndStringHash globalContextValue;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"defaultRoom\": {}, \"reverb\": "
        "{:08X}, "
        "\"reverbName\": {}, \"nearVerb\": 0x{:08X}, \"nearVerbName\": {}, "
        "\"farVerb\": 0x{:08X}, "
        "\"farVerbName\": {}, \"reverbDryLevel\": {}, \"reverbWetLevel\": {}, "
        "\"loop\": 0x{:08X}, "
        "\"duck\": {}, \"duckId\": 0x{:08X}, \"entityContextType0\": 0x{:08X}, "
        "\"entityContextValue0\": 0x{:08X}, "
        "\"entityContextType1\": 0x{:08X}, \"entityContextValue1\": 0x{:08X}, "
        "\"entityContextType2\": 0x{:08X}, "
        "\"entityContextValue2\": 0x{:08X}, \"globalContextType\": 0x{:08X}, "
        "\"globalContextValue\": 0x{:08X} }}",
        ::serialize(name), id, defaultRoom.serialize(), reverb,
        ::serialize(reverbName), nearVerb, ::serialize(nearVerbName), farVerb,
        ::serialize(farVerbName), reverbDryLevel, reverbWetLevel, loop,
        ::serialize(duck), duckId, entityContextType0, entityContextValue0,
        entityContextType1, entityContextValue1, entityContextType2,
        entityContextValue2, globalContextType, globalContextValue);
  }
});

struct SndEntState;

// Verified
PACKED(struct SndEntState {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"contexts\": {}, \"handle\": {}, \"next\": {:p}, "
        "\"ambientRoom\": {:p}, "
        "\"ambientRoomId\": 0x{:08X}, \"orientation\": {}, \"origin\": {}, "
        "\"velocity\": {}, "
        "\"lastUsed\": {}, \"hasPosition\": 0x{:08X} }}",
        ::serialize(contexts), handle.serialize(),
        reinterpret_cast<const void *>(next),
        reinterpret_cast<const void *>(ambientRoom), ambientRoomId,
        ::serialize(orientation), origin.serialize(), velocity.serialize(),
        lastUsed, hasPosition);
  }
});

PACKED(struct SndListener {
  SndEntHandle entHandle;
  orientation_t orient;
  ClientNum_t clientNum;
  ControllerIndex_t controllerNum;
  qboolean active;
  team_t team;
  float distanceTraveled;
  uint8_t _padding4C[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"entHandle\": {}, \"orient\": {}, \"clientNum\": {}, "
        "\"controllerNum\": {}, "
        "\"active\": {}, \"team\": {}, \"distanceTraveled\": {} }}",
        entHandle.serialize(), orient.serialize(), game::serialize(clientNum),
        game::serialize(controllerNum), active.serialize(),
        game::serialize(team), distanceTraveled);
  }
});

PACKED(struct SndDuck {
  str64_t name; // Verified
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
  qboolean updateWhilePaused; // Verified
  qboolean trackAmplitude;
  qboolean disableInSplitscreen; // Verified
  float aliasAttenuation;
  float aliasLpf;
  SndStringHash duckAlias;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"fadeIn\": {}, \"fadeOut\": {}, "
        "\"startDelay\": {}, \"distance\": {}, \"length\": {}, "
        "\"fadeInCurve\": 0x{:08X}, "
        "\"fadeOutCurve\": 0x{:08X}, \"attenuation\": {}, \"filter\": {}, "
        "\"updateWhilePaused\": {}, "
        "\"trackAmplitude\": {}, \"disableInSplitscreen\": {}, "
        "\"aliasAttenuation\": {}, "
        "\"aliasLpf\": {}, \"duckAlias\": 0x{:08X} }}",
        ::serialize(name), id, fadeIn, fadeOut, startDelay, distance, length,
        fadeInCurve, fadeOutCurve, ::serialize(attenuation),
        ::serialize(filter), updateWhilePaused.serialize(),
        trackAmplitude.serialize(), disableInSplitscreen.serialize(),
        aliasAttenuation, aliasLpf, duckAlias);
  }
});

struct SndVoice;

PACKED(struct SndDuckActive {
  const SndDuck *duck; // Verified
  SndVoice *voice;     // Verified
  SndDuckCategoryType category;
  vec3_t position;
  qboolean inUse; // Verified
  int32_t startTime;
  int32_t endTime;
  int32_t pingTime;
  SndStringHash id;
  float amount;
  float effect;
  float amplitude;

  std::string serialize() const noexcept;
});

PACKED(struct SndPlayState {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"entHandle\": {}, \"position\": {}, \"direction\": {}, "
        "\"aliasId\": 0x{:08X}, "
        "\"fadeTimeMs\": {}, \"startAttenuation\": {}, \"notify\": {}, "
        "\"startPaused\": {}, "
        "\"setScriptValues\": {}, \"pitch\": {}, \"pitchRate\": {}, "
        "\"attenuation\": {}, "
        "\"attenuationRate\": {}, \"continueLoop\": {}, \"magic\": 0x{:08X}, "
        "\"fireTimeMs\": {}, "
        "\"location\": {}, \"gpadFlags\": 0x{:08X} }}",
        entHandle.serialize(), position.serialize(), direction.serialize(),
        aliasId, fadeTimeMs, startAttenuation, notify.serialize(),
        startPaused.serialize(), setScriptValues.serialize(), pitch, pitchRate,
        attenuation, attenuationRate, continueLoop.serialize(), magic,
        fireTimeMs, game::snd::serialize(location), gpadFlags);
  }
});
ASSERT_SIZE(SndPlayState, 0x60);

typedef void *SndLengthNotifyData;
PACKED(struct SndLengthNotifyInfo {
  SndLengthType id[4];
  SndLengthNotifyData data[4];
  int32_t count;
  uint8_t _padding34[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"id\": {}, \"data\": {}, \"count\": {} }}", ::serialize(id),
        ::serialize(reinterpret_cast<const uintptr_t *const>(data), count),
        count);
  }
});
ASSERT_SIZE(SndLengthNotifyInfo, 0x38);

PACKED(struct SndFader {
  float value;
  float goal;
  float rate;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"value\": {}, \"goal\": {}, \"rate\": {} }}", value,
                       goal, rate);
  }
});
ASSERT_SIZE(SndFader, 0xC);

PACKED(struct SndSpeakerMap {
  int32_t inputChannelCount;
  int32_t outputChannelCount;
  float volumes[16];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"inputChannelCount\": {}, \"outputChannelCount\": "
                       "{}, \"volumes\": {} }}",
                       inputChannelCount, outputChannelCount,
                       ::serialize(volumes));
  }
});
ASSERT_SIZE(SndSpeakerMap, 0x48);

// Verified
struct SndPlayback;
PACKED(struct SndPlayback {
  SndPlaybackId id;
  float attenuation;
  int32_t lengthMs;
  int32_t playedMs;
  SndAliasId aliasId;
  int32_t use;
  SndEntHandle entHandle;
  SndPlayback *next;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"id\": {}, \"attenuation\": {}, \"lengthMs\": {}, \"playedMs\": "
        "{}, "
        "\"aliasId\": 0x{:08X}, \"use\": {}, \"entHandle\": {}, \"next\": {} "
        "}}",
        id, attenuation, lengthMs, playedMs, aliasId, use,
        entHandle.serialize(), ::serialize(next));
  }
});
ASSERT_SIZE(SndPlayback, 0x28);

PACKED(struct SndVoice {
  SndPlayState state;
  SndFileLoadingState loadingState;
  int32_t assetChannelCount;
  int32_t assetFrameRate;
  int32_t assetEndTime;
  int32_t assetTotalMsec;
  uint8_t _padding6C[4];
  SndEntHandle sndEnt;
  SndEntityUpdate entity_update;
  int32_t group;
  int32_t startDelay;
  int32_t startTime;
  uint32_t startFrame;
  int32_t looptime;
  int32_t totalMsec;
  uint8_t _padding94[4];
  SndPlayback *playback;
  SndPlaybackId playbackId;
  SndPlaybackId firstPlaybackId;
  SndPlaybackId secondaryId;
  SndPlaybackId primaryId;
  SndLengthNotifyInfo lengthNotifyInfo;
  bool isDiscontinuous;
  uint8_t _paddingE9[7];
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
  bool needsToStop;
  uint8_t _padding1E9[3];
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
  uint8_t _padding264[4];
  int64_t played;
  int64_t length;
  float amplitude;
  uint8_t _padding27C[4];
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
  bool gpadSound;
  uint8_t _padding29A[2];
  int32_t gpadFlags;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"state\": {}, \"loadingState\": {}, \"assetChannelCount\": {}, "
        "\"assetFrameRate\": {}, "
        "\"assetEndTime\": {}, \"assetTotalMsec\": {}, \"sndEnt\": {}, "
        "\"entity_update\": {}, "
        "\"group\": {}, \"startDelay\": {}, \"startTime\": {}, \"startFrame\": "
        "{}, "
        "\"looptime\": {}, \"totalMsec\": {}, \"playback\": {}, "
        "\"playbackId\": {}, "
        "\"firstPlaybackId\": {}, \"secondaryId\": {}, \"primaryId\": {}, "
        "\"lengthNotifyInfo\": {}, "
        "\"isDiscontinuous\": {}, \"alias\": {}, \"offset\": {}, "
        "\"position\": {}, "
        "\"velocity\": {}, \"orientation\": {}, \"direction\": {}, "
        "\"fluxVelocity\": {}, "
        "\"baseDistance\": {}, \"distanceAttenuation\": {}, "
        "\"reverbAttenuation\": {}, \"earlyAttenuation\": {}, "
        "\"volModSeed\": {}, \"volModStart\": {}, \"pitchModSeed\": {}, "
        "\"globalPriority\": {}, "
        "\"groupScalar\": {}, \"nearFar\": {}, \"doppler\": {}, \"fade\": {}, "
        "\"script_fade\": {}, \"script_pitch\": {}, \"losOcclusion\": {}, "
        "\"duckAttenuation\": {}, "
        "\"duckLpf\": {}, \"traceCache\": {}, \"needsToStop\": {}, "
        "\"category\": {}, "
        "\"closestListenerIndex\": {}, \"voiceStartTime\": {}, \"dryLevel\": "
        "{}, \"nearLevel\": {}, "
        "\"farLevel\": {}, \"earlyLevel\": {}, \"earlyTimes\": {}, "
        "\"futzBlend\": {}, "
        "\"pan\": {}, \"played\": {}, \"length\": {}, \"amplitude\": {}, "
        "\"duck\": {}, \"activeDuck\": {}, \"paused\": {}, "
        "\"startPaused\": {}, "
        "\"timescale\": {}, \"positionUpdated\": {}, \"pitchShift\": {}, "
        "\"aliasFadeOut\": {}, "
        "\"continueLoop\": {}, \"softStop\": {}, \"isSilent\": {}, "
        "\"gpadSound\": {}, "
        "\"gpadFlags\": 0x{:08X} }}",
        state.serialize(), game::snd::serialize(loadingState),
        assetChannelCount, assetFrameRate, assetEndTime, assetTotalMsec,
        sndEnt.serialize(), game::snd::serialize(entity_update), group,
        startDelay, startTime, startFrame, looptime, totalMsec,
        ::serialize(playback), playbackId, firstPlaybackId, secondaryId,
        primaryId, lengthNotifyInfo.serialize(), ::serialize(isDiscontinuous),
        ::serialize(alias), offset.serialize(), position.serialize(),
        velocity.serialize(), ::serialize(orientation), direction.serialize(),
        fluxVelocity.serialize(), baseDistance, distanceAttenuation,
        reverbAttenuation, earlyAttenuation, volModSeed, volModStart,
        pitchModSeed, globalPriority, groupScalar, nearFar, doppler.serialize(),
        fade.serialize(), script_fade.serialize(), script_pitch.serialize(),
        losOcclusion.serialize(), duckAttenuation.serialize(),
        duckLpf.serialize(), ::serialize(traceCache), ::serialize(needsToStop),
        game::snd::serialize(category), closestListenerIndex, voiceStartTime,
        dryLevel, nearLevel, farLevel, earlyLevel, ::serialize(earlyTimes),
        futzBlend, pan.serialize(), played, length, amplitude,
        ::serialize(duck), ::serialize(activeDuck), ::serialize(paused),
        ::serialize(startPaused), ::serialize(timescale),
        ::serialize(positionUpdated), ::serialize(pitchShift),
        ::serialize(aliasFadeOut), ::serialize(continueLoop),
        ::serialize(softStop), ::serialize(isSilent), ::serialize(gpadSound),
        gpadFlags);
  }
};);
// Verified
ASSERT_SIZE(SndVoice, 0x2A8);

// Verified
enum class SndMusicStateStatus : uint32_t {
  SND_MUSIC_STATE_INACTIVE = 0x0,
  SND_MUSIC_STATE_ACTIVE = 0x1,
};

inline constexpr const char *serialize(const SndMusicStateStatus status) {
  switch (status) {
  case SndMusicStateStatus::SND_MUSIC_STATE_INACTIVE:
    return "MUSIC_STATE_INACTIVE";
  case SndMusicStateStatus::SND_MUSIC_STATE_ACTIVE:
    return "MUSIC_STATE_ACTIVE";
  default:
    return "MUSIC_STATE_INVALID";
  }
}

// Unverified.
PACKED(struct SndMusicAsset {
  str64_t alias;
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"alias\": {}, \"aliasId\": 0x{:08X}, \"looping\": "
                       "{}, \"completeLoop\": {}, "
                       "\"removeAfterPlay\": {}, \"firstRandom\": {}, "
                       "\"startSync\": {}, \"stopSync\": {}, "
                       "\"completeOnStop\": {}, \"loopStartOffset\": {}, "
                       "\"bpm\": {}, \"assetType\": {}, "
                       "\"loopNumber\": {}, \"order\": {}, "
                       "\"startDelayBeats\": {}, \"startFadeBeats\": {}, "
                       "\"stopDelayBeats\": {}, \"stopFadeBeats\": {}, "
                       "\"startOffsetFrames\": {}, \"meter\": {} }}",
                       ::serialize(alias), aliasId, looping.serialize(),
                       completeLoop.serialize(), removeAfterPlay.serialize(),
                       firstRandom.serialize(), startSync.serialize(),
                       stopSync.serialize(), completeOnStop.serialize(),
                       loopStartOffset, bpm, assetType, loopNumber, order,
                       startDelayBeats, startFadeBeats, stopDelayBeats,
                       stopFadeBeats, startOffsetFrames, meter);
  }
});
ASSERT_SIZE(SndMusicAsset, 0x8C);

/*
  Mostly unverified.
  `name`, `id`, `loopCount`, `order`,
  `isSequential`, and `skipPreviousExit` are known to be correct.
*/
PACKED(struct SndMusicState {
  str64_t name;
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"id\": 0x{:08X}, \"intro\": {}, \"exit\": {}, "
        "\"loopCount\": {}, \"loops\": {}, \"order\": {}, \"isRandom\": {}, "
        "\"isSequential\": {}, \"skipPreviousExit\": {}, \"status\": {} }}",
        ::serialize(name), id, intro.serialize(), exit.serialize(), loopCount,
        ::serialize(loops, loopCount), order, ::serialize(isRandom),
        ::serialize(isSequential), ::serialize(skipPreviousExit),
        game::snd::serialize(status));
  }
});
ASSERT_SIZE(SndMusicState, 0x178);

// Unverified.
struct SndMusicSet {
  str64_t name;
  SndStringHash id;
  uint32_t stateCount;
  SndMusicState *states;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"id\": 0x{:08X}, \"stateCount\": {}, "
                       "\"states\": {} }}",
                       ::serialize(name), id, stateCount,
                       ::serialize(states, stateCount));
  }
};
ASSERT_SIZE(SndMusicSet, 0x50);

// Unverified.
PACKED(struct SndAmbientBspVolume {
  uint32_t id;
  int32_t planeIndex;
  int32_t planeCount;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"id\": 0x{:08X}, \"planeIndex\": {}, \"planeCount\": {} }}", id,
        planeIndex, planeCount);
  }
});
ASSERT_SIZE(SndAmbientBspVolume, 0xC);

// Unverified.
PACKED(struct SndAmbientBspTrigger {
  uint32_t id;
  int32_t priority;
  SndStringHash roomId;
  str64_t roomName;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"id\": 0x{:08X}, \"priority\": {}, \"roomId\": "
                       "{:08X}, \"roomName\": {} }}",
                       id, priority, roomId, ::serialize(roomName));
  }
});
ASSERT_SIZE(SndAmbientBspTrigger, 0x4C);

// Unverified.
PACKED(struct SndAmbientBspNode {
  vec4_t split;
  int32_t frontIndex;
  int32_t backIndex;
  int32_t frontCount;
  int32_t backCount;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"split\": {}, \"frontIndex\": {}, \"backIndex\": "
                       "{}, \"frontCount\": {}, \"backCount\": {} }}",
                       split.serialize(), frontIndex, backIndex, frontCount,
                       backCount);
  }
});
ASSERT_SIZE(SndAmbientBspNode, 0x20);

PACKED(struct SndAmbientBsp {
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"nodes\": {}, \"numNodes\": {}, \"planes\": {}, "
                       "\"numPlanes\": {}, "
                       "\"volumes\": {}, \"numVolumes\": {}, \"triggers\": {}, "
                       "\"numTriggers\": {} }}",
                       ::serialize(&nodes[0], numNodes), numNodes,
                       ::serialize(&planes[0], numPlanes), numPlanes,
                       ::serialize(&volumes[0], numVolumes), numVolumes,
                       ::serialize(&triggers[0], numTriggers), numTriggers);
  }
});
ASSERT_SIZE(SndAmbientBsp, 0x22020);

// Unverified.
PACKED(struct SndReverbEarly {
  float preDelayMs;
  float inputLpf;
  float feedback;
  float smear;
  float baseDelayMs;
  float returnDB;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"preDelayMs\": {}, \"inputLpf\": {}, \"feedback\": {}, \"smear\": "
        "{}, \"baseDelayMs\": {}, \"returnDB\": {} }}",
        preDelayMs, inputLpf, feedback, smear, baseDelayMs, returnDB);
  }
});
ASSERT_SIZE(SndReverbEarly, 0x18);

// Unverified.
PACKED(struct SndReverbLate {
  float inputLpf;
  float feedback;
  float returnDB;
  float lowDamp;
  float highDamp;
  float decayTime;
  float smear;
  float preDelayMs;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"inputLpf\": {}, \"feedback\": {}, \"returnDB\": "
                       "{}, \"lowDamp\": {}, "
                       "\"highDamp\": {}, \"decayTime\": {}, \"smear\": {}, "
                       "\"preDelayMs\": {} }}",
                       inputLpf, feedback, returnDB, lowDamp, highDamp,
                       decayTime, smear, preDelayMs);
  }
});
ASSERT_SIZE(SndReverbLate, 0x20);

// Unverified.
PACKED(struct SndReverb {
  str64_t name;
  SndStringHash id;
  float masterReturn;
  SndReverbEarly earlyVerb;
  SndReverbLate nearVerb;
  SndReverbLate farVerb;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"id\": 0x{:08X}, \"masterReturn\": "
                       "{}, \"earlyVerb\": "
                       "{}, \"nearVerb\": {}, \"farVerb\": {} }}",
                       ::serialize(name), id, masterReturn,
                       earlyVerb.serialize(), nearVerb.serialize(),
                       farVerb.serialize());
  }
});
ASSERT_SIZE(SndReverb, 0xA0);

// Unverified. Most likely unchanged.
PACKED(struct SndDialogScriptIdLookup {
  SndStringHash scriptId;
  SndStringHash aliasId;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"scriptId\": 0x{:08X}, \"aliasId\": 0x{:08X} }}",
                       scriptId, aliasId);
  }
});
ASSERT_SIZE(SndDialogScriptIdLookup, 0x8);

// Verified
PACKED(struct SndBank : public db::xasset::NamedXAsset {
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

  std::string serialize() const noexcept;
});
ASSERT_SIZE(SndBank, 0x220B0);
typedef SndBank *SndBankPtr;

// Verified.
PACKED(struct SndBankLoad {
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

  inline constexpr std::optional<std::string_view> zone() const noexcept {
    return bank && bank->zone ? std::optional(std::string_view(bank->zone))
                              : std::nullopt;
  }

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"bank\": {}, \"streamAssetBank\": {}, \"loadAssetBank\": {}, "
        "\"loadedEntries\": {}, "
        "\"loadedData\": {:p}, \"loadedAssetCount\": {}, \"loadedAssetTotal\": "
        "{}, \"loadedEntryCount\": {}, "
        "\"loadedDataSize\": {}, \"priority\": {}, \"patchZone\": {}, "
        "\"streamRequestId\": 0x{:08X}, "
        "\"pendingIo\": {}, \"ioError\": {}, \"state\": {}, "
        "\"pendingIoCount\": {} }}",
        ::serialize(bank), streamAssetBank.serialize(),
        loadAssetBank.serialize(), ::serialize(loadedEntries, loadedEntryCount),
        reinterpret_cast<const void *>(loadedData), loadedAssetCount,
        loadedAssetTotal, loadedEntryCount, loadedDataSize, priority,
        patchZone.serialize(), streamRequestId, pendingIo.serialize(),
        ioError.serialize(), game::snd::serialize(state), pendingIoCount);
  }
});

ASSERT_SIZE(SndBankLoad, 0x1278);

PACKED(struct SndBankGlobals {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"bankMagic\": 0x{:08X}, \"bankCount\": {}, \"patchCount\": {}, "
        "\"banks\": {}, \"patches\": {}, \"aliasCache\": {}, "
        "\"missingAliasCache\": {}, \"loads\": {}, "
        "\"loadGate\": {}, \"assetLoadIndex\": {}, \"assetLoadId\": 0x{:08X}, "
        "\"sequentialReadInProgress\": {}, "
        "\"bankHeader\": {}, \"headerMagic\": 0x{:08X}, \"assetLoadPercent\": "
        "{} "
        "}}",
        bankMagic, bankCount, patchCount, ::serialize(banks, bankCount),
        ::serialize(patches, patchCount), aliasCache.serialize(),
        ::serialize(missingAliasCache), ::serialize(loads),
        loadGate.serialize(), assetLoadIndex, assetLoadId,
        ::serialize(sequentialReadInProgress), bankHeader.serialize(),
        headerMagic, assetLoadPercent);
  }
});
ASSERT_SIZE(SndBankGlobals, 0x39594);

// Not yet verified to be correct.
PACKED(struct snd_fire_manager {
  int32_t active;
  vec3_t location;
  float level;
  int32_t id;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"active\": {}, \"location\": {}, \"level\": {}, \"id\": {} }}",
        active, location.serialize(), level, id);
  }
});
ASSERT_SIZE(snd_fire_manager, 0x18);

PACKED(struct snd_weapon_shot {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"localClientNum\": {}, \"shooter\": {}, \"weapon\": {}, "
        "\"origin\": {}, "
        "\"direction\": {}, \"tagName\": 0x{:08X}, \"burstCount\": {}, "
        "\"shooterIsPlayer\": {}, "
        "\"shooterIsLocalPlayer\": {}, \"leftGun\": {}, \"isLastShotInClip\": "
        "{}, \"isTurret\": {}, "
        "\"doubleTap\": {}, \"fakeFire\": {}, \"firstShotOfBurst\": {} }}",
        game::serialize(localClientNum), shooter.serialize(),
        weapon.serialize(), origin.serialize(), direction.serialize(), tagName,
        burstCount, ::serialize(shooterIsPlayer),
        ::serialize(shooterIsLocalPlayer), ::serialize(leftGun),
        ::serialize(isLastShotInClip), ::serialize(isTurret),
        ::serialize(doubleTap), ::serialize(fakeFire),
        ::serialize(firstShotOfBurst));
  }
});
ASSERT_SIZE(snd_weapon_shot, 0x40);

PACKED(struct snd_autosim {
  snd_weapon_shot shot;
  uint32_t lastPing;
  uint32_t lastShot;
  uint32_t fireTime;
  uint16_t shotCount;
  uint16_t eventCount;
  int32_t used;
  int32_t isNew;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"shot\": {}, \"lastPing\": {}, \"lastShot\": {}, \"fireTime\": "
        "{}, "
        "\"shotCount\": {}, \"eventCount\": {}, \"used\": {}, \"isNew\": {} }}",
        shot.serialize(), lastPing, lastShot, fireTime, shotCount, eventCount,
        used, isNew);
  }
});
ASSERT_SIZE(snd_autosim, 0x58);

PACKED(struct snd_autosim_play {
  uint32_t frame;
  uint8_t _padding04[4];
  snd_weapon_shot shot;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frame\": {}, \"shot\": {} }}", frame,
                       shot.serialize());
  }
});
ASSERT_SIZE(snd_autosim_play, 0x48);

PACKED(struct EntityImpactPositions {
  uint32_t entityImpactPositions[6];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"entityImpactPositions\": {} }}",
                       ::serialize(entityImpactPositions));
  }
});
ASSERT_SIZE(EntityImpactPositions, 0x18);

PACKED(struct EntitySoundImpacts {
  const char *name;
  EntityImpactPositions entityImpacts[11];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"entityImpacts\": {} }}",
                       ::serialize(name), ::serialize(entityImpacts));
  }
});
ASSERT_SIZE(EntitySoundImpacts, 0x110);

PACKED(struct SurfaceSoundDef {
  const char *name;
  snd::SndAliasId surfaceSound[40];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"surfaceSound\": {} }}",
                       ::serialize(name), ::serialize(surfaceSound));
  }
});
ASSERT_SIZE(SurfaceSoundDef, 0xA8);

typedef SurfaceSoundDef *SurfaceSoundDefPtr;
typedef EntitySoundImpacts *EntitySoundImpactsPtr;

PACKED(struct SoundsImpactTable {
  const char *name;
  SurfaceSoundDefPtr surfaceSoundTable;
  EntitySoundImpactsPtr entitySoundImpacts;
  EntitySoundImpactsPtr victimSoundImpacts;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"surfaceSoundTable\": {}, \"entitySoundImpacts\": "
        "{}, \"victimSoundImpacts\": {} }}",
        ::serialize(name), ::serialize(surfaceSoundTable),
        ::serialize(entitySoundImpacts), ::serialize(victimSoundImpacts));
  }
});
ASSERT_SIZE(SoundsImpactTable, 0x20);

typedef SoundsImpactTable *SoundsImpactTablePtr;

PACKED(struct SndEntLoop {
  SndAliasId id;
  uint8_t _padding04[4];
  SndEntHandle handle;
  vec3_t origin;
  int32_t fade;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"id\": 0x{:08X}, \"handle\": {}, \"origin\": {}, \"fade\": {} }}",
        id, handle.serialize(), origin.serialize(), fade);
  }
});
ASSERT_SIZE(SndEntLoop, 0x20);

PACKED(struct SndOcclusionStartCache {
  vec3_t position;
  float value;
  bool valid;
  uint8_t _padding11[3];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"position\": {}, \"value\": {}, \"valid\": {} }}",
                       position.serialize(), value, ::serialize(valid));
  }
});
ASSERT_SIZE(SndOcclusionStartCache, 0x14);

enum class SndMusicAssetPlaybackState : int32_t {
  SND_MUSIC_PLAYBACK_STOPPING = 0x0,
  SND_MUSIC_PLAYBACK_STOPPED = 0x1,
  SND_MUSIC_PLAYBACK_STARTED = 0x2,
  SND_MUSIC_PLAYBACK_COUNT = 0x3,
};

inline constexpr const char *serialize(const SndMusicAssetPlaybackState state) {
  switch (state) {
  case SndMusicAssetPlaybackState::SND_MUSIC_PLAYBACK_STOPPING:
    return "MUSIC_PLAYBACK_STOPPING";
  case SndMusicAssetPlaybackState::SND_MUSIC_PLAYBACK_STOPPED:
    return "MUSIC_PLAYBACK_STOPPED";
  case SndMusicAssetPlaybackState::SND_MUSIC_PLAYBACK_STARTED:
    return "MUSIC_PLAYBACK_STARTED";
  default:
    return "MUSIC_PLAYBACK_INVALID";
  }
}

PACKED(struct SndMusicAssetInstance {
  const SndMusicAsset *asset;
  const SndMusicState *state;
  SndPlaybackId id;
  SndMusicAssetPlaybackState playbackState;
  int32_t startFrame;
  int32_t loopNumber;
  int32_t queuedNextLoop;
  uint8_t _padding24[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"asset\": {}, \"state\": {}, \"id\": {}, \"playbackState\": "
        "{}, "
        "\"startFrame\": {}, \"loopNumber\": {}, \"queuedNextLoop\": {} }}",
        ::serialize(asset), ::serialize(state), id,
        game::snd::serialize(playbackState), startFrame, loopNumber,
        queuedNextLoop);
  }
});
ASSERT_SIZE(SndMusicAssetInstance, 0x28);

PACKED(struct LoopSoundInfo {
  SndAliasId id;
  int16_t fade;
  uint8_t _padding06[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"id\": {:8X}, \"fade\": {} }}", id, fade);
  }
});
ASSERT_SIZE(LoopSoundInfo, 0x8);

} // namespace snd
} // namespace game
