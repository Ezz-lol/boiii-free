#pragma once

#include "core.hpp"
#include "phys.hpp"
#include "quake/core.hpp"
#include "net/net.hpp"
#include "scr/core.hpp"
#include "scr/scr.hpp"
#include "snd/snd.hpp"
#include "asm.hpp"
#include "lobby/core.hpp"

#include <cstdint>

namespace game {

namespace user {
struct actor_t;
struct actorState_t;
struct cachedActor_t;
} // namespace user

namespace level {
struct MatchState;
struct clientState_s;
typedef clientState_s clientState_t;

struct playerState_s;
typedef playerState_s playerState_t;

struct entityState_s;
typedef entityState_s entityState_t;
struct archivedEntity_t;

struct gentity_s;
typedef gentity_s gentity_t;
} // namespace level

namespace sv {

enum class ReliableCommand : char {
  NOP = '\0',             // 0x00: Empty command / return
  GIVE_ACHIEVEMENT = '#', // 0x23: LiveAchievements_GiveAchievement
  BLUR_SERVER_CMD = '(',  // 0x28: CG_BlurServerCommand
  // 0x29: CG_TranslateHudElemMessage / CG_BoldGameMessage
  ANNOUNCEMENT_MSG = ')',
  CHAT_MSG = '+',                  // 0x2B: CG_ChatMessage
  NITROUS_VEHICLE_TELEPPORT = '/', // 0x2F: NitrousVehicle::Teleport
  SET_CLIENT_SYSTEM_STATE = '0',   // 0x30: CG_ParseClientSystemStateChange
  CHECKPOINT_COMMIT = '1',         // 0x31: CL_Checkpoint_Commit
  CONFIG_STRING_MODIFIED = '2',    // 0x32: CG_ConfigStringModified
  DYN_ENT_DESTROY_EVENT = '7',     // 0x37: DynEntCl_DestroyEvent
  EXPLODER = ':',                  // 0x3A: CG_ParseExploderCommand
  GAME_MSG = ';',                  // 0x3B: CG_GameMessage
  BOLD_GAME_MSG_CENTER = '<',      // 0x3C: CG_BoldGameMessageCenter
  CHECKPOINT_SAVE = '=',           // 0x3D: CL_Checkpoint_Save
  RESET_WEAPON_STATE = '>',        // 0x3E: PM_ResetWeaponState
  CLOSE_IN_GAME_MENU = '@',        // 0x40: UI_CloseInGameMenu
  LOCAL_SOUND = 'B',               // 0x42: LocalSound
  LOCAL_SOUND_STOP = 'C',          // 0x43: LocalSoundStop
  LUI_NOTIFY = 'D',                // 0x44: CG_ParseLUINotify
  RADIANT_EXPLODER = 'E',          // 0x45: CG_ParseRadiantExploderCommand
  MAP_RESTART = 'J',               // 0x4A: CG_MapRestart(..., 0)
  HIT_MARKER = 'M',                // 0x4D: HitMarker
  OPEN_SCRIPT_MENU = 'N',          // 0x4E: CG_OpenScriptMenu
  // 0x4F: LiveTracker_WriteForAllLocalUsers / CG_GameMessage
  TRACKER_GAME_MSG = 'O',
  MAP_RESTART_FAST = 'U',       // 0x55: CG_MapRestart(..., 1)
  AIRSUPPORT = 'V',             // 0x56: CG_ParseAirsupport
  BURN = 'W',                   // 0x57: CG_BurnServerCommand
  SCR_CAMERA = 'X',             // 0x58: CG_ScrCamera
  ELECTRIFIED = 'Y',            // 0x59: CG_ElectrifiedServerCommand
  SET_EQUIPPED_OFF_HAND = '\\', // 0x5C: CG_SetEquippedOffHand
  SET_CLIENT_DVAR = '^',        // 0x5E: CG_SetClientDvarFromServer
  // // 0x61: cgameGlob->hideViewModel = 0;
  SHOW_VIEW_MODEL = 'a',
  // 0x63: R_Stream_ProcessHintEntity (or CG_SpawnPrediction)
  PROCESS_HINT_ENTITY = 'c',
  START_FADING_BLUR = 'd', // 0x64: CG_StartFadingBlurServerCommand
  // 0x65: LiveStats_SetStatChanged / LiveStats_SetStatChangedNoCache
  SET_STAT_CHANGED = 'e',
  SET_CHECK_SUM = 'f',  // 0x66: LiveStats_SetCheckSumFromServer
  SELECT_WEAPON = 'h',  // 0x68: CG_SwitchToLatestPrimary / CG_SelectWeaponIndex
  SET_TEAM_SCORE = 'i', // 0x69: CG_SetTeamScore
  /*
    0x6A: Falls through to '+'; (CG_ChatMessage).
    May be handled differently in CG_ChatMessage (?).
    Unsure currently why there are two commands for chat messages.
  */
  CHAT_MSG_ALT = 'j',
  GET_USER_SPONSOR = 'm',        // 0x6D: Live_GetUserSponsor
  UPLOAD_STATS = 'n',            // 0x6E: LiveStats_CompareStatsVsStableBuffer /
                                 // LiveStorage_UploadStatsForController
  VISION_SET_LERP_TO = 'o',      // 0x6F: CG_VisionSetStartLerp_To
  WATER_DROPS = 'p',             // 0x70: CG_WaterDropsServerCommand
  SET_WORLD_FOG_BANK = 'r',      // 0x72: CG_SetWorldFogActiveBank
  UPDATE_FOV = 's',              // 0x73: CG_UpdateFov
  TEAM_OPS = 't',                // 0x74: CG_TeamOpsSetID / Progress / ShowHUD
  CHECKPOINT_RESTORE = 'u',      // 0x75: CL_Checkpoint_Restore
  FRIEND_CHAT_MSG = 'v',         // 0x76: CG_FriendChatMessage
  UPDATE_ZOMBIE_DOUBLE_XP = 'w', // 0x77: Live_DoubleXPUpdateZombieDoubleXP
  INC_LOOT_CURRENCY = 'x',       // 0x78: Loot_IncCurrency (Vials or MP Loot XP)
  CONSUME_INVENTORY_ITEM = 'y',  // 0x79: LiveInventory_ConsumeItem
};

struct client_s {
  int32_t state;
  char __pad0[0x28];
  net::netadr_t address;
  char __pad1[20468];
  int32_t reliableSequence;
  int32_t reliableAcknowledge;
  char __pad2[4];
  int32_t messageAcknowledge;
  char gap_5040[1368];
  level::gentity_t *gentity;
  playerName_t name;
  clanAbbrev_t clanAbbrev;
  XUID xuid;
  char __pad3[0xB5D84];
  int32_t guid;
  char __pad4[0x8];
  bool bIsTestClient;
  char __pad5[3];
  int32_t serverId;
  char __pad6[171432];
};
typedef client_s client_t;

#ifdef __cplusplus
ASSERT_SIZE(client_s, 0xE5110);

ASSERT_OFFSET(client_s, address, 0x2C);
ASSERT_OFFSET(client_s, xuid, 0x55C8);
ASSERT_OFFSET(client_s, guid, 0xBB354);
ASSERT_OFFSET(client_s, bIsTestClient, 0xBB360);
ASSERT_OFFSET(client_s, name, 0x55A0);
ASSERT_OFFSET(client_s, gentity, 0x5598);
ASSERT_OFFSET(client_s, clanAbbrev, 0x55C0);
#endif

struct client_s_cl : client_s {
  char __pad1_0[0x60];
};
typedef client_s_cl client_t_cl;

#ifdef __cplusplus
ASSERT_SIZE(client_s_cl, 0xE5170);
#endif

struct svEntity_t {
  uint16_t worldSector;
  uint16_t nextEntityInWorldSector;
  int32_t linkcontents;
  vec3_t linkmin;
  vec3_t linkmax;
};
ASSERT_SIZE(svEntity_t, 0x20);

struct CasterClientState {
  int32_t clientIndex;
  uint32_t gadgetPlayerStateFlags;
  float gadgetPowerRemaining;
  lobby::LobbyClientPool<int32_t> lastUpdateIndex;
};
// As per G_GetCasterClientSize();
ASSERT_SIZE(CasterClientState, 0x54);

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

ASSERT_SIZE(cachedSnapshot_t, 0x38);

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
  level::clientState_t *snapshotClients;
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
  level::clientState_t *firstClientState;
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
ASSERT_SIZE(serverStaticHeader_t, 0x1B0);
#pragma pack(pop)

// TODO
struct archivedEntity_s;
typedef archivedEntity_s archivedEntity_t;

#pragma pack(push, 1)
struct serverSnapshot_t {
  lobby::LobbyClientPool<level::playerState_t *> ps;
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
ASSERT_SIZE(serverSnapshot_t, 0xB8);
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
ASSERT_SIZE(tempBanSlot_t, 16);
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
ASSERT_SIZE(rateBoostingStats_t, 0x24);

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
  level::clientState_t *snapshotClients;
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

    The compiler would not be likely to optimize this way unless this field was
    padding.
  */
  uint8_t _padding12D0[16];
  tempBanSlot_t tempBans[16];
  lobby::LobbyClientPool<XUID> xuids;
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
ASSERT_SIZE(serverStatic_t, 0x1500);
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
  lobby::LobbyClientPool<flashbackState_t> clientsFlashbackStates;
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
ASSERT_SIZE(recentFrame, 0x1);

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
ASSERT_SIZE(playerAnimState_t, 0x10);
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
ASSERT_SIZE(AntilagPlayerAnimState, 0x14);
#pragma pack(pop)

struct clientControllers_t {
  vec3_t angles[6];
  vec3_t tag_origin_angles;
  vec3_t tag_origin_offset;
};
ASSERT_SIZE(clientControllers_t, 0x60);

struct clientsPositionArchive_t {
  int32_t time;
  lobby::LobbyClientPool<bool> valid;
  lobby::LobbyClientPool<uint8_t> flags;
  lobby::LobbyClientPool<vec3_t> positions;
  lobby::LobbyClientPool<vec3_t> angles;
  lobby::LobbyClientPool<vec3_t> viewangles;
  lobby::LobbyClientPool<playerAnimState_t> anim;
  lobby::LobbyClientPool<int32_t> lerpFlags;
  lobby::LobbyClientPool<int32_t> lerpFlags2;
  lobby::LobbyClientPool<int32_t> locBlendTime;
  AntilagPlayerAnimState animStates[200];
  lobby::LobbyClientPool<int32_t> animStatesPos;
  lobby::LobbyClientPool<int32_t> animStatesCount;
  int32_t animStatesUsed;
  lobby::LobbyClientPool<clientControllers_t> controllers;
};
ASSERT_SIZE(clientsPositionArchive_t, 0x1B9C);

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
  // Each field XORed with client XUID to generate client gamestate security
  // checksum
  scr::scrChecksum_t securityChecksum;
  uint8_t _padding7BEFC[4];
};
ASSERT_SIZE(server_t, 0xBC5C0);
#pragma pack(pop)

enum class SwimStateType : int32_t {
  SWIM_STATE_NONE = 0x0,
  SWIM_STATE_SURFACE = 0x1,
  SWIM_STATE_UNDERWATER = 0x2,
  NUM_SWIM_STATES = 0x3,
};
enum class ClientVisibleBodyMode : int32_t {
  CLIENT_VISIBLEBODYMODE_UNKNOWN = 0x0,
  CLIENT_VISIBLEBODYMODE_FULL = 0x1,
  CLIENT_VISIBLEBODYMODE_FIRST_PERSON_CINEMATIC = 0x2,
  CLIENT_VISIBLEBODYMODE_FIRST_PERSON_LEGS = 0x3,
};

struct score_t {
  int32_t ping;
  int32_t status_icon;
  int32_t place;
  int32_t score;
  int32_t kills;
  int32_t assists;
  int32_t deaths;
  int32_t scoreboardColumns[10];
  int32_t downs;
  int32_t revives;
  int32_t headshots;
  int32_t scoreMultiplier;
  int32_t currentStreak;
  int32_t lastKillTime;
};

enum class ClientFallSpeed : int32_t {
  FALL_SPEED_NONE = 0x0,
  FALL_SPEED_DEFAULT = 0x0,
  FALL_SPEED_SLOW = 0x1,
  FALL_SPEED_MED = 0x2,
  FALL_SPEED_FAST = 0x3,
  FALL_SPEED_RAPID = 0x4,
  FALL_SPEED_COUNT = 0x5,
};

struct PlayerSlideType {
  int32_t time;
  int32_t speed;
};

struct PlayerSlide {
  PlayerSlideType types[3];
};

struct PlayerArmorTiers {
  int32_t armorBarCount;
  int32_t armorBarMaxCount;
};

struct PlayerArmorInfo {
  PlayerArmorTiers tiers[3];
  float armorRegenDelay;
  float armorRegenTime;
  int32_t armorPerBar;
};

struct PMoveDef {
  const char *name;
  int32_t wallRun_maxTimeMs;
  float wallRun_speedScale;
  float wallRun_speedScaleADS;
  int32_t wallRun_fallStageTimeMs;
  int32_t slide_friction_duration_ms;
  float slide_min_continue_velocity;
  float slide_outSpeedScale;
  bool slide_outAllowSprint;
  float doubleJump_maxUpwardsVelocity;
  bool playerEnergy_slideEnergyEnabled;
  float sprint_maxSpeed;
  float sprint_minSpeed;
  float sprint_maxTime;
  float sprint_minTime;
  float sprint_slowdownTime;
  float sprint_recoveryTime;
  float sprint_recoveryDelay;
  float sprint_recoveryExtraDelay;
  float sprint_slowAnimRate;
  float sprint_boostSpeedScale;
};

struct CustomizationColorInfo {
  const char *name;
  XString displayName;
  gfx::GfxColor uiColor;
  gfx::GfxImageHandle icon;
  uint32_t numCamoMaterials;
  weapon::CamoMaterial *camoMaterials;
};

struct ColorOptionSet {
  uint32_t numOptions;
  CustomizationColorInfo **colors;
};

struct CharacterItemInfo {
  const char *assetName;
  const char *displayName;
  gfx::GfxImageHandle icon;
  db::xasset::XModelPtr models[11];
  uint32_t accentColorCount;
  ColorOptionSet *colorOptions;
  int32_t impactType[2];
  scr::ScriptBundlePtr gibDef;
  db::xasset::PlayerFXTablePtr characterMovementFx;
  phys::RagdollDef *ragdollDef;
  bool hideHead;
  uint32_t kvpCount;
  scr::ScriptBundleKVP *kvpItems;
};

enum class CharacterGenderTypes : int32_t {
  CHARACTER_GENDER_TYPE_MALE = 0x0,
  CHARACTER_GENDER_TYPE_MP = 0x0,
  CHARACTER_GENDER_TYPE_FIRST = 0x0,
  CHARACTER_GENDER_TYPE_FEMALE = 0x1,
  CHARACTER_GENDER_TYPE_COUNT = 0x2,
  CHARACTER_GENDER_TYPE_INVALID = 0x2,
  CHARACTER_GENDER_TYPE_ANY = 0x3,
};

struct CharacterBodyType {
  const char *name;
  const char *displayName;
  const char *description;
  const char *heroWeapon;
  const char *heroAbility;
  XString bodySoundContext;
  scr::ScriptBundlePtr mpDialog;
  XString chrName;
  gfx::GfxImageHandle rewardIcon;
  XString background;
  XString backgroundWithCharacter;
  XString lockedImage;
  XString personalizeRender;
  XString frozenMomentRender;
  XString frozenMomentOverlay;
  gfx::GfxImageHandle defaultHeroRender;
  gfx::GfxImageHandle defaultHeroRenderAbility;
  gfx::GfxImageHandle positionDraftPortrait;
  gfx::GfxImageHandle positionDraftIcon;
  XString equippedLoadoutIcons[2];
  XString unequippedLoadoutIcons[2];
  gfx::GfxImageHandle zombiePlayerIcon;
  uint32_t numBodyInfos;
  CharacterItemInfo *bodyInfo;
  uint32_t numHelmetInfos;
  CharacterItemInfo *helmetInfo;
  CharacterGenderTypes gender;
  bool disabled;
  XString frontendVignetteStruct;
  db::xasset::XCamPtr frontendVignetteXCam;
  db::xasset::XAnimPartsPtr frontendVignetteXAnim;
  db::xasset::XModelPtr frontendVignetteWeaponModel;
  uint32_t kvpCount;
  scr::ScriptBundleKVP *kvpItems;
  db::xasset::PlayerSoundsTablePtr characterMovementSounds;
  db::xasset::PlayerFXTablePtr characterMovementFx;
  db::xasset::FootstepTableDefPtr characterFootsteps;
  db::xasset::FootstepTableDefPtr characterFootstepsQuiet;
  db::xasset::FootstepTableDefPtr characterFootstepsNPC;
  db::xasset::FootstepTableDefPtr characterFootstepsNPCLoud;
  db::xasset::FootstepTableDefPtr characterFootstepsNPCQuiet;
  db::xasset::XModelPtr dogtagFriendly;
  db::xasset::XModelPtr dogTagEnemy;
  XString cardBackIcon;
  XString realName;
  int32_t age;
  XString genderString;
  XString bio;
  XString weaponCardBackIcon;
  XString weaponCardBackSubIcon;
  XString weaponCardBackDesc;
  XString weaponSubItemDesc;
  XString weaponSchema;
  XString abilityCardBackIcon;
  XString abilityCardBackSubIcon;
  XString abilityCardBackDesc;
  XString abilitySubItemDesc;
  XString abilitySchema;
};

struct PlayerRoleCategory {
  XString name;
  XString displayName;
  XString description;
  gfx::GfxImageHandle icon;
  int32_t sortOrder;
  uint32_t kvpCount;
  scr::ScriptBundleKVP *kvpItems;
};

struct __attribute__((aligned(8))) PlayerRoleDefaultItem {
  XString assetName;
  weapon::eAttachment *attachmentOptions;
  union {
    uint32_t attachmentOptionsCount;
    uint32_t grenadeCount;
  };
};

enum class PlayerRoleButtonOverrideTypes : int32_t {
  PRBO_NONE = 0x0,
  PRBO_FRAG = 0x1,
  PRBO_SMOKE = 0x2,
  PRBO_SPECIAL_OFFHAND = 0x3,
  PRBO_WEAPON_SWITCH_HERO = 0x4,
  PRBO_COUNT = 0x5,
};

struct PlayerRoleInventoryItemOption {
  XString assetName;
  PlayerRoleButtonOverrideTypes buttonOverride;
  uint32_t unlockIngameLevel;
  uint32_t cost;
  uint32_t grenadeCount;
};

struct PlayerRoleInventoryItem {
  uint32_t inventoryItemOptionCount;
  PlayerRoleInventoryItemOption *inventoryItemOptions;
};

struct PlayerEnergyRate {
  float useRate;
  float rechargeRate;
  int32_t rechargeDelay;
};

struct PlayerEnergy {
  bool enabled;
  bool affectedByEmp;
  float energyMax;
  float energyMin;
  float costRatioMin;
  PlayerEnergyRate rate;
  PlayerEnergyRate rateFast;
  PlayerEnergyRate rateEmp;
  float useRateFast;
  float useRateEmp;
  int32_t rechargeDelayFast;
  int32_t rechargeDelayEmp;
};

struct __attribute__((aligned(2))) PlayerRoleTemplate {
  XString name;
  CharacterBodyType *bodyType;
  PlayerRoleCategory *roleCategory;
  PMoveDef *pmoveDef;
  uint32_t defaultLoadoutCount;
  PlayerRoleDefaultItem defaultLoadoutItems[12];
  PlayerRoleInventoryItem inventoryItems[8];
  uint32_t kvpCount;
  scr::ScriptBundleKVP *kvpItems;
  int32_t customClassIndex;
  float moveSpeedScale;
  float adsSpeedScale;
  float sprintSpeedScale;
  float minimapVisionRange;
  PlayerSlide slide;
  PlayerArmorInfo armorInfo;
  int32_t spawnHealth;
  float healthRegenDelay;
  float healthRegenTime;
  PlayerEnergy doubleJumpEnergy;
  float resistances[8];
  float playerTunableFloats[14];
  int32_t playerTunableInts[1];
  bool playerTunableBools[3];
};

enum class team_t : int32_t {
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

struct SonarAttachmentInfo {
  vec3_t prevOrigin;
  float movementDistance[60];
  int32_t nextMovementDistanceIndex;
};

struct animScriptCondition_t {
  int32_t index;
  uint32_t value;
};

enum class animBodyPart_t : int32_t {
  ANIM_BP_UNUSED = 0x0,
  ANIM_BP_LEGS = 0x1,
  ANIM_BP_TORSO = 0x2,
  ANIM_BP_BOTH = 0x3,
  ANIM_BP_TORSO_2 = 0x4,
  ANIM_BP_TORSO_FULL = 0x5,
  ANIM_BP_MOTION_MATCHING = 0x6,
  NUM_ANIM_BODYPARTS = 0x7,
};

struct __attribute__((aligned(4))) animScriptCommand_t {
  snd::SndAliasList *soundAlias;
  scr::ScrString_t tagName;
  animBodyPart_t bodyPart[2];
  int16_t animIndex[2];
  uint16_t animDuration[2];
  union {
    uint8_t flags;
    struct {
      uint8_t motionMatchingEnabled : 1;
    };
  };
};

// sizeof=0x130
struct animScriptItem_t {
  int32_t numConditions;
  animScriptCondition_t conditions[5];
  int32_t numCommands;
  animScriptCommand_t commands[8];
};
ASSERT_SIZE(animScriptItem_t, 0x130);

#pragma pack(push, 1)
struct visionPulse_t {
  qboolean active;
  int32_t startTime;
  int32_t endTime;
  float radius;
  int32_t maxRadius;
  float pulseSpeed;
  weapon::gadgetPulseShareType_e revealShareType;
  weapon::gadgetPulseShareType_e orbShareType;
  vec3_t origin;
  qboolean friendly;
  int32_t ownerNum;
  int32_t pulseFadeOut;
  weapon::Weapon gadgetWeapon;
  int32_t numberEntsHit;
  snd::SndAliasId pulseSound;
  snd::SndAliasId pulseMissSound;
  uint8_t _padding4C[4];
};
#pragma pack(pop)

struct CasterHUDInfo {
  uint32_t gadgetPlayerStateFlags;
  float gadgetPowerRemaining;
};

/*
  Almost entirely unverified as of initial definition, except for
  those fields with a comment indicating otherwise.

  Verify and adjust as needed prior to usage.
*/
#pragma pack(push, 1)
struct clientInfo_t {
  qboolean infoValid;
  qboolean nextValid;
  ClientNum_t clientNum;
  playerName_t name; // Verified
  team_t team;
  team_t oldteam;
  qboolean clanAbbrev_IsEliteValidated;
  const PlayerRoleTemplate *role;
  uint8_t roleIndex;
  uint8_t level;
  uint16_t paintshopDataSize;
  int32_t arenaPoints;
  int32_t rank;
  int32_t paragonRank;
  int32_t paragonIconId;
  union {
    int32_t prestige;
    int32_t lastDaysPlayed;
  };
  qboolean needsRevive;
  uint32_t perks[4];
  uint32_t perksCache[4];
  uint8_t _unknown7C[4];
  clanAbbrev_t clanAbbrev; // Verified
  uint64_t xuid;
  score_t score;
  uint8_t _paddingEC[4];
  db::xasset::MaterialHandle hStatusIcon;
  gfx::GfxImage *hRankIcon;
  int32_t location;
  int32_t clientInfoHealth;
  int32_t clientInfoArmor;
  int32_t clientInfoArmorTier;
  int32_t clientInfoMaxArmor;
  int32_t damageState;
  char model[64];
  anim::BGModelAttachmentName attachmentNames[6];
  uint32_t modelIndex;
  anim::ModelAttachmentIndex attachments[6];
  ClientVisibleBodyMode bodyMode;
  anim::LerpFrameAngles legs;
  anim::LerpFrameAngles torso;
  anim::LerpFrameAnimation anim[3];
  anim::locBlendInfo_t locBlendInfo;
  anim::motionMatchingInfo_t motionMatchingInfo;
  float lerpMoveDir;
  float lerpLean;
  ClientFallSpeed priorFallSpeed;
  int32_t lastMoveSpeedTime;
  float moveSpeed;
  vec3_t velocity;
  vec3_t priorVelocity;
  vec3_t oldAnimPos;
  vec3_t oldOrigin;
  vec3_t playerAngles;
  vec3_t playerAnglesLastRotationLoop;
  vec3_t playerAnglesLastRotationStep;
  float lastYaw;
  float yawTurnSpeed;
  int32_t lastYawTurnUpdate;
  scr::ScrString_t animLeftHandTag;
  scr::ScrString_t animRightHandTag;
  qboolean leftHandGun;
  int32_t disableGunAlignTime;
  qboolean dobjDirty;
  clientControllers_t control;
  int32_t lastDamageTime;
  int32_t lastStandStartTime;
  int32_t lastSpawnTime;
  int32_t turnedHumanTime;
  uint8_t _padding664[4];
  int64_t moveType;
  int64_t eventType;
  ClientNum_t beingRevivedByClient;
  int32_t reviveTimeStart;
  uint64_t clientPMFlags;
  uint64_t prevClientPMFlags;
  int32_t prevEFlags2;
  bool wasWallRunFalling;
  bool wallRunningRight;
  uint8_t _padding696[2];
  uint32_t clientConditions[21];
  uint8_t _padding6EC[4];
  weapon::Weapon primaryWeapon;
  db::xasset::XAnimTree *pXAnimTree;
  weapon::Weapon DObjWeapon;
  weapon::AttachmentCosmeticVariantIndexes DObjWeaponACVI;
  uint8_t _padding70C[4];
  weapon::Weapon DObjMeleeWeapon;
  weapon::Weapon DObjOffhandWeapon;
  weapon::Weapon DObjStowedWeapon;
  int32_t DObjWeaponHealthLevel;
  int32_t DObjStowedHealthLevel;
  bool showReloadClipInHand;
  bool hideWeaponClipModel;
  uint8_t _padding732[2];
  int32_t stanceTransitionTime;
  int32_t turnAnimEndTime;
  char turnAnimType;
  uint8_t _padding73D[3];
  int32_t attachedEntNum;
  int32_t attachedVehSeat;
  qboolean hideWeapon;
  uint32_t weaponHideBits[12];
  bool usingKnife;
  bool usingGrenade;
  uint8_t _padding77E[2];
  int32_t nextHideWeaponTime;
  int32_t weaponHealth;
  SwimStateType previousSwimState;
  SwimStateType swimState;
  vec3_t vehAnimLastOrg;
  vec3_t vehAnimLastAngle;
  int32_t vehAnimLastBone;
  int32_t vehAnimLastTime;
  int32_t vehAnimLerpStartTime;
  int32_t vehAnimLerpEndTime;
  float weaponSpinLerp;
  float lastWeaponSpinLerp;
  float weaponSpin;
  qboolean wasFiring[5];
  qboolean fireEventsThisSnapshot;
  qboolean fireEventsLeftHandThisSnapshot;
  db::xasset::FxUniqueHandle jetFXHandles[4];
  db::xasset::FxUniqueHandle jetEMPFXHandles[4];
  bool wasJumping;
  bool wasDoubleJumpBoostActive;
  bool doubleJumpShakeRequiresReset;
  bool shouldPlayWallrunStartSound;
  bool wallRunUsingEnergy;
  bool wallRunUsingEnergyPrior;
  bool wallRunStartTime;
  uint8_t _padding807[1];
  db::xasset::FxUniqueHandle wallRunFXHandle;
  db::xasset::FxUniqueHandle underWaterFXHandle;
  db::xasset::FxUniqueHandle slideFXHandle;
  int32_t slideJetFXTimer;
  qboolean shouldSlideFX;
  snd::SndAliasId slideSound;
  db::xasset::TagFxSetHandles gadgetLoopFXHandleSet;
  db::xasset::FxUniqueHandle worldPersistentEffectHandle[2];
  db::xasset::TagFxSetHandles worldPersistentEffectSetHandles[2];
  int32_t clientUIVisibilityFlags;
  int32_t teleportTime;
  qboolean teleportBit;
  int32_t surfaceType;
  int32_t footstepEvent;
  int32_t footstepEventCount;
  SonarAttachmentInfo sonarAttachmentInfo;
  float infraredFadeOut;
  uint8_t _padding9D4[4];
  visionPulse_t revealPulse;
  int32_t cleanseTime;
  weapon::entityGadgetState_t priorEntityGadgetState[4];
  weapon::gadgetType_e previousHeldGadgetType;
  weapon::WeaponDobjInfo weaponDobjInfo;
  qboolean offhandWeaponVisible;
  scr::ScrString_t animScriptedNotify;
  float locBlendTime;
  float animScriptLerpTime;
  animScriptItem_t filteredScriptItem;
  int32_t meleeEntityImpactType;
  uint8_t _paddingEC4[4];
  weapon::Weapon lastWeapon;
  uint8_t lastLevel;
  uint8_t _paddingED1[3];
  CasterHUDInfo casterHUDInfo;
  uint8_t _paddingEDC[4];
};
ASSERT_OFFSET(clientInfo_t, _padding73D, 0x73D);
ASSERT_OFFSET(clientInfo_t, _padding9D4, 0x9D4);
ASSERT_OFFSET(clientInfo_t, _paddingEC4, 0xEC4);
ASSERT_OFFSET(clientInfo_t, _paddingED1, 0xED1);
ASSERT_OFFSET(clientInfo_t, _padding807, 0x807);
ASSERT_OFFSET(clientInfo_t, name, 0x0C);
ASSERT_OFFSET(clientInfo_t, clanAbbrev, 0x80);
ASSERT_SIZE(clientInfo_t, 0xEE0);
#pragma pack(pop)

#pragma pack(push, 1)
struct ucmd_t {
  const char *name;
  fastcall_t<void(client_t *cl)> func;
  qboolean fromOldServer;
  uint8_t _padding14[4];
};
ASSERT_SIZE(ucmd_t, 0x18);
#pragma pack(pop)

} // namespace sv
} // namespace game