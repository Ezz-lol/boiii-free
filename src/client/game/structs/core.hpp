#pragma once

#include <cstdint>

#include <structs/atomic.hpp>
#include <structs/func.hpp>
#include <structs/str.hpp>

#include <game/structs/macros.hpp>
#include <game/structs/quake/vec.hpp>
#include <game/symbol.hpp>

inline constexpr auto PROTOCOL = 8;
inline constexpr auto SUB_PROTOCOL = 1;

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
typedef bdInt bdInt32;
typedef uint32_t bdUInt32;
typedef uint16_t bdUInt16;
typedef bdUInt16 bdPort;
typedef bool bdBool;
typedef int16_t bdWord;
typedef uint16_t bdUWord;
typedef bdUInt ulong32;

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

inline constexpr size_t iw_asset_magic_len(const size_t ext_len) {
  return sizeof(IW_ASSET_SHEBANG) + ext_len + sizeof(CRLF) +
         sizeof(char(NULL)) + sizeof(char) /* latest version */;
}

template <const size_t ExtensionLen>
inline constexpr std::array<char, iw_asset_magic_len(ExtensionLen)>
iw_asset_magic(const char (&ext)[ExtensionLen], const uint8_t latest_version) {
  return concat_bytes<char>(IW_ASSET_SHEBANG, ext, CRLF, NULL, latest_version);
}

enum class CampaignMode : int32_t {
  DEFAULT = 0,
  ZOMBIES = 1,
  COUNT = 2,
  INVALID = 3,
};

enum class ContentFlagBits : uint32_t {
  // Explicitly used for unknown content by engine
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
    // Explicitly used for unknown content by engine
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

    uint32_t : 17;
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
    return ContentFlags{.bits = {
                            .unknown = 0,
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
                        }};
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

inline constexpr const char *serialize(const team_t team) {
  switch (team) {
  case team_t::TEAM_DEAD:
    return "TEAM_DEAD";
  case team_t::TEAM_ALLIES:
    return "TEAM_ALLIES";
  case team_t::TEAM_AXIS:
    return "TEAM_AXIS";
  case team_t::TEAM_THREE:
    return "TEAM_THREE";
  case team_t::TEAM_FOUR:
    return "TEAM_FOUR";
  case team_t::TEAM_FIVE:
    return "TEAM_FIVE";
  case team_t::TEAM_SIX:
    return "TEAM_SIX";
  case team_t::TEAM_SEVEN:
    return "TEAM_SEVEN";
  case team_t::TEAM_EIGHT:
    return "TEAM_EIGHT";
  case team_t::TEAM_NINE:
    return "TEAM_NINE";
  case team_t::TEAM_TEN:
    return "TEAM_TEN";
  case team_t::TEAM_ELEVEN:
    return "TEAM_ELEVEN";
  case team_t::TEAM_TWELVE:
    return "TEAM_TWELVE";
  case team_t::TEAM_THIRTEEN:
    return "TEAM_THIRTEEN";
  case team_t::TEAM_FOURTEEN:
    return "TEAM_FOURTEEN";
  case team_t::TEAM_FIFTEEN:
    return "TEAM_FIFTEEN";
  case team_t::TEAM_SIXTEEN:
    return "TEAM_SIXTEEN";
  case team_t::TEAM_SEVENTEEN:
    return "TEAM_SEVENTEEN";
  case team_t::TEAM_EIGHTEEN:
    return "TEAM_EIGHTEEN";
  case team_t::TEAM_NEUTRAL:
    return "TEAM_NEUTRAL";
  case team_t::TEAM_SPECTATOR:
    return "TEAM_SPECTATOR";
  case team_t::TEAM_LOCALPLAYERS:
    return "TEAM_LOCALPLAYERS";
  default:
    return "TEAM_INVALID";
  }
}

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
inline constexpr const char *serialize(const ClientNum_t clientNum) {
  switch (clientNum) {
  case CLIENT_INDEX_0:
    return "CLIENT_INDEX_0";
  case CLIENT_INDEX_1:
    return "CLIENT_INDEX_1";
  case CLIENT_INDEX_2:
    return "CLIENT_INDEX_2";
  case CLIENT_INDEX_3:
    return "CLIENT_INDEX_3";
  case CLIENT_INDEX_4:
    return "CLIENT_INDEX_4";
  case CLIENT_INDEX_5:
    return "CLIENT_INDEX_5";
  case CLIENT_INDEX_6:
    return "CLIENT_INDEX_6";
  case CLIENT_INDEX_7:
    return "CLIENT_INDEX_7";
  case CLIENT_INDEX_8:
    return "CLIENT_INDEX_8";
  case CLIENT_INDEX_9:
    return "CLIENT_INDEX_9";
  case CLIENT_INDEX_10:
    return "CLIENT_INDEX_10";
  case CLIENT_INDEX_11:
    return "CLIENT_INDEX_11";
  case CLIENT_INDEX_12:
    return "CLIENT_INDEX_12";
  case CLIENT_INDEX_13:
    return "CLIENT_INDEX_13";
  case CLIENT_INDEX_14:
    return "CLIENT_INDEX_14";
  case CLIENT_INDEX_15:
    return "CLIENT_INDEX_15";
  case CLIENT_INDEX_16:
    return "CLIENT_INDEX_16";
  case CLIENT_INDEX_17:
    return "CLIENT_INDEX_17";
  case CLIENT_INDEX_18:
    return "CLIENT_INDEX_18";
  case CLIENT_INDEX_19:
    return "CLIENT_INDEX_19";
  case CLIENT_INDEX_20:
    return "CLIENT_INDEX_20";
  case CLIENT_INDEX_21:
    return "CLIENT_INDEX_21";
  case CLIENT_INDEX_22:
    return "CLIENT_INDEX_22";
  case CLIENT_INDEX_23:
    return "CLIENT_INDEX_23";
  case CLIENT_INDEX_24:
    return "CLIENT_INDEX_24";
  case CLIENT_INDEX_25:
    return "CLIENT_INDEX_25";
  case CLIENT_INDEX_26:
    return "CLIENT_INDEX_26";
  case CLIENT_INDEX_27:
    return "CLIENT_INDEX_27";
  case CLIENT_INDEX_28:
    return "CLIENT_INDEX_28";
  case CLIENT_INDEX_29:
    return "CLIENT_INDEX_29";
  case CLIENT_INDEX_30:
    return "CLIENT_INDEX_30";
  case CLIENT_INDEX_31:
    return "CLIENT_INDEX_31";
  default:
    return "INVALID_CLIENT_INDEX";
  }
}

enum ClientNum16_t : int16_t {
  INVALID_CLIENT_INDEX_16 = -1,
  CLIENT_INDEX_16_0 = 0x0,
  CLIENT_INDEX_16_FIRST = 0x0,
  CLIENT_INDEX_16_1 = 0x1,
  CLIENT_INDEX_16_2 = 0x2,
  CLIENT_INDEX_16_3 = 0x3,
  CLIENT_INDEX_16_4 = 0x4,
  CLIENT_INDEX_16_5 = 0x5,
  CLIENT_INDEX_16_6 = 0x6,
  CLIENT_INDEX_16_7 = 0x7,
  CLIENT_INDEX_16_8 = 0x8,
  CLIENT_INDEX_16_9 = 0x9,
  CLIENT_INDEX_16_10 = 0xA,
  CLIENT_INDEX_16_11 = 0xB,
  CLIENT_INDEX_16_12 = 0xC,
  CLIENT_INDEX_16_13 = 0xD,
  CLIENT_INDEX_16_14 = 0xE,
  CLIENT_INDEX_16_15 = 0xF,
  CLIENT_INDEX_16_16 = 0x10,
  CLIENT_INDEX_16_17 = 0x11,
  CLIENT_INDEX_16_18 = 0x12,
  CLIENT_INDEX_16_19 = 0x13,
  CLIENT_INDEX_16_20 = 0x14,
  CLIENT_INDEX_16_21 = 0x15,
  CLIENT_INDEX_16_22 = 0x16,
  CLIENT_INDEX_16_23 = 0x17,
  CLIENT_INDEX_16_24 = 0x18,
  CLIENT_INDEX_16_25 = 0x19,
  CLIENT_INDEX_16_26 = 0x1A,
  CLIENT_INDEX_16_27 = 0x1B,
  CLIENT_INDEX_16_28 = 0x1C,
  CLIENT_INDEX_16_29 = 0x1D,
  CLIENT_INDEX_16_30 = 0x1E,
  CLIENT_INDEX_16_31 = 0x1F,
  CLIENT_INDEX_16_COUNT = 0x12,
};
IMPL_ENUM_OPERATORS(ClientNum16_t);
inline constexpr const char *serialize(const ClientNum16_t clientNum) {
  return serialize(clientNum == INVALID_CLIENT_INDEX_16
                       ? INVALID_CLIENT_INDEX
                       : static_cast<ClientNum_t>(clientNum));
}

enum ClientNum8_t : int8_t {
  INVALID_CLIENT_INDEX_8 = -1,
  CLIENT_INDEX_8_0 = 0x0,
  CLIENT_INDEX_8_FIRST = 0x0,
  CLIENT_INDEX_8_1 = 0x1,
  CLIENT_INDEX_8_2 = 0x2,
  CLIENT_INDEX_8_3 = 0x3,
  CLIENT_INDEX_8_4 = 0x4,
  CLIENT_INDEX_8_5 = 0x5,
  CLIENT_INDEX_8_6 = 0x6,
  CLIENT_INDEX_8_7 = 0x7,
  CLIENT_INDEX_8_8 = 0x8,
  CLIENT_INDEX_8_9 = 0x9,
  CLIENT_INDEX_8_10 = 0xA,
  CLIENT_INDEX_8_11 = 0xB,
  CLIENT_INDEX_8_12 = 0xC,
  CLIENT_INDEX_8_13 = 0xD,
  CLIENT_INDEX_8_14 = 0xE,
  CLIENT_INDEX_8_15 = 0xF,
  CLIENT_INDEX_8_16 = 0x10,
  CLIENT_INDEX_8_17 = 0x11,
  CLIENT_INDEX_8_18 = 0x12,
  CLIENT_INDEX_8_19 = 0x13,
  CLIENT_INDEX_8_20 = 0x14,
  CLIENT_INDEX_8_21 = 0x15,
  CLIENT_INDEX_8_22 = 0x16,
  CLIENT_INDEX_8_23 = 0x17,
  CLIENT_INDEX_8_24 = 0x18,
  CLIENT_INDEX_8_25 = 0x19,
  CLIENT_INDEX_8_26 = 0x1A,
  CLIENT_INDEX_8_27 = 0x1B,
  CLIENT_INDEX_8_28 = 0x1C,
  CLIENT_INDEX_8_29 = 0x1D,
  CLIENT_INDEX_8_30 = 0x1E,
  CLIENT_INDEX_8_31 = 0x1F,
  CLIENT_INDEX_8_COUNT = 0x12,
};
IMPL_ENUM_OPERATORS(ClientNum8_t);
inline constexpr const char *serialize(const ClientNum8_t clientNum) {
  return serialize(clientNum == INVALID_CLIENT_INDEX_8
                       ? INVALID_CLIENT_INDEX
                       : static_cast<ClientNum_t>(clientNum));
}

enum ControllerIndex_t : int32_t {
  INVALID_CONTROLLER_PORT = -1,
  CONTROLLER_INDEX_FIRST = 0x0,
  CONTROLLER_INDEX_0 = 0x0,
  CONTROLLER_INDEX_1 = 0x1,
  CONTROLLER_INDEX_COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(ControllerIndex_t);

inline constexpr const char *
serialize(const ControllerIndex_t controllerIndex) {
  switch (controllerIndex) {
  case CONTROLLER_INDEX_0:
    return "CONTROLLER_INDEX_0";
  case CONTROLLER_INDEX_1:
    return "CONTROLLER_INDEX_1";
  default:
    return "INVALID_CONTROLLER_PORT";
  }
}

enum LocalClientNum_t : int32_t {
  INVALID_LOCAL_CLIENT = -1,
  LOCAL_CLIENT_0 = 0x0,
  LOCAL_CLIENT_FIRST = 0x0,
  LOCAL_CLIENT_KEYBOARD_AND_MOUSE = 0x0,
  LOCAL_CLIENT_1 = 0x1,
  LOCAL_CLIENT_COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(LocalClientNum_t);

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
  SOFTRESTART_SILENT = 0x1000,
};
IMPL_ENUM_OPERATORS(errorParm);

template <typename T> using LocalClientPool = array<T, LOCAL_CLIENT_COUNT>;
template <typename T>
using AtomicLocalClientPool = atomicarray<T, LOCAL_CLIENT_COUNT>;

template <typename ClientType, typename ServerType> struct EngineDependent {
public:
  // Unions cannot be used as base struct/class, so we wrap an anonymous union
  // in a struct here as a workaround.
  union {
  public:
    const ClientType *cl;
    const ServerType *sv;
  };

  const void *ptr;

  inline constexpr operator const ServerType *() const noexcept { return sv; }

  inline constexpr operator const ClientType *() const noexcept { return cl; }

  inline constexpr operator const void *() const noexcept { return ptr; }

  inline constexpr operator bool() const noexcept { return sv != nullptr; }

  // Function-based constructors to allow usage without violating CPP 2003 PoD
  // conformance
  static inline constexpr EngineDependent<ClientType, ServerType>
  from(const ServerType *s) noexcept {
    return {.sv = s};
  }

  static inline constexpr EngineDependent<ClientType, ServerType>
  from(const ClientType *c) noexcept {
    return {.cl = c};
  }

  static inline constexpr EngineDependent<ClientType, ServerType>
  from(const void *v) noexcept {
    return {.ptr = v};
  }
};

template <typename ClientType, typename ServerType> struct EngineDependentMut {
public:
  // Unions cannot be used as base struct/class, so we wrap an anonymous union
  // in a struct here as a workaround.
  union {
  public:
    ClientType *cl;
    ServerType *sv;
  };

  void *ptr;

  inline constexpr operator ServerType *() noexcept { return sv; }

  inline constexpr operator const ServerType *() const noexcept { return sv; }

  inline constexpr operator ClientType *() noexcept { return cl; }

  inline constexpr operator const ClientType *() const noexcept { return cl; }

  inline constexpr operator void *() noexcept { return ptr; }

  inline constexpr operator const void *() const noexcept { return ptr; }

  inline constexpr operator bool() const noexcept { return sv != nullptr; }

  inline constexpr
  operator EngineDependent<ClientType, ServerType>() const noexcept {
    return {.sv = sv};
  }

  // Function-based constructors to allow usage without violating CPP 2003 PoD
  // conformance
  static inline constexpr EngineDependentMut<ClientType, ServerType>
  from(ServerType *s) noexcept {
    return {.sv = s};
  }

  static inline constexpr EngineDependentMut<ClientType, ServerType>
  from(ClientType *c) noexcept {
    return {.cl = c};
  }

  static inline constexpr EngineDependentMut<ClientType, ServerType>
  from(void *v) noexcept {
    return {.ptr = v};
  }
};

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
inline constexpr const char *serialize(ZoneType zoneType) {
  switch (zoneType) {
  case ZoneType::OFFICIAL:
    return "ZoneType::OFFICIAL";
  case ZoneType::MOD:
    return "ZoneType::MOD";
  case ZoneType::USERMAP:
    return "ZoneType::USERMAP";
  default:
    return "ZoneType::INVALID";
  }
}
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

inline constexpr const char *serialize(const StorageFileType type) {
  switch (type) {
  case StorageFileType::COMMON_SETTINGS:
    return "STORAGE_COMMON_SETTINGS";
  case StorageFileType::PROFILE_SHOUTCASTER:
    return "STORAGE_PROFILE_SHOUTCASTER";
  case StorageFileType::CP_SAVEGAME_ONLINE:
    return "STORAGE_CP_SAVEGAME_ONLINE";
  case StorageFileType::CP_SAVEGAME_OFFLINE:
    return "STORAGE_CP_SAVEGAME_OFFLINE";
  case StorageFileType::CPNIGHTMARE_SAVEGAME_ONLINE:
    return "STORAGE_CPNIGHTMARE_SAVEGAME_ONLINE";
  case StorageFileType::CPNIGHTMARE_SAVEGAME_OFFLINE:
    return "STORAGE_CPNIGHTMARE_SAVEGAME_OFFLINE";
  case StorageFileType::CP_STATS_ONLINE:
    return "STORAGE_CP_STATS_ONLINE";
  case StorageFileType::CP_STATS_OFFLINE:
    return "STORAGE_CP_STATS_OFFLINE";
  case StorageFileType::CP_STATS_NIGHTMARE_ONLINE:
    return "STORAGE_CP_STATS_NIGHTMARE_ONLINE";
  case StorageFileType::CP_STATS_NIGHTMARE_OFFLINE:
    return "STORAGE_CP_STATS_NIGHTMARE_OFFLINE";
  case StorageFileType::CP_LOADOUTS:
    return "STORAGE_CP_LOADOUTS";
  case StorageFileType::CP_LOADOUTS_OFFLINE:
    return "STORAGE_CP_LOADOUTS_OFFLINE";
  case StorageFileType::MP_STATS_ONLINE:
    return "STORAGE_MP_STATS_ONLINE";
  case StorageFileType::MP_STATS_OFFLINE:
    return "STORAGE_MP_STATS_OFFLINE";
  case StorageFileType::MP_LOADOUTS:
    return "STORAGE_MP_LOADOUTS";
  case StorageFileType::MP_LOADOUTS_OFFLINE:
    return "STORAGE_MP_LOADOUTS_OFFLINE";
  case StorageFileType::MP_CLASS_SETS:
    return "STORAGE_MP_CLASS_SETS";
  case StorageFileType::ZM_STATS_ONLINE:
    return "STORAGE_ZM_STATS_ONLINE";
  case StorageFileType::ZM_STATS_OFFLINE:
    return "STORAGE_ZM_STATS_OFFLINE";
  case StorageFileType::ZM_LOADOUTS:
    return "STORAGE_ZM_LOADOUTS";
  case StorageFileType::ZM_LOADOUTS_OFFLINE:
    return "STORAGE_ZM_LOADOUTS_OFFLINE";
  case StorageFileType::FR_STATS_ONLINE:
    return "STORAGE_FR_STATS_ONLINE";
  case StorageFileType::FR_STATS_OFFLINE:
    return "STORAGE_FR_STATS_OFFLINE";
  case StorageFileType::PAINTSHOP_DATA:
    return "STORAGE_PAINTSHOP_DATA";
  case StorageFileType::GUNSMITH:
    return "STORAGE_GUNSMITH";
  case StorageFileType::PAINTJOBS:
    return "STORAGE_PAINTJOBS";
  case StorageFileType::EMBLEMS:
    return "STORAGE_EMBLEMS";
  case StorageFileType::DEFAULT_EMBLEMS:
    return "STORAGE_DEFAULT_EMBLEMS";
  case StorageFileType::EMBLEMS_LOOT:
    return "STORAGE_EMBLEMS_LOOT";
  case StorageFileType::CUSTOM_GAMES:
    return "STORAGE_CUSTOM_GAMES";
  case StorageFileType::OFFICIAL_CUSTOM_GAMES:
    return "STORAGE_OFFICIAL_CUSTOM_GAMES";
  case StorageFileType::EXTERNAL_DATA:
    return "STORAGE_EXTERNAL_DATA";
  default:
    return "STORAGE_INVALID";
  }
}

// Verified
enum class consoleLabel : int32_t {
  DEFAULT = 0,
  TEMP = 0x1,
  GFX = 0x2,
  TASKMGR2 = 0x3,
  LIVE = 0x4,
  LIVE_XBOX = 0x5,
  LIVE_PS4 = 0x6,
  MATCHMAKING = 0x7,
  DEMONWARE = 0x8,
  LEADERBOARDS = 0x9,
  LOBBY = 0xA,
  LOBBYHOST = 0xB,
  LOBBYCLIENT = 0xC,
  LOBBYVM = 0xD,
  MIGRATION = 0xE,
  IG_MIGRATION_HOST = 0xF,
  IG_MIGRATION_CLIENT = 0x10,
  SCRIPTER = 0x11,
  VM = 0x12,
  DVAR = 0x13,
  TOOL = 0x14,
  ANIM = 0x15,
  NETCHAN = 0x16,
  BGCACHE = 0x17,
  PM = 0x18,
  MAPSWITCH = 0x19,
  AI = 0x1A,
  GADGET = 0x1B,
  SOUND = 0x1C,
  SNAPSHOT = 0x1D,
  PLAYGO = 0x1E,
  LUI = 0x1F,
  LUA = 0x20,
  VOIP = 0x21,
  DEMO = 0x22,
  DB = 0x23,
  HTTP = 0x24,
  COMPANION = 0x25,
  MEM = 0x26,
  CINEMATIC = 0x27,
  DDL = 0x28,
  STORAGE = 0x29,
  STEAM = 0x2A,
  CHKPTSAVE = 0x2B,
  THUNDERHEAD = 0x2C,
  COMSCORE = 0x2D,
  FILESHARE = 0x2E,
  LPC = 0x2F,
  MARKETING = 0x30,
  STORE = 0x31,
  TESTING = 0x32,
  LOOT = 0x33,
  MATCHRECORDER = 0x34,
  EXCHANGE = 0x35,
  SCRIPTERROR = 0x36,
  CONSOLE = 0x37,
  COUNT = 0x38
};
IMPL_ENUM_OPERATORS(consoleLabel);
typedef consoleLabel consoleLabel_e;

inline constexpr const char *serialize(const consoleLabel label) {
  switch (label) {
  case consoleLabel::DEFAULT:
    return "CONSOLE_LABEL_DEFAULT";
  case consoleLabel::TEMP:
    return "CONSOLE_LABEL_TEMP";
  case consoleLabel::GFX:
    return "CONSOLE_LABEL_GFX";
  case consoleLabel::TASKMGR2:
    return "CONSOLE_LABEL_TASKMGR2";
  case consoleLabel::LIVE:
    return "CONSOLE_LABEL_LIVE";
  case consoleLabel::LIVE_XBOX:
    return "CONSOLE_LABEL_LIVE_XBOX";
  case consoleLabel::LIVE_PS4:
    return "CONSOLE_LABEL_LIVE_PS4";
  case consoleLabel::MATCHMAKING:
    return "CONSOLE_LABEL_MATCHMAKING";
  case consoleLabel::DEMONWARE:
    return "CONSOLE_LABEL_DEMONWARE";
  case consoleLabel::LEADERBOARDS:
    return "CONSOLE_LABEL_LEADERBOARDS";
  case consoleLabel::LOBBY:
    return "CONSOLE_LABEL_LOBBY";
  case consoleLabel::LOBBYHOST:
    return "CONSOLE_LABEL_LOBBYHOST";
  case consoleLabel::LOBBYCLIENT:
    return "CONSOLE_LABEL_LOBBYCLIENT";
  case consoleLabel::LOBBYVM:
    return "CONSOLE_LABEL_LOBBYVM";
  case consoleLabel::MIGRATION:
    return "CONSOLE_LABEL_MIGRATION";
  case consoleLabel::IG_MIGRATION_HOST:
    return "CONSOLE_LABEL_IG_MIGRATION_HOST";
  case consoleLabel::IG_MIGRATION_CLIENT:
    return "CONSOLE_LABEL_IG_MIGRATION_CLIENT";
  case consoleLabel::SCRIPTER:
    return "CONSOLE_LABEL_SCRIPTER";
  case consoleLabel::VM:
    return "CONSOLE_LABEL_VM";
  case consoleLabel::DVAR:
    return "CONSOLE_LABEL_DVAR";
  case consoleLabel::TOOL:
    return "CONSOLE_LABEL_TOOL";
  case consoleLabel::ANIM:
    return "CONSOLE_LABEL_ANIM";
  case consoleLabel::NETCHAN:
    return "CONSOLE_LABEL_NETCHAN";
  case consoleLabel::BGCACHE:
    return "CONSOLE_LABEL_BGCACHE";
  case consoleLabel::PM:
    return "CONSOLE_LABEL_PM";
  case consoleLabel::MAPSWITCH:
    return "CONSOLE_LABEL_MAPSWITCH";
  case consoleLabel::AI:
    return "CONSOLE_LABEL_AI";
  case consoleLabel::GADGET:
    return "CONSOLE_LABEL_GADGET";
  case consoleLabel::SOUND:
    return "CONSOLE_LABEL_SOUND";
  case consoleLabel::SNAPSHOT:
    return "CONSOLE_LABEL_SNAPSHOT";
  case consoleLabel::PLAYGO:
    return "CONSOLE_LABEL_PLAYGO";
  case consoleLabel::LUI:
    return "CONSOLE_LABEL_LUI";
  case consoleLabel::LUA:
    return "CONSOLE_LABEL_LUA";
  case consoleLabel::VOIP:
    return "CONSOLE_LABEL_VOIP";
  case consoleLabel::DEMO:
    return "CONSOLE_LABEL_DEMO";
  case consoleLabel::DB:
    return "CONSOLE_LABEL_DB";
  case consoleLabel::HTTP:
    return "CONSOLE_LABEL_HTTP";
  case consoleLabel::COMPANION:
    return "CONSOLE_LABEL_COMPANION";
  case consoleLabel::MEM:
    return "CONSOLE_LABEL_MEM";
  case consoleLabel::CINEMATIC:
    return "CONSOLE_LABEL_CINEMATIC";
  case consoleLabel::DDL:
    return "CONSOLE_LABEL_DDL";
  case consoleLabel::STORAGE:
    return "CONSOLE_LABEL_STORAGE";
  case consoleLabel::STEAM:
    return "CONSOLE_LABEL_STEAM";
  case consoleLabel::CHKPTSAVE:
    return "CONSOLE_LABEL_CHKPTSAVE";
  case consoleLabel::THUNDERHEAD:
    return "CONSOLE_LABEL_THUNDERHEAD";
  case consoleLabel::COMSCORE:
    return "CONSOLE_LABEL_COMSCORE";
  case consoleLabel::FILESHARE:
    return "CONSOLE_LABEL_FILESHARE";
  case consoleLabel::LPC:
    return "CONSOLE_LABEL_LPC";
  case consoleLabel::MARKETING:
    return "CONSOLE_LABEL_MARKETING";
  case consoleLabel::STORE:
    return "CONSOLE_LABEL_STORE";
  case consoleLabel::TESTING:
    return "CONSOLE_LABEL_TESTING";
  case consoleLabel::LOOT:
    return "CONSOLE_LABEL_LOOT";
  case consoleLabel::MATCHRECORDER:
    return "CONSOLE_LABEL_MATCHRECORDER";
  case consoleLabel::EXCHANGE:
    return "CONSOLE_LABEL_EXCHANGE";
  case consoleLabel::SCRIPTERROR:
    return "CONSOLE_LABEL_SCRIPTERROR";
  case consoleLabel::CONSOLE:
    return "CONSOLE_LABEL_CONSOLE";
  default:
    return "CONSOLE_LABEL_INVALID";
  }
}

// Verified
enum class errorCode : uint32_t {
  NONE = 0,
  FATAL = 1,
  DROP = 2,
  FROM_STARTUP = 4,
  SERVERDISCONNECT = 8,
  DISCONNECT = 16,
  SCRIPT = 32,
  SCRIPT_DROP = 64,
  LOCALIZATION = 128,
  UI = 256,
  LUA = 512,
  SOFTRESTART = 1024,
  SOFTRESTART_KEEPDW = 2048,
  SOFTRESTART_SILENT = 4096,
};
IMPL_ENUM_OPERATORS(errorCode);

inline constexpr const char *serialize(const errorCode code) {
  switch (code) {
  case errorCode::NONE:
    return "ERROR_NONE";
  case errorCode::FATAL:
    return "ERROR_FATAL";
  case errorCode::DROP:
    return "ERROR_DROP";
  case errorCode::FROM_STARTUP:
    return "ERROR_FROM_STARTUP";
  case errorCode::SERVERDISCONNECT:
    return "ERROR_SERVERDISCONNECT";
  case errorCode::DISCONNECT:
    return "ERROR_DISCONNECT";
  case errorCode::SCRIPT:
    return "ERROR_SCRIPT";
  case errorCode::SCRIPT_DROP:
    return "ERROR_SCRIPT_DROP";
  case errorCode::LOCALIZATION:
    return "ERROR_LOCALIZATION";
  case errorCode::UI:
    return "ERROR_UI";
  case errorCode::LUA:
    return "ERROR_LUA";
  case errorCode::SOFTRESTART:
    return "ERROR_SOFTRESTART";
  case errorCode::SOFTRESTART_KEEPDW:
    return "ERROR_SOFTRESTART_KEEPDW";
  case errorCode::SOFTRESTART_SILENT:
    return "ERROR_SOFTRESTART_SILENT";
  default:
    return "ERROR_INVALID";
  }
}

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

inline constexpr const char *serialize(const PMemStack stack) {
  switch (stack) {
  case PMemStack::DB:
    return "PMEM_DB";
  case PMemStack::DB2:
    return "PMEM_DB2";
  case PMemStack::GAME:
    return "PMEM_GAME";
  case PMemStack::SERVER:
    return "PMEM_SERVER";
  case PMemStack::HOT:
    return "PMEM_HOT";
  case PMemStack::CINEMATICS:
    return "PMEM_CINEMATICS";
  case PMemStack::DYNAMIC_IMAGES:
    return "PMEM_DYNAMIC_IMAGES";
  case PMemStack::LIGHTING:
    return "PMEM_LIGHTING";
  case PMemStack::INVALID:
    return "PMEM_INVALID";
  default:
    return "PMEM_UNKNOWN";
  }
}

enum class PMemPool : uint32_t {
  MAIN = 0x0,
  GPU_PRIVATE = 0x1,
  GPU_PRT = 0x2,
  GPU_RENDER_TARGETS = 0x3,
  MAIN_COHERENT = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(PMemPool);

inline constexpr const char *serialize(const PMemPool pool) {
  switch (pool) {
  case PMemPool::MAIN:
    return "PMEMPOOL_MAIN";
  case PMemPool::GPU_PRIVATE:
    return "PMEMPOOL_GPU_PRIVATE";
  case PMemPool::GPU_PRT:
    return "PMEMPOOL_GPU_PRT";
  case PMemPool::GPU_RENDER_TARGETS:
    return "PMEMPOOL_GPU_RENDER_TARGETS";
  case PMemPool::MAIN_COHERENT:
    return "PMEMPOOL_MAIN_COHERENT";
  default:
    return "PMEMPOOL_INVALID";
  }
}

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

inline constexpr const char *serialize(const EMemTrack track) {
  switch (track) {
  case EMemTrack::TRACK_BINARIES:
    return "TRACK_BINARIES";
  case EMemTrack::TRACK_DEBUG:
    return "TRACK_DEBUG";
  case EMemTrack::TRACK_HUNK:
    return "TRACK_HUNK";
  case EMemTrack::TRACK_MISC_SWAP:
    return "TRACK_MISC_SWAP";
  case EMemTrack::TRACK_LOCALIZATION:
    return "TRACK_LOCALIZATION";
  case EMemTrack::TRACK_TL:
    return "TRACK_TL";
  case EMemTrack::TRACK_TEMP:
    return "TRACK_TEMP";
  case EMemTrack::TRACK_RECORDER:
    return "TRACK_RECORDER";
  case EMemTrack::TRACK_DEMO:
    return "TRACK_DEMO";
  case EMemTrack::TRACK_DVR:
    return "TRACK_DVR";
  case EMemTrack::TRACK_DVAR:
    return "TRACK_DVAR";
  case EMemTrack::TRACK_ASSET_DATABASE:
    return "TRACK_ASSET_DATABASE";
  case EMemTrack::TRACK_SAVEGAME:
    return "TRACK_SAVEGAME";
  case EMemTrack::TRACK_DELIMITER1:
    return "TRACK_DELIMITER1";
  case EMemTrack::TRACK_COMSCORE:
    return "TRACK_COMSCORE";
  case EMemTrack::TRACK_AI:
    return "TRACK_AI";
  case EMemTrack::TRACK_AI_BOTS:
    return "TRACK_AI_BOTS";
  case EMemTrack::TRACK_AI_NODES:
    return "TRACK_AI_NODES";
  case EMemTrack::TRACK_SCRIPT:
    return "TRACK_SCRIPT";
  case EMemTrack::TRACK_SCRIPT_DEBUG:
    return "TRACK_SCRIPT_DEBUG";
  case EMemTrack::TRACK_COLLISION_MISC:
    return "TRACK_COLLISION_MISC";
  case EMemTrack::TRACK_COLLISION_BRUSH:
    return "TRACK_COLLISION_BRUSH";
  case EMemTrack::TRACK_COLLISION_MODEL_TRI:
    return "TRACK_COLLISION_MODEL_TRI";
  case EMemTrack::TRACK_COLLISION_TERRAIN:
    return "TRACK_COLLISION_TERRAIN";
  case EMemTrack::TRACK_PHYSICS:
    return "TRACK_PHYSICS";
  case EMemTrack::TRACK_MAP_ENTS:
    return "TRACK_MAP_ENTS";
  case EMemTrack::TRACK_CLIENT:
    return "TRACK_CLIENT";
  case EMemTrack::TRACK_SERVER:
    return "TRACK_SERVER";
  case EMemTrack::TRACK_DDL:
    return "TRACK_DDL";
  case EMemTrack::TRACK_STORAGE:
    return "TRACK_STORAGE";
  case EMemTrack::TRACK_ONLINE:
    return "TRACK_ONLINE";
  case EMemTrack::TRACK_VEHICLEDEF:
    return "TRACK_VEHICLEDEF";
  case EMemTrack::TRACK_DELIMITER2:
    return "TRACK_DELIMITER2";
  case EMemTrack::TRACK_FASTFILE:
    return "TRACK_FASTFILE";
  case EMemTrack::TRACK_NETWORK_ENTITY:
    return "TRACK_NETWORK_ENTITY";
  case EMemTrack::TRACK_MISC:
    return "TRACK_MISC";
  case EMemTrack::TRACK_ANIMATION:
    return "TRACK_ANIMATION";
  case EMemTrack::TRACK_ANIMCACHE:
    return "TRACK_ANIMCACHE";
  case EMemTrack::TRACK_CLIENT_ANIMSCRIPT:
    return "TRACK_CLIENT_ANIMSCRIPT";
  case EMemTrack::TRACK_WORLD_GLOBALS:
    return "TRACK_WORLD_GLOBALS";
  case EMemTrack::TRACK_DELIMITER3:
    return "TRACK_DELIMITER3";
  case EMemTrack::TRACK_EMBLEM:
    return "TRACK_EMBLEM";
  case EMemTrack::TRACK_FX:
    return "TRACK_FX";
  case EMemTrack::TRACK_GLASS:
    return "TRACK_GLASS";
  case EMemTrack::TRACK_RENDERER_GLOBALS:
    return "TRACK_RENDERER_GLOBALS";
  case EMemTrack::TRACK_RENDERER_TARGETS:
    return "TRACK_RENDERER_TARGETS";
  case EMemTrack::TRACK_RENDERER_IMAGES:
    return "TRACK_RENDERER_IMAGES";
  case EMemTrack::TRACK_RENDERER_FX_IMAGES:
    return "TRACK_RENDERER_FX_IMAGES";
  case EMemTrack::TRACK_RENDERER_WORLD:
    return "TRACK_RENDERER_WORLD";
  case EMemTrack::TRACK_RENDERER_MODELS:
    return "TRACK_RENDERER_MODELS";
  case EMemTrack::TRACK_RENDERER_MISC:
    return "TRACK_RENDERER_MISC";
  case EMemTrack::TRACK_RENDERER_SIEGE:
    return "TRACK_RENDERER_SIEGE";
  case EMemTrack::TRACK_RENDERER_LIGHTS:
    return "TRACK_RENDERER_LIGHTS";
  case EMemTrack::TRACK_RENDERER_LIGHT_EXPORT:
    return "TRACK_RENDERER_LIGHT_EXPORT";
  case EMemTrack::TRACK_RENDERER_UMBRA:
    return "TRACK_RENDERER_UMBRA";
  case EMemTrack::TRACK_RENDERER_OIT:
    return "TRACK_RENDERER_OIT";
  case EMemTrack::TRACK_RSTREAM:
    return "TRACK_RSTREAM";
  case EMemTrack::TRACK_RENDERER_STREAMBUFFER:
    return "TRACK_RENDERER_STREAMBUFFER";
  case EMemTrack::TRACK_RENDERER_STREAMBUFFER_EXTRA:
    return "TRACK_RENDERER_STREAMBUFFER_EXTRA";
  case EMemTrack::TRACK_GEOSTREAM:
    return "TRACK_GEOSTREAM";
  case EMemTrack::TRACK_CINEMATICS:
    return "TRACK_CINEMATICS";
  case EMemTrack::TRACK_LUA:
    return "TRACK_LUA";
  case EMemTrack::TRACK_UI:
    return "TRACK_UI";
  case EMemTrack::TRACK_FLAME:
    return "TRACK_FLAME";
  case EMemTrack::TRACK_FIREMANAGER:
    return "TRACK_FIREMANAGER";
  case EMemTrack::TRACK_CONSOLE_IMAGES:
    return "TRACK_CONSOLE_IMAGES";
  case EMemTrack::TRACK_CGMEDIA_TABLE:
    return "TRACK_CGMEDIA_TABLE";
  case EMemTrack::TRACK_IMPACTS_FX_TABLE:
    return "TRACK_IMPACTS_FX_TABLE";
  case EMemTrack::TRACK_DELIMITER4:
    return "TRACK_DELIMITER4";
  case EMemTrack::TRACK_SOUND:
    return "TRACK_SOUND";
  case EMemTrack::TRACK_SOUND_GLOBALS:
    return "TRACK_SOUND_GLOBALS";
  case EMemTrack::TRACK_MINSPEC_SOUND_GLOBALS:
    return "TRACK_MINSPEC_SOUND_GLOBALS";
  case EMemTrack::TRACK_IMPACTS_SOUND_TABLE:
    return "TRACK_IMPACTS_SOUND_TABLE";
  case EMemTrack::TRACK_DELIMITER5:
    return "TRACK_DELIMITER5";
  case EMemTrack::TRACK_NONE:
    return "TRACK_NONE";
  default:
    return "TRACK_INVALID";
  }
}

enum class SwimStateType : int32_t {
  NONE = 0x0,
  SURFACE = 0x1,
  UNDERWATER = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(SwimStateType);

inline constexpr const char *serialize(const SwimStateType type) {
  switch (type) {
  case SwimStateType::NONE:
    return "SWIMSTATE_NONE";
  case SwimStateType::SURFACE:
    return "SWIMSTATE_SURFACE";
  case SwimStateType::UNDERWATER:
    return "SWIMSTATE_UNDERWATER";
  default:
    return "SWIMSTATE_INVALID";
  }
}

enum class KillCamEntityRestState : int32_t {
  MOVING = 0x0,
  AT_REST = 0x1,
  STUCK_GROUND = 0x2,
  STUCK_WALL = 0x3,
};
IMPL_ENUM_OPERATORS(KillCamEntityRestState);

inline constexpr const char *serialize(const KillCamEntityRestState state) {
  switch (state) {
  case KillCamEntityRestState::MOVING:
    return "KC_REST_MOVING";
  case KillCamEntityRestState::AT_REST:
    return "KC_REST_AT_REST";
  case KillCamEntityRestState::STUCK_GROUND:
    return "KC_REST_STUCK_GROUND";
  case KillCamEntityRestState::STUCK_WALL:
    return "KC_REST_STUCK_WALL";
  default:
    return "KC_REST_INVALID";
  }
}

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

inline constexpr const char *serialize(const KillCamEntityType type) {
  switch (type) {
  case KillCamEntityType::NO_ENTITY:
    return "KCENTITY_NO_ENTITY";
  case KillCamEntityType::HELICOPTER:
    return "KCENTITY_HELICOPTER";
  case KillCamEntityType::DESTRUCTIBLE:
    return "KCENTITY_DESTRUCTIBLE";
  case KillCamEntityType::SCRIPTED:
    return "KCENTITY_SCRIPTED";
  case KillCamEntityType::EXPLOSIVE:
    return "KCENTITY_EXPLOSIVE";
  case KillCamEntityType::FAST_EXPLOSIVE:
    return "KCENTITY_FAST_EXPLOSIVE";
  case KillCamEntityType::ROCKET:
    return "KCENTITY_ROCKET";
  case KillCamEntityType::DRONE:
    return "KCENTITY_DRONE";
  case KillCamEntityType::DAG:
    return "KCENTITY_DAG";
  case KillCamEntityType::COMBAT_ROBOT:
    return "KCENTITY_COMBAT_ROBOT";
  case KillCamEntityType::ARTILLERY:
    return "KCENTITY_ARTILLERY";
  case KillCamEntityType::VEHICLE:
    return "KCENTITY_VEHICLE";
  default:
    return "KCENTITY_INVALID";
  }
}

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

inline constexpr const char *serialize(const InvalidCmdHintType type) {
  switch (type) {
  case InvalidCmdHintType::NONE:
    return "HINT_NONE";
  case InvalidCmdHintType::NO_AMMO_BULLETS:
    return "HINT_NO_AMMO_BULLETS";
  case InvalidCmdHintType::NO_AMMO_FRAG_GRENADE:
    return "HINT_NO_AMMO_FRAG_GRENADE";
  case InvalidCmdHintType::NO_AMMO_SPECIAL_GRENADE:
    return "HINT_NO_AMMO_SPECIAL_GRENADE";
  case InvalidCmdHintType::NO_AMMO_FLASH_GRENADE:
    return "HINT_NO_AMMO_FLASH_GRENADE";
  case InvalidCmdHintType::NO_AMMO_EQUIPMENT:
    return "HINT_NO_AMMO_EQUIPMENT";
  case InvalidCmdHintType::STAND_BLOCKED:
    return "HINT_STAND_BLOCKED";
  case InvalidCmdHintType::CROUCH_BLOCKED:
    return "HINT_CROUCH_BLOCKED";
  case InvalidCmdHintType::TARGET_TOO_CLOSE:
    return "HINT_TARGET_TOO_CLOSE";
  case InvalidCmdHintType::LOCKON_REQUIRED:
    return "HINT_LOCKON_REQUIRED";
  case InvalidCmdHintType::NOT_ENOUGH_CLEARANCE:
    return "HINT_NOT_ENOUGH_CLEARANCE";
  case InvalidCmdHintType::CANT_PLACE_TURRET:
    return "HINT_CANT_PLACE_TURRET";
  case InvalidCmdHintType::CANT_EQUIP_WHILE_PRONE:
    return "HINT_CANT_EQUIP_WHILE_PRONE";
  case InvalidCmdHintType::CANT_PLANT_EQUIPMENT:
    return "HINT_CANT_PLANT_EQUIPMENT";
  case InvalidCmdHintType::ROUND_START_DELAY:
    return "HINT_ROUND_START_DELAY";
  case InvalidCmdHintType::GRENADES_NOT_ALLOWED:
    return "HINT_GRENADES_NOT_ALLOWED";
  case InvalidCmdHintType::GADGET_DENIED_ALREADY_ACTIVE:
    return "HINT_GADGET_DENIED_ALREADY_ACTIVE";
  case InvalidCmdHintType::GADGET_DENIED_CANNOT_ACTIVATE:
    return "HINT_GADGET_DENIED_CANNOT_ACTIVATE";
  case InvalidCmdHintType::TARGET_OUT_OF_RANGE:
    return "HINT_TARGET_OUT_OF_RANGE";
  case InvalidCmdHintType::TARGET_BEING_USED:
    return "HINT_TARGET_BEING_USED";
  case InvalidCmdHintType::TARGET_INVALID_TYPE:
    return "HINT_TARGET_INVALID_TYPE";
  case InvalidCmdHintType::NO_TARGET_IN_RANGE:
    return "HINT_NO_TARGET_IN_RANGE";
  case InvalidCmdHintType::TARGET_DISABLED:
    return "HINT_TARGET_DISABLED";
  case InvalidCmdHintType::TARGETING_ABORTED:
    return "HINT_TARGETING_ABORTED";
  case InvalidCmdHintType::TARGET_ALREADY_TARGETED:
    return "HINT_TARGET_ALREADY_TARGETED";
  case InvalidCmdHintType::SYSTEM_DISABLED:
    return "HINT_SYSTEM_DISABLED";
  default:
    return "HINT_INVALID";
  }
}

enum class SettingTeamIndicator : int32_t {
  FULL = 0x0,
  ABBREVIATED = 0x1,
  ICON = 0x2,
  COUNT = 0x3,
};
IMPL_ENUM_OPERATORS(SettingTeamIndicator);

inline constexpr const char *serialize(const SettingTeamIndicator indicator) {
  switch (indicator) {
  case SettingTeamIndicator::FULL:
    return "TEAMINDICATOR_FULL";
  case SettingTeamIndicator::ABBREVIATED:
    return "TEAMINDICATOR_ABBREVIATED";
  case SettingTeamIndicator::ICON:
    return "TEAMINDICATOR_ICON";
  default:
    return "TEAMINDICATOR_INVALID";
  }
}

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

inline constexpr const char *serialize(const CubemapShot shot) {
  switch (shot) {
  case CubemapShot::NONE:
    return "CUBEMAP_NONE";
  case CubemapShot::RIGHT:
    return "CUBEMAP_RIGHT";
  case CubemapShot::LEFT:
    return "CUBEMAP_LEFT";
  case CubemapShot::BACK:
    return "CUBEMAP_BACK";
  case CubemapShot::FRONT:
    return "CUBEMAP_FRONT";
  case CubemapShot::UP:
    return "CUBEMAP_UP";
  case CubemapShot::DOWN:
    return "CUBEMAP_DOWN";
  default:
    return "CUBEMAP_INVALID";
  }
}

enum class DemoType : int32_t {
  NONE = 0x0,
  CLIENT = 0x1,
  SERVER = 0x2,
  SERVER_SNAPSHOT = 0x3,
};
IMPL_ENUM_OPERATORS(DemoType);

inline constexpr const char *serialize(const DemoType dtype) {
  switch (dtype) {
  case DemoType::NONE:
    return "DEMOTYPE_NONE";
  case DemoType::CLIENT:
    return "DEMOTYPE_CLIENT";
  case DemoType::SERVER:
    return "DEMOTYPE_SERVER";
  case DemoType::SERVER_SNAPSHOT:
    return "DEMOTYPE_SERVER_SNAPSHOT";
  default:
    return "DEMOTYPE_INVALID";
  }
}

enum class TraceBrushType : uint32_t {
  NONE = 0x0,
  BRUSH = 0x1,
};
IMPL_ENUM_OPERATORS(TraceBrushType);

inline constexpr const char *serialize(const TraceBrushType btype) {
  switch (btype) {
  case TraceBrushType::NONE:
    return "TRACEBRUSH_NONE";
  case TraceBrushType::BRUSH:
    return "TRACEBRUSH_BRUSH";
  default:
    return "TRACEBRUSH_INVALID";
  }
}

enum class TraceHitType : uint32_t {
  NONE = 0x0,
  ENTITY = 0x1,
  DYNENT_MODEL = 0x2,
  DYNENT_BRUSH = 0x3,
  GLASS = 0x4,
};
IMPL_ENUM_OPERATORS(TraceHitType);

inline constexpr const char *serialize(const TraceHitType htype) {
  switch (htype) {
  case TraceHitType::NONE:
    return "TRACEHIT_NONE";
  case TraceHitType::ENTITY:
    return "TRACEHIT_ENTITY";
  case TraceHitType::DYNENT_MODEL:
    return "TRACEHIT_DYNENT_MODEL";
  case TraceHitType::DYNENT_BRUSH:
    return "TRACEHIT_DYNENT_BRUSH";
  case TraceHitType::GLASS:
    return "TRACEHIT_GLASS";
  default:
    return "TRACEHIT_INVALID";
  }
}

enum class MissileFlightMode : int32_t {
  TOP = 0x0,
  DIRECT = 0x1,
};
IMPL_ENUM_OPERATORS(MissileFlightMode);

inline constexpr const char *serialize(const MissileFlightMode mode) {
  switch (mode) {
  case MissileFlightMode::TOP:
    return "MISSILEFLIGHT_TOP";
  case MissileFlightMode::DIRECT:
    return "MISSILEFLIGHT_DIRECT";
  default:
    return "MISSILEFLIGHT_INVALID";
  }
}

enum class MissileStage : int32_t {
  SOFTLAUNCH = 0x0,
  ASCENT = 0x1,
  DESCENT = 0x2,
};
IMPL_ENUM_OPERATORS(MissileStage);

inline constexpr const char *serialize(const MissileStage stage) {
  switch (stage) {
  case MissileStage::SOFTLAUNCH:
    return "MISSILESTAGE_SOFTLAUNCH";
  case MissileStage::ASCENT:
    return "MISSILESTAGE_ASCENT";
  case MissileStage::DESCENT:
    return "MISSILESTAGE_DESCENT";
  default:
    return "MISSILESTAGE_INVALID";
  }
}

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

inline constexpr const char *serialize(const CameraMode mode) {
  switch (mode) {
  case CameraMode::NORMAL:
    return "CAMMODE_NORMAL";
  case CameraMode::LINKED:
    return "CAMMODE_LINKED";
  case CameraMode::VEHICLE:
    return "CAMMODE_VEHICLE";
  case CameraMode::VEHICLE_THIRDPERSON:
    return "CAMMODE_VEHICLE_THIRDPERSON";
  case CameraMode::VEHICLE_GUNNER:
    return "CAMMODE_VEHICLE_GUNNER";
  case CameraMode::TURRET:
    return "CAMMODE_TURRET";
  case CameraMode::MISSILE:
    return "CAMMODE_MISSILE";
  case CameraMode::SPIKECAMERA:
    return "CAMMODE_SPIKECAMERA";
  case CameraMode::SCRIPTED_EXTRACAM:
    return "CAMMODE_SCRIPTED_EXTRACAM";
  case CameraMode::SCOPE:
    return "CAMMODE_SCOPE";
  case CameraMode::PLAYER_BODY:
    return "CAMMODE_PLAYER_BODY";
  case CameraMode::XCAM:
    return "CAMMODE_XCAM";
  case CameraMode::SCRIPTED:
    return "CAMMODE_SCRIPTED";
  case CameraMode::RADIANT:
    return "CAMMODE_RADIANT";
  default:
    return "CAMMODE_INVALID";
  }
}

struct orientation_t {
  vec3_t origin;
  vec3<vec3_t> axis;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"origin\": {}, \"axis\": {} }}", origin.serialize(),
                       axis.serialize());
  }
};

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"nesting\": {}, \"localClientNum\": {}, \"controllerIndex\": {}, "
        "\"argshift\": {}, \"argc\": {}, \"argv\": {:p}, \"textPool\": \"{}\", "
        "\"argvPool\": {:p}, \"usedTextPool\": {}, \"totalUsedArgvPool\": {}, "
        "\"totalUsedTextPool\": {} }}",
        nesting, ::serialize(localClientNum), ::serialize(controllerIndex),
        ::serialize(argshift), ::serialize(argc),
        reinterpret_cast<const void *>(argv), textPool,
        reinterpret_cast<const void *>(argvPool), ::serialize(usedTextPool),
        totalUsedArgvPool, totalUsedTextPool);
  }
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

  inline constexpr auto chunks() const noexcept { return std::size(data); }

  inline std::string serialize() const noexcept { return ::serialize(data); }
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

inline constexpr const char *serialize(const UIModelDataType type) {
  switch (type) {
  case UIModelDataType::INVALID:
    return "UIMODEL_INVALID";
  case UIModelDataType::BOOL:
    return "UIMODEL_BOOL";
  case UIModelDataType::UINT64:
    return "UIMODEL_UINT64";
  case UIModelDataType::INT:
    return "UIMODEL_INT";
  case UIModelDataType::REAL:
    return "UIMODEL_REAL";
  case UIModelDataType::STRING:
    return "UIMODEL_STRING";
  case UIModelDataType::FUNCTION:
    return "UIMODEL_FUNCTION";
  default:
    return "UIMODEL_UNKNOWN";
  }
}

enum class objectiveDrawState_t : int32_t {
  ONSCREEN = 0x0,
  OFFSCREEN = 0x1,
};
IMPL_ENUM_OPERATORS(objectiveDrawState_t);

inline constexpr const char *serialize(const objectiveDrawState_t state) {
  switch (state) {
  case objectiveDrawState_t::ONSCREEN:
    return "OBJDRAWSTATE_ONSCREEN";
  case objectiveDrawState_t::OFFSCREEN:
    return "OBJDRAWSTATE_OFFSCREEN";
  default:
    return "OBJDRAWSTATE_INVALID";
  }
}

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

inline constexpr const char *serialize(const objectiveState_t state) {
  switch (state) {
  case objectiveState_t::EMPTY:
    return "OBJSTATE_EMPTY";
  case objectiveState_t::ACTIVE:
    return "OBJSTATE_ACTIVE";
  case objectiveState_t::INVISIBLE:
    return "OBJSTATE_INVISIBLE";
  case objectiveState_t::DONE:
    return "OBJSTATE_DONE";
  case objectiveState_t::CURRENT:
    return "OBJSTATE_CURRENT";
  case objectiveState_t::FAILED:
    return "OBJSTATE_FAILED";
  default:
    return "OBJSTATE_INVALID";
  }
}

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

  inline std::string serialize() const noexcept {
    std::string valueStr;
    switch (dataType) {
    case UIModelDataType::BOOL:
      valueStr = std::format("{}", ::serialize(boolean));
      break;
    case UIModelDataType::UINT64:
      valueStr = std::format("{}", uint64);
      break;
    case UIModelDataType::INT:
      valueStr = std::format("{}", integer);
      break;
    case UIModelDataType::REAL:
      valueStr = std::format("{}", real);
      break;
    case UIModelDataType::STRING:
      valueStr = std::format("{}", ::serialize(string));
      break;
    case UIModelDataType::FUNCTION:
      valueStr = std::format("{}", functionRef);
      break;
    default:
      valueStr = "null";
      break;
    }
    return std::format("{{ \"dataType\": {}, \"value\": {} }}",
                       game::serialize(dataType), valueStr);
  }
};
ASSERT_SIZE(UIModelData, 0x10);

struct objectiveUIModel_t {
  UIModelData modelData;
  int32_t modelName;
  uint8_t _padding14[12];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"modelData\": {}, \"modelName\": {} }}",
                       modelData.serialize(), modelName);
  }
};

PACKED(struct objective_t {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"objState\": {}, \"origin\": {}, \"entNum\": {}, \"color\": "
        "{:08X}, "
        "\"colorSetColor\": {}, \"objIconSize\": {}, \"objOwnerNum\": {}, "
        "\"name\": {}, "
        "\"teamMask\": {}, \"progress\": {}, \"clientUseMask\": {}, "
        "\"gamemodeFlags\": 0x{:04X}, "
        "\"objTeamNum\": {}, \"syncFlag\": {}, \"uiModels\": {} }}",
        game::serialize(objState), origin.serialize(), entNum, color,
        colorSetColor, objIconSize.serialize(), objOwnerNum, name, teamMask,
        progress, ::serialize(clientUseMask), gamemodeFlags, objTeamNum,
        ::serialize(syncFlag), ::serialize(uiModels));
  }
});
ASSERT_SIZE(objective_t, 0xC0);

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

  // TODO: how do we determine which union member to serialize?
  inline std::string serialize() const noexcept { return "TODO"; }
};
ASSERT_SIZE(renderOptions_t, 0x10);

typedef uint16_t modelNameIndex_t;

class EntHandle {
public:
  uint16_t number;
  uint16_t infoIndex;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"number\": {}, \"infoIndex\": {} }}", number,
                       infoIndex);
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"index\": 0x{:04X}, \"toggle\": {} }}", index,
                       ::serialize(static_cast<bool>(toggle)));
  }
};

struct outPacket_t {
  int32_t p_cmdNumber;
  int32_t p_serverTime;
  int32_t p_realtime;
};

// Verified
enum class consoleChannel_e : uint32_t {
  CHANNEL_DONT_FILTER = 0x0,
  CHANNEL_GAMENOTIFY = 0x1,
  CHANNEL_BOLDGAME = 0x2,
  CHANNEL_OBJNOTIFY = 0x3,
  CHANNEL_SUBTITLE = 0x4,
  CHANNEL_OBITUARY = 0x5,
  CHANNEL_COOPINFO = 0x6,
  CHANNEL_WARNING = 0x7,
  // Note: this may be missing in dedicated server.
  CHANNEL_ERROR = 0x8,
  CHANNEL_INFO = 0x9,
  BUILTIN_CHANNEL_COUNT = 0xA,
  FIRST_DEBUG_CHANNEL = 0x9,
};
IMPL_ENUM_OPERATORS(consoleChannel_e);

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

enum class BuildIntField : int32_t {
  BUILD_NUMBER = 0x0,
  CHANGELIST = 0x1,
  INFO_VERSION = 0x2,
};
IMPL_ENUM_OPERATORS(BuildIntField);

enum class BuildStringField : int32_t {
  MAJOR_VERSION = 0x0,
  MINOR_VERSION = 0x1,
  BUILD_ID = 0x2,
  BUILD_VERSION = 0x3,
  BUILD_MACHINE = 0x4,
  BUILD_TYPE = 0x5,
  BUILD_TIME = 0x6,
  BUILD_DISPLAY_NAME = 0x7,
  BUILD_NAME = 0x8,
  BUILD_BASE_NAME = 0x9,
  BUILD_CONFIG = 0xA,
  BUILD_MODE = 0xB,
  BUILD_PROJECT_NAME = 0xC,
  BUILD_LOCAL_CHANGES = 0xD,
  BUILD_INFO_NAME = 0xE,
  BUILD_INFO_SOURCE = 0xF,
  BUILD_INFO_STRING = 0x10,
};
IMPL_ENUM_OPERATORS(BuildStringField);
} // namespace game
