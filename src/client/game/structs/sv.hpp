#pragma once

#include "core.hpp"
#include "quake.hpp"
#include "net/net.hpp"
#include "scr.hpp"
#include <cstddef>
#include <cstdint>

namespace game {

namespace user {
struct actor_t;
struct actorState_t;
struct cachedActor_t;
} // namespace user

namespace level {
struct MatchState;

// TODO
struct playerState_s;
typedef playerState_s playerState_t;
struct entityState_t;
struct archivedEntity_t;

struct gentity_t;
} // namespace level
namespace sv {

// TODO
struct clientState_s;

typedef clientState_s clientState_t;

struct client_s {
  int32_t state;
  char __pad0[0x28];
  net::netadr_t address;
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
static_assert(sizeof(client_s) == 0xE5110,
              "client_s size must be 0xE5110 bytes");

static_assert(offsetof(client_s, address) == 0x2C,
              "client_s::address offset must be 0x2C bytes");
static_assert(offsetof(client_s, xuid) == 0x55C8,
              "client_s::xuid offset must be 0x55C8 bytes");
static_assert(offsetof(client_s, guid) == 0xBB354,
              "client_s::guid offset must be 0xBB354 bytes");
static_assert(offsetof(client_s, bIsTestClient) == 0xBB360,
              "client_s::bIsTestClient offset must be 0xBB360 bytes");
#endif

struct client_s_cl : client_s {
  char __pad1_0[0x60];
};

#ifdef __cplusplus
static_assert(sizeof(client_s_cl) == 0xE5170,
              "client_s_cl size must be 0xE5170 bytes");
#endif

struct svEntity_t {
  uint16_t worldSector;
  uint16_t nextEntityInWorldSector;
  int32_t linkcontents;
  vec3_t linkmin;
  vec3_t linkmax;
};
static_assert(sizeof(svEntity_t) == 0x20, "svEntity_t size must be 32 bytes");

struct CasterClientState {
  int32_t clientIndex;
  uint32_t gadgetPlayerStateFlags;
  float gadgetPowerRemaining;
  int32_t lastUpdateIndex[18];
};

struct CasterState {
  int32_t index;
  int32_t value;
};

struct cachedClient_t; // TODO

struct cachedSnapshot_t {
  int32_t archivedFrame;
  int32_t time;
  int32_t physicsTime;
  int32_t entityCount;
  int32_t clientCount;
  int32_t actorCount;
  int32_t destructibleCount;
  int32_t firstEntityIndex;
  int32_t firstClientIndex;
  int32_t firstActorIndex;
  int32_t firstDestructibleIndex;
  int32_t matchStateIndex;
  int32_t casterStateIndex;
  int32_t usesDelta;
};

static_assert(sizeof(cachedSnapshot_t) == 0x38,
              "cachedSnapshot_t size must be 56 bytes");

#pragma pack(push, 1)
// Verified, 100% correct on dedicated server.
// Currently unverified whether this is identical on client.
struct serverStaticHeader_t {
  sv::client_s *clients;
  user::actor_t *actors;
  int32_t time;
  int32_t physicsTime;
  int32_t snapFlagServerBit;
  int32_t packetBackupCount;
  int32_t packetBackupMask;
  uint8_t _padding24[4];
  svEntity_t *svEntities;
  int32_t numNoDeltaEntities;
  int32_t nextNoDeltaEntity;
  level::entityState_t *noDeltaEntities;
  int32_t nextSnapshotEntities;
  int32_t numSnapshotEntities;
  level::entityState_t *snapshotEntities;
  int32_t nextCachedSnapshotEntities;
  int32_t numCachedSnapshotEntities;
  level::archivedEntity_t *cachedSnapshotEntities;
  int32_t numSnapshotMatchStates;
  int32_t nextSnapshotMatchStates;
  level::MatchState *snapshotMatchStates;
  int32_t numCachedSnapshotMatchStates;
  int32_t nextCachedSnapshotMatchStates;
  level::MatchState *cachedSnapshotMatchStates;
  int32_t numSnapshotCasterStates;
  int32_t nextSnapshotCasterStates;
  CasterState *snapshotCasterStates;
  int32_t numCachedSnapshotCasterStates;
  int32_t nextCachedSnapshotCasterStates;
  CasterState *cachedSnapshotCasterStates;
  int32_t numSnapshotCasterClientStates;
  int32_t nextSnapshotCasterClientStates;
  CasterClientState *snapshotCasterClientStates;
  int32_t numSnapshotClients;
  int32_t nextSnapshotClients;
  clientState_t *snapshotClients;
  int32_t numCachedSnapshotClients;
  int32_t nextCachedSnapshotClients;
  cachedClient_t *cachedSnapshotClients;
  int32_t numSnapshotActors;
  int32_t nextSnapshotActors;
  user::actorState_t *snapshotActors;
  int32_t numCachedSnapshotActors;
  int32_t nextCachedSnapshotActors;
  user::cachedActor_t *cachedSnapshotActors;
  vec3_t mapCenter;
  vec3_t mapCenterBoundsMin;
  vec3_t mapCenterBoundsMax;
  uint8_t _padding114[4];
  uint8_t *archivedSnapshotBuffer;
  cachedSnapshot_t *cachedSnapshotFrames;
  int32_t nextCachedSnapshotFrames;
  int32_t nextArchivedSnapshotFrames;
  int32_t archivedEntityCount;
  int32_t num_entities;
  int32_t maxclients;
  int32_t fps;
  qboolean clientArchive;
  uint8_t _padding144[4];
  level::gentity_t *gentities;
  int32_t gentitySize;
  uint8_t _padding154[4];
  clientState_t *firstClientState;
  user::actorState_t *firstActorState;
  level::playerState_t *firstPlayerState;
  level::MatchState *matchState;
  CasterState *casterState;
  CasterClientState *firstCasterClientState;
  int32_t casterClientSize;
  int32_t clientSize;
  int32_t actorSize;
  int32_t serverFrameMS;
  qboolean constBaselineIsValid;
  int32_t constBaselineEntitesCount;
  int32_t constBaselineEntitesMax;
  uint8_t _padding1A4[4];
  const level::entityState_t *constBaselineEntities;
};
static_assert(sizeof(serverStaticHeader_t) == 0x1B0,
              "serverStaticHeader_t size must be 0x1B0 bytes");
#pragma pack(pop)

// TODO
struct archivedEntity_s;
typedef archivedEntity_s archivedEntity_t;

#pragma pack(push, 1)
// sizeof=0xB8
struct serverSnapshot_t {
  level::playerState_t *ps[18];
  int32_t entityCount;
  int32_t clientCount;
  int32_t actorCount;
  int32_t firstEntityIndex;
  int32_t firstClientIndex;
  int32_t firstActorIndex;
  int32_t matchStateIndex;
  int32_t casterStateIndex;
  int32_t firstCasterClientIndex;
  uint8_t _padding[4];
};
static_assert(sizeof(serverSnapshot_t) == 0xB8,
              "serverSnapshot_t size must be 0xB8 bytes");
#pragma pack(pop)

struct archivedSnapshot_s {
  int32_t start;
  int32_t size;
};
typedef archivedSnapshot_s archivedSnapshot_t;

#pragma pack(push, 1)
struct tempBanSlot_t {
  uint64_t guid;
  uint32_t banTime;
  uint8_t _padding[4];
};
static_assert(sizeof(tempBanSlot_t) == 16, "tempBanSlot_t must be 16 bytes");
#pragma pack(pop)

// Fields and types unverified, but size should be correct.
struct rateBoostingStats_t {
  int32_t unknown00;
  int32_t unknown04;
  int32_t bytesSent;
  int32_t serverMS;
  int32_t clientPings;
  int32_t clientAckFramesBehind;
  int32_t clientUsercmdTimeDiff;
  int32_t numClientFrames;
  int32_t numFrames;
};
static_assert(sizeof(rateBoostingStats_t) == 0x24,
              "rateBoostingStats_t size must be 0x24 bytes");

/*
  Verified correct on dedicated server - both total size (0x1500) and offsets of
  fields.
  Unverified on client, but likely either identical or very similar.
*/
#pragma pack(push, 1)
struct serverStatic_t {
  int32_t clientCount;
  int32_t packetBackupCount;
  int32_t packetBackupMask;
  uint8_t _padding0C[4];
  level::entityState_t *noDeltaEntities;
  level::entityState_t *snapshotEntities;
  int32_t numNoDeltaEntities;
  int32_t nextNoDeltaEntity;
  cachedSnapshot_t *cachedSnapshotFrames;
  archivedEntity_t *cachedSnapshotEntities;
  serverSnapshot_t snapshots[16];
  int32_t serverSnapshotTimes[16];
  int32_t nextServerSnapshotIndex;
  qboolean baselineCreated;
  qboolean initialized;
  uint32_t time;
  uint32_t physicsTime;
  uint32_t oldestArchiveTime;
  uint32_t checkpointRestoreTime;
  int32_t snapFlagServerBit;
  client_s *clients;
  int32_t numCachedSnapshotMatchStates;
  int32_t numCachedSnapshotCasterStates;
  int32_t numCachedSnapshotEntities;
  int32_t numCachedSnapshotClients;
  int32_t numCachedSnapshotActors;
  int32_t numSnapshotMatchStates;
  int32_t numSnapshotCasterStates;
  int32_t numSnapshotCasterClientStates;
  int32_t numSnapshotEntities;
  int32_t numSnapshotClients;
  int32_t numSnapshotActors;
  int32_t nextSnapshotMatchStates;
  int32_t nextSnapshotCasterStates;
  int32_t nextSnapshotCasterClientStates;
  int32_t nextSnapshotEntities;
  int32_t nextSnapshotClients;
  int32_t nextSnapshotActors;
  uint8_t _paddingC64[4];
  level::MatchState *snapshotMatchStates;
  CasterState *snapshotCasterStates;
  CasterClientState *casterClients;
  clientState_t *snapshotClients;
  user::actorState_t *snapshotActors;
  int32_t nextArchivedSnapshotFrames;
  uint8_t _paddingC94[4];
  archivedSnapshot_t *archivedSnapshotFrames;
  uint8_t *archivedSnapshotBuffer;
  int32_t nextArchivedSnapshotBuffer;
  int32_t nextCachedSnapshotMatchStates;
  int32_t nextCachedSnapshotCasterStates;
  int32_t nextCachedSnapshotEntities;
  int32_t nextCachedSnapshotClients;
  int32_t nextCachedSnapshotActors;
  int32_t nextCachedSnapshotFrames;
  uint8_t _paddingCC4[4];
  level::MatchState *cachedSnapshotMatchStates;
  CasterState *cachedSnapshotCasterStates;
  cachedClient_t *cachedSnapshotClients;
  user::cachedActor_t *cachedSnapshotActors;
  int32_t nextHeartbeatTime;
  int32_t nextStatusResponseTime;
  net::netProfileInfo_t OOBProf;
  /*
    Probably padding - never accessed as far as I can tell, except
    in one case of tempBans iteration, where offset iteration starts at
    _padding12D0 but the offset is always accessed with a constant +1 increment,
    which is likely a compiler optimization.

    The compiler would likely not optimize this way unless this field was
    padding.
  */
  uint8_t _padding12D0[16];
  tempBanSlot_t tempBans[16];
  XUID xuids[18];
  vec3_t mapCenter;
  vec3_t mapCenterBoundsMin;
  vec3_t mapCenterBoundsMax;
  int32_t dataSentLastSec;
  int32_t activeEntities;
  int32_t archivedEntityCount;
  int32_t snapshotBoostingSuspendTimeout;
  int32_t badServerFPSCount;
  int32_t badServerFPSLast;

  rateBoostingStats_t rateBoostingStats;
  qboolean rateBoostingEnabled;
  int32_t constBaselineEntitesMax;
  const level::entityState_t *constBaselineEntities;
  uint8_t _padding14E0[0x20];
};
static_assert(sizeof(serverStatic_t) == 0x1500,
              "serverStatic_t size must be 0x1500 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
struct flashbackState_t {
  vec3_t position;
  vec3_t angles;
  vec3_t velocity;
  float playerEnergyRatio;
  uint8_t psFlags;
  bool valid;
  uint8_t _padding2A[2];
};
#pragma pack(pop)

struct clientsFlashbackArchive_t {
  flashbackState_t clientsFlashbackStates[18];
  int32_t time;
};

enum class serverState_t : uint32_t {
  SS_DEAD = 0x0,
  SS_LOADING = 0x1,
  SS_GAME = 0x2,
};

struct recentFrame {
  bool lagged;
};
static_assert(sizeof(recentFrame) == 0x1, "sizeof(recentFrame) == 0x1");

struct animationNumber_t {
  union {
    struct {
      uint16_t index : 14;
      uint16_t toggle : 1;
    };
    uint16_t packed;
  };
};

#pragma pack(push, 1)
struct playerAnimState_t {
  float fTorsoPitch;
  float fWaistPitch;
  animationNumber_t animNum[3];
  union {
    int8_t flags;
    struct {
      uint8_t motionMatchingEnabled : 1;
      uint8_t mmTransitionBoost : 1;
      uint8_t unused_slots : 6;
    };
  };
  uint8_t _padding0F[1];
};
static_assert(sizeof(playerAnimState_t) == 0x10,
              "sizeof(playerAnimState_t) must equal 0x10");
#pragma pack(pop)

#pragma pack(push, 1)
struct AntilagPlayerAnimState {
  uint16_t notifyChild;
  int16_t notifyIndex;
  scr::ScrString_t notifyName;
  uint16_t animIndex;
  uint8_t goalTime;
  uint8_t weight;
  uint8_t goalWeight;
  uint8_t time;
  uint8_t oldTime;
  uint8_t _padding0F[1];
  float rate;
};
static_assert(sizeof(AntilagPlayerAnimState) == 0x14,
              "sizeof(AntilagPlayerAnimState) != 0x14");
#pragma pack(pop)

struct clientControllers_t {
  vec3_t angles[6];
  vec3_t tag_origin_angles;
  vec3_t tag_origin_offset;
};
static_assert(sizeof(clientControllers_t) == 0x60,
              "sizeof(clientControllers_t) == 0x60");

struct clientsPositionArchive_t {
  int32_t time;
  bool valid[18];
  uint8_t flags[18];
  vec3_t positions[18];
  vec3_t angles[18];
  vec3_t viewangles[18];
  playerAnimState_t anim[18];
  int32_t lerpFlags[18];
  int32_t lerpFlags2[18];
  int32_t locBlendTime[18];
  AntilagPlayerAnimState animStates[200];
  int32_t animStatesPos[18];
  int32_t animStatesCount[18];
  int32_t animStatesUsed;
  clientControllers_t controllers[18];
};
static_assert(sizeof(clientsPositionArchive_t) == 0x1B9C,
              "sizeof(clientsPositionArchive_t) must equal 0x1B9C");

#pragma pack(push, 1)
struct server_t {
  serverState_t state;
  int32_t physicsTime;
  int32_t timeResidual;
  int32_t lastTickMS;
  int32_t lastMainTickMS;
  int32_t levelTime;
  bool isUILevel;
  bool isRunnable;
  bool allowSelfTick;
  bool allowNetPackets;
  qboolean restarting;
  int32_t start_frameTime;
  int32_t checksumFeed;
  qboolean wroteConfigStrings;
  scr::ScrString_t emptyConfigString;
  scr::ScrString_t configstrings[3568];
  svEntity_t svEntities[2048];
  level::gentity_t *gentities;
  int32_t gentitySize;
  int32_t num_entities;
  level::playerState_t *gameClients;
  int32_t gameClientSize;
  uint8_t _padding80C[4];
  user::actor_t *gameActors;
  int32_t gameActorSize;
  int32_t checksum;
  int32_t skelTimeStamp;
  int32_t skelMemPos;
  int32_t bpsWindow[20];
  int32_t bpsWindowSteps;
  int32_t bpsTotalBytes;
  int32_t bpsMaxBytes;
  int32_t ubpsWindow[20];
  int32_t ubpsTotalBytes;
  int32_t ubpsMaxBytes;
  float ucompAve;
  int32_t ucompNum;
  volatile int32_t serverFrameTime;
  volatile int32_t serverFrameTimeMin;
  volatile int32_t serverFrameTimeMax;
  recentFrame recentFrameInfo[200];
  uint8_t _unknown139B8[36];
  char gametype[64];
  qboolean killServer;
  const char *killReason;
  int32_t currentFrameNum;
  int32_t nextClientsPositionArchive;
  clientsPositionArchive_t clientsPositionArchive[40];
  int32_t nextClientsFlashbackArchive;
  clientsFlashbackArchive_t clientsFlashBackArchive[512];
  clientsFlashbackArchive_t clientsLatestFlashBack;
  int32_t securityChecksum[3];
  uint8_t _padding7BEFC[4];
};
static_assert(sizeof(server_t) == 0xBC5C0,
              "sizeof(server_t) must equal 0xBC5C0");
#pragma pack(pop)

} // namespace sv
} // namespace game