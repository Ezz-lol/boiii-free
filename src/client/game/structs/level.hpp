#pragma once

#include "core.hpp"
#include "snd.hpp"
#include "vehicle.hpp"
#include "user.hpp"
#include "phys.hpp"
#include "ai.hpp"
#include "scr/scr.hpp"
#include "ui/ui.hpp"

namespace game {

namespace level {

typedef uint64_t EventParm_t;

#pragma pack(push, 1)
struct clientLinkInfo_t {
  int16_t parentEnt;
  uint8_t tagIndex;
  uint8_t flags;
};
#pragma pack(pop)

struct LerpEntityStateTurret {
  union vec3_t gunAngles;
  float heatVal;
  int32_t overheating;
  float pivotOffset;
  int32_t flags;
};
struct LerpEntityStateLoopFx {
  float cullDist;
  int32_t period;
};
struct LerpEntityStateActor {
  union {
    int32_t actorNum;
    int32_t corpseNum;
  } index;
  int32_t clonedFromActorNum;
  int32_t species;
  int32_t enemy;
  uint8_t freeCameraLockOnAllowed;
  uint8_t missingLegs;
  uint8_t aiType;
  uint8_t vehicleSeat;
  uint8_t ikPriority;
  uint8_t isAiClone;
  struct {
    int16_t fBodyPitch;
  } proneInfo;
  anim::ModelAttachmentIndex attachments[6];
  uint16_t tmodeHealth;
  uint8_t tmodeBehavior;
  uint8_t tmodeVisibility;
};
struct LerpEntityStatePrimaryLight {
  uint8_t colorAndExp[4];
  float intensity;
  float radius;
  float cosHalfFovOuter;
  uint32_t mixerIndex;
};
struct LerpEntityStateLensflare {
  float intensity;
  char name[64];
};
enum class PlayerCorpseDeathCamera : uint8_t {
  NORMAL = 0x0,
  FIXED_ORIGIN = 0x1,
  MAX = 0x2,
};

#pragma pack(push, 1)
struct LerpEntityStatePlayer {
  weapon::Weapon primaryWeapon;
  weapon::Weapon stowedWeapon;
  union {
    weapon::Weapon offhandWeapon;
    weapon::Weapon killedByWeapon;
  };
  weapon::Weapon meleeWeapon;
  weapon::entityGadgetState_t entityGadgetState[3];
  float leanf;
  int16_t movementDir;
  uint8_t moveType;
  uint8_t velYaw;
  uint8_t velSpeed;
  union {
    uint8_t locBlendTime;
    uint8_t killedByTeam;
  };
  union {
    uint8_t meleeWeaponCamo;
    PlayerCorpseDeathCamera deathCamera;
  };
  uint8_t vehicleType;
  uint8_t vehicleAnimBoneIndex;
  uint8_t vehicleSeat;
  uint8_t stowedWeaponCamo;
  uint8_t weaponHeat;
  uint8_t proneLegsAngle;
  uint8_t _padding61[7];
};
#pragma pack(pop)

struct LerpEntityStateVehicleGunnerAngles {
  int16_t pitch;
  int16_t yaw;
  int16_t targetEnt;
};

#pragma pack(push, 1)
struct LerpEntityStateVehicle {
  float steerYaw;
  float bodyRoll;
  LerpEntityStateVehicleGunnerAngles gunnerAngles[4];
  LerpEntityStateVehicleGunnerAngles turret;
  union {
    int16_t throttle;
    int16_t bodyPitch;
  };
  int16_t clonedFromEntityNum;
  uint8_t targetRotorSpeed;
  uint8_t tmodeVehicleHealth;
  uint8_t tmodeVehicleBehavior;
  uint8_t tmodeVehicleVisibility;
  uint8_t _padding2E[2];
};
#pragma pack(pop)

enum class hitLocation_t : uint32_t {
  NONE = 0x0,
  HELMET = 0x1,
  HEAD = 0x2,
  NECK = 0x3,
  TORSO_UPR = 0x4,
  TORSO_MID = 0x5,
  TORSO_LWR = 0x6,
  R_ARM_UPR = 0x7,
  L_ARM_UPR = 0x8,
  R_ARM_LWR = 0x9,
  L_ARM_LWR = 0xA,
  R_HAND = 0xB,
  L_HAND = 0xC,
  R_LEG_UPR = 0xD,
  L_LEG_UPR = 0xE,
  R_LEG_LWR = 0xF,
  L_LEG_LWR = 0x10,
  R_FOOT = 0x11,
  L_FOOT = 0x12,
  GUN = 0x13,
  SHIELD = 0x14,
  COUNT = 0x15,
};

struct LerpEntityStateMissile {
  int32_t launchTime;
  int32_t parentClientNum;
  int32_t fuseTime;
  int32_t forcedDud;
  float autoDetonateTime;
  vec3_t passThrough;
  int32_t targetEntnum;
  hitLocation_t hitLocation;
};
struct LerpEntityStateScriptMover {
  uint8_t attachTagIndex[4];
  int16_t attachModelIndex[4];
  int16_t exploderIndex;
  uint16_t scale;
  uint8_t flags;
  uint8_t aiType;
};
struct LerpEntityStateBulletHit {
  vec3_t start;
};
struct LerpEntityStateEarthquake {
  float radius;
  int32_t duration;
  int16_t scalePitch;
  int16_t scaleYaw;
  int16_t scaleRoll;
  int16_t freqPitch;
  int16_t freqYaw;
  int16_t freqRoll;
  float durationFadeUp;
  float durationFadeDown;
};
struct LerpEntityStateCustomExplode {
  int32_t startTime;
  int32_t effectIndex;
};
struct LerpEntityStateExplosion {
  float innerRadius;
  float magnitude;
  float innerDamage;
  int32_t dummy;
  float outerDamage;
};
struct LerpEntityStateExplosionJolt {
  float innerRadius;
  vec3_t impulse;
};
struct LerpEntityStateJetThrust {
  vec3_t thrustDir;
  int32_t dummy;
  float dotLimit;
};
struct LerpEntityStateStreamerHint {
  float factor;
  uint16_t flags;
  int16_t lightStateIndex;
};
struct LerpEntityStateZBarrierPiece {
  uint8_t flags;
  uint8_t animTime;
};

struct LerpEntityStateZBarrier {
  uint32_t barrierTypeIndex;
  LerpEntityStateZBarrierPiece pieces[6];
};
struct LerpEntityStateAnonymous {
  int32_t data[26];
};

#pragma pack(push, 1)
struct playerAnimState_t {
  float fTorsoPitch;
  float fWaistPitch;
  animationNumber_t animNum[3];
  union {
    uint8_t flags;
    struct {
      uint8_t motionMatchingEnabled : 1;
      uint8_t mmTransitionBoost : 1;
      uint8_t unused_slots : 6;
    };
  };
  uint8_t _padding0F[1];
};

#pragma pack(push, 1)
struct vehicleState_t {
  int32_t flags;
  int16_t animId;
  int16_t attachModelIndex[2];
  uint8_t attachTagIndex[2];
  uint8_t vehicleDefIndex;
  uint8_t _padding0D[3];
};
#pragma pack(pop)

struct hardlineHint_t {
  uint8_t team;
  uint8_t perk;
  uint8_t hint;
};

#pragma pack(pop)

#pragma pack(push, 1)
struct scriptMoverState_t {
  uint8_t fov;
  uint8_t _padding01[1];
  int16_t animId;
};
#pragma pack(pop)

struct fxLightingState_t {
  float primaryLightFraction;
  int32_t lightingOriginOffset;
};

constexpr uint32_t ENTITYSTATE_SIZE = 0x1f0;
constexpr uint32_t ENTITYSTATE_LOOPSOUND_OFFSET = 0xDC;
constexpr uint32_t ENTITYSTATE_UN3_OFFSET = 0x130;
constexpr uint32_t ENTITYSTATE_CLIENTMASK_OFFSET = 0x168;
constexpr uint32_t ENTITYSTATE_OTHERENTITYNUM_OFFSET = 0x1AE;
#pragma pack(push, 1)

union entityStateUn2 {
  playerAnimState_t anim;
  vehicleState_t vehicleState;
  hardlineHint_t hardline;
  scriptMoverState_t moverState;
  fxLightingState_t fxState;
};

union entityStateUn3 {
  int32_t hintString;
  int32_t vehicleXModel;
  uint32_t secondBcAlias;
  uint32_t soundTag;
};

union entityStateIndex {
  uint16_t brushmodel;
  uint16_t triggerModel;
  uint16_t xmodel;
  uint16_t primaryLight;
  uint16_t probe;
  BoneIndex bone;
};

union entityStateUn1 {
  uint8_t scale;
  uint8_t eventParm2;
  uint8_t helicopterStage;
  uint8_t destructibleid;
  uint8_t chainMeleeCounter;
};

#pragma pack(push, 1)
struct trajectory_t {
  uint8_t trType;
  uint8_t _padding01[3];
  int32_t trTime;
  int32_t trDuration;
  union vec3_t trBase;
  union vec3_t trDelta;
};
#pragma pack(pop)

union LerpEntityStateTypeUnion {
  LerpEntityStateTurret turret;
  LerpEntityStateLoopFx loopFx;
  LerpEntityStateActor actor;
  LerpEntityStatePrimaryLight primaryLight;
  LerpEntityStateLensflare lensFlareDef;
  LerpEntityStatePlayer player;
  LerpEntityStateVehicle vehicle;
  LerpEntityStateMissile missile;
  LerpEntityStateScriptMover scriptMover;
  LerpEntityStateBulletHit bulletHit;
  LerpEntityStateEarthquake earthquake;
  LerpEntityStateCustomExplode customExplode;
  LerpEntityStateExplosion explosion;
  LerpEntityStateExplosionJolt explosionJolt;
  LerpEntityStateJetThrust jetThrust;
  LerpEntityStateStreamerHint streamerHint;
  LerpEntityStateZBarrier zbarrier;
  LerpEntityStateAnonymous anonymous;
};

#pragma pack(push, 1)
struct LerpEntityState {
  int32_t eFlags;
  int32_t eFlags2;
  trajectory_t pos;
  trajectory_t apos;
  LerpEntityStateTypeUnion u;
  int16_t useCount;
  uint8_t _paddingBA[2];
  uint32_t clientFields[4];
  uint8_t _paddingCC[4];
};
#pragma pack(pop)

enum class entityType_t : uint32_t {
  GENERAL = 0x0,
  PLAYER = 0x1,
  PLAYER_CORPSE = 0x2,
  ITEM = 0x3,
  MISSILE = 0x4,
  PLAYER_INVISIBLE = 0x5,
  SCRIPTMOVER = 0x6,
  SOUND_BLEND = 0x7,
  FX = 0x8,
  LOOP_FX = 0x9,
  PRIMARY_LIGHT = 0xA,
  LENSFLARE = 0xB,
  REFLECTION_PROBE = 0xC,
  HELICOPTER = 0xD,
  PLANE = 0xE,
  VEHICLE = 0xF,
  VEHICLE_SPAWNER = 0x10,
  VEHICLE_CORPSE = 0x11,
  ACTOR = 0x12,
  ACTOR_SPAWNER = 0x13,
  ACTOR_CORPSE = 0x14,
  STREAMER_HINT = 0x15,
  ZBARRIER = 0x16,
  TRIGGER = 0x17,
  EVENTS = 0x18,
};

struct entityState_s {
  int32_t number;
  uint8_t _padding04[4];
  LerpEntityState lerp;
  int32_t time2;
  snd::LoopSoundInfo loopSound;
  int32_t solid;
  renderOptions_t renderOptions;
  weapon::AttachmentCosmeticVariantIndexes attachmentCosmeticVariantIndexes;
  anim::animState_t animState;
  entityStateUn2 un2;
  entityStateUn3 un3;
  uint32_t partBits[12];
  clientLinkInfo_t clientLinkInfo;
  int32_t clientMask[1];
  uint8_t teamMask;
  uint8_t events[4];
  uint8_t _padding171[7];
  EventParm_t eventParms[4];
  EventParm_t eventParm;
  entityType_t eType;
  team_t team;
  int16_t owner;
  int16_t groundEntityNum;
  entityStateIndex index;
  int16_t otherEntityNum;
  int16_t attackerEntityNum;
  int16_t enemyModel;
  uint8_t _padding1B4[4];
  weapon::Weapon weapon;
  weapon::Weapon lastStandPrevWeapon;
  scr::ScrString_t targetname;
  int16_t spawnVarIndex;
  uint8_t animtreeIndex;
  uint8_t _padding1CF[1];
  anim::AnimScriptedState animScripted;
  int16_t eventSequence;
  uint8_t surfType;
  uint8_t _padding1E7[1];
  int16_t clientNum;
  uint8_t iHeadIcon;
  entityStateUn1 un1;
  uint8_t _padding1EC[4];
};
typedef entityState_s entityState_t;
#pragma pack(pop)
static_assert(sizeof(entityState_t) == ENTITYSTATE_SIZE,
              "entityState_t size must be 0x1F0 bytes");
static_assert(offsetof(entityState_t, number) == 0,
              "entityState_t::number must be at offset 0");
static_assert(offsetof(entityState_t, loopSound) ==
                  ENTITYSTATE_LOOPSOUND_OFFSET,
              "entityState_t::loopSound must be at offset 0xDC");
static_assert(offsetof(entityState_t, clientMask) ==
                  ENTITYSTATE_CLIENTMASK_OFFSET,
              "entityState_t::clientMask must be at offset 0x168");
static_assert(offsetof(entityState_t, otherEntityNum) ==
                  ENTITYSTATE_OTHERENTITYNUM_OFFSET,
              "entityState_t::otherEntityNum must be at offset 0x1AE");

#pragma pack(push, 1)
struct entityShared_t {
  bool linked;
  bool bmodel;
  int16_t svFlags;
  bool inuse;
  uint8_t _padding05[3];
  int32_t broadcastTime;
  vec3_t mins;
  vec3_t maxs;
  contents_t contents;
  vec3_t absmin;
  vec3_t absmax;
  vec3_t currentOrigin;
  vec3_t currentAngles;
  EntHandle ownerNum;
  int32_t eventTime;
};
ASSERT_SIZE(entityShared_t, 0x60);
#pragma pack(pop)

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
ASSERT_SIZE(CasterState, 0x8);

// sizeof=0x54
struct CasterClientState {
  int32_t clientIndex;
  uint32_t gadgetPlayerStateFlags;
  float gadgetPowerRemaining;
  int32_t lastUpdateIndex[18];
};
ASSERT_SIZE(CasterClientState, 0x54);

struct ArchivedMatchState {
  int32_t matchUIVisibilityFlags;
  int32_t bombTimer[2];
  int32_t roundsPlayed;
  int32_t worldFields[64];
  int32_t worldUIModelFields[16];
  int32_t activeSkiptos;
  int32_t hiddentMiscModelGroups;
  int32_t umbraGates;
};

// sizeof=0x290
struct MatchState {
  uint32_t index;
  uint8_t _padding004[12];
  UnarchivedMatchState unarchivedState;
  ArchivedMatchState archivedState;
  uint8_t _padding28C[4];
};
ASSERT_SIZE(MatchState, 0x290);

enum class loading_t : uint32_t {
  LOADING_DONE = 0x0,
  LOADING_LEVEL = 0x1,
  LOADING_SAVEGAME = 0x2,
};
// sizeof=0x38
struct objcamCameraPoint {
  phys::objcamState ocState;
  scr::ScrString_t ocName;
  int32_t ocIndex;
  int32_t ocEntNum;
  scr::ScrString_t ocEntTag;
  team_t ocTeam;
  vec3_t ocPos;
  vec3_t ocAngles;
  float ocMaxHorizAngle;
  float ocMaxVertAngle;
};
ASSERT_SIZE(objcamCameraPoint, 0x38);

// sizeof=0x3800
struct objcamCameraTable {
  objcamCameraPoint cameraPoints[256];
};
ASSERT_SIZE(objcamCameraTable, 0x3800);
#pragma pack(pop)

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

enum class EViewAngleEaseMode : uint32_t {
  LINEAR = 0x0,
  CUBIC = 0x1,
  QUADRATIC = 0x2,
  SINUSOIDAL = 0x3,
};

struct playerViewAngleState_s {
  int32_t flags;
  int32_t time;
  int32_t startTimeMs;
  int32_t durationMs;
  EViewAngleEaseMode easeMode;
  vec2_t startAngles;
  vec2_t goalAngles;
};
ASSERT_SIZE(playerViewAngleState_s, 0x24);
typedef playerViewAngleState_s playerViewAngleState_t;

enum class PlayerLensState : int32_t {
  LENS_STATE_DEFAULT = 0x0,
  LENS_STATE_WALK = 0x1,
  LENS_STATE_SPRINT = 0x2,
  LENS_STATE_SWIM = 0x3,
  LENS_STATE_SWIMSPRINT = 0x4,
  LENS_STATE_SLIDE = 0x5,
  LENS_STATE_MAX = 0x6,
};

struct PlayerLens {
  PlayerLensState lensState;
  float focalLength;
  float focalDistance;
  float fStop;
};
ASSERT_SIZE(PlayerLens, 0x10);

#pragma pack(push, 1)
struct PlayerVehicleState {
  vec3_t origin;
  vec3_t angles;
  vec3_t velocity;
  vec3_t angVelocity;
  vec2_t tilt;
  vec2_t tiltVelocity;
  float targetHeightDelta;
  float lastGroundHeight;
  int32_t entity;
  int32_t flags;
  qboolean fullPhysics;
  int32_t focusEntNum;
  int32_t focusEntTime;
  vec2_t focusAngleOffset;
  uint32_t playerHeightMapsActive;
  float boostTimeLeft;
  vec3_t dogfightDesiredLocation;
  vec3_t jitterEndTime;
  vec3_t jitterAccel;
  int32_t drivableDuration;
  int32_t drivableEndTime;
  qboolean weaponOverheating;
  float weaponHeatPercent;
  float weaponLockonPercent;
  float damageMeter;
  int32_t lockedOnByEntNum;
  int32_t weaponWaitDuration;
  int32_t weaponWaitEndTime;
};
ASSERT_SIZE(PlayerVehicleState, 0xB4);
#pragma pack(pop)

enum class OffhandSecondaryClass : int32_t {
  SMOKE = 0x0,
  FLASH = 0x1,
  COUNT = 0x2,
};

enum class OffhandPrimaryClass : int32_t {
  FRAG = 0x0,
  GEAR = 0x1,
  COUNT = 0x2,
};

enum class ViewHeightState : int32_t {
  PRONE = 0x0,
  CROUCH = 0x1,
  STAND = 0x2,
  SLIDE = 0x3,
  SWIM = 0x4,
  LASTSTAND = 0x5,
  ASSASSINATION = 0x6,
  DEAD = 0x7,
  SPECTATOR = 0x8,
  COUNT = 0x9,
};

enum class ViewLockTypes : uint32_t {
  NONE = 0x0,
  FULL = 0x1,
  WEAPONJITTER = 0x2,
  COUNT = 0x3,
};

enum class locSel_t : uint32_t {
  NONE = 0x0,
  ARTILLERY = 0x1,
  AIRSTRIKE = 0x2,
  MORTAR = 0x3,
  NAPALM = 0x4,
  COMLINK = 0x5,
};

#pragma pack(push, 1)
struct SprintState {
  qboolean sprintButtonUpRequired;
  qboolean sprintRestore;
  int32_t sprintRestoreDelayStart;
  qboolean sprintDelay;
  int32_t lastSprintStart;
  int32_t lastSprintEnd;
  int32_t sprintStartMaxLength;
  uint8_t dir;
  uint8_t _padding1D[3];
  int32_t sprintDuration;
  int32_t sprintCooldown;
};
ASSERT_SIZE(SprintState, 0x28);
#pragma pack(pop)

enum class trmAnim_t : int32_t {
  ROOT = 0x0,
  STEP_F = 0x1,
  STEP_B = 0x2,
  STEP_L = 0x3,
  STEP_R = 0x4,
  ON_LOW_FORWARD_32 = 0x5,
  ON_LOW_BACK_32 = 0x6,
  ON_LOW_LEFT_32 = 0x7,
  ON_LOW_RIGHT_32 = 0x8,
  ON_MED_FORWARD_60 = 0x9,
  ON_MED_LEFT_50 = 0xA,
  ON_MED_RIGHT_50 = 0xB,
  OVER_LOW_FORWARD_VAULT = 0xC,
  OVER_LOW_FORWARD = 0xD,
  OVER_LOW_BACK = 0xE,
  OVER_LOW_LEFT = 0xF,
  OVER_LOW_RIGHT = 0x10,
  OVER_HIGH_75 = 0x11,
  OVER_HIGH_100 = 0x12,
  OVER_HIGH_140 = 0x13,
  ON_HIGH_100 = 0x14,
  ON_HIGH_140 = 0x15,
  ANIM_COUNT = 0x16,
  FIRST_OVER_ANIM = 0xC,
  LAST_OVER_ANIM = 0x13,
  FIRST_NONCOMBAT_ANIM = 0x12,
};

#pragma pack(push, 1)
struct TRMState {
  float yaw;
  float ledgeYaw;
  int32_t timer;
  int32_t flags;
  trmAnim_t animIndex;
  vec3_t moveVec;
  float animTimeScale;
  vec2_t restoreAngles;
  int32_t doublejumpEndedTime;
};
ASSERT_SIZE(TRMState, 0x30);
#pragma pack(pop)

#pragma pack(push, 1)
struct GrappleState {
  vec3_t startPos;
  int32_t startTime;
};
ASSERT_SIZE(GrappleState, 0x10);
struct JukeState {
  int32_t startTime;
  float jukeYaw;
  int32_t flags;
};
ASSERT_SIZE(JukeState, 0xC);
struct SlamState {
  int32_t startTime;
  float slamYaw;
  int32_t flags;
};
ASSERT_SIZE(SlamState, 0xC);

enum class SlideType : uint32_t {
  BOOST = 0x0,
  REDUCED = 0x1,
  BASE = 0x2,
  COUNT = 0x3,
};

struct SlideState {
  vec3_t slideAngleOffsets;
  int32_t flags;
  int32_t slideTime;
  int32_t slideOutDuration;
  int32_t subsequentCount;
  SlideType slideType;
};
ASSERT_SIZE(SlideState, 0x20);
#pragma pack(pop)

enum class DoubleJumpStateType : int32_t {
  OFF = 0x0,
  FIRST_UP_BOOST = 0x1,
  UP_BOOST = 0x2,
  IN_AIR = 0x3,
  COUNT = 0x4,
};
enum class DoubleJumpBoostUpDirection : int32_t {
  NONE = 0x0,
  LEFT = 0x1,
  RIGHT = 0x2,
  FORWARD = 0x3,
  BACKWARD = 0x4,
  COUNT = 0x5,
};

enum class gadgetButtonInput_e : int32_t {
  NO_CHANGE = 0x0,
  PRESSED = 0x1,
  HELD = 0x2,
  RELEASED = 0x3,
  COUNT = 0x4,
};

struct playerGadgetState_t {
  uint32_t gadgetPlayerStateFlags;
  int32_t gadgetLastUsedTime;
  int32_t gadgetFlickerTime;
  int32_t gadgetEntNum;
  int32_t gadgetPressTime;
  int32_t gadgetActivateTime;
  float gadgetPowerRemaining;
  gadgetButtonInput_e gadgetButtonState;
};
ASSERT_SIZE(playerGadgetState_t, 0x20);

#pragma pack(push, 1)
struct WallRunState {
  int32_t startTimeMs;
  int32_t endTimeMs;
  int16_t flags;
  uint8_t _padding0A[2];
  vec3_t normal;
  float startHeight;
  float verticality;
  float rollFrac;
  int32_t timeLastInputReceived;
  int32_t lastShotMs;
};
ASSERT_SIZE(WallRunState, 0x2C);
#pragma pack(pop)

struct LeapState {
  int32_t volumeTimer;
  int32_t state;
};
ASSERT_SIZE(LeapState, 0x8);

struct AnimMovement {
  vec3_t startOrigin;
  vec3_t originErrorNormal;
  int32_t timer;
  float originErrorDistance;
  float startYaw;
  float errorYaw;
  float timeScale;
  int32_t targetFlags;
  int32_t flags;
  float lerpTime;
};
ASSERT_SIZE(AnimMovement, 0x38);

struct playerStateCameraInfo {
  int32_t xcamIndex;
  int32_t xcamStartTime;
  int32_t xcamLerpEndTime;
  int32_t xcamSubCamera;
  vec3_t xcamBundleOrigin;
  vec4_t xcamBundleAnglesQuat;
};
ASSERT_SIZE(playerStateCameraInfo, 0x2C);

struct PlayerWeaponLock {
  int32_t lockFlags;
  int32_t targetEntNum;
  int32_t subtarget;
};
ASSERT_SIZE(PlayerWeaponLock, 0xC);

enum class ActionSlotType : int32_t {
  DONOTHING = 0x0,
  SPECIFYWEAPON = 0x1,
  ALTWEAPONTOGGLE = 0x2,
  BGB = 0x3,
  COUNT = 0x4,
};

struct ActionSlotParam {
  struct SpecifyWeapon {
    weapon::Weapon weapon;
  };

  union {
    SpecifyWeapon specifyWeapon;
  };
};
ASSERT_SIZE(ActionSlotParam, 0x8);

#pragma pack(push, 1)
struct PlayerRenderInfo {
  uint8_t lightingState;
  uint8_t _padding01[3];
  int32_t sunShadowSplitDistance;
};
ASSERT_SIZE(PlayerRenderInfo, 0x8);
#pragma pack(pop)

struct clientCmdAngles_t {
  int32_t time;
  int16_t pitch;
  int16_t yaw;
};
ASSERT_SIZE(clientCmdAngles_t, 0x8);

// Almost entirely unverified as of initial definition.
// Verify and adjust as needed prior to usage.
#pragma pack(push, 1)
struct playerState_s {
  ClientNum_t clientNum;
  int32_t commandTime;
  int32_t pm_type;
  int32_t bobCycle;
  uint64_t pm_flags;
  uint64_t weapFlags;
  int32_t otherFlags;
  int32_t pm_time;
  snd::LoopSoundInfo loopSound;
  uint8_t roleIndex;
  uint8_t level;
  uint8_t _padding32[2];
  vec3_t origin;
  vec3_t velocity;
  int32_t remoteEyesEnt;
  int32_t remoteEyesTagname;
  weapon::PlayerWeaponHandState handState[2];
  int32_t movementState;
  int32_t movementTime;
  int32_t weaponIdleTime;
  int32_t grenadeTimeLeft;
  int32_t throwBackGrenadeOwner;
  int32_t throwBackGrenadeTimeLeft;
  int32_t weaponRestrictKickTime;
  bool mountAvailable;
  bool bThirdPerson;
  uint8_t _paddingAA[2];
  int32_t foliageSoundTime;
  int32_t gravity;
  float leanf;
  int32_t speed;
  vec3_t delta_angles;
  playerViewAngleState_t viewAngleState;
  int32_t groundEntityNum;
  int32_t moverEntityNum;
  int32_t moverTimestamp;
  int32_t groundType;
  int32_t jumpTime;
  float jumpOriginZ;
  int32_t moveType;
  anim::PlayerAnimBodyPartState playerAnims[3];
  anim::AnimScriptedState animScripted;
  int32_t damageTimer;
  int32_t damageDuration;
  int32_t dmgDirection;
  int32_t dmgType;
  int32_t corpseIndex;
  int32_t movementDir;
  int32_t eFlags;
  int32_t eFlags2;
  PlayerLens playerLens;
  PlayerVehicleState vehicleState;
  int16_t predictableEventSequence;
  int16_t predictableEventSequenceOld;
  int32_t predictableEvents[4];
  EventParm_t predictableEventParms[4];
  int16_t unpredictableEventSequence;
  int16_t unpredictableEventSequenceOld;
  int32_t unpredictableEvents[4];
  uint8_t _padding26C[4];
  EventParm_t unpredictableEventParms[4];
  weapon::Weapon offHandWeapon;
  OffhandSecondaryClass offhandSecondary;
  OffhandPrimaryClass offhandPrimary;
  renderOptions_t renderOptions;
  weapon::AttachmentCosmeticVariantIndexes attachmentCosmeticVariantIndexes;
  int32_t momentum;
  weapon::Weapon weapon;
  weapon::Weapon lastWeaponAltModeSwitch;
  weapon::Weapon stowedWeapon;
  uint8_t unusedCompatibilityPadding;
  uint8_t _padding2D1[7];
  weapon::Weapon meleeWeapon;
  weapon::GadgetTargetResult gadgetTargetResult;
  float fWeaponPosFrac;
  float fMoveSpeedTransitionLerp;
  float fIdleSpeedFromFireTransitionLerp;
  int32_t adsDelayTime;
  int32_t spreadOverride;
  int32_t spreadOverrideState;
  float weaponSpinLerp;
  vec3_t viewangles;
  float viewHeightCurrent;
  int32_t viewHeightLerpTime;
  int32_t viewHeightLerpTarget;
  bool viewHeightLerpDown;
  uint8_t _padding319[3];
  ViewHeightState viewHeightState;
  ViewHeightState viewHeightStatePrev;
  vec2_t viewBobFactor;
  vec2_t viewAngleClampBase;
  vec2_t viewAngleClampRange;
  int32_t damageEvent;
  int32_t damageYaw;
  int32_t damagePitch;
  int32_t damageProjYaw;
  int32_t damageCount;
  int32_t stats[4];
  weapon::PlayerHeldWeapon heldWeapons[15];
  weapon::AmmoPool ammoNotInClip[15];
  weapon::AmmoClip ammoInClip[15];
  float proneDirection;
  float proneDirectionPitch;
  float proneTorsoPitch;
  ViewLockTypes viewlocked;
  int16_t viewlocked_entNum;
  uint8_t _padding6BA[2];
  int32_t vehiclePos;
  int32_t vehicleType;
  int32_t vehicleAnimBoneIndex;
  int32_t linkFlags;
  vec3_t linkAngles;
  vec3_t groundTiltAngles;
  int32_t cursorHint;
  uint32_t cursorHintString;
  int32_t cursorHintEntIndex;
  weapon::Weapon cursorHintWeapon;
  int32_t interactivePromptEntIndex;
  int32_t iCompassPlayerInfo;
  uint32_t spyplaneTypeEnabled;
  uint32_t satelliteTypeEnabled;
  int32_t spyplaneTime;
  int32_t satelliteTime;
  int32_t locationSelectionInfo;
  locSel_t locationSelectionType;
  SprintState sprintState;
  float fTorsoPitch;
  float fWaistPitch;
  float holdBreathScale;
  int32_t holdBreathTimer;
  int32_t chargeShotTimer;
  uint32_t chargeShotLevel;
  uint32_t shotsFiredFromChamber;
  float quickScopeScale;
  int32_t quickScopeTimer;
  uint32_t privateClientFields[8];
  uint32_t clientFields[8];
  uint32_t uiModelFields[8];
  float moveSpeedScaleMultiplier;
  TRMState trmState;
  GrappleState grappleState;
  int32_t grappledByEntNum;
  SlideState slideState;
  JukeState jukeState;
  SlamState slamState;
  int32_t impactTime;
  int32_t impactIntensity;
  DoubleJumpStateType doubleJumpState;
  DoubleJumpBoostUpDirection doubleJumpBoostUpDirection;
  float doubleJumpPitchOffset;
  float doubleJumpRollOffset;
  float playerEnergyRatio;
  int32_t playerEnergyTime;
  playerGadgetState_t playerGadgetState[4];
  WallRunState wallRunState;
  float playerSwimAnimRate;
  LeapState leapState;
  AnimMovement animMovement;
  int32_t vehicleAnimStage;
  int32_t vehicleEntryPoint;
  int32_t meleeChargeDist;
  int32_t meleeChargeTime;
  int32_t meleeChargeEnt;
  vec4_t meleeConstraintPlane;
  int32_t meleeStartTime;
  int32_t meleeServerResult;
  int32_t meleeFlags;
  int32_t meleeState;
  int32_t blockMeleeUsageTimer;
  int32_t meleeAttacker;
  int32_t meleeCounterTimer;
  int32_t chainMeleeCounter;
  playerStateCameraInfo mainCam;
  playerStateCameraInfo extraCam[4];
  PlayerWeaponLock weapLock[5];
  int32_t weapLockedPivotEntnum;
  int32_t weapLockTimer;
  int32_t lockOnRequiredMsgSuppressAfterFireTimer;
  uint32_t perks[4];
  uint32_t perksCache[4];
  uint8_t bonuscards[3];
  ActionSlotType actionSlotType[4];
  ActionSlotParam actionSlotParam[4];
  weapon::Weapon inventoryWeapon;
  weapon::Weapon inventoryHeldWeapon;
  uint8_t vehicleDefIndex;
  uint8_t _paddingB21[3];
  int32_t locBlendTime;
  PlayerRenderInfo renderInfo;
  bool motionMatchingEnabled;
  bool mmTransitionBoost;
  uint8_t _paddingB32[2];
  int32_t kf_index;
  qboolean attemptedDoubleJump;
  int32_t entityEventSequence;
  float aimSpreadScale;
  uint32_t statusEffectFlags;
  int32_t shellshockIndex;
  int32_t shellshockTime;
  int32_t shellshockDuration;
  int32_t meleeButtStartTime;
  uint8_t meleeButtDirection;
  uint8_t _paddingB58[3];
  float dofNearStart;
  float dofNearEnd;
  float dofFarStart;
  float dofFarEnd;
  float dofNearBlur;
  float dofFarBlur;
  float dofViewmodelStart;
  float dofViewmodelEnd;
  int32_t waterlevel;
  qboolean walkUnderwater;
  int32_t lastFootInWaterTime;
  int32_t lastOutWaterTime;
  int32_t smokeColorIndex;
  int32_t hudElemLastAssignedSoundID;
  int32_t adsZoomSelect;
  int32_t adsZoomLatchTime;
  qboolean adsZoomLatchState;
  int32_t adsPrevZoomSelect;
  int32_t adsPrevZoomSelectTime;
  int64_t artilleryInboundIconLocation;
  float visionSetLerpRatio;
  bool poisoned;
  bool binoculars;
  uint8_t _paddingBB5[2];
  uint32_t scriptCursorHintString;
  uint8_t _paddingBBB[4];
  objective_t objective[128];
  int32_t deltaTime;
  int32_t killCamEntity;
  int32_t killCamTargetEntity;
  uint8_t _padding6BCB[4];
  weapon::Weapon killCamWeapon;
  int32_t killCamMod;
  int32_t killCamFlag;
  int32_t introShotsFired;
  struct {
    ui::hud::hudelem_t current[31];
    ui::hud::hudelem_t archival[31];
  } hud;
  uint8_t _padding89EC[4];
  ui::hud::serverHudMenu_t hudMenus[32];
  struct {
    UIModelData modelData[8];
    int32_t modelNames[8];
  } serverModels;
  clientCmdAngles_t cmdAngles[20];
  int32_t cmdAnglesHead;
  uint8_t _padding[51];
};
static_assert(offsetof(playerState_s, _paddingBB5) == 0xBB5);
static_assert(offsetof(playerState_s, _paddingBBB) == 0xBBB);
static_assert(offsetof(playerState_s, _paddingB58) == 0xB58);
static_assert(offsetof(playerState_s, _padding6BCB) == 0x6BCB);
ASSERT_SIZE(playerState_s, 0xB566);
typedef playerState_s playerState_t;
#pragma pack(pop)

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

enum class sessionState_t : int32_t {
  PLAYING = 0x0,
  DEAD = 0x1,
  SPECTATOR = 0x2,
  INTERMISSION = 0x3,
};

/*
  Almost entirely unverified as of initial definition, except for
  those fields with a comment indicating otherwise.

  Verify and adjust as needed prior to usage.
*/
#pragma pack(push, 1)
struct clientState_s {
  ClientNum_t clientIndex;
  int32_t scriptClientId;
  team_t team;
  int32_t modelindex;
  int32_t riotShieldNext;
  anim::ModelAttachmentIndex attachments[6];
  char name[PLAYER_NAME_MAX_LEN]; // Verified
  float maxSprintTimeMultiplier;
  int32_t rank;
  int32_t paragonRank;
  int32_t paragonIconId;
  union {
    int32_t prestige;
    int32_t lastDaysPlayed;
  };
  int32_t lastDamageTime;
  int32_t lastStandStartTime;
  int32_t lastSpawnTime;
  int32_t turnedHumanTime;
  ClientNum_t beingRevivedByClient;
  int32_t reviveTimeStart;
  int32_t damageState;
  uint8_t _padding7C[4];
  uint64_t csPMFlags;
  union {
    XUID xuid;
    netUInt64 xuid64;
  };
  int32_t arenaPoints;
  uint32_t perks[4];
  uint8_t roleIndex;
  uint8_t _paddingA5[3];
  int32_t level;
  uint8_t _paddingAC[4];
  uint64_t bodyOptions;
  weapon::visLoadOut_s cacLoadouts[10];
  int32_t heroWeaponIdx;
  int32_t voiceConnectivityBits;
  uint8_t _unknown250[12];
  char clanAbbrev[PLAYER_CLAN_ABBREV_MAX_LEN]; // Verified
  uint8_t _padding264[4];
  weapon::Weapon currentLoadoutPrimaryWeapon;
  weapon::Weapon currentLoadoutSecondaryWeapon;
  SwimStateType csSwimState;
  int32_t weaponHealth;
  int32_t attachedEntNum;
  int32_t attachedVehSeat;
  bool needsRevive;
  bool clanAbbrevEV;
  uint8_t _padding28A[2];
  vehicle::VehicleAnimState vehAnimState;
  score_t score;
  int32_t clientUIVisibilityFlags;
  qboolean offhandWeaponVisible;
  clientLinkInfo_t clientLinkInfo;
  int32_t lastUpdateIndex[18];
  uint16_t paintshopDataSize;
  uint8_t _padding342[2];
  int32_t clientHealth;
  int32_t clientArmor;
  int32_t clientMaxArmor;
  int32_t clientArmorTier;
  int32_t currentWeaponKillCount;
};
typedef clientState_s clientState_t;
static_assert(offsetof(clientState_t, name) == 0x2C);
static_assert(offsetof(clientState_t, clanAbbrev) == 0x25C);
ASSERT_SIZE(clientState_t, 0x358);
#pragma pack(pop)

enum clientConnected_t : uint32_t {
  CON_DISCONNECTED = 0x0,
  CON_CONNECTING = 0x1,
  CON_CONNECTED = 0x2,
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
  scr::ScrVarIndex_t scriptPersId;
  clientConnected_t connected;
  user::usercmd_t cmd;
  user::usercmd_t oldcmd;
  bool localClient;
  bool predictItemPickup;
  char newnetname[32];
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
  int32_t scoreboardColumnCache[42];
  uint8_t _padding53C[4];
};
static_assert(offsetof(clientSession_t, localClient) == 0xF4);
static_assert(offsetof(clientSession_t, _padding14) == 0x14);
static_assert(offsetof(clientSession_t, _padding116) == 0x116);
static_assert(offsetof(clientSession_t, cs) == 0x138);
ASSERT_SIZE(clientSession_t, 0x540);
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
static_assert(sizeof(gclient_s) == 0x17200, "sizeof(gclient_s) != 0x17200");

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
  qboolean stoppable;
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
          enum MissileStage stage;
          enum MissileFlightMode flightMode;
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

struct HavokObj_t;

typedef uintptr_t PosedEntity_PoseHandle;

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

constexpr uint32_t GENTITY_POOL_COUNT = 2048;
struct gentity_pool {
  gentity_t pool[GENTITY_POOL_COUNT];
};

#ifdef __cplusplus
static_assert(offsetof(gentity_s, model) == GENTITY_MODEL_OFFSET,
              "offset of gentity_s::model must be 0x280");
static_assert(offsetof(gentity_s, classname) == 0x288,
              "offset of gentity_s::classname must be 0x288");
static_assert(offsetof(gentity_s, snd_wait) == GENTITY_SND_WAIT_OFFSET,
              "GENTITY_SND_WAIT_OFFSET must be 0x3D4");
static_assert(offsetof(gentity_s, s) == 0,
              "gentity_s must start with entityState_t");
static_assert(offsetof(gentity_s, client) == 0x250);
static_assert(sizeof(gentity_s) == GENTITY_SIZE,
              "gentity_s size must be 0x4F8 bytes");
#endif

#pragma pack(push, 16)
// Length of level_locals_t has size 0x23A10 on both client and server
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
  int32_t teamSortedClients[18];
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
  ClientNum_t sortedClients[18];
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
  objcamCameraTable objectiveCameras;
  uint8_t _unknown[1980];
};
ASSERT_SIZE(level_locals_t, 0x23A10);
#pragma pack(pop)

} // namespace level
} // namespace game