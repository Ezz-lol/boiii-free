#pragma once

#include "core.hpp"
#include "vehicle.hpp"
#include "user.hpp"
#include "phys.hpp"
#include "ai.hpp"
#include "scr.hpp"

namespace game {
namespace level {
struct EntityState {
  int32_t number;
}; // Incomplete

struct gentity_s {
  EntityState s;
  unsigned char __pad0[0x24C];
  gclient_s *client;
  unsigned char __pad1[0x17C];

  struct {
    uint32_t notifyString;
    uint32_t index;
    bool stoppable;
    int32_t basetime;
    int32_t duration;
  } snd_wait;

  unsigned char __pad2[0x110];
};

#ifdef __cplusplus
static_assert(sizeof(gentity_s) == 0x4F8);
#endif
#pragma pack(push, 1)
// sizeof=0x30
struct TopScorer {
  renderOptions_t renderOptions;
  weapon::AttachmentCosmeticVariantIndexes attachmentCosmeticVariantIndexes;
  uint8_t _padding14[4];
  weapon::Weapon weapon;
  uint8_t taunts[1];
  uint8_t gestures[3];
  int32_t clientNum;
  uint32_t
      padding[1]; // Note: this field is actually in the Treyarch struct, as-is.
  uint8_t _padding2C[4];
};
static_assert(sizeof(TopScorer) == 0x30, "TopScorer size must be 48 bytes");

// sizeof=0x120
struct UnarchivedMatchState {
  TopScorer topScorers[3];
  int32_t teamScores[20];
  int32_t matchUIVisibilityFlags;
  scoreboardColumnType_t scoreboardColumnTypes[10];
  uint32_t talkFlags;
  float topDownCameraYaw;
  uint16_t jumpHeight;
  bool initialPlayersConnected;
  bool sharedViewPort;
  uint8_t _padding118[8];
};
static_assert(sizeof(UnarchivedMatchState) == 0x120,
              "UnarchivedMatchState size must be 288 bytes");

// sizeof=8
struct CasterState {
  int32_t index;
  int32_t value;
};
static_assert(sizeof(CasterState) == 0x8, "CasterState size must be 8 bytes");

// sizeof=0x54
struct CasterClientState {
  int32_t clientIndex;
  uint32_t gadgetPlayerStateFlags;
  float gadgetPowerRemaining;
  int32_t lastUpdateIndex[18];
};
static_assert(sizeof(CasterClientState) == 0x54,
              "CasterClientState size must be 84 bytes");

// sizeof=0x290
struct MatchState {
  uint32_t index;
  char _padding[12];
  UnarchivedMatchState unarchivedState;
  ArchivedMatchState archivedState;
  char __padding[4];
};
static_assert(sizeof(MatchState) == 0x290, "MatchState size must be 656 bytes");

enum class loading_t : int32_t {
  LOADING_DONE = 0x0,
  LOADING_LEVEL = 0x1,
  LOADING_SAVEGAME = 0x2,
};
// sizeof=0x38
struct objcamCameraPoint {
  phys::objcamState ocState;
  scr::ScrString_t ocName;
  int ocIndex;
  int ocEntNum;
  scr::ScrString_t ocEntTag;
  team_t ocTeam;
  vec3_t ocPos;
  vec3_t ocAngles;
  float ocMaxHorizAngle;
  float ocMaxVertAngle;
};
static_assert(sizeof(objcamCameraPoint) == 0x38,
              "objcamCameraPoint size must be 56 bytes");

// sizeof=0x3800
struct objcamCameraTable {
  objcamCameraPoint cameraPoints[256];
};
static_assert(sizeof(objcamCameraTable) == 0x3800,
              "objcamCameraTable size must be 14336 bytes");

#pragma pack(pop)

#pragma pack(push, 16)
// Length of level_locals_t has size 0x23A10 on both client and server
struct level_locals_t {
  gclient_s *clients;
  gentity_s *gentities;
  int32_t gentitySize;
  int32_t num_entities;
  gentity_s *firstFreeEnt;
  gentity_s *lastFreeEnt;
  gentity_s *firstFreeFakeEnt;
  gentity_s *lastFreeFakeEnt;
  int32_t numFakeEntities;
  vechicle::vehicle_t *vehicles;
  gentity_s *firstFreeActor;
  gentity_s *lastFreeActor;
  int32_t num_actors;
  ai::sentient_t *sentients;
  user::actor_t *actors;
  int32_t actorCorpseCount;
  user::bot_t *bots;
  int32_t maxBots;
  bool initializing;
  bool clientIsSpawning;
  int32_t maxclients;
  int32_t teamSortedClients[18];
  objective_t objectives[128];
  int32_t objectivesClientMask[128][2];
  int32_t framenum;
  int32_t time;
  int32_t previousTime;
  int32_t frametime;
  int32_t startTime;
  SpawnVar spawnVar;
  EntHandle droppedWeaponCue[32];
  bool savepersist;
  float fFogOpaqueDist;
  float fFogOpaqueDistSqrd;
  bool bPlayerIgnoreRadiusDamage;
  bool bPlayerIgnoreRadiusDamageLatched;
  int32_t currentEntityThink;
  bool bRegisterLeaderboards;
  phys::cached_tag_mat_t cachedTagMat;
  phys::cached_tag_mat_t cachedEntTargetTagMat;
  phys::trigger_info_t pendingTriggerList[256];
  uint8_t triggerPlayerUsageCount[1024];
  phys::trigger_info_t currentTriggerList[256];
  int32_t pendingTriggerListSize;
  int32_t currentTriggerListSize;
  int32_t scriptPrintChannel;
  vec2_t compassMapUpperLeft;
  vec2_t compassMapLowerRight;
  vec2_t compassMapWorldSize;
  vec2_t compassMapCurrentUpperLeft;
  vec2_t compassMapCurrentLowerRight;
  vec2_t compassMapCurrentWorldSize;
  vec2_t compassNorth;
  int32_t finished;
  bool manualNameChange;
  modelNameIndex_t modelMap[8192];
  loading_t loading;
  int32_t script_ai_limit;
  uint32_t grenadeHintCount;
  uint8_t specialIndex[1024];
  phys::actor_prone_info_t cgData_actorProneInfo[96];
  uint8_t cgData_actorOnCompass[64];
  uint8_t cgData_actorTeam[64];
  bool bDrawCompassFriendlies;
  bool disable_grenade_suicide;
  int32_t numConnectedClients;
  ClientNum_t sortedClients[18];
  bool bUpdateScoresForIntermission;
  int32_t numVotingClients;
  MatchState matchState;
  CasterState casterState;
  CasterClientState *casterClients;
  int32_t teamScores[21];
  int32_t lastTeammateHealthTime;
  uint32_t teamHasSpyplane[21];
  uint32_t teamHasSatellite[21];
  int32_t teamSpyplaneTime[21];
  int32_t teamSatelliteTime[21];
  char voteString[1024];
  char voteDisplayString[1024];
  int32_t voteTime;
  int32_t voteExecuteTime;
  int32_t voteYes;
  int32_t voteNo;
  int32_t currentActorClone;
  int32_t iSearchFrame;
  phys::sv_FxVisBlock_t fxVisibilityEnts[32];
  int32_t currentPlayerClone;
  bool hostMigrationActive;
  int32_t hostMigrationStart;
  phys::actorAntilagFrame_t actorAntilagFrames[20];
  int32_t nextActorAntilagFrame;
  vechicle::vehicleAntilagFrame_t vehicleAntilagFrames[20];
  int32_t nextVehicleAntilagFrame;
  phys::ZBarrierType *zbarrierTypes[255];
  bool checkAnimChange;
  objcamCameraTable objectiveCameras;
  uint8_t _unknown[32];
};

static_assert(sizeof(level_locals_t) == 0x23A10,
              "level_locals_t size must be 144,336 bytes");

#pragma pack(pop)

} // namespace level
} // namespace game