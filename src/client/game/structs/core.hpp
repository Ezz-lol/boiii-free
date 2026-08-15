#pragma once

#include <cstddef>
#include <cstdint>
#include <stdfloat>
#include <csetjmp>
#include <variant>

#include <structs/str.hpp>
#include <structs/atomic.hpp>
#include <game/structs/macros.hpp>
#include <game/structs/quake/vec.hpp>

#define PROTOCOL 8
#define SUB_PROTOCOL 1

namespace game {

typedef void *UnknownPtr;

typedef uint32_t contents_t;
typedef const char *XString;
typedef XString *XStringPtr;
typedef const char ConstChar;
typedef ConstChar *ConstCharPtr;
typedef uint64_t bdUInt64;
typedef bdUInt64 bdEntityID;
typedef bdEntityID bdOnlineUserID;
typedef bdOnlineUserID XUID;

typedef double bdFloat64;
typedef float bdFloat32;
typedef int64_t bdInt64;
typedef short bdInt16;
typedef char bdByte8;
typedef uint8_t bdUByte8;
typedef char bdNChar8;
typedef uint32_t bdUInt;
typedef int32_t bdInt;
typedef uint32_t bdUInt32;
typedef uint16_t bdUInt16;
typedef bdUInt16 bdPort;
typedef bool bdBool;

typedef int16_t BoneIndex;
typedef int32_t time32_t;
typedef int64_t time64_t;
typedef time64_t time_t;

typedef uint32_t CanonHash_t;

inline constexpr const char IW_ASSET_SHEBANG = '\x80';
// Carriage return
inline constexpr const char CR = '\r';
// Linefeed
inline constexpr const char LF = '\n';
inline constexpr const char CRLF[2] = {'\r', '\n'};

enum class CampaignMode : int32_t {
  DEFAULT = 0,
  ZOMBIES = 1,
  COUNT = 2,
  INVALID = 3,
};

enum class ContentFlagBits : uint32_t {
  UNKNOWN = 0x1,
  ORIGINALMAPS = 0x2,
  DLC0ZM = 0x4,
  DLC0MP = 0x8,
  DLC1 = 0x10,
  DLC2 = 0x20,
  DLC3 = 0x40,
  DLC4 = 0x80,
  DLC5 = 0x100,
  DLC1ZM = 0x200,
  DLC2ZM = 0x400,
  DLC3ZM = 0x800,
  DLC4ZM = 0x1000,
  DLC6 = 0x2000,
  DLCPC = 0x4000,
};
IMPL_ENUM_OPERATORS(ContentFlagBits);

union ContentFlags {
  uint32_t mask;

  struct {
    uint32_t unknown : 1;      // 0x1
    uint32_t originalMaps : 1; // 0x2
    uint32_t dlc0zm : 1;       // 0x4
    uint32_t dlc0mp : 1;       // 0x8
    uint32_t dlc1 : 1;         // 0x10
    uint32_t dlc2 : 1;         // 0x20
    uint32_t dlc3 : 1;         // 0x40
    uint32_t dlc4 : 1;         // 0x80
    uint32_t dlc5 : 1;         // 0x100
    uint32_t dlc1zm : 1;       // 0x200
    uint32_t dlc2zm : 1;       // 0x400
    uint32_t dlc3zm : 1;       // 0x800
    uint32_t dlc4zm : 1;       // 0x1000
    uint32_t dlc6 : 1;         // 0x2000
    uint32_t dlcPC : 1;        // 0x4000

    uint32_t reserved : 17;
  } bits;

  template <IntegralLike T> inline constexpr operator T() const noexcept {
    return static_cast<T>(mask);
  }

  inline constexpr void set(ContentFlagBits bits) noexcept { mask |= +bits; }

  inline constexpr ContentFlags set(ContentFlagBits bits) const noexcept {
    return ContentFlags{mask | +bits};
  }

  inline constexpr void remove(ContentFlagBits bits) noexcept {
    mask &= ~+bits;
  }

  inline constexpr ContentFlags remove(ContentFlagBits bits) const noexcept {
    return ContentFlags{mask & ~+bits};
  }

  inline constexpr void clear() noexcept { mask = 0; }

  inline constexpr ContentFlags remove() const noexcept {
    return ContentFlags{0};
  }

  template <IntegralLike T> inline constexpr void operator|=(T rhs) noexcept {
    set(rhs);
  }
  template <IntegralLike T>
  inline constexpr ContentFlags operator|(T rhs) const noexcept {
    return set(rhs);
  }

  template <IntegralLike T> inline constexpr void operator&=(T rhs) noexcept {
    mask &= +rhs;
  }
  template <IntegralLike T>
  inline constexpr ContentFlags operator&(T rhs) const noexcept {
    return ContentFlags{mask & +rhs};
  }

  template <IntegralLike T> inline constexpr void operator^=(T rhs) noexcept {
    mask ^= +rhs;
  }
  template <IntegralLike T>
  inline constexpr ContentFlags operator^(T rhs) const noexcept {
    return ContentFlags{mask ^ +rhs};
  }

  static inline constexpr ContentFlags allContent() noexcept {
    return ContentFlags{.bits = {.unknown = 0,
                                 .originalMaps = 1,
                                 .dlc0zm = 1,
                                 .dlc0mp = 1,
                                 .dlc1 = 1,
                                 .dlc2 = 1,
                                 .dlc3 = 1,
                                 .dlc4 = 1,
                                 .dlc5 = 1,
                                 .dlc1zm = 1,
                                 .dlc2zm = 1,
                                 .dlc3zm = 1,
                                 .dlc4zm = 1,
                                 .dlc6 = 1,
                                 .dlcPC = 1,
                                 .reserved = 0}};
  }
};

enum class dlcIndex_t : int32_t {
  DEV_MAP_INDEX = -1,
  ORIGINAL_MAP_INDEX = 0,
  DLC0ZM_INDEX = 1,
  DLC0MP_INDEX = 2,
  DLC1_INDEX = 3,
  DLC2_INDEX = 4,
  DLC3_INDEX = 5,
  DLC4_INDEX = 6,
  DLC5_INDEX = 7,
  DLC1ZM_INDEX = 8,
  DLC2ZM_INDEX = 9,
  DLC3ZM_INDEX = 10,
  DLC4ZM_INDEX = 11,
  DLC6_INDEX = 12,
  DLCPC_INDEX = 13,
  DLC_INDEX_COUNT = 14,
  USERMAP_INDEX = 15,
};

enum clientplatform_t : int32_t {
  CLIENT_PLATFORM_PC = 0x0,      // PC
  CLIENT_PLATFORM_ORBIS = 0x1,   // PS4
  CLIENT_PLATFORM_DURANGO = 0x2, // Xbox One
  MAX_CLIENT_PLATFORMS = 0x3,
};
IMPL_ENUM_OPERATORS(clientplatform_t);

enum class team_t : uint32_t {
  TEAM_FREE = 0x0,
  TEAM_BAD = 0x0,
  TEAM_DEAD = 0x0,
  TEAM_ALLIES = 0x1,
  TEAM_AXIS = 0x2,
  TEAM_THREE = 0x3,
  TEAM_FOUR = 0x4,
  TEAM_FIVE = 0x5,
  TEAM_SIX = 0x6,
  TEAM_SEVEN = 0x7,
  TEAM_EIGHT = 0x8,
  TEAM_NINE = 0x9,
  TEAM_TEN = 0xA,
  TEAM_ELEVEN = 0xB,
  TEAM_TWELVE = 0xC,
  TEAM_THIRTEEN = 0xD,
  TEAM_FOURTEEN = 0xE,
  TEAM_FIFTEEN = 0xF,
  TEAM_SIXTEEN = 0x10,
  TEAM_SEVENTEEN = 0x11,
  TEAM_EIGHTEEN = 0x12,
  TEAM_NEUTRAL = 0x13,
  TEAM_SPECTATOR = 0x14,
  TEAM_NUM_PLAYING_TEAMS = 0x14,
  TEAM_NUM_TEAMS = 0x15,
  TEAM_LOCALPLAYERS = 0x16,
  TEAM_FIRST_PLAYING_TEAM = 0x1,
  TEAM_LAST_PLAYING_TEAM = 0x13,
};
IMPL_ENUM_OPERATORS(team_t);

enum ClientNum_t : int32_t {
  INVALID_CLIENT_INDEX = -1,
  CLIENT_INDEX_0 = 0x0,
  CLIENT_INDEX_FIRST = 0x0,
  CLIENT_INDEX_1 = 0x1,
  CLIENT_INDEX_2 = 0x2,
  CLIENT_INDEX_3 = 0x3,
  CLIENT_INDEX_4 = 0x4,
  CLIENT_INDEX_5 = 0x5,
  CLIENT_INDEX_6 = 0x6,
  CLIENT_INDEX_7 = 0x7,
  CLIENT_INDEX_8 = 0x8,
  CLIENT_INDEX_9 = 0x9,
  CLIENT_INDEX_10 = 0xA,
  CLIENT_INDEX_11 = 0xB,
  CLIENT_INDEX_12 = 0xC,
  CLIENT_INDEX_13 = 0xD,
  CLIENT_INDEX_14 = 0xE,
  CLIENT_INDEX_15 = 0xF,
  CLIENT_INDEX_16 = 0x10,
  CLIENT_INDEX_17 = 0x11,
  CLIENT_INDEX_18 = 0x12,
  CLIENT_INDEX_19 = 0x13,
  CLIENT_INDEX_20 = 0x14,
  CLIENT_INDEX_21 = 0x15,
  CLIENT_INDEX_22 = 0x16,
  CLIENT_INDEX_23 = 0x17,
  CLIENT_INDEX_24 = 0x18,
  CLIENT_INDEX_25 = 0x19,
  CLIENT_INDEX_26 = 0x1A,
  CLIENT_INDEX_27 = 0x1B,
  CLIENT_INDEX_28 = 0x1C,
  CLIENT_INDEX_29 = 0x1D,
  CLIENT_INDEX_30 = 0x1E,
  CLIENT_INDEX_31 = 0x1F,
  CLIENT_INDEX_COUNT = 0x12,
};
IMPL_ENUM_OPERATORS(ClientNum_t);

enum ControllerIndex_t : int32_t {
  INVALID_CONTROLLER_PORT = -1,
  CONTROLLER_INDEX_FIRST = 0x0,
  CONTROLLER_INDEX_0 = 0x0,
  CONTROLLER_INDEX_1 = 0x1,
  CONTROLLER_INDEX_COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(ControllerIndex_t);

enum LocalClientNum_t : int32_t {
  INVALID_LOCAL_CLIENT = -1,
  LOCAL_CLIENT_0 = 0x0,
  LOCAL_CLIENT_FIRST = 0x0,
  LOCAL_CLIENT_KEYBOARD_AND_MOUSE = 0x0,
  LOCAL_CLIENT_1 = 0x1,
  LOCAL_CLIENT_COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(LocalClientNum_t);

#ifndef NDEBUG
inline constexpr const char *serialize(LocalClientNum_t localClientNum) {
  switch (localClientNum) {
  case LOCAL_CLIENT_0:
    return "LOCAL_CLIENT_0";
  case LOCAL_CLIENT_1:
    return "LOCAL_CLIENT_1";
  default:
    return "INVALID_LOCAL_CLIENT";
  }
}
#endif

enum class LocalClientNum8_t : int8_t {
  INVALID_LOCAL_CLIENT = -1,
  LOCAL_CLIENT_0 = 0x0,
  LOCAL_CLIENT_FIRST = 0x0,
  LOCAL_CLIENT_KEYBOARD_AND_MOUSE = 0x0,
  LOCAL_CLIENT_1 = 0x1,
  LOCAL_CLIENT_COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(LocalClientNum8_t);

enum class eGameModes : int32_t {
  MATCHMAKING_PLAYLIST = 0x0,
  MATCHMAKING_MANUAL = 0x1,
  DEFAULT = 0x2,
  LEAGUE = 0x3,
  FREERUN = 0x4,
  THEATER = 0x5,
  MANUAL_PLAYLIST = 0x6,
  RTS = 0x7,
  COUNT = 0x8,
  INVALID = 0x8,
};
IMPL_ENUM_OPERATORS(eGameModes);

enum class eModes : uint32_t {
  ZOMBIES = 0x0,
  FIRST = 0x0,
  MULTIPLAYER = 0x1,
  CAMPAIGN = 0x2,
  COUNT = 0x3,
  INVALID = 0x3
};
IMPL_ENUM_OPERATORS(eModes);

inline constexpr bool valid_mode(eModes mode) noexcept {
  return +mode < +eModes::INVALID && +mode >= +eModes::ZOMBIES;
}

namespace com {
eModes Com_SessionMode_GetMode();
}

template <typename T> union SessionModePool {
  T pool[+eModes::COUNT];
  struct {
    T zombies;
    T multiplayer;
    T campaign;
  };

  inline constexpr T &get(eModes mode) noexcept { return pool[+mode]; }
  inline constexpr volatile T &get(eModes mode) volatile noexcept {
    return pool[+mode];
  }

  inline constexpr const T &get(eModes mode) const noexcept {
    return pool[+mode];
  }
  inline T &get() noexcept { return get(com::Com_SessionMode_GetMode()); }
  inline volatile T &get() volatile noexcept {
    return get(com::Com_SessionMode_GetMode());
  }
  inline const T &get() const noexcept {
    return get(com::Com_SessionMode_GetMode());
  }
  inline constexpr T &operator[](eModes index) noexcept { return get(index); }
  inline constexpr volatile T &operator[](eModes index) volatile noexcept {
    return get(index);
  }
  inline constexpr const T &operator[](eModes index) const noexcept {
    return get(index);
  }
};

enum class eNetworkModes : uint32_t {
  OFFLINE = 0x0,
  SYSTEMLINK = 0x1,
  ONLINE = 0x2,
  COUNT = 0x3,
  INVALID = 0x3,
};
IMPL_ENUM_OPERATORS(eNetworkModes);

enum class MapPreload : uint32_t {
  NONE = 0x0,
  FRONTEND = 0x1,
  IN_GAME = 0x2,
};
IMPL_ENUM_OPERATORS(MapPreload);

enum class itemTextStyle : uint32_t {
  NORMAL = 0x0,
  SHADOWED = 0x3,
  SHADOWEDMORE = 0x6,
  BORDERED = 0x7,
  BORDEREDMORE = 0x8,
  MONOSPACE = 0x80,
  MONOSPACESHADOWED = 0x84,
};
IMPL_ENUM_OPERATORS(itemTextStyle);

enum class errorParm : uint32_t {
  FATAL = 0x0,
  DROP = 0x1,
  SERVERDISCONNECT = 0x2,
  DISCONNECT = 0x3,
  SCRIPT = 0x4,
  SCRIPT_DROP = 0x5,
  LOCALIZATION = 0x6,
  MAPLOADERRORSUMMARY = 0x7,

  UI = 0x100,
  LUA = 0x200,
  SOFTRESTART = 0x400,
  SOFTRESTART_KEEPDW = 0x800,

};
IMPL_ENUM_OPERATORS(errorParm);

enum class errorCode : uint32_t {
  NONE = 0x0,
  FATAL = 0x1,
  DROP = 0x2,
  FROM_STARTUP = 0x4,
  SERVERDISCONNECT = 0x8,
  DISCONNECT = 0x10,
  SCRIPT = 0x20,
  SCRIPT_DROP = 0x40,
  LOCALIZATION = 0x80,
  UI = 0x100,
  LUA = 0x200,
  SOFTRESTART = 0x400,
  SOFTRESTART_KEEPDW = 0x800,
};
IMPL_ENUM_OPERATORS(errorCode);

template <typename T> using LocalClientPool = array<T, LOCAL_CLIENT_COUNT>;
template <typename T>
using AtomicLocalClientPool = atomicarray<T, LOCAL_CLIENT_COUNT>;

template <typename ClientType, typename ServerType>
using EngineDependent = std::variant<ClientType, ServerType>;

typedef str8_t clanAbbrev_t;
typedef str32_t name_t;
typedef name_t actorName_t;
typedef name_t playerName_t;

typedef int32_t cinematic_id;

struct netUInt64 {
  uint32_t low;
  uint32_t high;
};

enum class ZoneType : uint32_t {
  /*
     The `OFFICIAL` enumeration is almost never used, except in one function
     which checks if a ZoneType enumeration value represents UGC content.

     All other usage assumes value is either MOD or USERMAP, and
     ZoneType is never assigned to OFFICIAL by the engine.

     This enumeration was likely created with the intention of usage in
     a consolidated zone handling architecture, which never came to be.
  */
  OFFICIAL = 0x0,

  MOD = 0x1,
  USERMAP = 0x2,
  COUNT = 0x3
};
IMPL_ENUM_OPERATORS(ZoneType);

inline constexpr const char *dirname(ZoneType zoneType) {
  switch (zoneType) {
  case ZoneType::MOD:
    return "mods";
  case ZoneType::USERMAP:
    return "usermaps";
  default:
    return "";
  }
}

enum class StorageFileType : int32_t {
  COMMON_SETTINGS = 0,
  PROFILE_SHOUTCASTER = 1,
  CP_SAVEGAME_ONLINE = 2,
  CP_SAVEGAME_OFFLINE = 3,
  CPNIGHTMARE_SAVEGAME_ONLINE = 4,
  CPNIGHTMARE_SAVEGAME_OFFLINE = 5,
  CP_STATS_ONLINE = 6,
  CP_STATS_OFFLINE = 7,
  CP_STATS_NIGHTMARE_ONLINE = 8,
  CP_STATS_NIGHTMARE_OFFLINE = 9,
  CP_LOADOUTS = 10,
  CP_LOADOUTS_OFFLINE = 11,
  MP_STATS_ONLINE = 12,
  MP_STATS_OFFLINE = 13,
  MP_LOADOUTS = 14,
  MP_LOADOUTS_OFFLINE = 15,
  MP_CLASS_SETS = 16,
  ZM_STATS_ONLINE = 17,
  ZM_STATS_OFFLINE = 18,
  ZM_LOADOUTS = 19,
  ZM_LOADOUTS_OFFLINE = 20,
  FR_STATS_ONLINE = 21,
  FR_STATS_OFFLINE = 22,
  PAINTSHOP_DATA = 23,
  GUNSMITH = 24,
  PAINTJOBS = 25,
  EMBLEMS = 26,
  DEFAULT_EMBLEMS = 27,
  EMBLEMS_LOOT = 28,
  CUSTOM_GAMES = 29,
  OFFICIAL_CUSTOM_GAMES = 30,
  EXTERNAL_DATA = 31,
  FILE_COUNT = 32,
  FILE_FIRST = 0,
  FILE_INVALID = -1,
};
IMPL_ENUM_OPERATORS(StorageFileType);

enum class consoleLabel_e : int32_t {
  DEFAULT = 0x0,
  TEMP = 0x1,
  GFX = 0x2,
  TASKMGR2 = 0x3,
  LIVE = 0x4,
  DEMONWARE = 0x5,
  LEADERBOARDS = 0x6,
  LOBBY = 0x7,
  LOBBYHOST = 0x8,
  LOBBYCLIENT = 0x9,
  LOBBYVM = 0xA,
  MIGRATION = 0xB,
  INGAME_MIGRATION_HOST = 0xC,
  INGAME_MIGRATION_CLIENT = 0xD,
  SCRIPTER = 0xE,
  VM = 0xF,
  DVAR = 0x10,
  TOOL = 0x11,
  ANIM = 0x12,
  NETCHAN = 0x13,
  BG_CACHE = 0x14,
  PM = 0x15,
  MAPSWITCH = 0x16,
  AI = 0x17,
  GADGET = 0x18,
  SOUND = 0x19,
  SNAPSHOT = 0x1A,
  PLAYGO = 0x1B,
  LUI = 0x1C,
  LUA = 0x1D,
  VOICE = 0x1E,
  DEMO = 0x1F,
  DB = 0x20,
  HTTP = 0x21,
  COMPANION = 0x22,
  MEM = 0x23,
  CINEMATIC = 0x24,
  DDL = 0x25,
  STORAGE = 0x26,
  STEAM = 0x27,
  CHECKPOINT = 0x28,
  THUNDERHEAD = 0x29,
  COMSCORE = 0x2A,
  FILESHARE = 0x2B,
  LPC = 0x2C,
  MARKETING = 0x2D,
  STORE = 0x2E,
  TESTING = 0x2F,
  CHANNEL_ERROR = 0x36,
  COUNT = 0x37,
};
IMPL_ENUM_OPERATORS(consoleLabel_e);

enum class PMemStack : uint32_t {
  DB = 0x0,
  DB2 = 0x1,
  GAME = 0x2,
  SERVER = 0x3,
  HOT = 0x4,
  CINEMATICS = 0x5,
  DYNAMIC_IMAGES = 0x6,
  LIGHTING = 0x7,
  COUNT = 0x8,
  INVALID = 0xFFFFFFFF,
  PHYS_ALLOC_LOW = 0x0,
  PHYS_ALLOC_HIGH = 0x2,
  PHYS_ALLOC_COUNT = 0x8,
};
IMPL_ENUM_OPERATORS(PMemStack);

enum class PMemPool : uint32_t {
  MAIN = 0x0,
  GPU_PRIVATE = 0x1,
  GPU_PRT = 0x2,
  GPU_RENDER_TARGETS = 0x3,
  MAIN_COHERENT = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(PMemPool);

enum class EMemTrack : uint32_t {
  TRACK_BINARIES = 0x0,
  TRACK_DEBUG = 0x1,
  TRACK_HUNK = 0x2,
  TRACK_MISC_SWAP = 0x3,
  TRACK_LOCALIZATION = 0x4,
  TRACK_TL = 0x5,
  TRACK_TEMP = 0x6,
  TRACK_RECORDER = 0x7,
  TRACK_DEMO = 0x8,
  TRACK_DVR = 0x9,
  TRACK_DVAR = 0xA,
  TRACK_ASSET_DATABASE = 0xB,
  TRACK_SAVEGAME = 0xC,
  TRACK_DELIMITER1 = 0xD,
  TRACK_COMSCORE = 0xE,
  TRACK_AI = 0xF,
  TRACK_AI_BOTS = 0x10,
  TRACK_AI_NODES = 0x11,
  TRACK_SCRIPT = 0x12,
  TRACK_SCRIPT_DEBUG = 0x13,
  TRACK_COLLISION_MISC = 0x14,
  TRACK_COLLISION_BRUSH = 0x15,
  TRACK_COLLISION_MODEL_TRI = 0x16,
  TRACK_COLLISION_TERRAIN = 0x17,
  TRACK_PHYSICS = 0x18,
  TRACK_MAP_ENTS = 0x19,
  TRACK_CLIENT = 0x1A,
  TRACK_SERVER = 0x1B,
  TRACK_DDL = 0x1C,
  TRACK_STORAGE = 0x1D,
  TRACK_ONLINE = 0x1E,
  TRACK_VEHICLEDEF = 0x1F,
  TRACK_DELIMITER2 = 0x20,
  TRACK_FASTFILE = 0x21,
  TRACK_NETWORK_ENTITY = 0x22,
  TRACK_MISC = 0x23,
  TRACK_ANIMATION = 0x24,
  TRACK_ANIMCACHE = 0x25,
  TRACK_CLIENT_ANIMSCRIPT = 0x26,
  TRACK_WORLD_GLOBALS = 0x27,
  TRACK_DELIMITER3 = 0x28,
  TRACK_EMBLEM = 0x29,
  TRACK_FX = 0x2A,
  TRACK_GLASS = 0x2B,
  TRACK_RENDERER_GLOBALS = 0x2C,
  TRACK_RENDERER_TARGETS = 0x2D,
  TRACK_RENDERER_IMAGES = 0x2E,
  TRACK_RENDERER_FX_IMAGES = 0x2F,
  TRACK_RENDERER_WORLD = 0x30,
  TRACK_RENDERER_MODELS = 0x31,
  TRACK_RENDERER_MISC = 0x32,
  TRACK_RENDERER_SIEGE = 0x33,
  TRACK_RENDERER_LIGHTS = 0x34,
  TRACK_RENDERER_LIGHT_EXPORT = 0x35,
  TRACK_RENDERER_UMBRA = 0x36,
  TRACK_RENDERER_OIT = 0x37,
  TRACK_RSTREAM = 0x38,
  TRACK_RENDERER_STREAMBUFFER = 0x39,
  TRACK_RENDERER_STREAMBUFFER_EXTRA = 0x3A,
  TRACK_GEOSTREAM = 0x3B,
  TRACK_CINEMATICS = 0x3C,
  TRACK_LUA = 0x3D,
  TRACK_UI = 0x3E,
  TRACK_FLAME = 0x3F,
  TRACK_FIREMANAGER = 0x40,
  TRACK_CONSOLE_IMAGES = 0x41,
  TRACK_CGMEDIA_TABLE = 0x42,
  TRACK_IMPACTS_FX_TABLE = 0x43,
  TRACK_DELIMITER4 = 0x44,
  TRACK_SOUND = 0x45,
  TRACK_SOUND_GLOBALS = 0x46,
  TRACK_MINSPEC_SOUND_GLOBALS = 0x47,
  TRACK_IMPACTS_SOUND_TABLE = 0x48,
  TRACK_DELIMITER5 = 0x49,
  TRACK_NONE = 0x4A,
  TRACK_COUNT = 0x4B,
};
IMPL_ENUM_OPERATORS(EMemTrack);

enum class SwimStateType : int32_t {
  NONE = 0x0,
  SURFACE = 0x1,
  UNDERWATER = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(SwimStateType);

enum class KillCamEntityRestState : int32_t {
  MOVING = 0x0,
  AT_REST = 0x1,
  STUCK_GROUND = 0x2,
  STUCK_WALL = 0x3,
};
IMPL_ENUM_OPERATORS(KillCamEntityRestState);

enum class KillCamEntityType : int32_t {
  NO_ENTITY = 0x0,
  HELICOPTER = 0x1,
  DESTRUCTIBLE = 0x2,
  SCRIPTED = 0x3,
  EXPLOSIVE = 0x4,
  FAST_EXPLOSIVE = 0x5,
  ROCKET = 0x6,
  DRONE = 0x7,
  DAG = 0x8,
  COMBAT_ROBOT = 0x9,
  ARTILLERY = 0xA,
  VEHICLE = 0xB,
};
IMPL_ENUM_OPERATORS(KillCamEntityType);

enum class InvalidCmdHintType : uint32_t {
  NONE = 0x0,
  NO_AMMO_BULLETS = 0x1,
  NO_AMMO_FRAG_GRENADE = 0x2,
  NO_AMMO_SPECIAL_GRENADE = 0x3,
  NO_AMMO_FLASH_GRENADE = 0x4,
  NO_AMMO_EQUIPMENT = 0x5,
  STAND_BLOCKED = 0x6,
  CROUCH_BLOCKED = 0x7,
  TARGET_TOO_CLOSE = 0x8,
  LOCKON_REQUIRED = 0x9,
  NOT_ENOUGH_CLEARANCE = 0xA,
  CANT_PLACE_TURRET = 0xB,
  CANT_EQUIP_WHILE_PRONE = 0xC,
  CANT_PLANT_EQUIPMENT = 0xD,
  ROUND_START_DELAY = 0xE,
  GRENADES_NOT_ALLOWED = 0xF,
  GADGET_DENIED_ALREADY_ACTIVE = 0x10,
  GADGET_DENIED_CANNOT_ACTIVATE = 0x11,
  TARGET_OUT_OF_RANGE = 0x12,
  TARGET_BEING_USED = 0x13,
  TARGET_INVALID_TYPE = 0x14,
  NO_TARGET_IN_RANGE = 0x15,
  TARGET_DISABLED = 0x16,
  TARGETING_ABORTED = 0x17,
  TARGET_ALREADY_TARGETED = 0x18,
  SYSTEM_DISABLED = 0x19,
};
IMPL_ENUM_OPERATORS(InvalidCmdHintType);

enum class SettingTeamIndicator : int32_t {
  FULL = 0x0,
  ABBREVIATED = 0x1,
  ICON = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(SettingTeamIndicator);

enum class CubemapShot : int32_t {
  NONE = 0x0,
  RIGHT = 0x1,
  LEFT = 0x2,
  BACK = 0x3,
  FRONT = 0x4,
  UP = 0x5,
  DOWN = 0x6,
  COUNT = 0x7,
};
IMPL_ENUM_OPERATORS(CubemapShot);

enum class DemoType : int32_t {
  NONE = 0x0,
  CLIENT = 0x1,
  SERVER = 0x2,
  SERVER_SNAPSHOT = 0x3,
};
IMPL_ENUM_OPERATORS(DemoType);

enum class TraceBrushType : uint32_t {
  NONE = 0x0,
  BRUSH = 0x1,
};
IMPL_ENUM_OPERATORS(TraceBrushType);

enum class TraceHitType : uint32_t {
  NONE = 0x0,
  ENTITY = 0x1,
  DYNENT_MODEL = 0x2,
  DYNENT_BRUSH = 0x3,
  GLASS = 0x4,
};
IMPL_ENUM_OPERATORS(TraceHitType);

enum class MissileFlightMode : int32_t {
  TOP = 0x0,
  DIRECT = 0x1,
};
IMPL_ENUM_OPERATORS(MissileFlightMode);

enum class MissileStage : int32_t {
  SOFTLAUNCH = 0x0,
  ASCENT = 0x1,
  DESCENT = 0x2,
};
IMPL_ENUM_OPERATORS(MissileStage);

enum class CameraMode : int32_t {
  NORMAL = 0x0,
  LINKED = 0x1,
  VEHICLE = 0x2,
  VEHICLE_THIRDPERSON = 0x3,
  VEHICLE_GUNNER = 0x4,
  TURRET = 0x5,
  MISSILE = 0x6,
  SPIKECAMERA = 0x7,
  SCRIPTED_EXTRACAM = 0x8,
  SCOPE = 0x9,
  PLAYER_BODY = 0xA,
  XCAM = 0xB,
  SCRIPTED = 0xC,
  RADIANT = 0xD,
};
IMPL_ENUM_OPERATORS(CameraMode);

struct orientation_t {
  vec3_t origin;
  vec3_t axis[3];
};

#pragma pack(push, 16)
struct float128_t {
  int64_t LowPart;
  int64_t HighPart;
};
#pragma pack(pop)

struct CmdArgs {
  int32_t nesting;
  int32_t localClientNum[8];
  int32_t controllerIndex[8];
  int32_t argshift[8];
  int32_t argc[8];
  const char **argv[8];
  char textPool[8192];
  const char *argvPool[512];
  int32_t usedTextPool[8];
  int32_t totalUsedArgvPool;
  int32_t totalUsedTextPool;
};

struct va_info_t {
  char va_string[4][1024];
  int32_t index;
};

struct TLSData {
  va_info_t *vaInfo;
  jmp_buf *errorJmpBuf;
  void *traceInfo;
  CmdArgs *cmdArgs;
  void *errorData;
};

enum class connstate_t : int32_t {
  DISCONNECTED = 0x0,
  CINEMATIC = 0x1,
  UICINEMATIC = 0x2,
  LOGO = 0x3,
  CHALLENGING = 0x4,
  CONNECTING = 0x5,
  CONFIRMLOADING = 0x6,
  CONNECTED = 0x7,
  SENDINGDATA = 0x8,
  /*
     Note: standard loading order is
     LOADING -> PRIMED
     These two enumerations are out of order,
     but all others are in order of state transition.
  */
  PRIMED = 0x9,
  LOADING = 0xA,
  ACTIVE = 0xB,
  COUNT = 0xD,
};
IMPL_ENUM_OPERATORS(connstate_t);

enum class StanceState : int32_t {
  STAND = 0x0,
  CROUCH = 0x1,
  PRONE = 0x2,
  DIVE_TO_PRONE = 0x3,
  SLIDE = 0x4,
};
IMPL_ENUM_OPERATORS(StanceState);

typedef void *fileHandle_t;

struct emblemChallengeLookup_t {
  int16_t challengeIndex;
  uint8_t itemIndex;
};

enum class CharacterItemType : uint32_t {
  BODY = 0x0,
  HELMET = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(CharacterItemType);

using BGEmblemBackgroundID = int16_t;

typedef int32_t BitArrayChunk;
constexpr const size_t BITARRAY_CHUNK_BITS = bitsizeof<BitArrayChunk>();

#pragma pack(push, 1)
template <const IntegralLike<size_t> auto B> struct bitarray {
  array<BitArrayChunk, (static_cast<size_t>(B) + BITARRAY_CHUNK_BITS - 1) /
                           BITARRAY_CHUNK_BITS>
      data;

  inline constexpr void assert_range(size_t index) {
    assert(index < static_cast<size_t>(B) &&
           "Index to bitarray must be within bounds 0 <= index < B");
  }

  template <IntegralLike<size_t> Index>
  inline constexpr void set(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);

    return this->data[index / BITARRAY_CHUNK_BITS] |=
           (1 << (index % BITARRAY_CHUNK_BITS));
  }

  template <IntegralLike<size_t> Index>
  inline constexpr void clear(Index index_arg) noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);

    return this->data[index / BITARRAY_CHUNK_BITS] &=
           ~(1 << (index % BITARRAY_CHUNK_BITS));
  }

  template <IntegralLike<size_t> Index>
  inline constexpr bool get(Index index_arg) const noexcept {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);

    return (this->data[index / BITARRAY_CHUNK_BITS] &
            (1 << (index % BITARRAY_CHUNK_BITS))) != 0;
  }

  // Bit indexing operator
  template <IntegralLike Index>
  inline constexpr bool operator[](Index index) const noexcept {
    return get(static_cast<size_t>(index));
  }

  inline constexpr const BitArrayChunk &begin() const noexcept {
    return data[0];
  }

  inline constexpr const BitArrayChunk &end() const noexcept {
    return data[std::size(data) - 1];
  }

  inline constexpr BitArrayChunk &begin() noexcept { return data[0]; }

  inline constexpr BitArrayChunk &end() noexcept {
    return data[std::size(data) - 1];
  }

  template <IntegralLike<BitArrayChunk> T>
  inline constexpr void fill(const T val) noexcept {
    std::fill(&begin(), &end(), static_cast<BitArrayChunk>(val));
  }

  inline constexpr void reset() noexcept { fill(0); }

  // Function name used by engine
  inline constexpr void resetAllBits() noexcept { reset(); }
};
#pragma pack(pop)
ASSERT_SIZE(bitarray<32>, 0x4);
ASSERT_CPP03_POD(bitarray<32>);

typedef bitarray<72> game_button_bits_t;
ASSERT_SIZE(game_button_bits_t, 0xC);

enum class UIModelDataType : int32_t {
  INVALID = 0x0,
  BOOL = 0x1,
  UINT64 = 0x2,
  INT = 0x3,
  REAL = 0x4,
  STRING = 0x5,
  FUNCTION = 0x6,
  COUNT = 0x7,
};
IMPL_ENUM_OPERATORS(UIModelDataType);

enum class objectiveDrawState_t : int32_t {
  ONSCREEN = 0x0,
  OFFSCREEN = 0x1,
};
IMPL_ENUM_OPERATORS(objectiveDrawState_t);

enum class objectiveState_t : int32_t {
  EMPTY = 0x0,
  ACTIVE = 0x1,
  INVISIBLE = 0x2,
  DONE = 0x3,
  CURRENT = 0x4,
  FAILED = 0x5,
  NUMSTATES = 0x6,
};
IMPL_ENUM_OPERATORS(objectiveState_t);

struct UIModelData {
  UIModelDataType dataType;
  uint8_t _padding04[4];
  union {
    uint64_t uint64;
    const char *string;
    int32_t integer;
    float real;
    bool boolean;
    int32_t functionRef;
  };
};
ASSERT_SIZE(UIModelData, 0x10);

struct objectiveUIModel_t {
  UIModelData modelData;
  int32_t modelName;
  uint8_t _padding14[12];
};

#pragma pack(push, 1)
struct objective_t {
  objectiveState_t objState;
  vec3_t origin;
  int32_t entNum;
  int32_t color;
  int16_t colorSetColor;
  uint8_t _padding1A[2];
  vec2_t objIconSize;
  int32_t objOwnerNum;
  uint16_t name;
  int16_t teamMask;
  uint32_t progress;
  int clientUseMask[1];
  uint16_t gamemodeFlags;
  uint8_t objTeamNum;
  bool syncFlag;
  uint8_t _padding38[8];
  objectiveUIModel_t uiModels[4];
};
ASSERT_SIZE(objective_t, 0xC0);
#pragma pack(pop)

enum scoreboardColumnType_t : int32_t {
  SB_TYPE_INVALID = 0x0,
  SB_TYPE_NONE = 0x1,
  SB_TYPE_SCORE = 0x2,
  SB_TYPE_KILLS = 0x3,
  SB_TYPE_DEATHS = 0x4,
  SB_TYPE_ASSISTS = 0x5,
  SB_TYPE_DEFENDS = 0x6,
  SB_TYPE_PLANTS = 0x7,
  SB_TYPE_DEFUSES = 0x8,
  SB_TYPE_RETURNS = 0x9,
  SB_TYPE_CAPTURES = 0xA,
  SB_TYPE_DESTRUCTIONS = 0xB,
  SB_TYPE_DISABLES = 0xC,
  SB_TYPE_ESCORTS = 0xD,
  SB_TYPE_CARRIES = 0xE,
  SB_TYPE_THROWS = 0xF,
  SB_TYPE_KDRATIO = 0x10,
  SB_TYPE_SURVIVED = 0x11,
  SB_TYPE_STABS = 0x12,
  SB_TYPE_TOMAHAWKS = 0x13,
  SB_TYPE_HUMILIATED = 0x14,
  SB_TYPE_X2SCORE = 0x15,
  SB_TYPE_HEADSHOTS = 0x16,
  SB_TYPE_DOWNS = 0x17,
  SB_TYPE_REVIVES = 0x18,
  SB_TYPE_AGRKILLS = 0x19,
  SB_TYPE_HACKS = 0x1A,
  SB_TYPE_POINTS_TO_WIN = 0x1B,
  SB_TYPE_KILLS_CONFIRMED = 0x1C,
  SB_TYPE_KILLS_DENIED = 0x1D,
  SB_TYPE_SHOTS_MISSED = 0x1E,
  SB_TYPE_SHOTS_HIT = 0x1F,
  SB_TYPE_TIME_PLAYED = 0x20,
  SB_TYPE_VICTORY = 0x21,
  SB_TYPE_INCAPS = 0x22,
  SB_TYPE_GEMS = 0x23,
  NUM_SB_TYPES = 0x24,
};
IMPL_ENUM_OPERATORS(scoreboardColumnType_t);

struct uint128_t {
  uint64_t low;
  uint64_t high;
};
ASSERT_SIZE(uint128_t, 0x10);

struct renderOptions_t {
  union {
    struct {
      union {
        struct {
          uint32_t camo : 7;
          uint32_t reticle : 6;
          uint32_t playerTag : 1;
          uint32_t emblem : 1;
          uint32_t paintshop : 1;
          uint32_t paintjobIndex : 4;
          uint32_t paintjobSlot : 4;
          uint32_t extraCamPreview : 1;
          uint32_t playerNum : 5;
          uint32_t customClass : 4;
          uint32_t customClassPrimary : 1;
          uint32_t stowedPlayerTag : 1;
          uint32_t stowedEmblem : 1;
          uint32_t stowedPaintshop : 1;
          uint32_t stowedPlayerNum : 5;
          uint32_t stowedCustomClass : 4;
          uint32_t stowedCustomClassPrimary : 1;
        };
        uint64_t weaponPacked : 48;
      };
      union {
        struct {
          uint32_t characterMode : 3;
          uint32_t bodyType : 4;
          uint32_t bodyStyle : 4;
          uint32_t bodyColor1 : 3;
          uint32_t bodyColor2 : 3;
          uint32_t bodyColor3 : 3;
          uint32_t helmetStyle : 4;
          uint32_t helmetColor1 : 3;
          uint32_t helmetColor2 : 3;
          uint32_t helmetColor3 : 3;
          uint32_t headStyle : 4;
        };
        uint64_t bodyPacked : 37;
      };
    };
    uint128_t allPacked;
  };
};
ASSERT_SIZE(renderOptions_t, 0x10);

typedef uint16_t modelNameIndex_t;

class EntHandle {
public:
  uint16_t number;
  uint16_t infoIndex;
};
ASSERT_SIZE(EntHandle, 0x4);

struct Font_s; // TODO
typedef Font_s Font;
typedef Font *FontHandle;

struct animationNumber_t {
  union {
    struct {
      uint16_t index : 13;
      uint16_t toggle : 1;
    };
    uint16_t packed;
  };
};

struct outPacket_t {
  int32_t p_cmdNumber;
  int32_t p_serverTime;
  int32_t p_realtime;
};

#pragma pack(push, 1)
class tlAtomicMutex {
public:
  uint64_t ThreadId;
  int LockCount;
  uint8_t _padding0C[4];
  tlAtomicMutex *ThisPtr;
};
ASSERT_SIZE(tlAtomicMutex, 0x18);
#pragma pack(pop)

// Unverified.
enum class consoleChannel_e : uint32_t {
  CHANNEL_DONT_FILTER = 0x0,
  CHANNEL_GAMENOTIFY = 0x1,
  CHANNEL_BOLDGAME = 0x2,
  CHANNEL_OBJNOTIFY = 0x3,
  CHANNEL_SUBTITLE = 0x4,
  CHANNEL_OBITUARY = 0x5,
  CHANNEL_COOPINFO = 0x6,
  CHANNEL_WARNING = 0x7,
  CHANNEL_ERROR = 0x8,
  CHANNEL_INFO = 0x9,
  BUILTIN_CHANNEL_COUNT = 0xA,
  FIRST_DEBUG_CHANNEL = 0x9,
};

enum class RestartMethod_t : uint32_t {
  FULL = 0x0,
  ROUND = 0x1,
  MYCHANGES = 0x2,
};
IMPL_ENUM_OPERATORS(RestartMethod_t);

struct viewClamp {
  vec2_t start;
  vec2_t current;
  vec2_t goal;
};

struct viewClampState {
  viewClamp min;
  viewClamp max;
  float accelTime;
  float decelTime;
  float totalTime;
  float startTime;
};

} // namespace game