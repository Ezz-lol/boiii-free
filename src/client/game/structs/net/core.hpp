#pragma once

#include <game/structs/core.hpp>
#include <game/structs/quake/core.hpp>
#include <structs/array.hpp>

#include <macros/macros.hpp>

#include <charconv>

namespace game {
namespace net {

enum class connectionType_e : uint32_t {
  UNKNOWN = 0x0,
  WIRED = 0x1,
  WIRELESS = 0x2,
};

enum class NetChanMsgType_e : int32_t {
  INVALID_CHANNEL = -1,
  SNAPSHOT = 0x0,
  CLIENTMSG = 0x1,
  VOICE = 0x2,
  LOBBY_VOICE = 0x3,
  LOBBYPRIVATE_STATE = 0x4,
  LOBBYPRIVATE_HEARTBEAT = 0x5,
  LOBBYPRIVATE_RELIABLE = 0x6,
  LOBBYPRIVATE_UNRELIABLE = 0x7,
  LOBBYPRIVATE_MIGRATE = 0x8,
  LOBBYGAME_STATE = 0x9,
  LOBBYGAME_HEARTBEAT = 0xA,
  LOBBYGAME_RELIABLE = 0xB,
  LOBBYGAME_UNRELIABLE = 0xC,
  LOBBYGAME_MIGRATE = 0xD,
  LOBBY_JOIN = 0xE,
  PTP = 0xF,
  CLIENT_CONTENT = 0x10,
  TEST = 0x11,
  MAX_CHANNELS = 0x12,
};
IMPL_ENUM_OPERATORS(NetChanMsgType_e);

constexpr size_t UINT8_STR_BUF_LEN = 3; // Decimal; 0 - 255
constexpr size_t UINT16_STR_BUF_LEN = sizeof(uint16_t) * UINT8_STR_BUF_LEN;
constexpr size_t NET_IPV4_STR_BUF_LEN =
    sizeof(uint8_t) * 4 /* a, b, c, d */ * UINT8_STR_BUF_LEN +
    3 * sizeof(char) /* periods */ + 1 /* NULL */;
typedef str<NET_IPV4_STR_BUF_LEN> netipv4_str_t;

union netipv4_t {
  struct {
    uint8_t a = 0;
    uint8_t b = 0;
    uint8_t c = 0;
    uint8_t d = 0;
  };
  uint8_t parts[4];

  [[nodiscard]] inline std::string toString() const noexcept {
    return std::format("{}.{}.{}.{}", a, b, c, d);
  }

  inline constexpr ToStringResult
  toString(netipv4_str_t &buf, bool terminate = true) const noexcept {
    // a
    char *ptr = std::to_chars(buf, buf + UINT8_STR_BUF_LEN, a).ptr;

    // b
    APPENDCHAR(ptr, '.');
    ptr = std::to_chars(ptr, ptr + UINT8_STR_BUF_LEN, b).ptr;

    // c
    APPENDCHAR(ptr, '.');
    ptr = std::to_chars(ptr, ptr + UINT8_STR_BUF_LEN, c).ptr;

    // d
    APPENDCHAR(ptr, '.');
    ptr = std::to_chars(ptr, ptr + UINT8_STR_BUF_LEN, d).ptr;

    if (terminate) {
      APPENDCHAR(ptr, '\0');
    }
    return ToStringResult{.buf = buf, .ptr = ptr};
  }

  [[nodiscard]] constexpr bool is_zero() const noexcept {
    return a == 0 && b == 0 && c == 0 && d == 0;
  }
};

enum class netadrtype_t : int32_t {
  BOT = 0x0,
  BAD = 0x1,
  LOOPBACK = 0x2,
  RAWIP = 0x3,
  IP = 0x4,
  // Does not exist in engine
  INVALID = -1
};
IMPL_ENUM_OPERATORS(netadrtype_t);

inline constexpr const char *serialize(netadrtype_t type) {
  switch (type) {
  case netadrtype_t::BOT:
    return "netadrtype_t::BOT";
  case netadrtype_t::BAD:
    return "netadrtype_t::BAD";
  case netadrtype_t::LOOPBACK:
    return "netadrtype_t::LOOPBACK";
  case netadrtype_t::RAWIP:
    return "netadrtype_t::RAWIP";
  case netadrtype_t::IP:
    return "netadrtype_t::IP";
  default:
    return "netadrtype_t::INVALID";
  }
}

enum class netsrc_t : int32_t {
  NS_NULL = -1,
  CLIENT1 = 0x0,
  CLIENT2 = 0x1,
  CLIENT3 = 0x2,
  CLIENT4 = 0x3,
  SERVER = 0x4,
  MAXCLIENTS = 0x4,
  PACKET = 0x5,
};
IMPL_ENUM_OPERATORS(netsrc_t);

inline constexpr const char *serialize(netsrc_t src) {
  switch (src) {
  case netsrc_t::CLIENT1:
    return "netsrc_t::CLIENT1";
  case netsrc_t::CLIENT2:
    return "netsrc_t::CLIENT2";
  case netsrc_t::CLIENT3:
    return "netsrc_t::CLIENT3";
  case netsrc_t::CLIENT4:
    return "netsrc_t::CLIENT4";
  case netsrc_t::SERVER:
    return "netsrc_t::SERVER";
  case netsrc_t::PACKET:
    return "netsrc_t::PACKET";
  default:
    return "NS_NULL";
  }
}

constexpr size_t NETADR_STR_BUF_LEN =
    NET_IPV4_STR_BUF_LEN + sizeof(char) /*colon*/ +
    UINT16_STR_BUF_LEN /*port*/; /*  +1 for NULL is included in
                                     NET_IPV4_STR_BUF_LEN - only one can be
                                     terminated */
typedef str<NETADR_STR_BUF_LEN> netadr_str_t;

struct netadr_t {
  union {
    netipv4_t ipv4 = {};
    uint32_t addr;
  };

  uint16_t port = 0;
  netadrtype_t type = netadrtype_t::INVALID;
  netsrc_t localNetID = netsrc_t::NS_NULL;

  [[nodiscard]] inline std::string toString() const noexcept {
    return std::format("{}:{}", ipv4.toString(), port);
  }

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{}:{}", ipv4.toString(), port);
  }

  inline constexpr ToStringResult
  toString(netadr_str_t &buf, bool terminate = true) const noexcept {
    char *ptr = ipv4.toString(slice<NET_IPV4_STR_BUF_LEN>(buf), false).ptr;

    APPENDCHAR(ptr, ':');
    ptr = std::to_chars(ptr, ptr + UINT16_STR_BUF_LEN, port).ptr;

    if (terminate) {
      APPENDCHAR(ptr, '\0');
    }

    return {.buf = buf, .ptr = ptr};
  }

  [[nodiscard]] constexpr bool is_zero_ip() const noexcept {
    return ipv4.is_zero();
  }
};
ASSERT_SIZE(netadr_t, 0x10);

// `bdCommonAddr`, serialized using the `bdBytePacker`, which essentially just
// appends each field's bytes, sequentially, in network (big endian) byte order
struct XNADDR {
  static constexpr size_t SIZE = 37;

  uint8_t addrBuff[SIZE] = {0};

  constexpr operator const decltype(addrBuff) & () const noexcept {
    return addrBuff;
  }

  constexpr operator decltype(addrBuff) & () noexcept { return addrBuff; }

  constexpr operator std::span<const uint8_t, SIZE>() const noexcept {
    return std::span<const uint8_t, SIZE>(addrBuff, SIZE);
  }

  constexpr operator std::span<uint8_t, SIZE>() noexcept {
    return std::span<uint8_t, SIZE>(addrBuff, SIZE);
  }

  constexpr operator const uint8_t *() const noexcept { return addrBuff; }

  constexpr operator uint8_t *() noexcept { return addrBuff; }

  constexpr operator const void *() const noexcept { return addrBuff; }

  constexpr operator void *() noexcept { return addrBuff; }

  static constexpr auto size() noexcept { return SIZE; }

  typedef uint8_t index_t;
  static constexpr void assert_range([[maybe_unused]] index_t index) {
    assert(index < static_cast<index_t>(SIZE) &&
           "index to XNADDR must be < addrBuff length");
  }
  template <IntegralLike<index_t> Index>
  constexpr const uint8_t &get(Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return addrBuff[index];
  }
  template <IntegralLike<index_t> Index>
  constexpr uint8_t &get(Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return addrBuff[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr const uint8_t &operator[](Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  constexpr uint8_t &operator[](Index index) noexcept {
    return get(index);
  }
};

// ID constant always used by engine during construction.
constexpr size_t BD_SECURITY_ID_KEY = 0x101010101010101ULL;
struct bdSecurityID {
  size_t ab = BD_SECURITY_ID_KEY;

  constexpr operator const size_t &() const noexcept { return ab; }
  constexpr operator const volatile size_t &() const volatile noexcept {
    return ab;
  }
  constexpr operator volatile size_t &() volatile noexcept { return ab; }
  constexpr operator size_t &() noexcept { return ab; }
};

typedef bdSecurityID XNKID;

union bdSecurityKey {
  static constexpr auto SIZE = 2;
  bdSecurityID ab[SIZE];
  struct {
    bdSecurityID a = {};
    bdSecurityID b = {};
  };

  typedef uint8_t index_t;

  static constexpr void assert_range([[maybe_unused]] const index_t index) {
    assert(index < SIZE &&
           "index to bdSecurityKey must be within range 0 <= index < 2");
  }
  template <IntegralLike<index_t> Index>
  constexpr const bdSecurityID &get(const Index index_arg) const noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return ab[index];
  }
  template <IntegralLike<index_t> Index>
  constexpr bdSecurityID &get(const Index index_arg) noexcept {
    const index_t index = static_cast<index_t>(index_arg);
    assert_range(index);
    return ab[index];
  }

  template <IntegralLike<index_t> Index>
  constexpr const bdSecurityID &operator[](const Index index) const noexcept {
    return get(index);
  }
  template <IntegralLike<index_t> Index>
  constexpr bdSecurityID &operator[](const Index index) noexcept {
    return get(index);
  }

  static constexpr auto size() noexcept { return SIZE; }
};

typedef bdSecurityKey XNKEY;

// Verified
struct SerializedAdr {
  bool valid = false;
  XNADDR addrBuff = {};
};
ASSERT_SIZE(SerializedAdr, 0x26);

struct XSESSION_INFO {
  XNKID sessionID;
  XNADDR hostAddress;
  XNKEY keyExchangeKey;
};

enum class PacketModeList {
  FIRST = 0x0,
  UNDEFINED = 0x0,
  HEADER = 0x1,
  OVERHEAD = 0x2,
  DATA = 0x3,
  RELIABLEDATA = 0x4,
  ZEROFLOAT = 0x5,
  SMALLFLOAT = 0x6,
  LARGEFLOAT = 0x7,
  ZEROINT = 0x8,
  SMALLANGLE = 0x9,
  ZEROANGLE = 0xA,
  TIMEDELTA = 0xB,
  TIME = 0xC,
  _24BITFLAGINDEX = 0xD,
  GROUNDENTITY = 0xE,
  ENTITYNUM = 0xF,
  LASTFIELDCHANGED = 0x10,
  NOTNETWORKDATA = 0x11,
  ORIGINDELTA = 0x12,
  ORIGIN = 0x13,
  COUNT = 0x14,
};

struct PacketMode {
  uint32_t start;
  PacketModeList mode;
};

// Verified
PACKED(struct HostInfo {
  XUID xuid = 0;
  name_t name = {0};
  netadr_t netAdr = {};
  SerializedAdr serializedAdr = {};
  bdSecurityID secId = {};
  bdSecurityKey secKey = {};
  uint8_t _padding76[2];
  uint32_t serverLocation = 0;
  uint8_t _padding7C[4];
});
ASSERT_SIZE(HostInfo, 0x80);

enum class svscmd_type {
  CAN_IGNORE = 0x0,
  RELIABLE = 0x1,
};

enum class clientState_t : int32_t {
  FREE = 0x0, // can be used for a new connection
  /*
    Client has been disconnected, but don't use connection
    for a new client for a couple seconds
    (`sv_zombietime` dvar value) in case of reconnect
  */
  ZOMBIE = 0x1,
  RECONNECTING = 0x2,
  CONNECTED = 0x3, // has been assigned to a client_t, but no gamestate yet
  PRIMED = 0x4,    // gamestate has been sent, but client hasn't sent a usercmd
  ACTIVE = 0x5,    // client is fully in game
};
constexpr const char *serialize(const clientState_t state) {
  switch (state) {
  case clientState_t::FREE:
    return "CLIENT_STATE_FREE";
  case clientState_t::ZOMBIE:
    return "CLIENT_STATE_ZOMBIE";
  case clientState_t::RECONNECTING:
    return "CLIENT_STATE_RECONNECTING";
  case clientState_t::CONNECTED:
    return "CLIENT_STATE_CONNECTED";
  case clientState_t::PRIMED:
    return "CLIENT_STATE_PRIMED";
  case clientState_t::ACTIVE:
    return "CLIENT_STATE_ACTIVE";
  default:
    return "CLIENT_STATE_INVALID";
  }
}
IMPL_ENUM_OPERATORS(clientState_t);

struct netProfilePacket_t {
  int32_t iTime;
  int32_t iSize;
  int32_t bFragment;
};

struct netProfileStream_t {
  netProfilePacket_t packets[60];
  int32_t iCurrPacket;
  int32_t iBytesPerSecond;
  int32_t iLastBPSCalcTime;
  int32_t iCountedPackets;
  int32_t iCountedFragments;
  int32_t iFragmentPercentage;
  int32_t iLargestPacket;
  int32_t iSmallestPacket;
};
ASSERT_SIZE(netProfileStream_t, 0x2F0);

struct netProfileInfo_t {
  netProfileStream_t send;
  netProfileStream_t recieve;
};
ASSERT_SIZE(netProfileInfo_t, 0x5e0);

typedef UnknownPtr PacketQueuePtr;

// Verified
struct netchanOOBMessage_t {
  netadr_t from;
  volatile int32_t state;
  uint8_t data[1256];
  int32_t dataLen;
};
ASSERT_SIZE(netchanOOBMessage_t, 0x500);

struct EVENT_HANDLE {
  qboolean manualReset;
  uint8_t _padding[4];
};
enum class netchanMessageState_e : uint32_t {
  COMPLETE = 0x0,
  FAILED = 0x1,
  TIMEOUT = 0x2,
};

struct netchanMsgConfig_s {
  EVENT_HANDLE completedEvent;
  netchanMessageState_e status;
  int32_t timeoutMS;
};

typedef netchanMsgConfig_s netchanMsgConfig_t;

PACKED(struct netchan_t {
  int32_t outgoingSequence;
  netsrc_t sock;
  int32_t incomingSequence;
  netadr_t remoteAddress;
  int32_t qport;
  netchanMsgConfig_t messageConfig;
  qboolean waitingForAcks;
  netProfileInfo_t prof;
  uint8_t _padding[4];
});
ASSERT_SIZE(netchan_t, 0x618);

// Unverified
PACKED(struct serverInfo_t {
  XNADDR xnaddr;
  XNKEY xnkey;
  XNKID xnkid;
  uint8_t _padding03D[3];
  int32_t publicSlots;
  int32_t publicSlotsUsed;
  int32_t privateSlots;
  int32_t privateSlotsUsed;
  uint64_t nonce;
  netadr_t adr;
  uint8_t netType;
  uint8_t clients;
  uint8_t maxClients;
  uint8_t dirty;
  bool friendlyfire;
  bool killcam;
  uint8_t hardware;
  uint8_t mod;
  uint8_t requestCount;
  uint8_t _padding071;
  int16_t minPing;
  int16_t maxPing;
  int16_t ping;
  str32_t hostName;
  str32_t mapName;
  str24_t game;
  str16_t gameType;
  bool isInGame;
  uint8_t _padding0E1[7];
});
ASSERT_SIZE(serverInfo_t, 0xE8);

} // namespace net
} // namespace game
