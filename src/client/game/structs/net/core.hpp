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

using XNKID = bdSecurityID;

struct bdSecurityKey {
  uint8_t ab[16];
};

using XNKEY = bdSecurityKey;

struct SerializedAdr {
  uint8_t valid;
  uint8_t addrBuff[37];
};

struct XSESSION_INFO {
  XNKID sessionID;
  XNADDR hostAddress;
  XNKEY keyExchangeKey;
};

enum PacketModeList {
  PACKETDATA_FIRST = 0x0,
  PACKETDATA_UNDEFINED = 0x0,
  PACKETDATA_HEADER = 0x1,
  PACKETDATA_OVERHEAD = 0x2,
  PACKETDATA_DATA = 0x3,
  PACKETDATA_RELIABLEDATA = 0x4,
  PACKETDATA_ZEROFLOAT = 0x5,
  PACKETDATA_SMALLFLOAT = 0x6,
  PACKETDATA_LARGEFLOAT = 0x7,
  PACKETDATA_ZEROINT = 0x8,
  PACKETDATA_SMALLANGLE = 0x9,
  PACKETDATA_ZEROANGLE = 0xA,
  PACKETDATA_TIMEDELTA = 0xB,
  PACKETDATA_TIME = 0xC,
  PACKETDATA_24BITFLAGINDEX = 0xD,
  PACKETDATA_GROUNDENTITY = 0xE,
  PACKETDATA_ENTITYNUM = 0xF,
  PACKETDATA_LASTFIELDCHANGED = 0x10,
  PACKETDATA_NOTNETWORKDATA = 0x11,
  PACKETDATA_ORIGINDELTA = 0x12,
  PACKETDATA_ORIGIN = 0x13,
  NUM_PACKETDATA_MODES = 0x14,
};

struct PacketMode {
  uint32_t start;
  PacketModeList mode;
};

using bdCommonAddrRef = void *;

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

enum {
  CS_FREE = 0x0,
  CS_ZOMBIE = 0x1,
  CS_RECONNECTING = 0x2,
  CS_CONNECTED = 0x3,
  CS_CLIENTLOADING = 0x4,
  CS_ACTIVE = 0x5,
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
static_assert(sizeof(netProfileStream_t) == 0x2f0,
              "netProfileStream_t size must be 0x2f0 bytes");

struct netProfileInfo_t {
  netProfileStream_t send;
  netProfileStream_t recieve;
};
static_assert(sizeof(netProfileInfo_t) == 0x5e0,
              "netProfileInfo_t size must be 0x5e0 bytes");

typedef UNKNOWN_PTR_TYPE PacketQueuePtr;

// Verified
struct netchanOOBMessage_t {
  netadr_t from;
  volatile int32_t state;
  uint8_t data[1256];
  int32_t dataLen;
};
static_assert(sizeof(netchanOOBMessage_t) == 0x500,
              "netchanOOBMessage_t must be 0x500 bytes");

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
static_assert(sizeof(netchan_t) == 0x618, "sizeof(netchan_t) != 0x618");
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
  char hostName[32];
  char mapName[32];
  char game[24];
  char gameType[16];
  uint8_t isInGame;
  uint8_t _padding0E1[7];
};
static_assert(sizeof(serverInfo_t) == 0xE8, "sizeof(serverInfo_t) != 0xE8");
#pragma pack(pop)

} // namespace net
} // namespace game