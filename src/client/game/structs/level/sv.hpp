#pragma once

#include "core.hpp"
#include "../core.hpp"
#include "../vehicle.hpp"
#include "../user.hpp"
#include "../phys.hpp"
#include "../ai.hpp"
#include "../scr/scr.hpp"
#include "../lobby/core.hpp"

namespace game {

namespace level {

enum class loading_t : uint32_t {
  DONE = 0x0,
  LEVEL = 0x1,
  SAVEGAME = 0x2,
};

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

enum class sessionState_t : int32_t {
  PLAYING = 0x0,
  DEAD = 0x1,
  SPECTATOR = 0x2,
  INTERMISSION = 0x3,
};

enum class clientConnected_t : uint32_t {
  DISCONNECTED = 0x0,
  CONNECTING = 0x1,
  CONNECTED = 0x2,
};

struct playerTeamState_t {
  int32_t location;
};
ASSERT_SIZE(playerTeamState_t, 4);

/*
  Almost entirely unverified as of initial definition, except for
  those fields with a comment indicating otherwise.

  Verify and adjust as needed prior to usage.
*/
#pragma pack(push, 1)
constexpr uint32_t CLIENTSESSION_T_SIZE = 0x540;
struct clientSession_t {
  sessionState_t sessionState;
  ClientNum_t forceSpectatorClient;
  int32_t isInKillcam;
  int32_t killCamEntity;
  int32_t killCamTargetEntity;
  uint8_t _padding14[4];
  weapon::Weapon killCamWeapon;
  int32_t killCamMod;
  int32_t killCamFlag;
  int32_t archiveTime;
  scr::var::ScrVarIndex_t scriptPersId;
  clientConnected_t connected;
  user::usercmd_t cmd;
  user::usercmd_t oldcmd;
  bool localClient;
  bool predictItemPickup;
  name_t newnetname;
  uint8_t _padding116[2];
  int32_t maxHealth;
  int32_t enterTime;
  playerTeamState_t teamState;
  int32_t voteCount;
  int32_t teamVoteCount;
  float moveSpeedScaleMultiplier;
  int32_t noSpectate;
  qboolean teamInfo;
  clientState_t cs; // Verified
  int32_t psOffsetTime;
  /*
     Unsure if this is the correct length. Array was expanded to ensure
     correct total struct size (0x540) was met.
     Correct as needed.
  */
  int32_t scoreboardColumnCache[51];
};
ASSERT_OFFSET(clientSession_t, localClient, 0xF4);
ASSERT_OFFSET(clientSession_t, _padding14, 0x14);
ASSERT_OFFSET(clientSession_t, _padding116, 0x116);
ASSERT_OFFSET(clientSession_t, cs, 0x138);
ASSERT_SIZE(clientSession_t, CLIENTSESSION_T_SIZE);
#pragma pack(pop)

/*
  Almost entirely unverified  as of initial definition, except for
  those fields with a comment indicating otherwise, and total size.

  Verify and adjust as needed prior to usage.
*/
#pragma pack(push, 1)
struct gclient_s {
  playerState_t ps;
  playerState_t lastPlayerPS;
  uint8_t _padding16ACC[4];
  clientSession_t sess; // Verified
  ClientNum_t spectatorClient;
  uint8_t _unknown[100];
  int32_t flags;
  int32_t lastCmdTime;
  game_button_bits_t button_bits;
  game_button_bits_t oldbutton_bits;
  game_button_bits_t latched_button_bits;
  game_button_bits_t button_bitsSinceLastFrame;
  float fGunPitch;
  float fGunYaw;
  int32_t damage_blood;
  vec3_t damage_from;
  int32_t damage_fromWorld;
  int32_t inactivityTime;
  int32_t inactivityWarning;
  int32_t lastVoiceTime;
  int32_t outWaterTime;
  int32_t switchSeatTime;
  float currentAimSpreadScale;
  int32_t dropWeaponTime;
  EntHandle pLookatEnt;
  vec4_t prevLinkedInvQuat;
  bool prevLinkAnglesSet;
  bool link_doCollision;
  bool link_useTagAnglesForViewAngles;
  bool link_useBaseAnglesForViewClamp;
  float linkAnglesFrac;
  viewClampState link_viewClamp;
  EntHandle useHoldEntity;
  int32_t useHoldTime;
  int32_t useButtonDone;
  int32_t iLastCompassPlayerInfoEnt;
  int32_t compassPingTime;
  int32_t damageTime;
  float v_dmg_roll;
  float v_dmg_pitch;
  vec3_t swayViewAngles;
  vec3_t swayOffset;
  vec3_t swayAngles;
  vec3_t baseAngles;
  vec3_t baseOrigin;
  vec3_t recoilAngles;
  float fLastIdleFactor;
  int32_t weaponIdleTime;
  vec3_t recoilSpeed;
  int32_t previousRecoilTime;
  float previousRecoilRatio;
  int32_t lastServerTime;
  int32_t lastSpawnTime;
  weapon::Weapon lastWeapon;
  bool previouslyFiring;
  bool previouslyUsingNightVision;
  bool previouslyDTP;
  bool previouslyBeganWeaponRaise;
  bool previouslySprinting;
  uint8_t _padding171DD[3];
  uint32_t hasSpyplane;
  uint32_t hasSatellite;
  int32_t revive;
  int32_t reviveTime;
  int32_t disallowVehicleUsage;
  uint16_t attachShieldTagName;
  uint8_t _padding171F6[2];
  int32_t lastStand;
  int32_t lastStandTime;
};
typedef gclient_s gclient_t;
ASSERT_SIZE(gclient_s, 0x17200);

#pragma pack(pop)

#pragma pack(push, 1)
struct SpawnVar {
  qboolean spawnVarsValid;
  uint32_t numSpawnVars;
  char *spawnVars[100][2];
  uint32_t numSpawnVarChars;
  char spawnVarChars[2048];
  char spawnVarTypes[100];
};
ASSERT_SIZE(SpawnVar, 0xEB0);
#pragma pack(pop)

struct archivedEntity_t;

struct gentity_snd_wait {
  uint32_t notifyString;
  uint32_t index;
  bool stoppable;
  uint8_t _padding09[3];
  int32_t basetime;
  int32_t duration;
};

struct gclient_s;
static const uint32_t GENTITY_SIZE = 0x4F8;
static const uint32_t GENTITY_MODEL_OFFSET = 0x280;
static const uint32_t GENTITY_SND_WAIT_OFFSET = 0x3D4;

struct gentity_s;
typedef gentity_s gentity_t;

#pragma pack(push, 1)
struct flame_timed_damage_t {
  gentity_t *attacker;
  int32_t damage;
  float damageDuration;
  float damageInterval;
  int32_t start_timestamp;
  int32_t end_timestamp;
  int32_t lastupdate_timestamp;
};
ASSERT_SIZE(flame_timed_damage_t, 0x20);
#pragma pack(pop)

struct item_ent_t {
  uint32_t ammoCount;
  uint32_t clipAmmoCount;
  weapon::Weapon weapon;
};
ASSERT_SIZE(item_ent_t, 0x10);

#pragma pack(push, 1)
struct trigger_ent_t {
  int32_t threshold;
  int32_t accumulate;
  int32_t timestamp;
  int32_t singleUserEntIndex;
  uint8_t perk;
  uint8_t _padding11[3];
  int32_t exposureIndex;
  float exposureLerpToLighter;
  float exposureLerpToDarker;
  vec3_t exposureFeather;
  int16_t flags;
  uint8_t playerPrompts[4];
  bool requireAllPlayers;
  uint8_t _padding33[1];
};
ASSERT_SIZE(trigger_ent_t, 0x34);
#pragma pack(pop)

struct mover_slidedata_t {
  vec3_t mins;
  vec3_t maxs;
  vec3_t velocity;
};

struct mover_positions_t {
  float decelTime;
  float speed;
  float midTime;
  vec3_t pos1;
  vec3_t pos2;
  vec3_t pos3;
};
ASSERT_SIZE(mover_positions_t, 0x30);

struct mover_ent_t {
  union {
    mover_positions_t pos;
    mover_slidedata_t slide;
  };
  mover_positions_t angle;
};

struct corpse_ent_t {
  int32_t deathAnimStartTime;
};

#pragma pack(push, 1)
struct missile_ent_t {
  vec3_t predictLandPos;
  int32_t predictLandTime;
  int32_t timestamp;
  float time;
  int32_t timeOfBirth;
  float travelDist;
  vec3_t surfaceNormal;
  union {
    struct {
      uint8_t forcedDud : 1;
      uint8_t rolling : 1;
    };
    uint8_t _raw;
  };
  uint8_t flags;
  uint8_t _padding2E[2];
  int32_t antilagTimeOffset;
  int32_t timeAlive;
  union {
    struct {
      vec3_t curvature;
      vec3_t targetOffset;
      union {
        struct {
          MissileStage stage;
          MissileFlightMode flightMode;
        } missile;
        struct {
          float pitch;
          float yaw;
          float frequency;
        } grenade;
      };
    };

    int32_t effectIndex;
  };
  float grenadeWobbleCycle;
  float grenadeCurve;
  int32_t destructibleBounceCount;
};
ASSERT_SIZE(missile_ent_t, 0x68);
#pragma pack(pop)

struct blend_ent_t {
  vec3_t pos;
  vec3_t vel;
  vec4_t viewQuat;
  bool changed;
  float posAccelTime;
  float posDecelTime;
  float angleAccelTime;
  float angleDecelTime;
  float startTime;
  float posTotalTime;
  float angleTotalTime;
};

struct actor_ent_t {
  int32_t spawnTime;
};

struct spawner_ent_t {
  int32_t timestamp;
};

#pragma pack(push, 1)
struct zbarrier_piece_t {
  uint8_t state;
  uint8_t scalar;
  uint8_t nextState;
  uint8_t _padding03[1];
  int32_t startTime;
};
ASSERT_SIZE(zbarrier_piece_t, 0x8);
#pragma pack(pop)

struct zbarrier_ent_t {
  zbarrier_piece_t pieces[6];
};

#pragma pack(push, 1)
struct tagInfo_t {
  gentity_t *parent;
  gentity_t *next;
  scr::ScrString_t name;
  bool blendToParent;
  bool blendOnlyYaw;
  bool collisionPhysics;
  uint8_t _padding17[1];
  BoneIndex index;
  uint8_t _padding1A[2];
  vec3_t axis[4];
  vec3_t parentInvAxis[4];
  uint8_t _padding7C[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct EntityModelAttachment {
  modelNameIndex_t model;
  uint8_t _padding02[2];
  scr::ScrString_t tag;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct gentity_s {
  entityState_t s;
  entityShared_t r;
  gclient_t *client;
  user::actor_t *actor;
  ai::sentient_t *sentient;
  phys::Destructible *destructible;
  vehicle::vehicle_t *vehicle;
  user::bot_t *bot;
  modelNameIndex_t model;
  bool physicsObject;
  bool takedamage;
  bool active;
  bool nopickup;
  bool handler;
  bool avoidHandle;
  scr::ScrString_t classname;
  scr::ScrString_t target;
  scr::ScrString_t targetname;
  scr::ScrString_t script_noteworthy;
  scr::ScrString_t animname;
  scr::ScrString_t script_animname;
  uint32_t attachIgnoreCollision;
  int32_t spawnflags;
  int32_t flags;
  int32_t eventTime;
  qboolean freeAfterEvent;
  qboolean unlinkAfterEvent;
  contents_t clipmask;
  int32_t processedFrame;
  EntHandle parent;
  int32_t nextthink;
  int32_t health;
  int32_t maxHealth;
  int32_t damage;
  uint8_t _padding2D4[4];
  flame_timed_damage_t flame_timed_damage[4];
  int32_t last_timed_radius_damage;
  int32_t count;
  gentity_t *activator;
  union {
    item_ent_t item[2];
    trigger_ent_t trigger;
    mover_ent_t mover;
    corpse_ent_t corpse;
    missile_ent_t missile;
    blend_ent_t blend;
    actor_ent_t actorInfo;
    spawner_ent_t spawner;
    zbarrier_ent_t zbarrier;
  };
  union {
    EntHandle missileTargetEnt;
    EntHandle grenadeOriginalOwner;
  };
  gentity_snd_wait snd_wait;
  tagInfo_t *tagInfo;
  gentity_t *tagChildren;
  anim::animscripted_t *scripted;
  EntityModelAttachment attachments[19];
  anim::XAnimTree *pAnimTree;
  uint16_t disconnectedLinks;
  uint8_t _padding4A2[2];
  int32_t iDisconnectTime;
  phys::PhysObjId physObjId;
  gentity_t *nextFree;
  int32_t birthTime;
  int32_t ikPlayerclipTerrainTime;
  int32_t ikDisableTerrainMappingTime;
  uint32_t shieldAttachmentIndex;
  weapon::DelayedWeaponFire delayedFire;
  HavokObj_t *havokObj;
  PosedEntity_PoseHandle instancePoseHandle;
  int32_t teleportTime;
  int32_t debugRenderTime;
  char tmodeTimeOut;
  uint8_t _padding4F1[7];
};
#pragma pack(pop)

constexpr uint32_t GENTITY_POOL_LEN = 2048;
struct gentity_pool {
  gentity_t pool[GENTITY_POOL_LEN];
};

#ifdef __cplusplus
ASSERT_OFFSET(gentity_s, model, GENTITY_MODEL_OFFSET);
ASSERT_OFFSET(gentity_s, classname, 0x288);
ASSERT_OFFSET(gentity_s, snd_wait, GENTITY_SND_WAIT_OFFSET);
ASSERT_OFFSET(gentity_s, s, 0);
ASSERT_OFFSET(gentity_s, client, 0x250);
ASSERT_SIZE(gentity_s, GENTITY_SIZE);
#endif

#pragma pack(push, 16)
// level_locals_t has size 0x23A10 on both client and server
struct level_locals_t {
  gclient_s *clients;
  gentity_t *gentities;
  int32_t gentitySize;
  int32_t num_entities;
  gentity_t *firstFreeEnt;
  gentity_t *lastFreeEnt;
  gentity_t *firstFreeFakeEnt;
  gentity_t *lastFreeFakeEnt;
  int32_t numFakeEntities;
  uint8_t _padding3C[4];
  vehicle::vehicle_t *vehicles;
  gentity_t *firstFreeActor;
  gentity_t *lastFreeActor;
  int32_t num_actors;
  uint8_t _padding5C[4];
  ai::sentient_t *sentients;
  user::actor_t *actors;
  int32_t actorCorpseCount;
  uint8_t _padding74[4];
  user::bot_t *bots;
  int32_t maxBots;
  bool initializing;
  bool clientIsSpawning;
  uint8_t _padding86[2];
  int32_t maxclients;
  lobby::LobbyClientPool<int32_t> teamSortedClients;
  uint8_t _paddingD4[12];
  objective_t objectives[128];
  int32_t objectivesClientMask[128][2];
  int32_t framenum;
  int32_t time;
  int32_t previousTime;
  int32_t frametime;
  int32_t startTime;
  uint8_t _padding64F4[4];
  SpawnVar spawnVar;
  EntHandle droppedWeaponCue[32];
  qboolean savepersist;
  float fFogOpaqueDist;
  float fFogOpaqueDistSqrd;
  bool bPlayerIgnoreRadiusDamage;
  bool bPlayerIgnoreRadiusDamageLatched;
  uint8_t _padding7436[2];
  int32_t currentEntityThink;
  qboolean bRegisterLeaderboards;
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
  uint8_t _padding9101[1];
  modelNameIndex_t modelMap[8192];
  uint8_t _paddingD102[2];
  loading_t loading;
  int32_t script_ai_limit;
  uint32_t grenadeHintCount;
  uint8_t specialIndex[1024];
  phys::actor_prone_info_t cgData_actorProneInfo[96];
  uint8_t cgData_actorOnCompass[64];
  uint8_t cgData_actorTeam[64];
  bool bDrawCompassFriendlies;
  bool disable_grenade_suicide;
  uint8_t _paddingDE92[2];
  int32_t numConnectedClients;
  lobby::LobbyClientPool<ClientNum_t> sortedClients;
  qboolean bUpdateScoresForIntermission;
  int32_t numVotingClients;
  uint8_t _paddingDEE8[8];
  MatchState matchState;
  CasterState casterState;
  CasterClientState casterClients;
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
  uint8_t _paddingEB9C[4];
  phys::sv_FxVisBlock_t fxVisibilityEnts[32];
  int32_t currentPlayerClone;
  qboolean hostMigrationActive;
  int32_t hostMigrationStart;
  phys::actorAntilagFrame_t actorAntilagFrames[20];
  int32_t nextActorAntilagFrame;
  vehicle::vehicleAntilagFrame_t vehicleAntilagFrames[20];
  int32_t nextVehicleAntilagFrame;
  uint8_t _padding1F254[4];
  phys::ZBarrierType *zbarrierTypes[255];
  qboolean checkAnimChange;
  phys::objcamCameraTable objectiveCameras;
  uint8_t _unknown[0x7AC];
};
ASSERT_SIZE(level_locals_t, 0x23A10);
#pragma pack(pop)

} // namespace level
} // namespace game