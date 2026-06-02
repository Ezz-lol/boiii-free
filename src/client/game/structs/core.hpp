#pragma once

#include <cstddef>
#include <cstdint>
#include <stdfloat>
#include <csetjmp>
#include <variant>

#define PROTOCOL 8
#define SUB_PROTOCOL 1

namespace game {

// Automatically pad a partially defined (reverse-engineered, in our case)
// struct to a fixed, known-correct total length
#define partial_def(TotalSize, PrimType, Fixed, Verified)                      \
  PrimType Fixed {                                                             \
    union {                                                                    \
      PrimType Verified;                                                       \
      uint8_t __raw[TotalSize];                                                \
    };                                                                         \
  }

#define inline_partial_def(InlineNum, FixedTotalSize, PrimType, Verified)      \
  union {                                                                      \
    PrimType Verified verified_##InlineNum;                                    \
    uint8_t __raw_##InlineNum[FixedTotalSize];                                 \
  }

typedef void *UnknownPtr;

#define STR(x) #x

#define ASSERT_SIZE(type, size)                                                \
  static_assert(sizeof(type) == (size), "sizeof(" #type ") != " STR(size))

#define ASSERT_OFFSET(type, field, offset)                                     \
  static_assert(offsetof(type, field) == (offset),                             \
                "offsetof(" #type ", " #field ") != " STR(offset))

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif
template <typename T, typename... Args>
using stdcall_t = T(__stdcall *)(Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename... Args>
using stdcallPtr = stdcall_t<T, Args...> *;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif
template <typename T, typename... Args>
using fastcall_t = T(__fastcall *)(Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename... Args>
using fastcallPtr = fastcall_t<T, Args...> *;

template <typename T, typename... Args> using cdecl_t = T(__cdecl *)(Args...);
template <typename T, typename... Args> using cdeclPtr = cdecl_t<T, Args...> *;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-attributes"
#endif
template <typename T, typename This = void, typename... Args>
using thiscall_t = T(__thiscall *)(This *, Args...);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

template <typename T, typename This = void, typename... Args>
using thiscallPtr = thiscall_t<T, This, Args...> *;

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

enum clientplatform_t : int32_t {
  CLIENT_PLATFORM_PC = 0x0,      // PC
  CLIENT_PLATFORM_ORBIS = 0x1,   // PS4
  CLIENT_PLATFORM_DURANGO = 0x2, // Xbox One
  MAX_CLIENT_PLATFORMS = 0x3,
};

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

inline ClientNum_t &operator++(ClientNum_t &s) {
  using underlying = std::underlying_type_t<ClientNum_t>;
  s = static_cast<ClientNum_t>(static_cast<underlying>(s) + 1);
  return s;
}

inline ClientNum_t operator++(ClientNum_t &s, int) {
  ClientNum_t temp = s;
  ++s;
  return temp;
}

enum ControllerIndex_t : int32_t {
  INVALID_CONTROLLER_PORT = -1,
  CONTROLLER_INDEX_FIRST = 0x0,
  CONTROLLER_INDEX_0 = 0x0,
  CONTROLLER_INDEX_1 = 0x1,
  CONTROLLER_INDEX_COUNT = 0x2,
};

inline ControllerIndex_t &operator++(ControllerIndex_t &s) {
  using underlying = std::underlying_type_t<ControllerIndex_t>;
  s = static_cast<ControllerIndex_t>(static_cast<underlying>(s) + 1);
  return s;
}

inline ControllerIndex_t operator++(ControllerIndex_t &s, int) {
  ControllerIndex_t temp = s;
  ++s;
  return temp;
}

enum LocalClientNum_t : int32_t {
  INVALID_LOCAL_CLIENT = -1,
  LOCAL_CLIENT_0 = 0x0,
  LOCAL_CLIENT_FIRST = 0x0,
  LOCAL_CLIENT_KEYBOARD_AND_MOUSE = 0x0,
  LOCAL_CLIENT_1 = 0x1,
  LOCAL_CLIENT_COUNT = 0x2,
};

inline LocalClientNum_t &operator++(LocalClientNum_t &s) {
  using underlying = std::underlying_type_t<LocalClientNum_t>;
  s = static_cast<LocalClientNum_t>(static_cast<underlying>(s) + 1);
  return s;
}

inline LocalClientNum_t operator++(LocalClientNum_t &s, int) {
  LocalClientNum_t temp = s;
  ++s;
  return temp;
}

enum class LocalClientNum8_t : int8_t {
  INVALID_LOCAL_CLIENT = -1,
  LOCAL_CLIENT_0 = 0x0,
  LOCAL_CLIENT_FIRST = 0x0,
  LOCAL_CLIENT_KEYBOARD_AND_MOUSE = 0x0,
  LOCAL_CLIENT_1 = 0x1,
  LOCAL_CLIENT_COUNT = 0x2,
};

inline LocalClientNum8_t &operator++(LocalClientNum8_t &s) {
  using underlying = std::underlying_type_t<LocalClientNum8_t>;
  s = static_cast<LocalClientNum8_t>(static_cast<underlying>(s) + 1);
  return s;
}

inline LocalClientNum8_t operator++(LocalClientNum8_t &s, int) {
  LocalClientNum8_t temp = s;
  ++s;
  return temp;
}

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
enum class eModes : uint32_t {
  ZOMBIES = 0x0,
  FIRST = 0x0,
  MULTIPLAYER = 0x1,
  CAMPAIGN = 0x2,
  COUNT = 0x3,
  INVALID = 0x3,
};

inline eModes &operator++(eModes &s) {
  using underlying = std::underlying_type_t<eModes>;
  s = static_cast<eModes>(static_cast<underlying>(s) + 1);
  return s;
}

inline eModes operator++(eModes &s, int) {
  eModes temp = s;
  ++s;
  return temp;
}

enum class eNetworkModes : uint32_t {
  OFFLINE = 0x0,
  SYSTEMLINK = 0x1,
  ONLINE = 0x2,
  COUNT = 0x3,
  INVALID = 0x3,
};

enum class MapPreload : uint32_t {
  NONE = 0x0,
  FRONTEND = 0x1,
  IN_GAME = 0x2,
};

enum class itemTextStyle : uint32_t {
  NORMAL = 0x0,
  SHADOWED = 0x3,
  SHADOWEDMORE = 0x6,
  BORDERED = 0x7,
  BORDEREDMORE = 0x8,
  MONOSPACE = 0x80,
  MONOSPACESHADOWED = 0x84,
};

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

template <typename T, const auto N> using array = T[N];

template <typename T, const auto X, const auto Y>
using matrix2d = array<array<T, Y>, X>;

constexpr uint32_t PLAYER_NAME_MAX_LEN = 32;
constexpr uint32_t PLAYER_CLAN_ABBREV_MAX_LEN = 8;

template <typename T> using LocalClientPool = array<T, LOCAL_CLIENT_COUNT>;

template <typename ClientType, typename ServerType>
using EngineDependent = std::variant<ClientType, ServerType>;

template <const auto T> using str = array<char, T>;

typedef str<16> str16_t;
typedef str<24> str24_t;
typedef str<32> str32_t;
typedef str<64> str64_t;
typedef str<128> str128_t;

typedef int32_t cinematic_id;

typedef float vec_t;

struct netUInt64 {
  uint32_t low;
  uint32_t high;
};

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

enum class CriticalSection : int32_t {
  ALLOC_MARK = 0x0,
  FX_VIS = 0x1,
  OCCLUSION_QUERY = 0x2,
  PHYSICS = 0x3,
  PHYSICS_UPDATE = 0x4,
  PHYSICS_DESTRUCTIBLE_HIT = 0x5,
  STREAM_ALLOC = 0x6,
  STREAM_BACKEND_UPDATE = 0x7,
  STREAM_CANCEL = 0x8,
  STREAM_MODEL_BITS = 0x9,
  FX_CHUNK_ALLOC = 0xA,
  FX_UNIQUE_HANDLE = 0xB,
  SOUND_COMMAND_ALLOC = 0xC,
  SOUND_COMMAND_PUSH = 0xD,
  SOUND_NOTIFY_ALLOC = 0xE,
  SOUND_NOTIFY_PUSH = 0xF,
  SOUND_BANK = 0x10,
  SOUND_LOOKUP_CACHE = 0x11,
  SOUND_SUBMIT = 0x12,
  CAREER_STATS = 0x13,
  CONSOLE = 0x14,
  DEBUG_SOCKET = 0x15,
  COM_ERROR = 0x16,
  STATMON = 0x17,
  MEM_ALLOC0 = 0x18,
  MEM_ALLOC1 = 0x19,
  MEM_ALLOC2 = 0x1A,
  MEM_ALLOC3 = 0x1B,
  MEM_ALLOC4 = 0x1C,
  MEM_ALLOC5 = 0x1D,
  MEM_ALLOC6 = 0x1E,
  MEM_ALLOC7 = 0x1F,
  MEM_ALLOC8 = 0x20,
  MEM_ASYNC_COMMIT = 0x21,
  CLIENT_MESSAGE = 0x22,
  CLIENT_CMD = 0x23,
  DOBJ_ALLOC = 0x24,
  XANIM_ALLOC = 0x25,
  KEY_BINDINGS = 0x26,
  SERVER_MESSAGE = 0x27,
  SERVER_PLAYERINFO = 0x28,
  SERVER_TICK = 0x29,
  SCRIPT_STRING = 0x2A,
  MEMORY_TREE = 0x2B,
  SCRIPT_DEBUGGER = 0x2C,
  VM = 0x2D,
  THREADED_NOTIFY_QUEUE = 0x2E,
  LIVE = 0x2F,
  PCACHE = 0x30,
  AUDIO_PHYSICS = 0x31,
  LUA = 0x32,
  LUI = 0x33,
  LOBBY_LUA = 0x34,
  EXTRACAM = 0x35,
  CINEMATIC = 0x36,
  CINEMATIC_MP4 = 0x37,
  CINEMATIC_UPDATEFRAME = 0x38,
  SAVE_PROFILE = 0x39,
  MEM_CONTAINER_SAVEGAME = 0x3A,
  VIRTUALALLOC = 0x3B,
  RB_TRANSFER = 0x3C,
  STREAM_SYNC_COMMAND = 0x3D,
  STREAM_FIXEDIMAGE_UPDATE = 0x3E,
  STREAM_MODELHINT_UPDATE = 0x3F,
  DWLSG = 0x40,
  DWNET = 0x41,
  IK = 0x42,
  TL_MEMALLOC = 0x43,
  VA_ALLOC = 0x44,
  MEMTRACK = 0x45,
  CBUF = 0x46,
  CURVEALLOC = 0x47,
  NETQUEUE = 0x48,
  ZLIB = 0x49,
  BLACKBOX = 0x4A,
  GDT_COMMAND = 0x4B,
  STRINGED_COMMAND = 0x4C,
  RADIANT_SERVER_COMMAND = 0x4D,
  RADIANT_CLIENT_COMMAND = 0x4E,
  RECORDER = 0x4F,
  SERVERDEMO = 0x50,
  IO_SCHEDULER = 0x51,
  FILE_ID_ARRAY = 0x52,
  MEMFIRSTFIT = 0x53,
  FXBEAM = 0x54,
  GLASS_ACTIONS = 0x55,
  DBHASH = 0x56,
  CLUMP = 0x57,
  SNAPSHOT_PROFILE = 0x58,
  CRITSEC_WEBM_STREAM_ACCESS = 0x59,
  CRITSEC_SV_LEADERBOARDS = 0x5A,
  FONT_CACHE = 0x5B,
  CRITSEC_NETCHAN = 0x5C,
  SWITCHMAP_SV = 0x5D,
  SWITCHMAP_CL = 0x5E,
  DB_THROTTLE_DECOMPRESSION = 0x5F,
  PROFILE_BUF = 0x60,
  SAVE_DEVICE = 0x61,
  WATER_INTERACTION = 0x62,
  WATER_GRID_ALLOC = 0x63,
  WATER_NODE_ALLOC = 0x64,
  MEM_PAGED = 0x65,
  ENTITY_COLL_LINK = 0x66,
  MINIZ = 0x67,
  COMPOSITING = 0x68,
  CREATE_DYNENT = 0x69,
  LENSFLARES_INSTANCE_POOL_ACCESS0 = 0x6A,
  LENSFLARES_INSTANCE_POOL_ACCESS1 = 0x6B,
  LENSFLARES_INSTANCE_POOL_ACCESS2 = 0x6C,
  LENSFLARES_INSTANCE_POOL_ACCESS3 = 0x6D,
  LENSFLARES_INSTANCE_POOL_ACCESS4 = 0x6E,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS0 = 0x6F,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS1 = 0x70,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS2 = 0x71,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS3 = 0x72,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS4 = 0x73,
  LENSFLARES_SOURCE_ACCESS0 = 0x74,
  LENSFLARES_SOURCE_ACCESS1 = 0x75,
  LENSFLARES_SOURCE_ACCESS2 = 0x76,
  LENSFLARES_SOURCE_ACCESS3 = 0x77,
  LENSFLARES_SOURCE_ACCESS4 = 0x78,
  COMSCORE = 0x79,
  WEAPON_DOBJ_INFO_ALLOC = 0x7A,
  LIVE_ALLOC = 0x7B,
  UMBRA_SPOTOMNI_CACHE = 0x7C,
  UMBRA_JOB = 0x7D,
  COUNT = 0x7E,
};

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
  COUNT = 0x30,
};

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

enum class SwimStateType : int32_t {
  NONE = 0x0,
  SURFACE = 0x1,
  UNDERWATER = 0x2,
  COUNT = 0x3,
};

enum class KillCamEntityRestState : int32_t {
  MOVING = 0x0,
  AT_REST = 0x1,
  STUCK_GROUND = 0x2,
  STUCK_WALL = 0x3,
};

enum class KillCamEntityType : int32_t {
  NO_ENTITY = 0x0,
  HELICOPTER = 0x1,
  DESTRUCTIBLE = 0x2,
  SCRIPTED = 0x3,
  EXPLOSIVE = 0x4,
  FAST_EXPLOSIVE = 0x5,
  ROCKET = 0x6,
  DRONE = 0x7,
  DOG = 0x8,
  COMBAT_ROBOT = 0x9,
  ARTILLERY = 0xA,
  VEHICLE = 0xB,
};

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

enum class SettingTeamIndicator : int32_t {
  FULL = 0x0,
  ABBREVIATED = 0x1,
  ICON = 0x2,
  COUNT = 0x3,
};

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

enum class DemoType : int32_t {
  NONE = 0x0,
  CLIENT = 0x1,
  SERVER = 0x2,
  SERVER_SNAPSHOT = 0x3,
};

enum class TraceHitType : int32_t {
  NONE = 0x0,
  ENTITY = 0x1,
  DYNENT_MODEL = 0x2,
  DYNENT_BRUSH = 0x3,
  GLASS = 0x4,
};

enum class MissileFlightMode : int32_t {
  TOP = 0x0,
  DIRECT = 0x1,
};

enum class MissileStage : int32_t {
  SOFTLAUNCH = 0x0,
  ASCENT = 0x1,
  DESCENT = 0x2,
};

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

template <typename T = vec_t> union vec2 {
  T v[2];
  struct {
    T x;
    T y;
  };
};
typedef vec2<vec_t> vec2_t;

template <typename T = vec_t> union vec3_main {
  struct {
    T x;
    T y;
    T z;
  };

  T v[3];
};

template <typename T = vec_t> union vec3 {
  struct {
    T x;
    T y;
    T z;
  };

  vec2<T> xy;
  T v[3];
};

typedef vec3<vec_t> vec3_t;

template <typename T = vec_t> union vec4 {
  T v[4];
  struct {
    T x;
    T y;
    T z;
    T w;
  };
  struct {
    T r;
    T g;
    T b;
    T a;
  };
  vec2<T> xy;
  vec3_main<T> xyz;
};

typedef vec4<vec_t> vec4_t;

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

enum class StanceState : int32_t {
  STAND = 0x0,
  CROUCH = 0x1,
  PRONE = 0x2,
  DIVE_TO_PRONE = 0x3,
  SLIDE = 0x4,
};

struct clientActive_t {
  char __pad0[0xB8C8];
  float viewangles[3];
  char __pad1[0x18C15C];
};

ASSERT_SIZE(clientActive_t, 0x197A30);

using fileHandle_t = void *;

struct DDLMember {
  const char *name;
  int32_t index;
  void *parent;
  int32_t bitSize;
  int32_t limitSize;
  int32_t offset;
  int32_t type;
  int32_t externalIndex;
  uint32_t rangeLimit;
  uint32_t serverDelta;
  uint32_t clientDelta;
  int32_t arraySize;
  int32_t enumIndex;
  int32_t permission;
};

struct DDLHash {
  int32_t hash;
  int32_t index;
};

struct DDLHashTable {
  DDLHash *list;
  int32_t count;
  int32_t max;
};

struct DDLStruct {
  const char *name;
  int32_t bitSize;
  int32_t memberCount;
  DDLMember *members;
  DDLHashTable hashTableUpper;
  DDLHashTable hashTableLower;
};

struct DDLEnum {
  const char *name;
  int32_t memberCount;
  const char **members;
  DDLHashTable hashTable;
};

struct DDLDef {
  char *name;
  uint16_t version;
  uint32_t checksum;
  uint8_t flags;
  int32_t bitSize;
  int32_t byteSize;
  DDLStruct *structList;
  int32_t structCount;
  DDLEnum *enumList;
  int32_t enumCount;
  DDLDef *next;
  int32_t headerBitSize;
  int32_t headerByteSize;
  int32_t reserveSize;
  int32_t userFlagsSize;
  bool paddingUsed;
};

struct DDLContext;
using DDLWriteCB = void (*)(DDLContext *, void *);

struct DDLContext {
  void *buff;
  int32_t len;
  const DDLDef *def;
  DDLWriteCB writeCB;
  void *userData;
};

struct emblemChallengeLookup_t {
  int16_t challengeIndex;
  uint8_t itemIndex;
};

enum class CharacterItemType : uint32_t {
  BODY = 0x0,
  HELMET = 0x1,
  COUNT = 0x2,
};

using BGEmblemBackgroundID = int16_t;

#pragma pack(push, 1)
template <const size_t B> struct bitarray {
  array<int32_t, (B + 31) / 32> data;

  friend constexpr void set(bitarray<B> *b, size_t index) noexcept {
    return b->data[index / 32] |= (1 << (index % 32));
  }

  friend constexpr void clear(bitarray<B> *b, size_t index) noexcept {
    return b->data[index / 32] &= ~(1 << (index % 32));
  }

  friend constexpr bool get(bitarray<B> *b, size_t index) noexcept {
    return (b->data[index / 32] & (1 << (index % 32))) != 0;
  }
};
#pragma pack(pop)

typedef bitarray<72> game_button_bits_t;
ASSERT_SIZE(game_button_bits_t, 0xC);

#pragma pack(push, 1)

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

enum class objectiveDrawState_t : int32_t {
  ONSCREEN = 0x0,
  OFFSCREEN = 0x1,
};

enum class objectiveState_t : int32_t {
  EMPTY = 0x0,
  ACTIVE = 0x1,
  INVISIBLE = 0x2,
  DONE = 0x3,
  CURRENT = 0x4,
  FAILED = 0x5,
  NUMSTATES = 0x6,
};

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
#pragma pack(pop)

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

} // namespace game