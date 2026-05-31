#pragma once

#include "../core.hpp"
#include "../quake.hpp"
namespace game {
namespace net {
struct netipv4_t {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
};

enum netadrtype_t : int32_t {
  NA_BOT = 0x0,
  NA_BAD = 0x1,
  NA_LOOPBACK = 0x2,
  NA_RAWIP = 0x3,
  NA_IP = 0x4,
};

enum netsrc_t : int32_t {
  NS_NULL = -1,
  NS_CLIENT1 = 0x0,
  NS_CLIENT2 = 0x1,
  NS_CLIENT3 = 0x2,
  NS_CLIENT4 = 0x3,
  NS_SERVER = 0x4,
  NS_MAXCLIENTS = 0x4,
  NS_PACKET = 0x5,
};

struct netadr_t {
  union {
    netipv4_t ipv4;
    uint32_t addr;
  };

  uint16_t port;
  netadrtype_t type;
  netsrc_t localNetID;
};

struct XNADDR {
  uint8_t addrBuff[37];
};

struct bdSecurityID {
  uint8_t ab[8];
};

typedef bdSecurityID XNKID;

struct bdSecurityKey {
  uint8_t ab[16];
};

typedef bdSecurityKey XNKEY;

struct SerializedAdr {
  uint8_t valid;
  uint8_t addrBuff[37];
};

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

struct HostInfo {
  XUID xuid;
  char name[32];
  netadr_t netAdr;
  SerializedAdr serializedAdr;
  bdSecurityID secId;
  bdSecurityKey secKey;
  uint32_t serverLocation;
};
enum svscmd_type {
  SV_CMD_CAN_IGNORE_0 = 0x0,
  SV_CMD_RELIABLE_0 = 0x1,
};

enum clientState_t {
  CS_FREE = 0x0, // can be used for a new connection
  /*
    Client has been disconnected, but don't use connection
    for a new client for a couple seconds
    (`sv_zombietime` dvar value) in case of reconnect
  */
  CS_ZOMBIE = 0x1,
  CS_RECONNECTING = 0x2,
  CS_CONNECTED = 0x3, // has been assigned to a client_t, but no gamestate yet
  CS_PRIMED = 0x4, // gamestate has been sent, but client hasn't sent a usercmd
  CS_ACTIVE = 0x5, // client is fully in game
};

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
ASSERT_SIZE(netProfileStream_t, 0x2f0);

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
enum netchanMessageState_e : int32_t {
  NETCHAN_MESSAGE_COMPLETE = 0x0,
  NETCHAN_MESSAGE_FAILED = 0x1,
  NETCHAN_MESSAGE_TIMEOUT = 0x2,
};

struct netchanMsgConfig_s {
  EVENT_HANDLE completedEvent;
  netchanMessageState_e status;
  int32_t timeoutMS;
};

typedef netchanMsgConfig_s netchanMsgConfig_t;

#pragma pack(push, 1)
struct netchan_t {
  int32_t outgoingSequence;
  netsrc_t sock;
  int32_t incomingSequence;
  netadr_t remoteAddress;
  int32_t qport;
  netchanMsgConfig_t messageConfig;
  qboolean waitingForAcks;
  netProfileInfo_t prof;
  uint8_t _padding[4];
};
ASSERT_SIZE(netchan_t, 0x618);
#pragma pack(pop)

// Unverified
#pragma pack(push, 1)
struct serverInfo_t {
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
};
ASSERT_SIZE(serverInfo_t, 0xE8);
#pragma pack(pop)

} // namespace net
} // namespace game