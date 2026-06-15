#pragma once
#include <cstdint>
#include "../core.hpp"
#include "../asm.hpp"
#include "../snd/snd.hpp"
#include "../ui/ui.hpp"
#include "../vehicle.hpp"
#include "../lobby.hpp"

namespace game {

namespace level {

typedef uint64_t EventParm_t;

struct HavokObj_t;

typedef uintptr_t PosedEntity_PoseHandle;

struct LerpEntityStateTurret {
  vec3_t gunAngles;
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
struct trajectory_t {
  uint8_t trType;
  uint8_t _padding01[3];
  int32_t trTime;
  int32_t trDuration;
  vec3_t trBase;
  vec3_t trDelta;
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
ASSERT_SIZE(LerpEntityState, 0xD0);
#pragma pack(pop)

#pragma pack(push, 1)
struct clientLinkInfo_t {
  int16_t parentEnt;
  uint8_t tagIndex;
  uint8_t flags;
};
#pragma pack(pop)

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

/*
  Up to date and correct.

  A type enclosing the discretely enumerated entity event types does not exist
  in the alpha or prior game engines. This enum was generated from a table of
  entity event display strings in the server engine.
*/
enum class entityEvent_t : uint32_t {
  EV_NONE = 0x0,
  EV_FOLIAGE_SOUND = 0x1,
  EV_STOP_WEAPON_SOUND = 0x2,
  EV_STOP_SOUND_ALIAS = 0x3,
  EV_SOUND_ALIAS = 0x4,
  EV_SOUND_ALIAS_NOTIFY = 0x5,
  EV_SOUND_BATTLECHAT_ALIAS = 0x6,
  EV_STOPSOUNDS = 0x7,
  EV_ITEM_PICKUP = 0x8,
  EV_AMMO_PICKUP = 0x9,
  EV_NOAMMO = 0xA,
  EV_NOAMMO_LEFT = 0xB,
  EV_EMPTY_OFFHAND = 0xC,
  EV_RESET_ADS = 0xD,
  EV_RELOAD = 0xE,
  EV_RELOAD_FROM_EMPTY = 0xF,
  EV_RELOAD_START = 0x10,
  EV_RELOAD_END = 0x11,
  EV_RELOAD_START_NOTIFY = 0x12,
  EV_RELOAD_ADDAMMO = 0x13,
  EV_RAISE_WEAPON = 0x14,
  EV_FIRST_RAISE_WEAPON = 0x15,
  EV_RAISE_PICKUP_WEAPON = 0x16,
  EV_PUTAWAY_WEAPON = 0x17,
  EV_WEAPON_ALT = 0x18,
  EV_WEAPON_SWITCH_STARTED = 0x19,
  EV_PULLBACK_WEAPON = 0x1A,
  EV_HOLDFIRE_WEAPON = 0x1B,
  EV_FIRE_WEAPON_DELAY_START = 0x1C,
  EV_FIRE_WEAPON_SCRIPTED = 0x1D,
  EV_FIRE_WEAPON = 0x1E,
  EV_FIRE_WEAPON_LASTSHOT = 0x1F,
  EV_FIRE_WEAPON_LEFT = 0x20,
  EV_FIRE_WEAPON_LASTSHOT_LEFT = 0x21,
  EV_FIRE_WEAPON_BALL_PASS = 0x22,
  EV_FIRE_RICOCHET = 0x23,
  EV_FIRE_WEAPON_GRAPPLE = 0x24,
  EV_RECHAMBER_WEAPON = 0x25,
  EV_EJECT_BRASS = 0x26,
  EV_FIRE_MELEE_SWIPE = 0x27,
  EV_FIRE_MELEE_POWER = 0x28,
  EV_FIRE_MELEE_POWER_LEFT = 0x29,
  EV_FIRE_MELEE = 0x2A,
  EV_MELEE_JUKE = 0x2B,
  EV_MELEE_JUKE_END = 0x2C,
  EV_MELEE_LUNGE = 0x2D,
  EV_MELEE_STRUGGLE = 0x2E,
  EV_MELEE_COUNTER = 0x2F,
  EV_FIRE_MELEE_DELAYED = 0x30,
  EV_MELEE_HIT = 0x31,
  EV_MELEE_HIT_ENT = 0x32,
  EV_MELEE_HIT_FROM_BEHIND = 0x33,
  EV_MELEE_MISS = 0x34,
  EV_MELEE_BLOOD = 0x35,
  EV_WEAPON_DEPLOYING = 0x36,
  EV_WEAPON_FINISH_DEPLOYING = 0x37,
  EV_WEAPON_FINISH_BREAKING_DOWN = 0x38,
  EV_INC_CHARGESHOT_LEVEL = 0x39,
  EV_HOLDFIRE_CANCEL = 0x3A,
  EV_HOLDFIRE_FIRED = 0x3B,
  EV_PREP_OFFHAND = 0x3C,
  EV_USE_OFFHAND = 0x3D,
  EV_SWITCH_OFFHAND = 0x3E,
  EV_PREP_RIOTSHIELD = 0x3F,
  EV_DEPLOY_RIOTSHIELD = 0x40,
  EV_LOWER_RIOTSHIELD = 0x41,
  EV_FIRE_BARREL_1 = 0x42,
  EV_FIRE_BARREL_2 = 0x43,
  EV_FIRE_BARREL_3 = 0x44,
  EV_FIRE_BARREL_4 = 0x45,
  EV_BULLET_HIT = 0x46,
  EV_BULLET_HIT_SHIELD = 0x47,
  EV_BULLET_HIT_CLIENT_SHIELD = 0x48,
  EV_EXPLOSIVE_IMPACT_ON_SHIELD = 0x49,
  EV_EXPLOSIVE_SPLASH_ON_SHIELD = 0x4A,
  EV_BULLET_HIT_CLIENT = 0x4B,
  EV_DESTRUCTIBLE_BULLET_HIT = 0x4C,
  EV_DESTRUCTIBLE_EXPLOSION_HIT = 0x4D,
  EV_GRENADE_BOUNCE = 0x4E,
  EV_GRENADE_EXPLODE = 0x4F,
  EV_ROCKET_EXPLODE = 0x50,
  EV_ROCKET_EXPLODE_NOMARKS = 0x51,
  EV_EXPLODE_ON_CLIENT = 0x52,
  EV_FLASHBANG_EXPLODE = 0x53,
  EV_CUSTOM_EXPLODE = 0x54,
  EV_CUSTOM_EXPLODE_NOMARKS = 0x55,
  EV_SENSOR_GRENADE_EXPLODE = 0x56,
  EV_CHANGE_TO_DUD = 0x57,
  EV_DUD_EXPLODE = 0x58,
  EV_DUD_IMPACT = 0x59,
  EV_MISSILE_REPULSED = 0x5A,
  EV_FIRE_EXPLODE = 0x5B,
  EV_TIMED_FX = 0x5C,
  EV_HIT_MARKER = 0x5D,
  EV_FAKE_FIRE = 0x5E,
  EV_PLAY_FX = 0x5F,
  EV_PLAY_FX_IGNORE_PAUSE = 0x60,
  EV_PLAY_FX_ON_TAG = 0x61,
  EV_PLAY_FX_ON_TAG_IGNORE_PAUSE = 0x62,
  EV_PLAY_FX_ON_CAMERA = 0x63,
  EV_PLAY_FX_ON_CAMERA_IGNORE_PAUSE = 0x64,
  EV_PHYS_EXPLOSION_SPHERE = 0x65,
  EV_PHYS_EXPLOSION_CYLINDER = 0x66,
  EV_PHYS_EXPLOSION_JOLT = 0x67,
  EV_PHYS_JET_THRUST = 0x68,
  EV_PHYS_LAUNCH = 0x69,
  EV_BALLISTIC_TARGET = 0x6A,
  EV_CREATE_DYNENT = 0x6B,
  EV_CREATE_ROPE = 0x6C,
  EV_DETACH_ENTITY = 0x6D,
  EV_DELETE_ROPE = 0x6E,
  EV_ROPE_COLLIDE_WORLD = 0x6F,
  EV_ROPE_COLLIDE_ENT = 0x70,
  EV_ROPE_SETFLAG = 0x71,
  EV_ROPE_SETPARAM = 0x72,
  EV_ROPE_ADDANCHOR = 0x73,
  EV_ROPE_REMOVEANCHOR = 0x74,
  EV_ROPE_MOVEANCHOR = 0x75,
  EV_EARTHQUAKE = 0x76,
  EV_SCREEN_SHAKE = 0x77,
  EV_GRENADE_DROP = 0x78,
  EV_GRENADE_SUICIDE = 0x79,
  EV_DETONATE = 0x7A,
  EV_DETONATE_OFFHAND = 0x7B,
  EV_MISSILE_REMOTE_BOOST = 0x7C,
  EV_MISSILE_REMOTE_BRAKE = 0x7D,
  EV_PLAY_RUMBLE_ON_ENT = 0x7E,
  EV_PLAY_RUMBLE_ON_POS = 0x7F,
  EV_PLAY_RUMBLELOOP_ON_ENT = 0x80,
  EV_PLAY_RUMBLELOOP_ON_POS = 0x81,
  EV_STOP_RUMBLE = 0x82,
  EV_STOP_ALL_RUMBLES = 0x83,
  EV_OBITUARY = 0x84,
  EV_REVIVE_OBITUARY = 0x85,
  EV_DIRECTIONAL_HIT_INDICATOR = 0x86,
  EV_ANIMATE_UI = 0x87,
  EV_KILLSTREAK_DAMAGE = 0x88,
  EV_ROUND_START_DELAY_HINT = 0x89,
  EV_NO_FRAG_GRENADE_HINT = 0x8A,
  EV_NO_SPECIAL_GRENADE_HINT = 0x8B,
  EV_GRENADE_NOT_ALLOWED_HINT = 0x8C,
  EV_NO_TARGET_IN_RANGE_HINT = 0x8D,
  EV_TARGET_TOO_CLOSE_HINT = 0x8E,
  EV_TARGET_NOT_ENOUGH_CLEARANCE = 0x8F,
  EV_LOCKON_REQUIRED_HINT = 0x90,
  EV_FOOTSTEP_SPRINT = 0x91,
  EV_FOOTSTEP_RUN = 0x92,
  EV_FOOTSTEP_WALK = 0x93,
  EV_FOOTSTEP_CROUCHRUN = 0x94,
  EV_FOOTSTEP_CROUCHWALK = 0x95,
  EV_FOOTSTEP_PRONE = 0x96,
  EV_LADDER_HAND_GRAB = 0x97,
  EV_MANTLE = 0x98,
  EV_JUMP = 0x99,
  EV_LEAP = 0x9A,
  EV_TRM = 0x9B,
  EV_WALLRUN_START_LEFT = 0x9C,
  EV_WALLRUN_START_RIGHT = 0x9D,
  EV_WALLRUN_FALL_START = 0x9E,
  EV_WALLRUN_JUMP = 0x9F,
  EV_WALLRUN_END = 0xA0,
  EV_WALLRUN_FALL_END = 0xA1,
  EV_DOUBLEJUMP_DISABLED = 0xA2,
  EV_DOUBLEJUMP_BOOST = 0xA3,
  EV_DOUBLEJUMP_FULLENERGY_BOOST = 0xA4,
  EV_SLIDE_START = 0xA5,
  EV_SOFT_LANDING = 0xA6,
  EV_MEDIUM_LANDING = 0xA7,
  EV_HARD_LANDING = 0xA8,
  EV_DAMAGE_LANDING = 0xA9,
  EV_WATER_LANDING = 0xAA,
  EV_FIRE_VEHICLE_TURRET = 0xAB,
  EV_FIRE_GUNNER_1 = 0xAC,
  EV_FIRE_GUNNER_2 = 0xAD,
  EV_FIRE_GUNNER_3 = 0xAE,
  EV_FIRE_GUNNER_4 = 0xAF,
  EV_FIRE_GUNNER_1A = 0xB0,
  EV_FIRE_GUNNER_2A = 0xB1,
  EV_FIRE_GUNNER_3A = 0xB2,
  EV_FIRE_GUNNER_4A = 0xB3,
  EV_FIRE_GUNNER_1B = 0xB4,
  EV_FIRE_GUNNER_2B = 0xB5,
  EV_FIRE_GUNNER_3B = 0xB6,
  EV_FIRE_GUNNER_4B = 0xB7,
  EV_FIRE_GUNNER_1C = 0xB8,
  EV_FIRE_GUNNER_2C = 0xB9,
  EV_FIRE_GUNNER_3C = 0xBA,
  EV_FIRE_GUNNER_4C = 0xBB,
  EV_START_CAMERA_TWEEN = 0xBC,
  EV_START_CAMERA_TWEEN_QUICK_STOP = 0xBD,
  EV_DESTRUCTIBLE_DISABLE_PIECES = 0xBE,
  EV_FOOTPRINT = 0xBF,
  EV_CANNOTPLANT = 0xC0,
  EV_CANT_EQUIP_PRONE = 0xC1,
  EV_CANT_JUKE = 0xC2,
  EV_CANT_SLAM = 0xC3,
  EV_DTP_LAUNCH = 0xC4,
  EV_DTP_LAND = 0xC5,
  EV_LEAP_LAND = 0xC6,
  EV_SCOPE_ZOOM = 0xC7,
  EV_JAM_WEAPON = 0xC8,
  EV_STACKFIRE = 0xC9,
  EV_MISSILE_STICK = 0xCA,
  EV_BOLT_IMPALE = 0xCB,
  EV_PLAY_WEAPON_DEATH_EFFECTS = 0xCC,
  EV_PLAY_WEAPON_DAMAGE_EFFECTS = 0xCD,
  EV_FACE_EVENT = 0xCE,
  EV_SETLOCALWIND = 0xCF,
  EV_FLOAT_LONGER = 0xD0,
  EV_FORCE_BUOYANCY = 0xD1,
  EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS = 0xD2,
  EV_SCALE_BUOYANCY = 0xD3,
  EV_GIB = 0xD4,
  EV_STANCE_INVALID = 0xD5,
  EV_ENTER_VEHICLE = 0xD6,
  EV_EXIT_VEHICLE = 0xD7,
  EV_IMMEDIATE_RAGDOLL = 0xD8,
  EV_ENT_SPAWNED_SENTINEL = 0xD9,
  EV_PROJECTILE_PASS_THROUGH = 0xDA,
  EV_CTX_COVER_START = 0xDB,
  EV_CTX_COVER_END = 0xDC,
  EV_GADGET_0 = 0xDD,
  EV_GADGET_1 = 0xDE,
  EV_GADGET_2 = 0xDF,
  EV_GADGET_0_DENIAL = 0xE0,
  EV_GADGET_1_DENIAL = 0xE1,
  EV_GADGET_2_DENIAL = 0xE2,
  EV_LOCKON_CHANGE = 0xE3,
  EV_TARGETOOR = 0xE4,
  EV_TARGETHIJACKED = 0xE5,
  EV_TARGETNOTVALID = 0xE6,
  EV_NO_TARGETS = 0xE7,
  EV_NO_LOS_TO_TARGET = 0xE8,
  EV_TARGET_DISABLED = 0xE9,
  EV_TARGETING_ABORTED = 0xEA,
  EV_TARGET_ALREADY_TARGETED = 0xEB,
  EV_GRAPPLE_BEAM_ON = 0xEC,
  EV_GRAPPLE_BEAM_OFF = 0xED,
  EV_LMG_INFINITE_SOUNDS = 0xEE,

  /*
    Does not exist in the engine.
  */
  EV_COUNT = 0xEF
};

enum class entityType8_t : uint8_t {
  /*
   Comments above each entity type
   enumeration below were generated from
   a table of type information strings
   for each entity type enumeration found in the server
   engine.
  */

  // General
  ET_GENERAL = 0x0,
  // Player
  ET_PLAYER = 0x1,
  // Corpse
  ET_PLAYER_CORPSE = 0x2,
  // Item
  ET_ITEM = 0x3,
  // Missile
  ET_MISSILE = 0x4,
  // Invisible entity
  ET_PLAYER_INVISIBLE = 0x5,
  // Scriptmover
  ET_SCRIPTMOVER = 0x6,
  // Sound blend
  ET_SOUND_BLEND = 0x7,
  // FX
  ET_FX = 0x8,
  // Loop FX
  ET_LOOP_FX = 0x9,
  // Primary Light
  ET_PRIMARY_LIGHT = 0xA,
  // Lensflare
  ET_LENSFLARE = 0xB,
  // Reflection Probe
  ET_REFLECTION_PROBE = 0xC,
  // Helicopter
  ET_HELICOPTER = 0xD,
  // Plane
  ET_PLANE = 0xE,
  // Vehicle
  ET_VEHICLE = 0xF,
  // Vehicle spawner
  ET_VEHICLE_SPAWNER = 0x10,
  // Vehicle corpse
  ET_VEHICLE_CORPSE = 0x11,
  // Actor
  ET_ACTOR = 0x12,
  // Actor spawner
  ET_ACTOR_SPAWNER = 0x13,
  // Actor corpse
  ET_ACTOR_CORPSE = 0x14,
  // Streamer Hint
  ET_STREAMER_HINT = 0x15,
  // ZBarrier
  ET_ZBARRIER = 0x16,
  // Trigger
  ET_TRIGGER = 0x17,

  /*
    Does not exist in the engine.
  */
  ET_COUNT = 18,

  /*
   Entity events are tagged with an entityType_t value
   >= entityType_t::EV_NONE. They are accessed with
   `static_cast<entityEvent_t>(entityType - entityType_t::EV_NONE)` and
   handled in several CG_ functions in the engine which match and handle
   each entity event type.

   Entity event enumerations were added to the entity type enum for
   the sake of convenience.
  */

  EV_NONE = 0x18,
  EV_FOLIAGE_SOUND = 0x19,
  EV_STOP_WEAPON_SOUND = 0x1A,
  EV_STOP_SOUND_ALIAS = 0x1B,
  EV_SOUND_ALIAS = 0x1C,
  EV_SOUND_ALIAS_NOTIFY = 0x1D,
  EV_SOUND_BATTLECHAT_ALIAS = 0x1E,
  EV_STOPSOUNDS = 0x1F,
  EV_ITEM_PICKUP = 0x20,
  EV_AMMO_PICKUP = 0x21,
  EV_NOAMMO = 0x22,
  EV_NOAMMO_LEFT = 0x23,
  EV_EMPTY_OFFHAND = 0x24,
  EV_RESET_ADS = 0x25,
  EV_RELOAD = 0x26,
  EV_RELOAD_FROM_EMPTY = 0x27,
  EV_RELOAD_START = 0x28,
  EV_RELOAD_END = 0x29,
  EV_RELOAD_START_NOTIFY = 0x2A,
  EV_RELOAD_ADDAMMO = 0x2B,
  EV_RAISE_WEAPON = 0x2C,
  EV_FIRST_RAISE_WEAPON = 0x2D,
  EV_RAISE_PICKUP_WEAPON = 0x2E,
  EV_PUTAWAY_WEAPON = 0x2F,
  EV_WEAPON_ALT = 0x30,
  EV_WEAPON_SWITCH_STARTED = 0x31,
  EV_PULLBACK_WEAPON = 0x32,
  EV_HOLDFIRE_WEAPON = 0x33,
  EV_FIRE_WEAPON_DELAY_START = 0x34,
  EV_FIRE_WEAPON_SCRIPTED = 0x35,
  EV_FIRE_WEAPON = 0x36,
  EV_FIRE_WEAPON_LASTSHOT = 0x37,
  EV_FIRE_WEAPON_LEFT = 0x38,
  EV_FIRE_WEAPON_LASTSHOT_LEFT = 0x39,
  EV_FIRE_WEAPON_BALL_PASS = 0x3A,
  EV_FIRE_RICOCHET = 0x3B,
  EV_FIRE_WEAPON_GRAPPLE = 0x3C,
  EV_RECHAMBER_WEAPON = 0x3D,
  EV_EJECT_BRASS = 0x3E,
  EV_FIRE_MELEE_SWIPE = 0x3F,
  EV_FIRE_MELEE_POWER = 0x40,
  EV_FIRE_MELEE_POWER_LEFT = 0x41,
  EV_FIRE_MELEE = 0x42,
  EV_MELEE_JUKE = 0x43,
  EV_MELEE_JUKE_END = 0x44,
  EV_MELEE_LUNGE = 0x45,
  EV_MELEE_STRUGGLE = 0x46,
  EV_MELEE_COUNTER = 0x47,
  EV_FIRE_MELEE_DELAYED = 0x48,
  EV_MELEE_HIT = 0x49,
  EV_MELEE_HIT_ENT = 0x4A,
  EV_MELEE_HIT_FROM_BEHIND = 0x4B,
  EV_MELEE_MISS = 0x4C,
  EV_MELEE_BLOOD = 0x4D,
  EV_WEAPON_DEPLOYING = 0x4E,
  EV_WEAPON_FINISH_DEPLOYING = 0x4F,
  EV_WEAPON_FINISH_BREAKING_DOWN = 0x50,
  EV_INC_CHARGESHOT_LEVEL = 0x51,
  EV_HOLDFIRE_CANCEL = 0x52,
  EV_HOLDFIRE_FIRED = 0x53,
  EV_PREP_OFFHAND = 0x54,
  EV_USE_OFFHAND = 0x55,
  EV_SWITCH_OFFHAND = 0x56,
  EV_PREP_RIOTSHIELD = 0x57,
  EV_DEPLOY_RIOTSHIELD = 0x58,
  EV_LOWER_RIOTSHIELD = 0x59,
  EV_FIRE_BARREL_1 = 0x5A,
  EV_FIRE_BARREL_2 = 0x5B,
  EV_FIRE_BARREL_3 = 0x5C,
  EV_FIRE_BARREL_4 = 0x5D,
  EV_BULLET_HIT = 0x5E,
  EV_BULLET_HIT_SHIELD = 0x5F,
  EV_BULLET_HIT_CLIENT_SHIELD = 0x60,
  EV_EXPLOSIVE_IMPACT_ON_SHIELD = 0x61,
  EV_EXPLOSIVE_SPLASH_ON_SHIELD = 0x62,
  EV_BULLET_HIT_CLIENT = 0x63,
  EV_DESTRUCTIBLE_BULLET_HIT = 0x64,
  EV_DESTRUCTIBLE_EXPLOSION_HIT = 0x65,
  EV_GRENADE_BOUNCE = 0x66,
  EV_GRENADE_EXPLODE = 0x67,
  EV_ROCKET_EXPLODE = 0x68,
  EV_ROCKET_EXPLODE_NOMARKS = 0x69,
  EV_EXPLODE_ON_CLIENT = 0x6A,
  EV_FLASHBANG_EXPLODE = 0x6B,
  EV_CUSTOM_EXPLODE = 0x6C,
  EV_CUSTOM_EXPLODE_NOMARKS = 0x6D,
  EV_SENSOR_GRENADE_EXPLODE = 0x6E,
  EV_CHANGE_TO_DUD = 0x6F,
  EV_DUD_EXPLODE = 0x70,
  EV_DUD_IMPACT = 0x71,
  EV_MISSILE_REPULSED = 0x72,
  EV_FIRE_EXPLODE = 0x73,
  EV_TIMED_FX = 0x74,
  EV_HIT_MARKER = 0x75,
  EV_FAKE_FIRE = 0x76,
  EV_PLAY_FX = 0x77,
  EV_PLAY_FX_IGNORE_PAUSE = 0x78,
  EV_PLAY_FX_ON_TAG = 0x79,
  EV_PLAY_FX_ON_TAG_IGNORE_PAUSE = 0x7A,
  EV_PLAY_FX_ON_CAMERA = 0x7B,
  EV_PLAY_FX_ON_CAMERA_IGNORE_PAUSE = 0x7C,
  EV_PHYS_EXPLOSION_SPHERE = 0x7D,
  EV_PHYS_EXPLOSION_CYLINDER = 0x7E,
  EV_PHYS_EXPLOSION_JOLT = 0x7F,
  EV_PHYS_JET_THRUST = 0x80,
  EV_PHYS_LAUNCH = 0x81,
  EV_BALLISTIC_TARGET = 0x82,
  EV_CREATE_DYNENT = 0x83,
  EV_CREATE_ROPE = 0x84,
  EV_DETACH_ENTITY = 0x85,
  EV_DELETE_ROPE = 0x86,
  EV_ROPE_COLLIDE_WORLD = 0x87,
  EV_ROPE_COLLIDE_ENT = 0x88,
  EV_ROPE_SETFLAG = 0x89,
  EV_ROPE_SETPARAM = 0x8A,
  EV_ROPE_ADDANCHOR = 0x8B,
  EV_ROPE_REMOVEANCHOR = 0x8C,
  EV_ROPE_MOVEANCHOR = 0x8D,
  EV_EARTHQUAKE = 0x8E,
  EV_SCREEN_SHAKE = 0x8F,
  EV_GRENADE_DROP = 0x90,
  EV_GRENADE_SUICIDE = 0x91,
  EV_DETONATE = 0x92,
  EV_DETONATE_OFFHAND = 0x93,
  EV_MISSILE_REMOTE_BOOST = 0x94,
  EV_MISSILE_REMOTE_BRAKE = 0x95,
  EV_PLAY_RUMBLE_ON_ENT = 0x96,
  EV_PLAY_RUMBLE_ON_POS = 0x97,
  EV_PLAY_RUMBLELOOP_ON_ENT = 0x98,
  EV_PLAY_RUMBLELOOP_ON_POS = 0x99,
  EV_STOP_RUMBLE = 0x9A,
  EV_STOP_ALL_RUMBLES = 0x9B,
  EV_OBITUARY = 0x9C,
  EV_REVIVE_OBITUARY = 0x9D,
  EV_DIRECTIONAL_HIT_INDICATOR = 0x9E,
  EV_ANIMATE_UI = 0x9F,
  EV_KILLSTREAK_DAMAGE = 0xA0,
  EV_ROUND_START_DELAY_HINT = 0xA1,
  EV_NO_FRAG_GRENADE_HINT = 0xA2,
  EV_NO_SPECIAL_GRENADE_HINT = 0xA3,
  EV_GRENADE_NOT_ALLOWED_HINT = 0xA4,
  EV_NO_TARGET_IN_RANGE_HINT = 0xA5,
  EV_TARGET_TOO_CLOSE_HINT = 0xA6,
  EV_TARGET_NOT_ENOUGH_CLEARANCE = 0xA7,
  EV_LOCKON_REQUIRED_HINT = 0xA8,
  EV_FOOTSTEP_SPRINT = 0xA9,
  EV_FOOTSTEP_RUN = 0xAA,
  EV_FOOTSTEP_WALK = 0xAB,
  EV_FOOTSTEP_CROUCHRUN = 0xAC,
  EV_FOOTSTEP_CROUCHWALK = 0xAD,
  EV_FOOTSTEP_PRONE = 0xAE,
  EV_LADDER_HAND_GRAB = 0xAF,
  EV_MANTLE = 0xB0,
  EV_JUMP = 0xB1,
  EV_LEAP = 0xB2,
  EV_TRM = 0xB3,
  EV_WALLRUN_START_LEFT = 0xB4,
  EV_WALLRUN_START_RIGHT = 0xB5,
  EV_WALLRUN_FALL_START = 0xB6,
  EV_WALLRUN_JUMP = 0xB7,
  EV_WALLRUN_END = 0xB8,
  EV_WALLRUN_FALL_END = 0xB9,
  EV_DOUBLEJUMP_DISABLED = 0xBA,
  EV_DOUBLEJUMP_BOOST = 0xBB,
  EV_DOUBLEJUMP_FULLENERGY_BOOST = 0xBC,
  EV_SLIDE_START = 0xBD,
  EV_SOFT_LANDING = 0xBE,
  EV_MEDIUM_LANDING = 0xBF,
  EV_HARD_LANDING = 0xC0,
  EV_DAMAGE_LANDING = 0xC1,
  EV_WATER_LANDING = 0xC2,
  EV_FIRE_VEHICLE_TURRET = 0xC3,
  EV_FIRE_GUNNER_1 = 0xC4,
  EV_FIRE_GUNNER_2 = 0xC5,
  EV_FIRE_GUNNER_3 = 0xC6,
  EV_FIRE_GUNNER_4 = 0xC7,
  EV_FIRE_GUNNER_1A = 0xC8,
  EV_FIRE_GUNNER_2A = 0xC9,
  EV_FIRE_GUNNER_3A = 0xCA,
  EV_FIRE_GUNNER_4A = 0xCB,
  EV_FIRE_GUNNER_1B = 0xCC,
  EV_FIRE_GUNNER_2B = 0xCD,
  EV_FIRE_GUNNER_3B = 0xCE,
  EV_FIRE_GUNNER_4B = 0xCF,
  EV_FIRE_GUNNER_1C = 0xD0,
  EV_FIRE_GUNNER_2C = 0xD1,
  EV_FIRE_GUNNER_3C = 0xD2,
  EV_FIRE_GUNNER_4C = 0xD3,
  EV_START_CAMERA_TWEEN = 0xD4,
  EV_START_CAMERA_TWEEN_QUICK_STOP = 0xD5,
  EV_DESTRUCTIBLE_DISABLE_PIECES = 0xD6,
  EV_FOOTPRINT = 0xD7,
  EV_CANNOTPLANT = 0xD8,
  EV_CANT_EQUIP_PRONE = 0xD9,
  EV_CANT_JUKE = 0xDA,
  EV_CANT_SLAM = 0xDB,
  EV_DTP_LAUNCH = 0xDC,
  EV_DTP_LAND = 0xDD,
  EV_LEAP_LAND = 0xDE,
  EV_SCOPE_ZOOM = 0xDF,
  EV_JAM_WEAPON = 0xE0,
  EV_STACKFIRE = 0xE1,
  EV_MISSILE_STICK = 0xE2,
  EV_BOLT_IMPALE = 0xE3,
  EV_PLAY_WEAPON_DEATH_EFFECTS = 0xE4,
  EV_PLAY_WEAPON_DAMAGE_EFFECTS = 0xE5,
  EV_FACE_EVENT = 0xE6,
  EV_SETLOCALWIND = 0xE7,
  EV_FLOAT_LONGER = 0xE8,
  EV_FORCE_BUOYANCY = 0xE9,
  EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS = 0xEA,
  EV_SCALE_BUOYANCY = 0xEB,
  EV_GIB = 0xEC,
  EV_STANCE_INVALID = 0xED,
  EV_ENTER_VEHICLE = 0xEE,
  EV_EXIT_VEHICLE = 0xEF,
  EV_IMMEDIATE_RAGDOLL = 0xF0,
  EV_ENT_SPAWNED_SENTINEL = 0xF1,
  EV_PROJECTILE_PASS_THROUGH = 0xF2,
  EV_CTX_COVER_START = 0xF3,
  EV_CTX_COVER_END = 0xF4,
  EV_GADGET_0 = 0xF5,
  EV_GADGET_1 = 0xF6,
  EV_GADGET_2 = 0xF7,
  EV_GADGET_0_DENIAL = 0xF8,
  EV_GADGET_1_DENIAL = 0xF9,
  EV_GADGET_2_DENIAL = 0xFA,
  EV_LOCKON_CHANGE = 0xFB,
  EV_TARGETOOR = 0xFC,
  EV_TARGETHIJACKED = 0xFD,
  EV_TARGETNOTVALID = 0xFE,
  EV_COUNT = 0xFF
};

enum class entityType32_t : uint32_t {
  /*
   Comments above each entity type
   enumeration below were generated from
   a table of type information strings
   for each entity type enumeration found in the server
   engine.
  */

  // General
  ET_GENERAL = 0x0,
  // Player
  ET_PLAYER = 0x1,
  // Corpse
  ET_PLAYER_CORPSE = 0x2,
  // Item
  ET_ITEM = 0x3,
  // Missile
  ET_MISSILE = 0x4,
  // Invisible entity
  ET_PLAYER_INVISIBLE = 0x5,
  // Scriptmover
  ET_SCRIPTMOVER = 0x6,
  // Sound blend
  ET_SOUND_BLEND = 0x7,
  // FX
  ET_FX = 0x8,
  // Loop FX
  ET_LOOP_FX = 0x9,
  // Primary Light
  ET_PRIMARY_LIGHT = 0xA,
  // Lensflare
  ET_LENSFLARE = 0xB,
  // Reflection Probe
  ET_REFLECTION_PROBE = 0xC,
  // Helicopter
  ET_HELICOPTER = 0xD,
  // Plane
  ET_PLANE = 0xE,
  // Vehicle
  ET_VEHICLE = 0xF,
  // Vehicle spawner
  ET_VEHICLE_SPAWNER = 0x10,
  // Vehicle corpse
  ET_VEHICLE_CORPSE = 0x11,
  // Actor
  ET_ACTOR = 0x12,
  // Actor spawner
  ET_ACTOR_SPAWNER = 0x13,
  // Actor corpse
  ET_ACTOR_CORPSE = 0x14,
  // Streamer Hint
  ET_STREAMER_HINT = 0x15,
  // ZBarrier
  ET_ZBARRIER = 0x16,
  // Trigger
  ET_TRIGGER = 0x17,

  /*
    Does not exist in the engine.
  */
  ET_COUNT = 18,

  /*
   Entity events are tagged with an entityType_t value
   >= entityType_t::EV_NONE. They are accessed with
   `static_cast<entityEvent_t>(entityType - entityType_t::EV_NONE)` and
   handled in several CG_ functions in the engine which match and handle
   each entity event type.

   Entity event enumerations were added to the entity type enum for
   the sake of convenience.
  */

  EV_NONE = 0x18,
  EV_FOLIAGE_SOUND = 0x19,
  EV_STOP_WEAPON_SOUND = 0x1A,
  EV_STOP_SOUND_ALIAS = 0x1B,
  EV_SOUND_ALIAS = 0x1C,
  EV_SOUND_ALIAS_NOTIFY = 0x1D,
  EV_SOUND_BATTLECHAT_ALIAS = 0x1E,
  EV_STOPSOUNDS = 0x1F,
  EV_ITEM_PICKUP = 0x20,
  EV_AMMO_PICKUP = 0x21,
  EV_NOAMMO = 0x22,
  EV_NOAMMO_LEFT = 0x23,
  EV_EMPTY_OFFHAND = 0x24,
  EV_RESET_ADS = 0x25,
  EV_RELOAD = 0x26,
  EV_RELOAD_FROM_EMPTY = 0x27,
  EV_RELOAD_START = 0x28,
  EV_RELOAD_END = 0x29,
  EV_RELOAD_START_NOTIFY = 0x2A,
  EV_RELOAD_ADDAMMO = 0x2B,
  EV_RAISE_WEAPON = 0x2C,
  EV_FIRST_RAISE_WEAPON = 0x2D,
  EV_RAISE_PICKUP_WEAPON = 0x2E,
  EV_PUTAWAY_WEAPON = 0x2F,
  EV_WEAPON_ALT = 0x30,
  EV_WEAPON_SWITCH_STARTED = 0x31,
  EV_PULLBACK_WEAPON = 0x32,
  EV_HOLDFIRE_WEAPON = 0x33,
  EV_FIRE_WEAPON_DELAY_START = 0x34,
  EV_FIRE_WEAPON_SCRIPTED = 0x35,
  EV_FIRE_WEAPON = 0x36,
  EV_FIRE_WEAPON_LASTSHOT = 0x37,
  EV_FIRE_WEAPON_LEFT = 0x38,
  EV_FIRE_WEAPON_LASTSHOT_LEFT = 0x39,
  EV_FIRE_WEAPON_BALL_PASS = 0x3A,
  EV_FIRE_RICOCHET = 0x3B,
  EV_FIRE_WEAPON_GRAPPLE = 0x3C,
  EV_RECHAMBER_WEAPON = 0x3D,
  EV_EJECT_BRASS = 0x3E,
  EV_FIRE_MELEE_SWIPE = 0x3F,
  EV_FIRE_MELEE_POWER = 0x40,
  EV_FIRE_MELEE_POWER_LEFT = 0x41,
  EV_FIRE_MELEE = 0x42,
  EV_MELEE_JUKE = 0x43,
  EV_MELEE_JUKE_END = 0x44,
  EV_MELEE_LUNGE = 0x45,
  EV_MELEE_STRUGGLE = 0x46,
  EV_MELEE_COUNTER = 0x47,
  EV_FIRE_MELEE_DELAYED = 0x48,
  EV_MELEE_HIT = 0x49,
  EV_MELEE_HIT_ENT = 0x4A,
  EV_MELEE_HIT_FROM_BEHIND = 0x4B,
  EV_MELEE_MISS = 0x4C,
  EV_MELEE_BLOOD = 0x4D,
  EV_WEAPON_DEPLOYING = 0x4E,
  EV_WEAPON_FINISH_DEPLOYING = 0x4F,
  EV_WEAPON_FINISH_BREAKING_DOWN = 0x50,
  EV_INC_CHARGESHOT_LEVEL = 0x51,
  EV_HOLDFIRE_CANCEL = 0x52,
  EV_HOLDFIRE_FIRED = 0x53,
  EV_PREP_OFFHAND = 0x54,
  EV_USE_OFFHAND = 0x55,
  EV_SWITCH_OFFHAND = 0x56,
  EV_PREP_RIOTSHIELD = 0x57,
  EV_DEPLOY_RIOTSHIELD = 0x58,
  EV_LOWER_RIOTSHIELD = 0x59,
  EV_FIRE_BARREL_1 = 0x5A,
  EV_FIRE_BARREL_2 = 0x5B,
  EV_FIRE_BARREL_3 = 0x5C,
  EV_FIRE_BARREL_4 = 0x5D,
  EV_BULLET_HIT = 0x5E,
  EV_BULLET_HIT_SHIELD = 0x5F,
  EV_BULLET_HIT_CLIENT_SHIELD = 0x60,
  EV_EXPLOSIVE_IMPACT_ON_SHIELD = 0x61,
  EV_EXPLOSIVE_SPLASH_ON_SHIELD = 0x62,
  EV_BULLET_HIT_CLIENT = 0x63,
  EV_DESTRUCTIBLE_BULLET_HIT = 0x64,
  EV_DESTRUCTIBLE_EXPLOSION_HIT = 0x65,
  EV_GRENADE_BOUNCE = 0x66,
  EV_GRENADE_EXPLODE = 0x67,
  EV_ROCKET_EXPLODE = 0x68,
  EV_ROCKET_EXPLODE_NOMARKS = 0x69,
  EV_EXPLODE_ON_CLIENT = 0x6A,
  EV_FLASHBANG_EXPLODE = 0x6B,
  EV_CUSTOM_EXPLODE = 0x6C,
  EV_CUSTOM_EXPLODE_NOMARKS = 0x6D,
  EV_SENSOR_GRENADE_EXPLODE = 0x6E,
  EV_CHANGE_TO_DUD = 0x6F,
  EV_DUD_EXPLODE = 0x70,
  EV_DUD_IMPACT = 0x71,
  EV_MISSILE_REPULSED = 0x72,
  EV_FIRE_EXPLODE = 0x73,
  EV_TIMED_FX = 0x74,
  EV_HIT_MARKER = 0x75,
  EV_FAKE_FIRE = 0x76,
  EV_PLAY_FX = 0x77,
  EV_PLAY_FX_IGNORE_PAUSE = 0x78,
  EV_PLAY_FX_ON_TAG = 0x79,
  EV_PLAY_FX_ON_TAG_IGNORE_PAUSE = 0x7A,
  EV_PLAY_FX_ON_CAMERA = 0x7B,
  EV_PLAY_FX_ON_CAMERA_IGNORE_PAUSE = 0x7C,
  EV_PHYS_EXPLOSION_SPHERE = 0x7D,
  EV_PHYS_EXPLOSION_CYLINDER = 0x7E,
  EV_PHYS_EXPLOSION_JOLT = 0x7F,
  EV_PHYS_JET_THRUST = 0x80,
  EV_PHYS_LAUNCH = 0x81,
  EV_BALLISTIC_TARGET = 0x82,
  EV_CREATE_DYNENT = 0x83,
  EV_CREATE_ROPE = 0x84,
  EV_DETACH_ENTITY = 0x85,
  EV_DELETE_ROPE = 0x86,
  EV_ROPE_COLLIDE_WORLD = 0x87,
  EV_ROPE_COLLIDE_ENT = 0x88,
  EV_ROPE_SETFLAG = 0x89,
  EV_ROPE_SETPARAM = 0x8A,
  EV_ROPE_ADDANCHOR = 0x8B,
  EV_ROPE_REMOVEANCHOR = 0x8C,
  EV_ROPE_MOVEANCHOR = 0x8D,
  EV_EARTHQUAKE = 0x8E,
  EV_SCREEN_SHAKE = 0x8F,
  EV_GRENADE_DROP = 0x90,
  EV_GRENADE_SUICIDE = 0x91,
  EV_DETONATE = 0x92,
  EV_DETONATE_OFFHAND = 0x93,
  EV_MISSILE_REMOTE_BOOST = 0x94,
  EV_MISSILE_REMOTE_BRAKE = 0x95,
  EV_PLAY_RUMBLE_ON_ENT = 0x96,
  EV_PLAY_RUMBLE_ON_POS = 0x97,
  EV_PLAY_RUMBLELOOP_ON_ENT = 0x98,
  EV_PLAY_RUMBLELOOP_ON_POS = 0x99,
  EV_STOP_RUMBLE = 0x9A,
  EV_STOP_ALL_RUMBLES = 0x9B,
  EV_OBITUARY = 0x9C,
  EV_REVIVE_OBITUARY = 0x9D,
  EV_DIRECTIONAL_HIT_INDICATOR = 0x9E,
  EV_ANIMATE_UI = 0x9F,
  EV_KILLSTREAK_DAMAGE = 0xA0,
  EV_ROUND_START_DELAY_HINT = 0xA1,
  EV_NO_FRAG_GRENADE_HINT = 0xA2,
  EV_NO_SPECIAL_GRENADE_HINT = 0xA3,
  EV_GRENADE_NOT_ALLOWED_HINT = 0xA4,
  EV_NO_TARGET_IN_RANGE_HINT = 0xA5,
  EV_TARGET_TOO_CLOSE_HINT = 0xA6,
  EV_TARGET_NOT_ENOUGH_CLEARANCE = 0xA7,
  EV_LOCKON_REQUIRED_HINT = 0xA8,
  EV_FOOTSTEP_SPRINT = 0xA9,
  EV_FOOTSTEP_RUN = 0xAA,
  EV_FOOTSTEP_WALK = 0xAB,
  EV_FOOTSTEP_CROUCHRUN = 0xAC,
  EV_FOOTSTEP_CROUCHWALK = 0xAD,
  EV_FOOTSTEP_PRONE = 0xAE,
  EV_LADDER_HAND_GRAB = 0xAF,
  EV_MANTLE = 0xB0,
  EV_JUMP = 0xB1,
  EV_LEAP = 0xB2,
  EV_TRM = 0xB3,
  EV_WALLRUN_START_LEFT = 0xB4,
  EV_WALLRUN_START_RIGHT = 0xB5,
  EV_WALLRUN_FALL_START = 0xB6,
  EV_WALLRUN_JUMP = 0xB7,
  EV_WALLRUN_END = 0xB8,
  EV_WALLRUN_FALL_END = 0xB9,
  EV_DOUBLEJUMP_DISABLED = 0xBA,
  EV_DOUBLEJUMP_BOOST = 0xBB,
  EV_DOUBLEJUMP_FULLENERGY_BOOST = 0xBC,
  EV_SLIDE_START = 0xBD,
  EV_SOFT_LANDING = 0xBE,
  EV_MEDIUM_LANDING = 0xBF,
  EV_HARD_LANDING = 0xC0,
  EV_DAMAGE_LANDING = 0xC1,
  EV_WATER_LANDING = 0xC2,
  EV_FIRE_VEHICLE_TURRET = 0xC3,
  EV_FIRE_GUNNER_1 = 0xC4,
  EV_FIRE_GUNNER_2 = 0xC5,
  EV_FIRE_GUNNER_3 = 0xC6,
  EV_FIRE_GUNNER_4 = 0xC7,
  EV_FIRE_GUNNER_1A = 0xC8,
  EV_FIRE_GUNNER_2A = 0xC9,
  EV_FIRE_GUNNER_3A = 0xCA,
  EV_FIRE_GUNNER_4A = 0xCB,
  EV_FIRE_GUNNER_1B = 0xCC,
  EV_FIRE_GUNNER_2B = 0xCD,
  EV_FIRE_GUNNER_3B = 0xCE,
  EV_FIRE_GUNNER_4B = 0xCF,
  EV_FIRE_GUNNER_1C = 0xD0,
  EV_FIRE_GUNNER_2C = 0xD1,
  EV_FIRE_GUNNER_3C = 0xD2,
  EV_FIRE_GUNNER_4C = 0xD3,
  EV_START_CAMERA_TWEEN = 0xD4,
  EV_START_CAMERA_TWEEN_QUICK_STOP = 0xD5,
  EV_DESTRUCTIBLE_DISABLE_PIECES = 0xD6,
  EV_FOOTPRINT = 0xD7,
  EV_CANNOTPLANT = 0xD8,
  EV_CANT_EQUIP_PRONE = 0xD9,
  EV_CANT_JUKE = 0xDA,
  EV_CANT_SLAM = 0xDB,
  EV_DTP_LAUNCH = 0xDC,
  EV_DTP_LAND = 0xDD,
  EV_LEAP_LAND = 0xDE,
  EV_SCOPE_ZOOM = 0xDF,
  EV_JAM_WEAPON = 0xE0,
  EV_STACKFIRE = 0xE1,
  EV_MISSILE_STICK = 0xE2,
  EV_BOLT_IMPALE = 0xE3,
  EV_PLAY_WEAPON_DEATH_EFFECTS = 0xE4,
  EV_PLAY_WEAPON_DAMAGE_EFFECTS = 0xE5,
  EV_FACE_EVENT = 0xE6,
  EV_SETLOCALWIND = 0xE7,
  EV_FLOAT_LONGER = 0xE8,
  EV_FORCE_BUOYANCY = 0xE9,
  EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS = 0xEA,
  EV_SCALE_BUOYANCY = 0xEB,
  EV_GIB = 0xEC,
  EV_STANCE_INVALID = 0xED,
  EV_ENTER_VEHICLE = 0xEE,
  EV_EXIT_VEHICLE = 0xEF,
  EV_IMMEDIATE_RAGDOLL = 0xF0,
  EV_ENT_SPAWNED_SENTINEL = 0xF1,
  EV_PROJECTILE_PASS_THROUGH = 0xF2,
  EV_CTX_COVER_START = 0xF3,
  EV_CTX_COVER_END = 0xF4,
  EV_GADGET_0 = 0xF5,
  EV_GADGET_1 = 0xF6,
  EV_GADGET_2 = 0xF7,
  EV_GADGET_0_DENIAL = 0xF8,
  EV_GADGET_1_DENIAL = 0xF9,
  EV_GADGET_2_DENIAL = 0xFA,
  EV_LOCKON_CHANGE = 0xFB,
  EV_TARGETOOR = 0xFC,
  EV_TARGETHIJACKED = 0xFD,
  EV_TARGETNOTVALID = 0xFE,
  EV_NO_TARGETS = 0xFF,
  EV_NO_LOS_TO_TARGET = 0x100,
  EV_TARGET_DISABLED = 0x101,
  EV_TARGETING_ABORTED = 0x102,
  EV_TARGET_ALREADY_TARGETED = 0x103,
  EV_GRAPPLE_BEAM_ON = 0x104,
  EV_GRAPPLE_BEAM_OFF = 0x105,
  EV_LMG_INFINITE_SOUNDS = 0x106,

  /*
    Does not exist in the engine.
  */
  EV_COUNT = 0xEF
};

/*
   Up to date and correct.

   Generated from two tables of display strings in the server engine -
   one for entity type enumerations, and the other entity event enumerations.
*/
struct entityType_t {

  enum entityType : uint16_t {
    /*
     Comments above each entity type
     enumeration below were generated from
     a table of type information strings
     for each entity type enumeration found in the server
     engine.
    */

    // General
    ET_GENERAL = 0x0,
    // Player
    ET_PLAYER = 0x1,
    // Corpse
    ET_PLAYER_CORPSE = 0x2,
    // Item
    ET_ITEM = 0x3,
    // Missile
    ET_MISSILE = 0x4,
    // Invisible entity
    ET_PLAYER_INVISIBLE = 0x5,
    // Scriptmover
    ET_SCRIPTMOVER = 0x6,
    // Sound blend
    ET_SOUND_BLEND = 0x7,
    // FX
    ET_FX = 0x8,
    // Loop FX
    ET_LOOP_FX = 0x9,
    // Primary Light
    ET_PRIMARY_LIGHT = 0xA,
    // Lensflare
    ET_LENSFLARE = 0xB,
    // Reflection Probe
    ET_REFLECTION_PROBE = 0xC,
    // Helicopter
    ET_HELICOPTER = 0xD,
    // Plane
    ET_PLANE = 0xE,
    // Vehicle
    ET_VEHICLE = 0xF,
    // Vehicle spawner
    ET_VEHICLE_SPAWNER = 0x10,
    // Vehicle corpse
    ET_VEHICLE_CORPSE = 0x11,
    // Actor
    ET_ACTOR = 0x12,
    // Actor spawner
    ET_ACTOR_SPAWNER = 0x13,
    // Actor corpse
    ET_ACTOR_CORPSE = 0x14,
    // Streamer Hint
    ET_STREAMER_HINT = 0x15,
    // ZBarrier
    ET_ZBARRIER = 0x16,
    // Trigger
    ET_TRIGGER = 0x17,

    /*
      Does not exist in the engine.
    */
    ET_COUNT = 18,

    /*
     Entity events are tagged with an entityType_t value
     >= entityType_t::EV_NONE. They are accessed with
     `static_cast<entityEvent_t>(entityType - entityType_t::EV_NONE)` and
     handled in several CG_ functions in the engine which match and handle
     each entity event type.

     Entity event enumerations were added to the entity type enum for
     the sake of convenience.
    */

    EV_NONE = 0x18,
    EV_FOLIAGE_SOUND = 0x19,
    EV_STOP_WEAPON_SOUND = 0x1A,
    EV_STOP_SOUND_ALIAS = 0x1B,
    EV_SOUND_ALIAS = 0x1C,
    EV_SOUND_ALIAS_NOTIFY = 0x1D,
    EV_SOUND_BATTLECHAT_ALIAS = 0x1E,
    EV_STOPSOUNDS = 0x1F,
    EV_ITEM_PICKUP = 0x20,
    EV_AMMO_PICKUP = 0x21,
    EV_NOAMMO = 0x22,
    EV_NOAMMO_LEFT = 0x23,
    EV_EMPTY_OFFHAND = 0x24,
    EV_RESET_ADS = 0x25,
    EV_RELOAD = 0x26,
    EV_RELOAD_FROM_EMPTY = 0x27,
    EV_RELOAD_START = 0x28,
    EV_RELOAD_END = 0x29,
    EV_RELOAD_START_NOTIFY = 0x2A,
    EV_RELOAD_ADDAMMO = 0x2B,
    EV_RAISE_WEAPON = 0x2C,
    EV_FIRST_RAISE_WEAPON = 0x2D,
    EV_RAISE_PICKUP_WEAPON = 0x2E,
    EV_PUTAWAY_WEAPON = 0x2F,
    EV_WEAPON_ALT = 0x30,
    EV_WEAPON_SWITCH_STARTED = 0x31,
    EV_PULLBACK_WEAPON = 0x32,
    EV_HOLDFIRE_WEAPON = 0x33,
    EV_FIRE_WEAPON_DELAY_START = 0x34,
    EV_FIRE_WEAPON_SCRIPTED = 0x35,
    EV_FIRE_WEAPON = 0x36,
    EV_FIRE_WEAPON_LASTSHOT = 0x37,
    EV_FIRE_WEAPON_LEFT = 0x38,
    EV_FIRE_WEAPON_LASTSHOT_LEFT = 0x39,
    EV_FIRE_WEAPON_BALL_PASS = 0x3A,
    EV_FIRE_RICOCHET = 0x3B,
    EV_FIRE_WEAPON_GRAPPLE = 0x3C,
    EV_RECHAMBER_WEAPON = 0x3D,
    EV_EJECT_BRASS = 0x3E,
    EV_FIRE_MELEE_SWIPE = 0x3F,
    EV_FIRE_MELEE_POWER = 0x40,
    EV_FIRE_MELEE_POWER_LEFT = 0x41,
    EV_FIRE_MELEE = 0x42,
    EV_MELEE_JUKE = 0x43,
    EV_MELEE_JUKE_END = 0x44,
    EV_MELEE_LUNGE = 0x45,
    EV_MELEE_STRUGGLE = 0x46,
    EV_MELEE_COUNTER = 0x47,
    EV_FIRE_MELEE_DELAYED = 0x48,
    EV_MELEE_HIT = 0x49,
    EV_MELEE_HIT_ENT = 0x4A,
    EV_MELEE_HIT_FROM_BEHIND = 0x4B,
    EV_MELEE_MISS = 0x4C,
    EV_MELEE_BLOOD = 0x4D,
    EV_WEAPON_DEPLOYING = 0x4E,
    EV_WEAPON_FINISH_DEPLOYING = 0x4F,
    EV_WEAPON_FINISH_BREAKING_DOWN = 0x50,
    EV_INC_CHARGESHOT_LEVEL = 0x51,
    EV_HOLDFIRE_CANCEL = 0x52,
    EV_HOLDFIRE_FIRED = 0x53,
    EV_PREP_OFFHAND = 0x54,
    EV_USE_OFFHAND = 0x55,
    EV_SWITCH_OFFHAND = 0x56,
    EV_PREP_RIOTSHIELD = 0x57,
    EV_DEPLOY_RIOTSHIELD = 0x58,
    EV_LOWER_RIOTSHIELD = 0x59,
    EV_FIRE_BARREL_1 = 0x5A,
    EV_FIRE_BARREL_2 = 0x5B,
    EV_FIRE_BARREL_3 = 0x5C,
    EV_FIRE_BARREL_4 = 0x5D,
    EV_BULLET_HIT = 0x5E,
    EV_BULLET_HIT_SHIELD = 0x5F,
    EV_BULLET_HIT_CLIENT_SHIELD = 0x60,
    EV_EXPLOSIVE_IMPACT_ON_SHIELD = 0x61,
    EV_EXPLOSIVE_SPLASH_ON_SHIELD = 0x62,
    EV_BULLET_HIT_CLIENT = 0x63,
    EV_DESTRUCTIBLE_BULLET_HIT = 0x64,
    EV_DESTRUCTIBLE_EXPLOSION_HIT = 0x65,
    EV_GRENADE_BOUNCE = 0x66,
    EV_GRENADE_EXPLODE = 0x67,
    EV_ROCKET_EXPLODE = 0x68,
    EV_ROCKET_EXPLODE_NOMARKS = 0x69,
    EV_EXPLODE_ON_CLIENT = 0x6A,
    EV_FLASHBANG_EXPLODE = 0x6B,
    EV_CUSTOM_EXPLODE = 0x6C,
    EV_CUSTOM_EXPLODE_NOMARKS = 0x6D,
    EV_SENSOR_GRENADE_EXPLODE = 0x6E,
    EV_CHANGE_TO_DUD = 0x6F,
    EV_DUD_EXPLODE = 0x70,
    EV_DUD_IMPACT = 0x71,
    EV_MISSILE_REPULSED = 0x72,
    EV_FIRE_EXPLODE = 0x73,
    EV_TIMED_FX = 0x74,
    EV_HIT_MARKER = 0x75,
    EV_FAKE_FIRE = 0x76,
    EV_PLAY_FX = 0x77,
    EV_PLAY_FX_IGNORE_PAUSE = 0x78,
    EV_PLAY_FX_ON_TAG = 0x79,
    EV_PLAY_FX_ON_TAG_IGNORE_PAUSE = 0x7A,
    EV_PLAY_FX_ON_CAMERA = 0x7B,
    EV_PLAY_FX_ON_CAMERA_IGNORE_PAUSE = 0x7C,
    EV_PHYS_EXPLOSION_SPHERE = 0x7D,
    EV_PHYS_EXPLOSION_CYLINDER = 0x7E,
    EV_PHYS_EXPLOSION_JOLT = 0x7F,
    EV_PHYS_JET_THRUST = 0x80,
    EV_PHYS_LAUNCH = 0x81,
    EV_BALLISTIC_TARGET = 0x82,
    EV_CREATE_DYNENT = 0x83,
    EV_CREATE_ROPE = 0x84,
    EV_DETACH_ENTITY = 0x85,
    EV_DELETE_ROPE = 0x86,
    EV_ROPE_COLLIDE_WORLD = 0x87,
    EV_ROPE_COLLIDE_ENT = 0x88,
    EV_ROPE_SETFLAG = 0x89,
    EV_ROPE_SETPARAM = 0x8A,
    EV_ROPE_ADDANCHOR = 0x8B,
    EV_ROPE_REMOVEANCHOR = 0x8C,
    EV_ROPE_MOVEANCHOR = 0x8D,
    EV_EARTHQUAKE = 0x8E,
    EV_SCREEN_SHAKE = 0x8F,
    EV_GRENADE_DROP = 0x90,
    EV_GRENADE_SUICIDE = 0x91,
    EV_DETONATE = 0x92,
    EV_DETONATE_OFFHAND = 0x93,
    EV_MISSILE_REMOTE_BOOST = 0x94,
    EV_MISSILE_REMOTE_BRAKE = 0x95,
    EV_PLAY_RUMBLE_ON_ENT = 0x96,
    EV_PLAY_RUMBLE_ON_POS = 0x97,
    EV_PLAY_RUMBLELOOP_ON_ENT = 0x98,
    EV_PLAY_RUMBLELOOP_ON_POS = 0x99,
    EV_STOP_RUMBLE = 0x9A,
    EV_STOP_ALL_RUMBLES = 0x9B,
    EV_OBITUARY = 0x9C,
    EV_REVIVE_OBITUARY = 0x9D,
    EV_DIRECTIONAL_HIT_INDICATOR = 0x9E,
    EV_ANIMATE_UI = 0x9F,
    EV_KILLSTREAK_DAMAGE = 0xA0,
    EV_ROUND_START_DELAY_HINT = 0xA1,
    EV_NO_FRAG_GRENADE_HINT = 0xA2,
    EV_NO_SPECIAL_GRENADE_HINT = 0xA3,
    EV_GRENADE_NOT_ALLOWED_HINT = 0xA4,
    EV_NO_TARGET_IN_RANGE_HINT = 0xA5,
    EV_TARGET_TOO_CLOSE_HINT = 0xA6,
    EV_TARGET_NOT_ENOUGH_CLEARANCE = 0xA7,
    EV_LOCKON_REQUIRED_HINT = 0xA8,
    EV_FOOTSTEP_SPRINT = 0xA9,
    EV_FOOTSTEP_RUN = 0xAA,
    EV_FOOTSTEP_WALK = 0xAB,
    EV_FOOTSTEP_CROUCHRUN = 0xAC,
    EV_FOOTSTEP_CROUCHWALK = 0xAD,
    EV_FOOTSTEP_PRONE = 0xAE,
    EV_LADDER_HAND_GRAB = 0xAF,
    EV_MANTLE = 0xB0,
    EV_JUMP = 0xB1,
    EV_LEAP = 0xB2,
    EV_TRM = 0xB3,
    EV_WALLRUN_START_LEFT = 0xB4,
    EV_WALLRUN_START_RIGHT = 0xB5,
    EV_WALLRUN_FALL_START = 0xB6,
    EV_WALLRUN_JUMP = 0xB7,
    EV_WALLRUN_END = 0xB8,
    EV_WALLRUN_FALL_END = 0xB9,
    EV_DOUBLEJUMP_DISABLED = 0xBA,
    EV_DOUBLEJUMP_BOOST = 0xBB,
    EV_DOUBLEJUMP_FULLENERGY_BOOST = 0xBC,
    EV_SLIDE_START = 0xBD,
    EV_SOFT_LANDING = 0xBE,
    EV_MEDIUM_LANDING = 0xBF,
    EV_HARD_LANDING = 0xC0,
    EV_DAMAGE_LANDING = 0xC1,
    EV_WATER_LANDING = 0xC2,
    EV_FIRE_VEHICLE_TURRET = 0xC3,
    EV_FIRE_GUNNER_1 = 0xC4,
    EV_FIRE_GUNNER_2 = 0xC5,
    EV_FIRE_GUNNER_3 = 0xC6,
    EV_FIRE_GUNNER_4 = 0xC7,
    EV_FIRE_GUNNER_1A = 0xC8,
    EV_FIRE_GUNNER_2A = 0xC9,
    EV_FIRE_GUNNER_3A = 0xCA,
    EV_FIRE_GUNNER_4A = 0xCB,
    EV_FIRE_GUNNER_1B = 0xCC,
    EV_FIRE_GUNNER_2B = 0xCD,
    EV_FIRE_GUNNER_3B = 0xCE,
    EV_FIRE_GUNNER_4B = 0xCF,
    EV_FIRE_GUNNER_1C = 0xD0,
    EV_FIRE_GUNNER_2C = 0xD1,
    EV_FIRE_GUNNER_3C = 0xD2,
    EV_FIRE_GUNNER_4C = 0xD3,
    EV_START_CAMERA_TWEEN = 0xD4,
    EV_START_CAMERA_TWEEN_QUICK_STOP = 0xD5,
    EV_DESTRUCTIBLE_DISABLE_PIECES = 0xD6,
    EV_FOOTPRINT = 0xD7,
    EV_CANNOTPLANT = 0xD8,
    EV_CANT_EQUIP_PRONE = 0xD9,
    EV_CANT_JUKE = 0xDA,
    EV_CANT_SLAM = 0xDB,
    EV_DTP_LAUNCH = 0xDC,
    EV_DTP_LAND = 0xDD,
    EV_LEAP_LAND = 0xDE,
    EV_SCOPE_ZOOM = 0xDF,
    EV_JAM_WEAPON = 0xE0,
    EV_STACKFIRE = 0xE1,
    EV_MISSILE_STICK = 0xE2,
    EV_BOLT_IMPALE = 0xE3,
    EV_PLAY_WEAPON_DEATH_EFFECTS = 0xE4,
    EV_PLAY_WEAPON_DAMAGE_EFFECTS = 0xE5,
    EV_FACE_EVENT = 0xE6,
    EV_SETLOCALWIND = 0xE7,
    EV_FLOAT_LONGER = 0xE8,
    EV_FORCE_BUOYANCY = 0xE9,
    EV_DISABLE_DEPTH_BUOYANCY_ADJUSTMENTS = 0xEA,
    EV_SCALE_BUOYANCY = 0xEB,
    EV_GIB = 0xEC,
    EV_STANCE_INVALID = 0xED,
    EV_ENTER_VEHICLE = 0xEE,
    EV_EXIT_VEHICLE = 0xEF,
    EV_IMMEDIATE_RAGDOLL = 0xF0,
    EV_ENT_SPAWNED_SENTINEL = 0xF1,
    EV_PROJECTILE_PASS_THROUGH = 0xF2,
    EV_CTX_COVER_START = 0xF3,
    EV_CTX_COVER_END = 0xF4,
    EV_GADGET_0 = 0xF5,
    EV_GADGET_1 = 0xF6,
    EV_GADGET_2 = 0xF7,
    EV_GADGET_0_DENIAL = 0xF8,
    EV_GADGET_1_DENIAL = 0xF9,
    EV_GADGET_2_DENIAL = 0xFA,
    EV_LOCKON_CHANGE = 0xFB,
    EV_TARGETOOR = 0xFC,
    EV_TARGETHIJACKED = 0xFD,
    EV_TARGETNOTVALID = 0xFE,
    EV_NO_TARGETS = 0xFF,
    EV_NO_LOS_TO_TARGET = 0x100,
    EV_TARGET_DISABLED = 0x101,
    EV_TARGETING_ABORTED = 0x102,
    EV_TARGET_ALREADY_TARGETED = 0x103,
    EV_GRAPPLE_BEAM_ON = 0x104,
    EV_GRAPPLE_BEAM_OFF = 0x105,
    EV_LMG_INFINITE_SOUNDS = 0x106,

    /*
      Does not exist in the engine.
    */
    EV_COUNT = 0xEF
  };

  /*
   Usually accessed as a 16-bit value in the engine,
   but always packed as a padded 32-bit value,
   even where alignment does not require it.

   This struct is thus defined accordingly.
  */
  entityType type;
  uint8_t _pad[2];
};
ASSERT_SIZE(entityType_t, 0x4);

constexpr uint32_t ENTITYSTATE_SIZE = 0x1F0;
constexpr uint32_t ENTITYSTATE_LOOPSOUND_OFFSET = 0xDC;
constexpr uint32_t ENTITYSTATE_UN3_OFFSET = 0x130;
constexpr uint32_t ENTITYSTATE_CLIENTMASK_OFFSET = 0x168;
constexpr uint32_t ENTITYSTATE_ETYPE_OFFSET = 0x198;
constexpr uint32_t ENTITYSTATE_OTHERENTITYNUM_OFFSET = 0x1AE;

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
  entityType_t eType;
  uint32_t _unknown19C;
  team_t team;
  int16_t owner;
  int16_t groundEntityNum;
  entityStateIndex index;
  int16_t attackerEntityNum;
  int16_t enemyModel;
  int16_t otherEntityNum;
  weapon::Weapon weapon;
  weapon::Weapon lastStandPrevWeapon;
  scr::ScrString_t targetname;
  int16_t spawnVarIndex;
  uint8_t animtreeIndex;
  uint8_t _padding1C7[1];
  anim::AnimScriptedState animScripted;
  int16_t eventSequence;
  uint8_t surfType;
  uint8_t _padding1DF[1];
  int16_t clientNum;
  uint8_t iHeadIcon;
  entityStateUn1 un1;
  uint32_t _unknown1E4;
  int16_t previousEventSequence;
  uint8_t _padding1EA[6];
};

typedef entityState_s entityState_t;
#pragma pack(pop)
ASSERT_SIZE(entityState_t, ENTITYSTATE_SIZE);
ASSERT_OFFSET(entityState_t, eType, ENTITYSTATE_ETYPE_OFFSET);
ASSERT_OFFSET(entityState_t, number, 0);
ASSERT_OFFSET(entityState_t, loopSound, ENTITYSTATE_LOOPSOUND_OFFSET);
ASSERT_OFFSET(entityState_t, clientMask, ENTITYSTATE_CLIENTMASK_OFFSET);
ASSERT_OFFSET(entityState_t, otherEntityNum, ENTITYSTATE_OTHERENTITYNUM_OFFSET);
ASSERT_OFFSET(entityState_t, un3, ENTITYSTATE_UN3_OFFSET);

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
  DEFAULT = 0x0,
  WALK = 0x1,
  SPRINT = 0x2,
  SWIM = 0x3,
  SWIMSPRINT = 0x4,
  SLIDE = 0x5,
  COUNT = 0x6,
};

struct PlayerLens {
  PlayerLensState lensState;
  float focalLength;
  float focalDistance;
  float fStop;
};
ASSERT_SIZE(PlayerLens, 0x10);

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
  vehicle::PlayerVehicleState vehicleState;
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
ASSERT_OFFSET(playerState_s, _paddingBB5, 0xBB5);
ASSERT_OFFSET(playerState_s, _paddingBBB, 0xBBB);
ASSERT_OFFSET(playerState_s, _paddingB58, 0xB58);
ASSERT_OFFSET(playerState_s, _padding6BCB, 0x6BCB);
ASSERT_SIZE(playerState_s, 0xB566);
typedef playerState_s playerState_t;
#pragma pack(pop)

struct score_s {
  int32_t ping;
  int32_t status_icon;
  int32_t place;
  int32_t score;
  int32_t kills;
  int32_t assists;
  int32_t deaths;
  int32_t scoreboardColumns[9];
  int32_t downs;
  int32_t revives;
  int32_t headshots;
  int32_t scoreMultiplier;
  int32_t currentStreak;
};
typedef score_s score_t;
ASSERT_SIZE(score_t, 0x54);

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
  playerName_t name; // Verified
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
  clanAbbrev_t clanAbbrev; // Verified
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
  lobby::LobbyClientPool<int32_t> lastUpdateIndex;
};
typedef clientState_s clientState_t;
ASSERT_OFFSET(clientState_t, name, 0x2C);
ASSERT_OFFSET(clientState_t, clanAbbrev, 0x25C);
ASSERT_SIZE(clientState_t, 0x338); // Correct
#pragma pack(pop)

#pragma pack(push, 1)
struct actorState_t {
  int32_t actorIndex;
  int32_t entityNum;
  int32_t modelindex;
  uint32_t attachIgnoreCollision;
  actorName_t name;
  int32_t hudwarningType;
  int32_t lookAtEntNum;
  vec3_t lookAtPos;
  qboolean lookAtAiming;
  qboolean lookAtNoTorso;
  qboolean gibbed;
};
ASSERT_SIZE(actorState_t, 0x50); // Correct
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
ASSERT_SIZE(TopScorer, 0x30);
#pragma pack(pop)

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
  lobby::LobbyClientPool<int32_t> lastUpdateIndex;
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

#pragma pack(push, 1)
constexpr uint32_t MATCHSTATE_SIZE = 0x2A0;
partial_def(MATCHSTATE_SIZE, struct, MatchState, {
  uint32_t index;
  uint8_t _padding004[12];
  UnarchivedMatchState unarchivedState;
  ArchivedMatchState archivedState;
  uint8_t _padding28C[4];
});
ASSERT_SIZE(MatchState, MATCHSTATE_SIZE); // Correct
#pragma pack(pop)

} // namespace level
} // namespace game