#pragma once

#include <cstdint>
#include "core.hpp"
#include "net/net.hpp"
#include "db/xasset/core.hpp"
#include "scr/core.hpp"
#include "weapon.hpp"
#include "vehicle.hpp"
#include "hunk.hpp"
#include "level/core.hpp"
#include "steam.hpp" // IWYU pragma: keep

namespace game {

namespace level {
struct MatchState;
struct clientState_s;
typedef clientState_s clientState_t;

struct playerState_s;
typedef playerState_s playerState_t;

struct archivedEntity_t;

struct gentity_s;
typedef gentity_s gentity_t;

struct CasterState;
struct CasterClientState;
} // namespace level

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
ASSERT_SIZE(CLSwitchMessage, 0x10);
#pragma pack(pop)

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
ASSERT_SIZE(CLMapSwitch, 0x7B0);
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
struct ClientSteamAuth {
  steam::HAuthTicket authTicket;
  uint8_t pTicket[1024];
  uint32_t pcbTicket;
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
  ClientSteamAuth steamAuth;
  uint8_t *transferBuffer;
  int32_t transferBufferCompressedSize;
  uint8_t _padding25564[4];
  int64_t statPacketsToSend[2];
  int32_t statPacketSendTime[127];
  int32_t loadoutNonceSent;
  int32_t loadoutNonceReceived;
  int32_t loadoutNextRetry;
};
ASSERT_SIZE(clientConnection_t, 0x25780);
ASSERT_OFFSET(clientConnection_t, transferBuffer, 0x25558);
ASSERT_OFFSET(clientConnection_t, transferBufferCompressedSize, 0x25560);
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
ASSERT_SIZE(clientLogo_t, 0x20);
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
ASSERT_SIZE(vidConfig_t, 0x34);
#pragma pack(pop)

#pragma pack(push, 1)

struct clSnapshot_t {
  bool valid;
  uint8_t _padding01[3];
  int32_t snapFlags;
  int32_t serverTime;
  int32_t physicsTime;
  int32_t messageNum;
  int32_t deltaNum;
  int32_t ping;
  int32_t cmdNum;
  level::playerState_t ps;
  int32_t numEntities;
  int32_t numClients;
  int32_t numActors;
  int32_t numDestructibles;
  int32_t parseMatchStateIndex;
  int32_t parseCasterStateIndex;
  int32_t numCasterClients;
  int32_t parseCasterClientsIndex;
  int32_t parseEntitiesIndex;
  int32_t parseClientsIndex;
  int32_t parseActorsIndex;
  int32_t parseDestructiblesIndex;
  int32_t serverCommandNum;
  uint8_t _paddingB574[6];
};
ASSERT_SIZE(clSnapshot_t, 0xB5C0); // Correct
#pragma pack(pop)

constexpr uint32_t GAMESTATE_T_SIZE = 0xB8C80; // Correct
/*
   This struct is obviously very incorrect.
   Without `partial_def`'s padding, it has size 0x171B8.
   TODO: correct this when needed.
*/
partial_def(GAMESTATE_T_SIZE, struct, gameState_t, {
  int32_t stringOffsets[3635];
  uint32_t stringChecksums[3635];
  char stringData[65536];
  int32_t dataCount;
  int32_t stringUpdateFrameNumber;
  int32_t matchUIVisibilityFlags;
  scr::scrChecksum_t scr_checksum;
  uint32_t clientfield_hash;
  uint32_t server_highest_clientfield_version;
});
ASSERT_SIZE(gameState_t, GAMESTATE_T_SIZE);

#pragma pack(push, 1)
struct clientStatic_t {
  qboolean hunkUsersStarted;
  str256_t servername;
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
  // lobby::LobbyClientPool<net::XNADDR> xnaddrs;
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

struct ClientArchiveData {
  int32_t serverTime;
  vec3_t origin;
  vec3_t velocity;
  int32_t bobCycle;
  int32_t movementDir;
  vehicle::PlayerVehicleState playerVehStateClientArchive;
};
ASSERT_SIZE(ClientArchiveData, 0xD8);

constexpr uint32_t CL_SNAPSHOT_COUNT = 0x10;
constexpr int32_t PACKET_BACKUP_MASK = CL_SNAPSHOT_COUNT - 1;

struct clSnapshotPool_t {
  array<clSnapshot_t, CL_SNAPSHOT_COUNT> snapshots;
};
ASSERT_SIZE(clSnapshotPool_t, 0xB5C00);

struct clOutPacketsPool_t {
  array<outPacket_t, CL_SNAPSHOT_COUNT> outPackets;
};
ASSERT_SIZE(clOutPacketsPool_t, 0xC0);

constexpr uint32_t CL_ENTITY_COUNT = 0xC80;

struct clEntityStatePool_t {
  array<level::entityState_t, CL_ENTITY_COUNT> states;
};
ASSERT_SIZE(clEntityStatePool_t, 0x183800);

struct clClientStates_t {
  array<level::clientState_t, CL_SNAPSHOT_COUNT> states;
};
ASSERT_SIZE(clClientStates_t, 0x3380);

constexpr uint32_t CL_ACTOR_COUNT = 0x400;

struct clActorStatePool_t {
  array<level::actorState_t, CL_ACTOR_COUNT> states;
};
ASSERT_SIZE(clActorStatePool_t, 0x14000);

struct clCasterStatePool_t {
  array<level::CasterState, CL_SNAPSHOT_COUNT> states;
};
ASSERT_SIZE(clCasterStatePool_t, 0x80);

struct clCasterClientStatePool_t {
  array<level::CasterClientState, CL_SNAPSHOT_COUNT> states;
};
ASSERT_SIZE(clCasterClientStatePool_t, 0x540);

struct clMatchStatePool_t {
  array<level::MatchState, CL_SNAPSHOT_COUNT> states;
};
ASSERT_SIZE(clMatchStatePool_t, 0x2A00);

#pragma pack(push, 1)
constexpr uint32_t CLIENTACTIVE_SIZE = 0x197A30;
partial_def(CLIENTACTIVE_SIZE, struct, clientActive_t, {
  clMatchStatePool_t *parseMatchStatesBuf;
  int32_t numParseMatchStates;
  int32_t maxParseMatchStates;
  clCasterStatePool_t *parseCasterStatesBuf;
  int32_t numParseCasterStates;
  int32_t maxParseCasterStates;
  clCasterClientStatePool_t *parseCasterClientsBuf;
  int32_t numParseCasterClients;
  int32_t maxParseCasterClients;
  clEntityStatePool_t *parseEntitiesBuf;
  int32_t numParseEntities;
  int32_t maxParseEntities;
  clClientStates_t *parseClientsBuf;
  int32_t numParseClients;
  int32_t maxParseClients;
  clActorStatePool_t *parseActorsBuf;
  int32_t numParseActors;
  int32_t maxParseActors;
  clOutPacketsPool_t *outPackets;
  clSnapshotPool_t *snapshots;
  int32_t packetBackupCount;
  int32_t packetBackupMask;
  int32_t gameStateMarker;
  bool usingAds;
  uint8_t _padding07C[3];
  int32_t timeoutcount;
  uint8_t _padding084[12];
  clSnapshot_t snap;
  int32_t snapServerTime;
  int32_t oldSnapServerTime;
  bool alwaysFalse;
  uint8_t _paddingB659[3];
  int32_t serverTime;
  int32_t oldServerTime;
  int32_t oldFrameServerTime;
  int32_t serverTimeDelta;
  int32_t estimatedServerTime;
  int32_t realTimeLatestSnaphot;
  bool extrapolatedSnapshot;
  bool newSnapshots;
  uint8_t _paddingB676[2];
  int32_t serverId;
  bool forceNewSnapshots;
  uint8_t _paddingB67D[3];
  int32_t unpausedTimeBehind;
  int32_t serverTimeErrorIndex;
  int32_t serverTimeErrorCount;
  int32_t serverTimeErrorCumul;
  int32_t serverTimeErrorAvg;
  int32_t serverTimeErrorPrev;
  int32_t serverTimeError[20];
  int32_t serverTimeError2[20];
  str64_t mapname;
  int32_t parseMatchStateIndex;
  int32_t parseCasterStateIndex;
  int32_t parseCasterClientsIndex;
  int32_t parseEntitiesIndex;
  int32_t parseClientsIndex;
  int32_t parseActorsIndex;
  int32_t parseDestructiblesIndex;
  bool stanceHeld;
  uint8_t _paddingB795[3];
  StanceState stance;
  StanceState stancePosition;
  int32_t stanceTime;
  bool wasSprinting;
  bool wasJumping;
  bool wasWallRunning;
  uint8_t _paddingB7A7[1];
  uint64_t priorPMFlags;
  bool weapNextHeld;
  uint8_t _paddingB7B1[3];
  int32_t weapNextTime;
  bool switchingToInventory;
  uint8_t _paddingB7B9[7];
  weapon::Weapon cgameUserCmdWeapon;
  weapon::Weapon cgameUserCmdOffHandWeapon;
  weapon::Weapon cgameUserCmdLastWeaponForAlt;
  float cgameMaxPitchSpeed;
  float cgameMaxYawSpeed;
  vec3_t cgameKickAngles;
  vec3_t cgameOrigin;
  vec3_t cgameVelocity;
  vehicle::PlayerVehicleState cgamePlayerVehState;
  int32_t cgameBobCycle;
  int32_t cgameMovementDir;
  game_button_bits_t cgameExtraButton_bits;
  int32_t cgamePredictedDataServerTime;
  uint16_t cgameUserCmdPlayerLevel;
  uint8_t _paddingB892[2];
  vec3_t viewangles;
  int32_t skelTimeStamp;
  char skelMemory[0x100000];
  uint8_t _padding10B8E4[4];
  hunk::HunkUser *skelMemoryPool;
  user::usercmd_t cmds[0x80];
  int32_t cmdNumber;
  ClientArchiveData clientArchive[0x100];
  int32_t clientArchiveIndex;
  /*
     Struct seems to nearly be complete and correct,
     but slightly exceeds correct size when these fields are added.
     TODO: finish struct definition and add remaining fields.
  */
  // level::entityState_t entityBaselines[CL_ACTOR_COUNT];
  // bool corruptedTranslationFile;
  // str256_t translationVersion;
  // uint8_t _padding1961F9[3];
  // int32_t lastFireTime;
  // bool useHeld;
  // uint8_t _padding196201[3];
  // int32_t useTime;
  // int32_t useCount;
  // bool wasInVehicle;
  // uint8_t _padding19620D[3];
  // int32_t serverFrameMS;
  // int32_t serverKbps;
  // int32_t serverKbpsAvg[2];
  // int32_t serverKbpsCount[2];
  // uint8_t _padding196228[8];
});
#pragma pack(pop)
ASSERT_OFFSET(clientActive_t, parseCasterStatesBuf, 0x10);
ASSERT_OFFSET(clientActive_t, parseActorsBuf, 0x50);
ASSERT_OFFSET(clientActive_t, packetBackupCount, 0x70);
ASSERT_OFFSET(clientActive_t, packetBackupMask, 0x74);
ASSERT_OFFSET(clientActive_t, outPackets, 0x60);
ASSERT_OFFSET(clientActive_t, snapshots, 0x68);
ASSERT_OFFSET(clientActive_t, parseEntitiesBuf, 0x30);
ASSERT_OFFSET(clientActive_t, maxParseEntities, 0x3C);
ASSERT_OFFSET(clientActive_t, numParseEntities, 0x38);
ASSERT_OFFSET(clientActive_t, parseClientsBuf, 0x40);
ASSERT_OFFSET(clientActive_t, numParseClients, 0x48);
ASSERT_OFFSET(clientActive_t, maxParseClients, 0x4C);
ASSERT_OFFSET(clientActive_t, numParseActors, 0x58);
ASSERT_OFFSET(clientActive_t, maxParseActors, 0x5C);
ASSERT_OFFSET(clientActive_t, parseMatchStatesBuf, 0x00);
ASSERT_OFFSET(clientActive_t, numParseMatchStates, 0x8);
ASSERT_OFFSET(clientActive_t, maxParseMatchStates, 0xC);
ASSERT_OFFSET(clientActive_t, numParseCasterStates, 0x18);
ASSERT_OFFSET(clientActive_t, maxParseCasterStates, 0x1C);
ASSERT_OFFSET(clientActive_t, parseCasterClientsBuf, 0x20);
ASSERT_OFFSET(clientActive_t, numParseCasterClients, 0x28);
ASSERT_OFFSET(clientActive_t, maxParseCasterClients, 0x2C);
ASSERT_SIZE(clientActive_t, CLIENTACTIVE_SIZE);

struct ClientActives {
  LocalClientPool<clientActive_t> actives;
};

/*
  Enumerated flags are not easily discoverable - they are not defined
  in any prior engine versions with debug symbols.

  Some flags can be found by inspecting debug assertion messages.
  Those listed below were found this way.

  Add enumerations as they are discovered.
*/
struct clientAllocFlag_t {
  enum clientAllocFlag : uint32_t {
    DRY_RUN = 0b00000000000000000000000000000001,
    UNKNOWN_B1 = 0b00000000000000000000000000000010,
    UNKNOWN_B2 = 0b00000000000000000000000000000100,
    UNKNOWN_B3 = 0b00000000000000000000000000001000,
    UNKNOWN_B4 = 0b00000000000000000000000000010000,
    UNKNOWN_B5 = 0b00000000000000000000000000100000,
    UNKNOWN_B6 = 0b00000000000000000000000001000000,
    UNKNOWN_B7 = 0b00000000000000000000000010000000,
    UNKNOWN_B8 = 0b00000000000000000000000100000000,
    UNKNOWN_B9 = 0b00000000000000000000001000000000,
    UNKNOWN_B10 = 0b00000000000000000000010000000000,
    UNKNOWN_B11 = 0b00000000000000000000100000000000,
    UNKNOWN_B12 = 0b00000000000000000001000000000000,
    UNKNOWN_B13 = 0b00000000000000000010000000000000,
    UNKNOWN_B14 = 0b00000000000000000100000000000000,
    UNKNOWN_B15 = 0b00000000000000001000000000000000,
    UNKNOWN_B16 = 0b00000000000000010000000000000000,
    UNKNOWN_B17 = 0b00000000000000100000000000000000,
    UNKNOWN_B18 = 0b00000000000001000000000000000000,
    UNKNOWN_B19 = 0b00000000000010000000000000000000,
    UNKNOWN_B20 = 0b00000000000100000000000000000000,
    UNKNOWN_B21 = 0b00000000001000000000000000000000,
    UNKNOWN_B22 = 0b00000000010000000000000000000000,
    UNKNOWN_B23 = 0b00000000100000000000000000000000,
    UNKNOWN_B24 = 0b00000001000000000000000000000000,
    UNKNOWN_B25 = 0b00000010000000000000000000000000,
    UNKNOWN_B26 = 0b00000100000000000000000000000000,
    UNKNOWN_B27 = 0b00001000000000000000000000000000,
    UNKNOWN_B28 = 0b00010000000000000000000000000000,
    UNKNOWN_B29 = 0b00100000000000000000000000000000,
    UNKNOWN_B30 = 0b01000000000000000000000000000000,
    UNKNOWN_B31 = 0b10000000000000000000000000000000
  };
};

union clientAllocFlags_t {
  uint32_t flags;
  struct {
    uint32_t dryRun : 1;
    uint32_t unknown_b1 : 1;
    uint32_t unknown_b2 : 1;
    uint32_t unknown_b3 : 1;
    uint32_t unknown_b4 : 1;
    uint32_t unknown_b5 : 1;
    uint32_t unknown_b6 : 1;
    uint32_t unknown_b7 : 1;
    uint32_t unknown_b8 : 1;
    uint32_t unknown_b9 : 1;
    uint32_t unknown_b10 : 1;
    uint32_t unknown_b11 : 1;
    uint32_t unknown_b12 : 1;
    uint32_t unknown_b13 : 1;
    uint32_t unknown_b14 : 1;
    uint32_t unknown_b15 : 1;
    uint32_t unknown_b16 : 1;
    uint32_t unknown_b17 : 1;
    uint32_t unknown_b18 : 1;
    uint32_t unknown_b19 : 1;
    uint32_t unknown_b20 : 1;
    uint32_t unknown_b21 : 1;
    uint32_t unknown_b22 : 1;
    uint32_t unknown_b23 : 1;
    uint32_t unknown_b24 : 1;
    uint32_t unknown_b25 : 1;
    uint32_t unknown_b26 : 1;
    uint32_t unknown_b27 : 1;
    uint32_t unknown_b28 : 1;
    uint32_t unknown_b29 : 1;
    uint32_t unknown_b30 : 1;
    uint32_t unknown_b31 : 1;
  };
};

} // namespace cl
} // namespace game