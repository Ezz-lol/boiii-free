#pragma once

#include "core.hpp"
#include "quake.hpp"
namespace game {
namespace net {
struct netipv4_t {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
};

enum netadrtype_t {
  NA_BOT = 0x0,
  NA_BAD = 0x1,
  NA_LOOPBACK = 0x2,
  NA_RAWIP = 0x3,
  NA_IP = 0x4,
};

enum netsrc_t {
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

struct msg_t {
  qboolean overflowed;
  qboolean readOnly;
  uint8_t *data;
  uint8_t *splitData;
  int32_t maxsize;
  int32_t cursize;
  int32_t splitSize;
  int32_t readcount;
  int32_t bit;
  int32_t lastEntityRef;
  qboolean flush;
  netsrc_t targetLocalNetID;
  // PacketMode analysis;
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

struct client_s {
  int32_t state;
  char __pad0[0x28];
  netadr_t address;
  char __pad1[20468];
  int32_t reliableSequence;
  int32_t reliableAcknowledge;
  char __pad2[4];
  int32_t messageAcknowledge;
  char gap_5040[1416];
  XUID xuid;
  char __pad3[0xB5D84];
  int32_t guid;
  char __pad4[0x8];
  bool bIsTestClient;
  char __pad5[3];
  int32_t serverId;
  char __pad6[171432];
};

#ifdef __cplusplus
static_assert(sizeof(client_s) == 0xE5110);

static_assert(offsetof(client_s, address) == 0x2C);
static_assert(offsetof(client_s, xuid) == 0x55C8);
static_assert(offsetof(client_s, guid) == 0xBB354);
static_assert(offsetof(client_s, bIsTestClient) == 0xBB360);
#endif

struct client_s_cl : client_s {
  char __pad1_0[0x60];
};

#ifdef __cplusplus
static_assert(sizeof(client_s_cl) == 0xE5170);
#endif

} // namespace net
} // namespace game