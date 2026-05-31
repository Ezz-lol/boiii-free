#pragma once

#include <cstdint>
#include "core.hpp"
#include "net/net.hpp"
#include "db/xasset.hpp"

namespace game {
namespace cl {

enum class CLSwitchState : uint32_t {
  IDLE = 0x0,
  WAIT_FOR_CLIENTS_TO_CONNECT = 0x1,
  PRELOAD_RESERVE = 0x2,
  LOAD_FASTFILE = 0x3,
  PRIME_STREAMER = 0x4,
  SWITCH_MAPS = 0x5,
};

enum class CLSwitchMsg : uint32_t {
  LOAD_MAP = 0x0,
  SWITCH_MAPS = 0x1,
  IDLE = 0x2,
};

#pragma pack(push, 1)
// sizeof=0x10
struct CLSwitchMessage {
  CLSwitchMsg id;
  uint8_t _padding04[4];
  union {
    str64_t *str;
    int64_t integer;
    uint64_t unsignedInteger;
    float real;
  } p;
};
static_assert(sizeof(CLSwitchMessage) == 0x10,
              "CLSwitchMessage size must be 0x10 bytes");
#pragma pack(pop)

// sizeof=0x7B0
#pragma pack(push, 1)
struct CLMapSwitch {
  CLSwitchState state;
  int32_t switchCount;
  str128_t prevConfigString[3];
  str128_t preloadMap;
  str128_t loadingMovie;
  PMemStack preloadPMemStack;
  bool preload;
  uint8_t _padding28D[3];
  int32_t preloadReserveFrameCount;
  uint32_t preloadReservePagesTaken;
  uint32_t preloadReservePagesNeeded;
  cinematic_id loadingMovieId;
  CLSwitchMessage messages[16];
  int32_t messageCount;
  str64_t strArgs[16];
  int32_t strCount;
  int32_t timeFastfileLoadStarted;
  int32_t timeTextureStreamingStarted;
};
static_assert(sizeof(CLMapSwitch) == 0x7B0,
              "CLMapSwitch size must be 0x7B0 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
struct ReliableCommands {
  int32_t sequence;
  int32_t acknowledge;
  char *commands[128];
  int32_t commandBufferNext;
  char commandBuffer[16384];
  uint8_t _padding[4];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct clientConnection_t {
  int32_t qport;
  ClientNum_t clientNum;
  int32_t lastPacketSentTime;
  int32_t lastPacketTime;
  net::netadr_t serverAddress;
  int32_t connectTime;
  int32_t connectPacketCount;
  char serverMessage[256];
  int32_t challenge;
  int32_t checksumFeed;
  ReliableCommands reliableCommands;
  int32_t serverMessageSequence;
  int32_t serverCommandSequence;
  int32_t lastExecutedServerCommand;
  char serverCommands[128][1024];
  bool isServerRestarting;
  bool areTexturesLoaded;
  bool waitForMovie;
  bool hostCompromised;
  net::netchan_t netchan;
  uint64_t serverXUID;
  net::netProfileInfo_t OOBProf;
  uint8_t _unknown25150[1032];
  uint8_t *transferBuffer;
  int32_t transferBufferCompressedSize;
  uint8_t _padding25564[4];
  int64_t statPacketsToSend[2];
  int32_t statPacketSendTime[127];
  int32_t loadoutNonceSent;
  int32_t loadoutNonceReceived;
  int32_t loadoutNextRetry;
};
static_assert(sizeof(clientConnection_t) == 0x25780,
              "sizeof(clientConnection_t) != 0x25780");
#pragma pack(pop)

struct ClientConnections {
  LocalClientPool<clientConnection_t> connections;
};

#pragma pack(push, 1)
// Unverified
struct clientLogo_t {
  int32_t startTime;
  int32_t duration;
  int32_t fadein;
  int32_t fadeout;
  db::xasset::MaterialHandle material[2];
};
static_assert(sizeof(clientLogo_t) == 0x20, "sizeof(clientLogo_t) != 0x20");
#pragma pack(pop)

#pragma pack(push, 1)
// Unverified
struct vidConfig_t {
  uint32_t sceneWidth;
  uint32_t sceneHeight;
  float sceneAspectRatio;
  uint32_t displayWidth;
  uint32_t displayHeight;
  float displayAspectRatio;
  qboolean isWideScreen;
  qboolean isFullscreen;
  float aspectRatioWindow;
  float aspectRatioUI;
  float aspectRatioDisplayPixel;
  uint32_t maxTextureSize;
  qboolean deviceSupportsGamma;
};
static_assert(sizeof(vidConfig_t) == 0x34, "sizeof(vidConfig_t) != 0x34");
#pragma pack(pop)

// TODO when playerState_t defined and verified
// #pragma pack(pop)
// #pragma pack(push, 1)
// struct clSnapshot_t {
//   qboolean valid;
//   int32_t snapFlags;
//   int32_t serverTime;
//   int32_t physicsTime;
//   int32_t messageNum;
//   int32_t deltaNum;
//   int32_t ping;
//   int32_t cmdNum;
//   level::playerState_t ps;
//   int32_t numEntities;
//   int32_t numClients;
//   int32_t numActors;
//   int32_t parseMatchStateIndex;
//   int32_t parseEntitiesIndex;
//   int32_t parseClientsIndex;
//   int32_t parseActorsIndex;
//   int32_t serverCommandNum;
// };
// #pragma pack(pop)

#pragma pack(push, 1)
struct clientStatic_t {
  qboolean hunkUsersStarted;
  char servername[256];
  qboolean rendererStarted;
  qboolean soundStarted;
  qboolean uiStarted;
  int32_t frametime;
  float frametime_base;
  int32_t _unknown118;
  int32_t realtime;
  int32_t realFrametime; // only verified to here
  // TODO when following fields, underlying structs
  // defined and verified

  // qboolean inputUpdatedPrevFrame;
  // clientLogo_t logo;
  // vec3_t mapCenter;
  // int32_t numlocalservers;
  // net::serverInfo_t localServers[64];
  // int32_t pingUpdateSource;
  // uint8_t _padding3B5C[4];
  // phys::MaterialHandle whiteMaterial;
  // phys::MaterialHandle consoleMaterial;
  // phys::MaterialHandle spinnerMaterial;
  // phys::MaterialHandle loaderMaterial;
  // phys::MaterialHandle cinematicMaterial;
  // FontHandle consoleFont;
  // cinematic_id logoMovieId;
  // qboolean playedLogoCinematic;
  // vidConfig_t vidConfig;
  // vec3_t renderForward;
  // vec3_t renderPos;
  // net::XNADDR xnaddrs[18];
  // uint8_t _padding3E7E[2];
  // volatile int32_t scriptError;
  // vec3_t debugRenderForward;
  // vec3_t debugRenderPos;
  // int32_t serverId;
  // qboolean forceFullscreen;
  // qboolean exitForceFullscreen;
  // uint8_t _padding3EA8[8];
  // clSnapshot_t noDeltaSnapshot;
  // int32_t nextNoDeltaEntity;
  // uint8_t _paddingF414[4];
  // level::entityState_t noDeltaEntities[1024];
  // level::gameState_t gameState;
};

#pragma pack(pop)

} // namespace cl
} // namespace game