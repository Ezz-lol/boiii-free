#pragma once
#include <cstdint>
#include "../core.hpp"
#include "../scr/scr.hpp"
#include "../snd/snd.hpp"
#include "../vehicle.hpp"
#include "../ui/ui.hpp"
#include "../sv.hpp"
#include "core.hpp"

namespace game {
namespace level {
namespace cl {

struct cLinkInfo_t {
  vec3_t axis[4];
  int32_t linkEnt;
  scr::ScrString_t linkTag;
  vec3_t relativeAngles;
  union {
    uint32_t packed_bits;
    struct {
      uint32_t relativeAnglesSet : 1;
      uint32_t identityLinkto : 1;
    };
  };
};

struct cgVehicleWheelEffect {
  int32_t surfaceType;
  int32_t lastSurfaceType;
};

enum class TraceHitType : uint32_t {
  NONE = 0x0,
  ENTITY = 0x1,
  DYNENT_MODEL = 0x2,
  DYNENT_BRUSH = 0x3,
  GLASS = 0x4,
};

#pragma pack(push, 1)
class vehicle_cache_t {
public:
  vec3_t lastOrigin;
  vec3_t lastAngles;
  int32_t hit_indices[6];
  int32_t hit_sflags[6];
  vec3_t hit_normals[6];
  float hit_fractions[6];
  TraceHitType hit_type[6];
  uint16_t hit_id[6];
  uint8_t _paddingCC[4];
  vehicle::vehicle_proximity_data_t proximity_data;
  int32_t wheel_mask;
  uint8_t _padding1424[12];
};
#pragma pack(pop)

struct Engine {
  float throttle;
  float radPS;
  float lastRadPS;
  float outputTq;
  float inertia;
  float differentialRatio;
  float load;
  float targetShiftRadPS;
  float speedSamples[20];
  float speedSamplesSum;
  int32_t speedSampleIndex;
  int32_t currentGear;
  int32_t nextGear;
  int32_t clutchState;
  float clutchTimer;
};

struct VehicleRotorArm {
  int16_t pitch;
  int16_t roll;
};

struct __attribute__((aligned(8))) cgVehicle_t {
  struct {
    float pitch;
    float yaw;
    int32_t lastFireTime;
  } prevTurretsState[5];
  int32_t wheelSurfType[6];
  int32_t fireTime;
  int32_t compassPingTime;
  int32_t flags;
  float materialTime;
  float materialTime2;
  vehicle_cache_t *vehicle_cache;
  cgVehicleWheelEffect wheelEffects[3];
  scr::ScrString_t notes[2];
  int32_t driveBySoundIndex[2];
  float driveBySoundDelay[2];
  float driveBySoundTimeout[2];
  float prevAngularVelocity[5];
  Engine engine;
  float minigunRotationSpeed;
  vec3_t gunSoundOrigins[5];
  uint8_t wasFiring;
  bool wasAds;
  vec3_t antennaPos[4];
  vec3_t antennaVel[4];
  vec3_t antennaParentBoneWorldAxis[4][3];
  vec3_t antennaRelativePos[4];
  vec3_t poseAxis[4];
  int32_t surfaceSoundTime;
  ai::AIBody aiBody;
  VehicleRotorArm rotors[8];
  float infraredFadeOut;
};

struct cached_client_tag_t {
  int32_t time;
  vec3_t lastLocalTagOrigin;
  bool visible;
  int32_t predictedTime;
  vec3_t predictedPosition;
  vec3_t predictedAngles;
};

struct ClientCache {
  cached_client_tag_t aim_head_tag;
  cached_client_tag_t aim_highest_tag;
  cached_client_tag_t tag_flash;
};

struct cgTagCacheItem {
  vec3_t tagLocalMat[4];
  int32_t timeStamp;
  uint16_t entNum;
  int32_t entUseCount;
  scr::ScrString_t tagName;
};
struct cgScriptMover_t {
  float decelTime;
  float aDecelTime;
  float speed;
  float aSpeed;
  float midTime;
  float aMidTime;
  vec3_t pos1;
  vec3_t pos2;
  vec3_t pos3;
  vec3_t apos1;
  vec3_t apos2;
  vec3_t apos3;
  float waterDisturbanceForce;
  float waterDisturbanceLength;
  float waterDisturbanceWidth;
  float waterDisturbanceDamping;
  float waterDisturbanceAngle;
};

struct cgZBarrierPiece;
struct centity_t;
struct cg_t;
struct ZBarrierDef;
struct LerpEntityStateZBarrierPiece;
typedef fastcall_t<void(LocalClientNum_t localClientNum, cg_t *cGameGlob,
                        centity_t *cent, ZBarrierDef *def,
                        cgZBarrierPiece *piece,
                        LerpEntityStateZBarrierPiece *lerp, uint32_t)>
    zbarrierPieceLogicPtr_t;

#pragma pack(push, 1)
struct cgZBarrierPiece {
  int32_t index;
  uint8_t _padding04[4];
  centity_t *pPiece;
  db::xasset::XModel *curXModel;
  zbarrierPieceLogicPtr_t pStateLogic;
  int32_t weaponLookahead[4];
  int32_t timeVal;
  int32_t weaponEntNum;
  uint8_t state;
  uint8_t prevState;
  uint8_t subState;
  uint8_t nextSubState;
  uint8_t curAnim;
  bool flagsChanged;
  uint8_t _padding3E[2];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct cgZBarrier_t {
  bool zbInUse;
  bool forceUpdateForDemoJump;
  uint8_t _padding02[2];
  cgZBarrierPiece pieces[6];
};
#pragma pack(pop)

struct CEntityModelAttachment {
  scr::ScrString_t model;
  scr::ScrString_t tag;
};

struct __attribute__((aligned(2))) OverheadFade {
  int32_t nextCheckTime;
  int32_t lastTime;
  int32_t startTime;
  bool visible;
  bool traceVisible;
  bool draw;
};

struct __attribute__((aligned(8))) centity_overheadName_t {
  char drawname[48];
  OverheadFade fade;
  centity_t *cent;
  bool useOwnersName;
};

struct ArmPulse {
  float duration;
  float value;
  float speed;
  snd::SndAliasId pulseSound;
};
enum class IdentitySightState : int32_t {
  NORMAL = 0x0,
  FRIENDLY = 0x1,
  ENEMY = 0x2,
};

struct ClientLoopSoundInfo {
  snd::LoopSoundInfo loopSound;
  snd::SndPlaybackId handle;
};
ASSERT_SIZE(ClientLoopSoundInfo, 0xC);

struct clientControllers_t {
  vec3_t angles[6];
  vec3_t tag_origin_angles;
  vec3_t tag_origin_offset;
};

#pragma pack(push, 1)
struct CEntPlayerInfo {
  clientControllers_t *control;
  BoneIndex tag[6];
  float waterHeight;
  int32_t nextWaterHeightCheck;
  int32_t nextRippleTime;
  vec3_t viewangles;
  uint8_t _padding2C[4];
};
ASSERT_SIZE(CEntPlayerInfo, 0x30);
#pragma pack(pop)

struct CEntVehicleDefGround {
  float wheelHeight[6];
  float nitrousWheelRotation[6];
  int16_t nitrousWheelYaw[6];
  BoneIndex tag_extra_tank_wheels[4];
};
struct CEntVehicleRotorRotor {
  float rotation;
  float speed;
};

struct CEntVehicleRotorArm {
  int16_t rotationRoll;
  int16_t rotationPitch;
};

struct CEntVehicleRotor {
  uint16_t type;
  BoneIndex tag;
  union {
    CEntVehicleRotorRotor rotor;
    CEntVehicleRotorArm arm;
  };
};

struct CEntVehicleDefAir {
  CEntVehicleRotor rotor[8];
};

struct CEntVehicleDef {
  struct {
    int16_t yaw;
    int16_t pitch;
    BoneIndex tag_turret;
    BoneIndex tag_barrel;
  } turrets[5];
  float barrelRecoil;
  int16_t pitch;
  int16_t roll;
  int16_t steerYaw;
  int16_t steerPitch;
  int16_t extraSteerPitch;
  int16_t minigunRotation;
  int16_t antennaPitch[4];
  int16_t antennaYaw[4];
  int16_t antennaRoll[4];
  float waterHeight;
  int32_t nextWaterHeightCheck;
  int32_t nextRippleTime;
  BoneIndex wheelBoneIndex[6];
  BoneIndex tag_body;
  BoneIndex tag_barrel_recoil;
  BoneIndex tag_player;
  BoneIndex tag_minigun_spin;
  BoneIndex tag_antennas[4];
  union {
    CEntVehicleDefGround ground;
    CEntVehicleDefAir air;
  };
};
ASSERT_SIZE(CEntVehicleDef, 0xD8);

struct AimTargetCache {
  int32_t lastUpdateTime;
  float targetHeight;
};

struct canimscripted_t {
  vec3_t axis[4];
  animationNumber_t anim;
  uint8_t bStarted;
  uint8_t bEntityLinked;
  vec3_t originError;
  float originErrorReductionSpeed;
  vec3_t anglesError;
  vec3_t anglesErrorReductionSpeed;
  int32_t lastErrorUpdateTime;
  int32_t lerpTime;
};

enum class CharacterVoice : uint32_t {
  AMERICAN = 0x0,
  RUSSIAN = 0x1,
  UNITA = 0x2,
  CUBAN = 0x3,
  MUJAHIDEEN = 0x4,
  PDF = 0x5,
  PMC = 0x6,
  ISI = 0x7,
  DIGBAT = 0x8,
  YEMENI = 0x9,
  TERRORIST = 0xA,
  LAPD = 0xB,
  SECRETSERVICE = 0xC,
  MP = 0xD,
  CHINESE = 0xE,
  COUNT = 0xF,
};

enum class CharacterSkeleton : uint32_t {
  BASE = 0x0,
  SCALED80 = 0x1,
  PROP80 = 0x2,
  COUNT = 0x3,
};

struct Character {
  const char *name;
  CharacterSkeleton skeleton;
  CharacterVoice voice;
  db::xasset::ScriptBundlePtr destructibleDef;
  db::xasset::ScriptBundlePtr gibDef;
  db::xasset::ScriptBundlePtr fxDef;
  db::xasset::ScriptBundlePtr deathFxDef;
  db::xasset::ScriptBundlePtr altFxDef1;
  db::xasset::ScriptBundlePtr altFxDef2;
  db::xasset::ScriptBundlePtr altFxDef3;
  db::xasset::ScriptBundlePtr altFxDef4;
  db::xasset::ScriptBundlePtr altFxDef5;
  db::xasset::ScriptBundlePtr altFxDef6;
  db::xasset::XModelPtr body;
  db::xasset::XModelAliasPtr bodyAlias;
  db::xasset::XModelPtr head;
  db::xasset::XModelAliasPtr headAlias;
  db::xasset::XModelAliasPtr headAliasZombie;
  db::xasset::XModelPtr hat;
  db::xasset::XModelAliasPtr hatAlias;
  db::xasset::XModelPtr gear;
  db::xasset::XModelAliasPtr gearAlias;
  db::xasset::XModelPtr viewModel;
  db::xasset::XModelPtr shadowCharacter;
  db::xasset::XModelPtr damagedTorso_Clean;
  db::xasset::XModelAliasPtr damagedTorso_Clean_Alias;
  db::xasset::XModelPtr damagedTorso_RightArmGone;
  db::xasset::XModelAliasPtr damagedTorso_RightArmGone_Alias;
  db::xasset::XModelPtr damagedTorso_LeftArmGone;
  db::xasset::XModelAliasPtr damagedTorso_LeftArmGone_Alias;
  db::xasset::XModelPtr damagedTorso_GutsGone;
  db::xasset::XModelAliasPtr damagedTorso_GutsGone_Alias;
  db::xasset::XModelPtr damagedTorso_HeadGone;
  db::xasset::XModelAliasPtr damagedTorso_HeadGone_Alias;
  db::xasset::XModelPtr damagedLegs_Clean;
  db::xasset::XModelAliasPtr damagedLegs_Clean_Alias;
  db::xasset::XModelPtr damagedLegs_RightLegGone;
  db::xasset::XModelAliasPtr damagedLegs_RightLegGone_Alias;
  db::xasset::XModelPtr damagedLegs_LeftLegGone;
  db::xasset::XModelAliasPtr damagedLegs_LeftLegGone_Alias;
  db::xasset::XModelPtr damagedLegs_NoLegs;
  db::xasset::XModelAliasPtr damagedLegs_NoLegs_Alias;
  db::xasset::XModelPtr chestGear_Right;
  bool chestGear_Right_Required;
  db::xasset::XModelPtr chestGear_Left;
  bool chestGear_Left_Required;
  db::xasset::XModelPtr frontPack;
  bool frontPack_Required;
  db::xasset::XModelPtr frontPackLow;
  bool frontPackLow_Required;
  db::xasset::XModelPtr backPack;
  bool backPack_Required;
  db::xasset::XModelPtr rearGear_Right;
  bool rearGear_Right_Required;
  db::xasset::XModelPtr rearGear_Left;
  bool rearGear_Left_Required;
  db::xasset::XModelPtr rearGearSide_Right;
  bool rearGearSide_Right_Required;
  db::xasset::XModelPtr rearGearSide_Left;
  bool rearGearSide_Left_Required;
  db::xasset::XModelPtr sideGear_Right;
  bool sideGear_Right_Required;
  db::xasset::XModelPtr sideGear_Left;
  bool sideGear_Left_Required;
  db::xasset::XModelPtr frontGear_Right;
  bool frontGear_Right_Required;
  db::xasset::XModelPtr frontGear_Left;
  bool frontGear_Left_Required;
  db::xasset::XModelPtr helmet;
  bool helmet_Required;
  db::xasset::XModelPtr misc1;
  bool misc1_Required;
  db::xasset::XModelPtr misc2;
  bool misc2_Required;
  db::xasset::XModelPtr misc3;
  bool misc3_Required;
  phys::RagdollDef *ragdollDef;
  const char *eyeGlowFX;
  const char *eyeGlowBone;
  int32_t headImpactType;
  int32_t bodyImpactType;
};

typedef Character *CharacterPtr;
typedef CharacterPtr CharacterHandle;

#pragma pack(push, 1)
struct GfxEntityPreFrame {
  uint32_t activePartBits[12];
  int32_t modelCount;
  int32_t numBones;
  int32_t numCosmeticBones;
  int32_t boneArrayOffset;
  uint32_t sanimIndex;
  uint32_t sanimStateOffset;
  uint32_t blendShapeFrame;
  uint32_t blendShapeVertOffset;
  uint32_t blendShapeVertOffsetLast;
  uint8_t _padding54[4];
  db::xasset::XSurfaceShared *blendShapeVertOffsetLastShared;
};
ASSERT_SIZE(GfxEntityPreFrame, 0x60);
#pragma pack(pop)

struct CEntFx {
  int32_t triggerTime;
  db::xasset::FxUniqueHandle effect;
};
ASSERT_SIZE(CEntFx, 0x8);
#pragma pack(push, 1)
struct CEntActorInfo {
  int32_t proneType;
  float pitch;
  float roll;
  BoneIndex headBone;
  uint8_t _padding0E[2];
  union {
    float height;
    float yaw;
  };
  float waterHeight;
  int32_t nextWaterHeightCheck;
  int32_t nextRippleTime;
  float lookAtBlend;
  uint8_t lookAtFlags;
  uint8_t _padding25[3];
};
ASSERT_SIZE(CEntActorInfo, 0x28);
#pragma pack(pop)

struct CEntGeneral {
  contents_t contents;
  qboolean linked;
  vec3_t mins;
  vec3_t maxs;
  int32_t triggerflags;
};
ASSERT_SIZE(CEntGeneral, 0x24);

#pragma pack(push, 1)
struct cpose_t {
  entityType8_t eType;
  entityType8_t eTypePrev;
  LocalClientNum8_t localClientNum;
  bool isRagdoll;
  uint16_t sceneModelSeed;
  uint8_t _padding06[2];
  intptr_t ragdollHandle;
  phys::PhysObjId physObjId;
  phys::PhysObjId physUserBody;
  intptr_t killcamRagdollHandle;
  phys::PhysObjId physUserBodyProneFeet;
  uint8_t physConstraintPoseID;
  uint8_t _padding31[7];
  PosedEntity_PoseHandle instancePoseHandle;
  vec3_t origin;
  vec3_t angles;
  vec3_t absmin;
  vec3_t absmax;
  GfxEntityPreFrame prevFrame;
  union {
    CEntPlayerInfo player;
    CEntVehicleDef vehicle;
    CEntFx fx;
    CEntActorInfo actor;
    CEntGeneral general;
  };
  gfx::ShaderConstantSet constantSet;
  bool skipControllers;
  uint8_t _padding221[3];
};
ASSERT_SIZE(cpose_t, 0x330);
#pragma pack(pop)

enum class FaceNotifyEvent : uint64_t {
  CASUAL = 0x0,
  ALERT = 0x1,
  CQB = 0x2,
  RUNNING = 0x3,
  SHOOT_SINGLE = 0x4,
  SHOOT_BURST = 0x5,
  MELEE = 0x6,
  REACT = 0x7,
  TALK = 0x8,
  TALK_LONG = 0x9,
  PAIN = 0xA,
  DEATH = 0xB,
  NONE = 0xC
};

#pragma pack(push, 1)
struct centity_t {
  cpose_t pose;
  LerpEntityState prevState;
  entityState_t nextState;
  union {
    int32_t miscTime;
    int32_t nameVisTime;
  };
  int32_t spawnTime;
  int32_t lastMuzzleFlash;
  int32_t numShotsFiredLast;
  bool attachWeaponModel;
  uint8_t _padding601[7];
  anim::XAnimTree *tree;
  phys::Destructible *destructible;
  vehicle::NitrousVehicle *nitrousVeh;
  cLinkInfo_t *linkInfo;
  cgVehicle_t *vehicle;
  ClientCache *clientCache;
  cgTagCacheItem *tagCacheArray;
  AimTargetCache *aimTargetInfo;
  cgScriptMover_t *cScriptMover;
  canimscripted_t *scripted;
  cgZBarrier_t *zbarrier;
  weapon::DroppedWeaponDobjInfo *droppedWeaponDobjInfo;
  Character *character;
  CEntityModelAttachment attachments[8];
  db::xasset::FxUniqueHandle worldPersistentEffectHandle;
  uint8_t _padding6B4[4];
  db::xasset::TagFxSetHandles worldPersistentEffectSetHandles;
  db::xasset::MaterialHandle compassMaterial;
  int32_t lastTrailTime;
  db::xasset::FxUniqueHandle fxTrailHandle;
  db::xasset::FxUniqueHandle fxProjExplosion;
  db::xasset::FxUniqueHandle fxHeartbeat;
  db::xasset::FxUniqueHandle fxLaserSight;
  uint8_t _padding704[4];
  db::xasset::ManagedNoteTrackList managedNotetracks;
  vec3_t oldLinkOrigin;
  uint8_t tracerDrawRateCounter;
  uint8_t _padding80D[3];
  centity_t *updateDelayedNext;
  scr::ScrString_t classname;
  uint32_t stepSound;
  uint8_t footstepSurfaceOverride;
  char tmodeVisibilityEntity;
  int16_t tmodeFlags;
  int16_t tmodeTacscanTime;
  uint16_t duplicateRenderIndex;
  anim::AnimScriptedState animScripted;
  uint16_t animScriptedOverrideIndex;
  uint8_t _padding83E[2];
  anim::TraversalState traversalState;
  uint8_t corpseIndex;
  uint8_t _padding89D[3];
  centity_overheadName_t *overheadName;
  ArmPulse armPulse;
  IdentitySightState identitySight;
  ClientLoopSoundInfo loopSounds[4];
  int32_t lastAimTargetVisCheckTime;
  int32_t lastMeleeTargetClearanceCheckTime;
  union {
    uint32_t packed_bits[3];
    struct {
      uint32_t applyLeftHandIK : 1;
      uint32_t nextValid : 1;
      uint32_t bMuzzleFlash : 1;
      uint32_t bMuzzleFlashLeft : 1;
      uint32_t bMuzzleFlashLastShot : 1;
      uint32_t bShellEject : 1;
      uint32_t bShellEjectLeft : 1;
      uint32_t bStartedEffects : 1;
      uint32_t bPlayedWhizby : 1;
      uint32_t skipBloodImpacts : 1;
      uint32_t heroBloodImpact : 1;
      uint32_t scriptThreaded : 1;
      uint32_t didOverheatFx : 1;
      uint32_t leftFootstep : 1;
      uint32_t rightFootstep : 1;
      uint32_t leftRearFootstep : 1;
      uint32_t rightRearFootstep : 1;
      uint32_t bIsOneShot : 1;
      uint32_t bEntityOnRadar : 1;
      uint32_t bFloatLonger : 1;
      uint32_t bForceBuoyancy : 1;
      uint32_t bRenderedLastFrame : 1;
      uint32_t bIgnoreRagdoll : 1;
      uint32_t bUpdatedSinceReset : 1;
      uint32_t bUpdateToggle : 1;
      uint32_t bZombieEyeGlowHasBeenTurnedOn : 1;
      uint32_t bIsOnRadar : 1;
      uint32_t bAllowPitchAngle : 1;
      uint32_t bFakeEntityMoving : 1;
      uint32_t bFakeEntityTurning : 1;
      uint32_t bFakeEntityRemoved : 1;
      uint32_t partOfZBarrier : 1;
      uint32_t linkRelativeAngles : 1;
      uint32_t sndTurretWasFiring : 1;
      uint32_t clientMainThreaded : 1;
      uint32_t renderInThirdPersonSpectate : 1;
      uint32_t forceNotSimple : 1;
      uint32_t bForceNameVisible : 2;
      uint32_t bWaterDisturbance : 1;
      uint32_t bIsHiddenOnClient : 1;
      uint32_t bNotifyOnBulletImpact : 1;
      uint32_t bFakeAi : 1;
      uint32_t bHideWhenAnimationEnds : 1;
      uint32_t bAimTargetVisible : 1;
      uint32_t bAimTargetMeleeClearance : 1;
      uint32_t doNotDelayNotetrackEvents : 1;
      uint32_t bHiddenFromRadarOutsideVisionCircle : 1;
    };
  };
};
ASSERT_SIZE(centity_t, 0x900); // correct
ASSERT_OFFSET(centity_t, tmodeVisibilityEntity, 0x821);
ASSERT_OFFSET(centity_t, tmodeFlags, 0x822);
ASSERT_OFFSET(centity_t, miscTime, 0x5F0);
ASSERT_OFFSET(centity_t, loopSounds, 0x8BC);
ASSERT_OFFSET(centity_t, lastAimTargetVisCheckTime, 0x8EC);
#pragma pack(pop)

struct ExtentBounds {
  vec3_t mins;
  vec3_t maxs;
};

struct OrientedExtentBounds {
  ExtentBounds bounds;
  vec3_t origin;
  vec3_t axis[3];
};

struct __attribute__((aligned(16))) snapshot_s {
  int32_t snapFlags;
  int32_t ping;
  int32_t serverTime;
  int32_t physicsTime;
  playerState_t ps;
  int32_t numEntities;
  int32_t numClients;
  int32_t numActors;
  entityState_t entities[1024];
  lobby::LobbyClientPool<clientState_t> clients;
  actorState_t actors[64];
  MatchState matchState;
  CasterState casterState;
  lobby::LobbyClientPool<CasterClientState> casterClients;
  uint16_t entIndices[1024];
  int32_t serverCommandSequence;
};

typedef snapshot_s snapshot_t;

struct playerEntity_t {
  float fLastWeaponPosFrac;
  bool bPositionToADS;
  vec3_t vPositionLastOrg;
  float fLastIdleFactor;
  vec3_t baseMoveOrigin;
  vec3_t baseMoveAngles;
};

enum class ColorVisionDeficiencies : uint32_t {
  OFF = 0x0,
  DEUTERANOMALY = 0x1,
  PROTANOMALY = 0x2,
  TRITANOMALY = 0x3,
  COUNT = 0x4,
};

enum class SettingTeamIndicator : uint32_t {
  FULL = 0x0,
  ABBREVIATED = 0x1,
  ICON = 0x2,
  COUNT = 0x3,
};

struct HitMarker {
  int32_t timeToPlay;
  int32_t frameToPlay;
  snd::SndAliasId sound;
  int32_t damageFeedback;
};

struct __attribute__((aligned(8))) objectiveMapInfo_t {
  gfx::GfxImage *icon;
  float iconSize;
  union {
    uint32_t rgba;
    uint8_t bytes[4];
  };
  struct {
    uint8_t ignoreEntity : 1;
    uint8_t rotateWithEntity : 1;
    uint8_t flipMaterial : 1;
    uint8_t clipToMap : 1;
    uint8_t colorMaterialInCode : 1;
    uint8_t stencilToPartialMap : 1;
    uint8_t pulseIcon : 1;
  };
};

enum class InvalidCmdHintType : uint32_t {
  NONE = 0x0,
  NO_AMMO_BULLETS = 0x1,
  NO_AMMO_FRAG_GRENADE = 0x2,
  NO_AMMO_SPECIAL_GRENADE = 0x3,
  NO_AMMO_FLASH_GRENADE = 0x4,
  NO_AMMO_EQUIPMENT = 0x5,
  STAND_BLOCKED = 0x6,
  CROUCH_BLOCKED = 0x7,
  TARGET_TOO_CLOSE = 0x8,
  LOCKON_REQUIRED = 0x9,
  NOT_ENOUGH_CLEARANCE = 0xA,
  CANT_PLACE_TURRET = 0xB,
  CANT_EQUIP_WHILE_PRONE = 0xC,
  CANT_PLANT_EQUIPMENT = 0xD,
  ROUND_START_DELAY = 0xE,
  GRENADES_NOT_ALLOWED = 0xF,
  GADGET_DENIED_ALREADY_ACTIVE = 0x10,
  GADGET_DENIED_CANNOT_ACTIVATE = 0x11,
  TARGET_OUT_OF_RANGE = 0x12,
  TARGET_BEING_USED = 0x13,
  TARGET_INVALID_TYPE = 0x14,
  NO_TARGET_IN_RANGE = 0x15,
  TARGET_DISABLED = 0x16,
  TARGETING_ABORTED = 0x17,
  TARGET_ALREADY_TARGETED = 0x18,
  SYSTEM_DISABLED = 0x19,
  NO_LOS = 0x1A,
  COUNT = 0x1B
};

struct viewDamage_t {
  int32_t time;
  int32_t duration;
  vec_t yaw;
};
struct viewAwareness_t {
  int32_t time;
  int32_t duration;
  vec_t yaw;
  db::xasset::MaterialHandle material;
};
struct viewDirectionalHitIndicator_t {
  int32_t time;
  vec3_t entOrigin;
};

enum class ShockViewTypes : int32_t {
  BLURRED = 0x0,
  FLASHED = 0x1,
  CONCUSSED = 0x2,
  SHOCKED = 0x3,
  NONE = 0x4,
  COUNT = 0x5,
};

struct ShellshockScreenBlend {
  int blurredFadeTime;
  int blurredEffectTime;
  int flashWhiteFadeTime;
  int flashShotFadeTime;
  ShockViewTypes type;
};

struct ShellshockView {
  int fadeTime;
  int kickPeriod;
  float kickRadius;
};

struct __attribute__((aligned(8))) ShellshockSound {
  bool affect;
  snd::SndAliasId loop;
  snd::SndAliasId loopSilent;
  snd::SndAliasId end;
  snd::SndAliasId endAbort;
  const char *roomtype;
  const char *snapshot;
  int fadeInTime;
  int fadeOutTime;
  float drylevel;
  float wetlevel;
  int modEndDelay;
  int loopFadeTime;
  int loopEndDelay;
};
struct ShellshockLookControl {
  bool affect;
  bool disableAimAssistLockOn;
  bool disableAimAssistSlowDown;
  int fadeInTime;
  int fadeOutTime;
  float mouseSensitivity;
  float maxPitchSpeed;
  float maxYawSpeed;
};
struct ShellshockVisionSet {
  const char *name;
  int inTime;
  int outTime;
};
struct __attribute__((aligned(4))) ShellshockMovement {
  float scale;
  bool cancelsMovement;
};
struct ShellshockAnimation {
  bool affect;
};

struct __attribute__((aligned(8))) ShellshockParams {
  const char *name;
  int duration;
  ShellshockScreenBlend screenBlend;
  ShellshockView view;
  ShellshockSound sound;
  ShellshockLookControl lookControl;
  ShellshockVisionSet visionSet;
  ShellshockMovement movement;
  ShellshockAnimation animation;
};

struct statusEffect_t {
  const ShellshockParams *parms;
  int32_t startTime;
  int32_t duration;
};

struct __attribute__((aligned(8))) shellshock_t {
  statusEffect_t statusEffects[3];
  int32_t startTime;
  int32_t duration;
  int32_t loopEndTime;
  vec2_t viewDelta;
  bool hasSavedScreen;
  bool visionSetStarted;
  snd::SndStringHash loopSound;
};
struct fancycam_t {
  int32_t type;
  int32_t postfx;
  vec3_t cameraOffset;
  float wobbleScale;
  int32_t kickPeriod;
  float kickRadius;
  int32_t startTime;
  float fstop;
  float focalDistance;
  float focalLength;
  int32_t lastState;
};

enum class KillCamEntityType : uint32_t {
  NO_ENTITY = 0x0,
  HELICOPTER = 0x1,
  DESTRUCTIBLE = 0x2,
  SCRIPTED = 0x3,
  EXPLOSIVE = 0x4,
  FAST_EXPLOSIVE = 0x5,
  DOG = 0x6,
  COMBAT_ROBOT = 0x7,
  ARTILLERY = 0x8,
  VEHICLE = 0x9,
  COUNT = 0xA,
};

enum class KillCamEntityRestState : uint32_t {
  MOVING = 0x0,
  AT_REST = 0x1,
  STUCK_GROUND = 0x2,
  STUCK_WALL = 0x3,
  COUNT = 0x4
};

struct visionSetVars_t {
  float vkTT;
  float vkTS;
  vec4_t vkTC;
  vec3_t vkTO;
  vec3_t vkRGB0;
  float vkL0;
  float vkM0;
  vec3_t vkRGB1;
  float vkL1;
  float vkM1;
  vec3_t vkRGB2;
  float vkL2;
  vec3_t vkRGB3;
  float vkL3;
  float vkM3;
  vec3_t vkRGB4;
  float vkL4;
  float vkM4;
  float vkRM;
  float filmLut;
  bool reviveEnable;
  float reviveEdgeColorTemp;
  float reviveEdgeSaturation;
  vec3_t reviveEdgeScale;
  vec3_t reviveEdgeContrast;
  vec3_t reviveEdgeOffset;
  float reviveEdgeMaskAdjust;
  float reviveEdgeAmount;
  float masterRingmod;
  float reverbRingmod;
  float hiFilter;
  float lowFilter;
  vec4_t sunFlareTint;
  float atmoBlurNS;
  float atmoBlurHD;
  float atmoBlurBA;
  char name[64];
};

enum class visionSetLerpStyle_t : uint32_t {
  UNDEFINED = 0x0,
  NONE = 0x1,
  TO_LINEAR = 0x2,
  TO_SMOOTH = 0x3,
  BACKFORTH_LINEAR = 0x4,
  BACKFORTH_SMOOTH = 0x5,
  CUSTOM_VALUE = 0x6,
};

struct visionSetLerpData_t {
  int32_t timeStart;
  int32_t timeDuration;
  float lerpFraction;
  visionSetLerpStyle_t style;
};

struct bgs_t {
  scr::bg::bgsAnim_t *animData;
  scr::bg::bgsClientFields_t *clientFields;
  int32_t time;
  int32_t latestSnapshotTime;
  int32_t frametime;
  int32_t deltaTime;
  int32_t anim_user;
  lobby::LobbyClientPool<int32_t> proneTime;
  lobby::LobbyClientPool<game::sv::clientInfo_t> clientinfo;
  user::actorInfo_t actorinfo[64];
  scr::bg::bgs_clientfieldapi_t clientFieldAPI;
  db::xasset::XModel *(*GetXModel)(const char *);
  void (*CreateDObj)(anim::DObjModel *, uint16_t, anim::XAnimTree *, int32_t,
                     const LocalClientNum_t, game::sv::clientInfo_t *);
  uint16_t (*AttachWeapon)(anim::DObjModel *, uint16_t, entityState_t *,
                           game::sv::clientInfo_t *, bool);
  uint16_t (*AttachStowedWeapon)(const LocalClientNum_t, anim::DObjModel *,
                                 uint16_t, entityState_t *,
                                 weapon::WeaponDobjInfo *, weapon::Weapon,
                                 uint16_t *);
  anim::DObj *(*GetDObj)(int, LocalClientNum_t);
  void (*SafeDObjFree)(int, LocalClientNum_t);
  void *(*AllocXAnim)(size_t);
  void (*AnimCmdAddDependent)(short, short);
  int32_t (*GetEnemyEntNum)(int32_t, LocalClientNum_t);
  ai::AIBody *(*ASMGetAIBody)(int32_t, LocalClientNum_t);
  void (*ASMAnimUpdated)(int32_t);
  void (*ASMSetFiringFlag)(int32_t, bool);
  bool (*ASMGetSingleAnim)(ai::AIBody *, int32_t, LocalClientNum_t,
                           scr::ScrString_t, scr::ScrString_t, int32_t *,
                           int32_t *, int32_t *, const bool);
  bool (*ASMGetPrimaryDeltaAnimation)(ai::AIBody *, int32_t, LocalClientNum_t);
  bool (*ASMGetParametricAnimGroup)(ai::AIBody *, int32_t, LocalClientNum_t);
  bool (*ASMGetAimAnims)(ai::AIBody *, int32_t, LocalClientNum_t);
  bool (*ASMGetShootAnims)(ai::AIBody *, int32_t, LocalClientNum_t);
  bool (*ASMShootLayerShouldBeActive)(const ai::AIBody *const, int32_t,
                                      LocalClientNum_t);
  bool (*ASMAimLayerShouldBeActive)(const ai::AIBody *, int32_t,
                                    LocalClientNum_t);
  void (*ASMGetTargetPos)(int, LocalClientNum_t, int32_t, vec3_t *);
  ai::AIWeaponFireType (*GetAIFireMode)(int32_t, LocalClientNum_t);
  int32_t (*GetAIBurstCount)(ai::AIBody *, int32_t, LocalClientNum_t);
  int32_t (*GetAIFireInterval)(ai::AIBody *, int32_t, LocalClientNum_t);
  void (*GetAIHeadingVec)(int32_t, LocalClientNum_t, vec3_t *);
  void (*GetAIMoveVec)(int32_t, LocalClientNum_t, vec3_t *);
  void (*GetAIVelocityRatio)(int32_t, LocalClientNum_t, float *, const float);
  void (*GetAITurningAngle)(int32_t, LocalClientNum_t, float *);
  bool (*GetTagMatrix)(int, LocalClientNum_t, scr::ScrString_t, vec3_t *);
  bool (*GetTagMatrixTimeAllowed)(int, LocalClientNum_t, scr::ScrString_t,
                                  vec3_t *, int);
  void (*ASMGetPlayerViewOrigin)(int32_t, LocalClientNum_t, vec3_t *);
  const entityState_t *(*GetEntityState)(LocalClientNum_t, int);
  const weapon::entityGadgetState_t *(*GetEntityGadgetState)(
      LocalClientNum_t, int32_t, weapon::gadgetHeldSlot_e);
  const weapon::entityGadgetState_t *(*GetEntityGadgetStateByType)(
      LocalClientNum_t, int32_t, weapon::gadgetType_e);
  bool (*GetEntityOriginAngles)(const LocalClientNum_t, int32_t, vec3_t *,
                                vec3_t *);
  void (*GetEntityCentroid)(int, LocalClientNum_t, vec3_t *);
  void (*SetEntAnimSpeed)(int, float);
  float (*GetEntAnimSpeed)(int);
  void (*PlayerDobjUpdateStreaming)(const LocalClientNum_t, entityState_t *,
                                    game::sv::clientInfo_t *,
                                    const anim::DObj *, const weapon::Weapon);
  void (*PlayerDobjUpdate)(const LocalClientNum_t, game::sv::clientInfo_t *,
                           const weapon::Weapon);
  void (*ScrVm_AddInt)(int);
  void (*ScrVm_AddFloat)(float);
  void (*ScrVm_AddString)(const char *);
  void (*Scr_AddLocalClientNumber)(LocalClientNum_t);
  void (*ScrVm_ExecThread)(scr::scr_func_t, int);
  void (*ScrVm_ExecThreadEntNum)(LocalClientNum_t, int32_t, scr::scr_func_t,
                                 int);
};

struct transPlayerState_t {
  int32_t eFlags;
  int32_t otherFlags;
  int32_t damageEvent;
  int16_t predictableEventSequence;
  int32_t predictableEvents[4];
  int16_t unpredictableEventSequence;
  int32_t unpredictableEvents[4];
};

struct CompassSensor {
  vec2_t pos;
  int32_t startTime;
  int32_t handle;
};
struct RigUIModel {
  ui::UIModelIndex nameModel;
  ui::UIModelIndex imageModel;
};

struct __attribute__((aligned(8))) zmWeaponCost {
  weapon::Weapon weapon;
  uint32_t cost;
  uint32_t ammoCost;
  uint32_t playerCost;
};

struct __attribute__((aligned(4))) zmWeaponInfo {
  uint32_t numWeapons;
  zmWeaponCost costs[256];
  uint32_t bgbCost;
  bool forceUpdate;
};

struct EntityMinimapVisionCircle {
  int32_t entNum;
  vec2_t center;
  float radius;
  float radiusSq;
};

struct entityMinimapVisionInfo {
  EntityMinimapVisionCircle visionCircle[64];
  int32_t totalCircles;
};

struct GrappleTargetInfo {
  int32_t lastRetargetTime;
  int32_t lastTargetTested;
  float curTargetScore;
};

struct refdef_t {
  gfx::GfxLodBias gfxLodBias;
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
  float sceneViewportX;
  float sceneViewportY;
  float sceneViewportWidth;
  float sceneViewportHeight;
  uint32_t sceneResolutionIndex;
  uint32_t sceneResolutionIndexHistory[4];
  gfx::GfxCameraData gfxCameraData;
  float tanHalfFovX;
  float tanHalfFovY;
  float maincamTanHalfFovY;
  float fov_x;
  vec3_t vieworg;
  vec3_t lastvieworg;
  float yaw;
  vec3_t viewaxis[3];
  int32_t frametime;
  int32_t time;
  float zNear;
  float zFar;
  gfx::GfxVisionSet visionset;
  float blurRadius;
  gfx::GfxDepthOfField dof;
  float rimIntensity;
  vec4_t genericSceneVector0;
  vec4_t genericSceneVector1;
  vec4_t genericSceneVector2;
  vec4_t genericSceneVector3;
  vec4_t cameraVelocity;
  const gfx::GfxImage *sceneCaptureImage;
  gfx::GfxImage *emblemIMAGE[4];
  gfx::GfxPostFx postFx;
  vec4_t skyParams[3];
  gfx::GfxDoubleVision doubleVision;
  gfx::GfxCompositeFx flameFx;
  gfx::GfxCompositeFx waterSheetingFx;
  gfx::GfxGenericFilters genericFilters;
  gfx::GfxPoison poisonFx;
  gfx::GfxCompositeFx electrifiedFx;
  gfx::GfxCompositeFx transportedFx;
  gfx::GfxSaveScreenFx saveScreenFx;
  float sunVisibility;
  gfx::GfxSceneLight primaryLights[3000];
  uint32_t sunVolumePrimaryLightCount;
  const uint16_t *sunVolumePrimaryLightIndices;
  gfx::GfxConfig_Lensflare lensFlareDef[32];
  uint32_t numLensFlares;
  gfx::GfxViewport scissorViewport;
  LocalClientNum_t localClientNum;
  bool hideMatureContent;
  bool splitscreen;
  bool splitscreenBlurEdges;
  bool playerTeleported;
  OrientedExtentBounds viewmodelBounds;
  int32_t oldExposureId;
  int32_t newExposureId;
  float lerpcount;
  int32_t lastTime;
  int32_t evIdx;
  float evLA[8];
  float evL;
  float exposure;
  float linearExposure;
  uint16_t exposureMode;
  bool exposureAlwaysAdapt;
  bool exposureInstantSnap;
  uint16_t worldFogMode;
  vec3_t worldFogVolumeEntryPosition;
  int16_t newWorldFogVolumeId;
  int16_t oldWorldFogVolumeId;
  int16_t newWorldFogBankId;
  int32_t worldFogVolumeTimeStamp;
  int32_t worldFogFlags;
  bool worldFogSnapTransition;
  bool worldFogUseSystemTime;
  gfx::GfxConfig_Fog worldFogOld;
  gfx::GfxConfig_Fog worldFogNow;
  int16_t worldFogModifierVolumeId;
  int32_t worldFogModifierTimeStamp;
  float worldFogModifierTimeScale;
  float worldFogModifierScaleD;
  float worldFogModifierScaleH;
  vec4_t worldFogModifierColorA;
  float nWorldFogModifierScaleD;
  float nWorldFogModifierScaleH;
  vec4_t nWorldFogModifierColor;
  vec3_t worldFogVolumeLastViewOrg;
  uint32_t pbrFogBankMask;
  uint32_t pbgBankMask;
  float ukkoScriptBlend;
  int16_t ukkoScriptIndex;
  uint16_t lutBankMask;
  int16_t lutScriptIndex;
  int16_t newLutVolumeId;
  int16_t oldLutVolumeId;
  int32_t lutVolumeTimeStamp;
  float lutBlendWeights[32];
  bool noLodCullOut;
  float lodScaleRigid;
  float lodScaleSkinned;
  gfx::WaterFogDef waterFog;
  float adsZScale;
  const gfx::GfxExtraCamActiveState *extraCamStates;
  vec3_t preExtraCamVieworg;
  vec3_t preExtraCamViewaxis[3];
  float preExtraCamTanHalfFovX;
  float preExtraCamTanHalfFovY;
  bool sonarRenderTargetInUse;
  bool drawInfrared;
  bool underwater;
  bool screenDarkeningEnabled;
  float screenDarkeningValue;
  gfx::GfxRippleWave rWaves[1];
  bool enableOIT;
  bool drawPostBlur;
  int32_t fxShadowsMode;
  uint32_t lightStateIndex;
  const gfx::GfxSunVolume *sunVolume;
  uint32_t sunVolumeID;
  const gfx::GfxReflectionProbeArray *probeArray;
  float scriptShadowSplitDistance;
  float scriptModelLodBias;
  bool scriptDisableLodCullOut;
  bool sunVolumeFailedToStream;
  float expFdist;
  float expProbe;
  float expEvMin;
  float expEvMax;
  float expEvCmp;
  float expProbeContribution;
  bool isFirstPerson;
  const gfx::GfxImage *debugResolveTarget;
};
struct XAnimNotifyInfo {
  scr::ScrString_t type;
  float time;
  scr::ScrString_t param1;
  scr::ScrString_t param2;
  scr::ScrString_t param3;
};

struct __attribute__((aligned(8))) delayedNotetrackEvent_t {
  centity_t *cent;
  const XAnimNotifyInfo *action;
  bool bIsFirstPersonPlayer;
};

struct clientHudMenu_t {
  char modelPath[64];
  scr::ScrString_t menuName;
};

enum class FootstepMoveTypes : int32_t {
  INVALID = -1,
  PRONE = 0x0,
  CROUCH_WALK = 0x1,
  CROUCH_RUN = 0x2,
  WALK = 0x3,
  RUN = 0x4,
  SPRINT = 0x5,
  LADDER_HAND = 0x6,
  COUNT = 0x7,
};

struct footstepEvent_t {
  centity_t *cent;
  scr::ScrString_t bone;
  scr::ScrString_t notetrack;
  FootstepMoveTypes moveType;
  uint8_t surfaceFlags;
  struct {
    uint64_t playerView : 1;
    uint64_t quiet : 1;
    uint64_t isLouder : 1;
    uint64_t footprint : 1;
  };
};

struct ShieldHitDelayedEvent {
  uint32_t eventParm;
};

struct CompassScrambler {
  vec2_t pos;
  int startTime;
  int handle;
};

template <typename T> using CentityPool = array<T, 0x700>;

struct centityPool_t {
  CentityPool<centity_t> entities;
};
ASSERT_SIZE(centityPool_t, 0x3F0000);

constexpr uint32_t CG_T_SIZE = 0x342720;
partial_def(CG_T_SIZE, struct __attribute__((aligned(16))), cg_t, {
  struct FogSettings {
    float start;
    float r;
    float g;
    float b;
    float density;
    float heightDensity;
    float baseHeight;
    float fogColorScale;
    float sunFogColorR;
    float sunFogColorG;
    float sunFogColorB;
    float sunFogDirX;
    float sunFogDirY;
    float sunFogDirZ;
    float sunFogStartAng;
    float sunFogEndAng;
    float fogMaxOpacity;
  };

  struct __attribute__((aligned(4))) SpeedBlur {
    bool scripted;
    bool shouldScale;
    bool shouldOffset;
    int32_t time;
    float amount;
    float scale;
    int32_t timeIn;
    int32_t timeOut;
    float current;
    float start;
    bool wasDoingSpeedBlur;
  };

  ClientNum_t clientNum;
  LocalClientNum_t localClientNum;
  DemoType demoType;
  CubemapShot cubemapShot;
  int32_t cubemapSize;
  int32_t cameraMode;
  bool hiResShotMode;
  bool renderScreen;
  int32_t latestSnapshotNum;
  int32_t latestSnapshotTime;
  snapshot_t *snap;
  snapshot_t *nextSnap;
  snapshot_t activeSnapshots[2];
  bool slowMotionModified;
  float frameInterpolation;
  int32_t frametime;
  int32_t time;
  int32_t animTime;
  int32_t oldTime;
  int32_t initialSnapShotTime;
  int32_t physicsTime;
  int32_t bodyModelSwitched;
  bool mapRestart;
  bool bThirdPerson;
  bool renderingThirdPerson;
  bool renderingThirdPersonDeath;
  bool renderingThirdPersonLastFrame;
  playerState_t predictedPlayerState;
  // int32_t lastPlayerStateOverride;
  // centity_t predictedPlayerEntity;
  // playerEntity_t playerEntity;
  // int32_t predictedErrorTime;
  // vec3_t predictedError;
  // bool playerStateClientNumChangedThisSnapshot;
  // bool playerStateClientNumChangedSinceLastPrediction;
  // bool allowActionSlots;
  // bool disallowScoreboard;
  // playerState_t lastUIPlayerState;
  // float landChange;
  // int32_t landTime;
  // float heightToCeiling;
  // int32_t heightToCeilingTS;
  // uint8_t roleIndex;
  // const game::sv::PlayerRoleTemplate *role;
  // uint8_t playerLevel;
  // uint8_t lastPlayerLevel;
  // bool isPlayerRoleUpdated;
  // refdef_t refdef;
  // vec3_t refdefViewAngles;
  // scr::Camera cameraData;
  // float lastPelvisCameraOffset;
  // vec3_t baseGunAngles;
  // vec3_t swayViewAngles;
  // vec3_t swayAngles;
  // vec3_t swayOffset;
  // vec3_t recoilAngles;
  // vec3_t recoilSpeed;
  // vec3_t damageKickAngles;
  // int32_t previousRecoilTime;
  // float previousRecoilRatio;
  // vec3_t lastStandSwayAngles;
  // vec3_t lastStandSwayAVel;
  // vec3_t lastStandSwayTarget;
  // vec3_t movieCameraOrigin;
  // vec3_t movieCameraAngles;
  // vec3_t movieCameraVelocity;
  // vec3_t radiantCameraOrigin;
  // vec3_t radiantCameraAngles;
  // bool radiantCamInUse;
  // CentityPool<int32_t> iEntityLastType;
  // CentityPool<db::xasset::XModel *> pEntityLastXModel;
  // CentityPool<bool> bEntityDObjDirty;
  // bool anyClientDObjChanged;
  // CentityPool<int32_t> iEntityLastAnimtree;
  // bool isLoading;
  // char objectiveText[1024];
  // SettingTeamIndicator teamIndicator;
  // ColorVisionDeficiencies colorBlindAssistMode;
  // int32_t teamPlayerMask[21][1];
  // int32_t vehicleInitView;
  // vec3_t prevVehicleInvAxis[3];
  // bool missileInitView;
  // vec3_t storedRemoteCameraAngles;
  // vec3_t storedRemoteCameraOrigin;
  // char scriptMainMenu[256];
  // bool initialPlayersConnected;
  // bool initialPlayersConnectedRespondedTo;
  // bool sharedViewPort;
  // int32_t numScores;
  // int32_t teamScores[21];
  // int32_t teamPings[21];
  // int32_t teamPlayers[21];
  // lobby::LobbyClientPool<ClientNum_t> scoreOrder;
  // scoreboardColumnType_t scoreboardColumnTypes[10];
  // bool showScoreboard;
  // HitMarker hitMarkers[20];
  // int32_t currentHitMarker;
  // int32_t matchUIVisibilityFlags;
  // int32_t hiddentMiscModelGroups;
  // uint32_t talkFlags;
  // objectiveMapInfo_t objectiveMapInfo[192][2];
  // objective_t clientObjectives[64];
  // int32_t bombTimer[2];
  // int32_t drawHud;
  // int32_t drawAltHud;
  // int32_t timeScaleTimeStart;
  // int32_t timeScaleTimeEnd;
  // float timeScaleStart;
  // float timeScaleEnd;
  // uint16_t crosshairEntId;
  // ClientNum_t crosshairClientNum;
  // int32_t crosshairClientLastTime;
  // int32_t crosshairClientStartTime;
  // bool crosshairClientRangeActive;
  // ClientNum_t recentlyDamagedEntityNum;
  // int32_t recentlyDamagedEntityTime;
  // int32_t gpsJammerLastInactive;
  // int32_t gpsJammerLastActive;
  // int32_t bulletsFiredContinuously;
  // int32_t bulletsFiredContinuouslyLeft;
  // int32_t bulletsFiredStopTime;
  // int32_t cursorHintIcon;
  // int32_t cursorHintEntity;
  // uint32_t cursorHintString;
  // weapon::Weapon cursorHintWeapon;
  // int32_t vehicleHintTime;
  // int32_t lastVehicleSeat;
  // bool driverHintShown;
  // bool gunnerHintShown;
  // int32_t lastClipFlashTime;
  // InvalidCmdHintType invalidCmdHintType;
  // int32_t invalidCmdHintTime;
  // int32_t lastHealthLerpDelay;
  // float lastHealth;
  // int32_t lastShowDamageWidgetTime;
  // float healthOverlayFromAlpha;
  // float healthOverlayToAlpha;
  // int32_t healthOverlayPulseTime;
  // int32_t healthOverlayPulseDuration;
  // int32_t healthOverlayPulsePhase;
  // bool healthOverlayHurt;
  // int32_t healthOverlayLastHitTime;
  // float healthOverlayOldHealth;
  // int32_t healthOverlayPulseIndex;
  // int32_t directionalDamageIndicatorType;
  // int32_t proneBlockedEndTime;
  // int32_t lastStance;
  // int32_t lastStanceChangeTime;
  // int32_t lastStanceFlashTime;
  // int32_t voiceTime;
  // int32_t arcPredictionLastDrawTime;
  // weapon::Weapon weaponSelect;
  // int32_t weaponSelectTime;
  // bool weaponForcedSelect;
  // weapon::Weapon weaponLatestPrimary[2];
  // weapon::Weapon lastWeaponForAlt[15];
  // db::xasset::FxUniqueHandle viewModelFx;
  // int32_t dynzoomTargetLastTime;
  // float dynzoomTargetFocalLength;
  // float dynzoomTargetfStop;
  // weapon::Weapon prevViewmodelWeapon;
  // bool prevViewmodelWeaponInADS;
  // int32_t prevWeaponDamageLevel;
  // int32_t weaponWalkEndTime;
  // int32_t weaponWalkBobCycle;
  // int32_t adsStartTime;
  // weapon::Weapon equippedOffHand;
  // weapon::entityGadgetState_t gadgetsHeld[4];
  // viewDirectionalHitIndicator_t directionalHitIndicator[4];
  // viewDamage_t viewDamage[8];
  // viewAwareness_t viewAwareness[8];
  // int32_t damageTime;
  // float damageX;
  // float damageY;
  // float damageValue;
  // float viewFade;
  // int32_t waterDropCount;
  // int32_t waterDropStartTime;
  // int32_t waterDropStopTime;
  // int32_t weaponIdleTime;
  // bool nomarks;
  // bool clearMarks;
  // bool newPlayerViewmodel;
  // bool weaponLaserOff;
  // int32_t v_dmg_time;
  // float v_dmg_pitch;
  // float v_dmg_roll;
  // float v_dmg_flash;
  // float fBobCycle;
  // float xyspeed;
  // vec3_t kickAVel;
  // vec3_t kickAngles;
  // vec3_t offsetAngles;
  // float gunPitch;
  // float gunYaw;
  // float gunXOfs;
  // float gunYOfs;
  // float gunZOfs;
  // vec3_t flamethrowerKickOffset;
  // float minigunSpinAngle;
  // vec3_t viewModelAxis[4];
  // int32_t hideViewModel;
  // float rumbleScale;
  // float compassNorthYaw;
  // vec2_t compassNorth;
  // gfx::GfxImagePtr compassMapImage;
  // gfx::GfxImagePtr compassMapImageFull;
  // gfx::GfxImagePtr compassMapImageSpawn;
  // vec2_t compassMapUpperLeft;
  // vec2_t compassMapWorldSize;
  // vec2_t compassMapCurrentUpperLeft;
  // vec2_t compassMapCurrentWorldSize;
  // vec2_t compassMapCurrentLeftTopUV;
  // vec2_t compassMapCurrentRightBottomUV;
  // int32_t compassLastTime;
  // float compassYaw;
  // float compassSpeed;
  // int32_t compassFadeTime;
  // int32_t stanceFadeTime;
  // int32_t waypointFadeTime;
  // int32_t vehicleControlsFadeTime;
  // float compassRight;
  // float compassBottom;
  // shellshock_t shellshock;
  // fancycam_t fancycam;
  // struct {
  //   int32_t time;
  //   int32_t duration;
  // } testShock;
  // int32_t shellshockSoundActive;
  // uint32_t priorStatusEffectsFlags;
  // db::xasset::FxUniqueHandle viewPersistentEffectHandle;
  // db::xasset::TagFxSetHandles viewPersistentEffectSetHandles;
  // lobby::LobbyClientPool<db::xasset::FxUniqueHandle> playerJukeEffects;
  // int32_t holdBreathTime;
  // int32_t holdBreathInTime;
  // int32_t holdBreathDelay;
  // float holdBreathFrac;
  // float radarProgress;
  // float satelliteProgress;
  // int32_t radarType;
  // vec2_t selectedLocation;
  // vec2_t selectedYaw;
  // int32_t typewriterStartTime;
  // vec2_t selectedLocations[3];
  // int32_t lastTimeSelected;
  // SprintState sprintStates;
  // db::xasset::ManagedNoteTrackList viewmodelManagedNotetracks;
  // float lastForwardSpeed;
  // vec3_t jukeStartOrigin;
  // int32_t packetAnalysisFrameCount;
  // int32_t packetAnalysisGameTime;
  // uint8_t bitsSent[130][18][27];
  // int32_t bitsSentGameTime[130];
  // int32_t entBitsUsed[130][18][44];
  // int32_t numEntsSent[130][18][44];
  // int32_t numEntFields[130][18][44];
  // char serverCmdChars[130][18][16];
  // int32_t snapshotSamplesGameTime[130];
  // int32_t snapshotFlags[130];
  // int32_t numSnapshots;
  // int32_t startSamplingTime;
  // bool adsViewErrorDone;
  // bool inKillCam;
  // bool wasInKillCam;
  // bool cleanUpDynEntsSpawnedAfterThisFrame;
  // bool killCamFirstFrameRan;
  // int32_t killCamEntity;
  // int32_t invalidKillCamEntity;
  // KillCamEntityType killCamEntityType;
  // int32_t killCamLastEntityNum;
  // vec3_t killCamLastEntityOrg;
  // vec3_t killCamLastEntityAngles;
  // KillCamEntityRestState killCamEntityRestState;
  // int32_t killCamLookAtEntity;
  // vec3_t killCamLookAt;
  // vec3_t killCamHelicopterOffset;
  // int16_t killCamLastVehicleDefIndex;
  // weapon::Weapon killCamWeapon;
  // int32_t killCamMod;
  // int32_t killCamStoppedTime;
  // vec_t killCamStoppedDecelTime;
  // vec3_t killCamStoppedPos;
  // vec3_t killCamStoppedVel;
  // vec3_t killCamPrevBombOrigin;
  // int32_t killCamLerpEndTime;
  // vec3_t killCamOldViewAngles;
  // vec3_t killCamOldViewOrg;
  // float killCamSettings[6];
  // bgs_t bgs;
  // cpose_t viewModelPose;
  // visionSetVars_t visionSetPreLoaded[8];
  // char visionSetPreLoadedName[8][64];
  // visionSetVars_t visionSetFrom[8];
  // visionSetVars_t visionSetTo[8];
  // visionSetVars_t visionSetCurrent[8];
  // visionSetLerpData_t visionSetLerpData[8];
  // char visionNameNaked[64];
  // char visionNameLastStand[64];
  // char visionNameFlare[64];
  // char visionNameInfrared[64];
  // char visionNameTvguided[64];
  // char visionNameUnderwater[64];
  // bool drewReticle;
  // game_button_bits_t extraButton_bits;
  // bool breathPressedLastFrame;
  // int32_t lastActionSlotTime;
  // bool playerTeleported;
  // bool firstFrame;
  // bool inAssassinationMode;
  // float flareRatioLastFrame;
  // int32_t stepViewStart;
  // float stepViewChange;
  // struct {
  //   float aimSpreadScale;
  // } lastFrame;
  // int32_t oldCommandTime;
  // vec3_t oldOrigin;
  // vec3_t oldViewangles;
  // transPlayerState_t oldTransPlayerState;
  // ui::hud::hudElemSoundInfo_t hudElemSound[32];
  // int32_t nextRippleTime;
  // uint32_t impactEffectsNext;
  // db::xasset::FxUniqueHandle impactEffects[256];
  // float warDesiredMomentumProgress;
  // float warCurrentMomentumProgress;
  // int32_t warMomentumMultiplier;
  // int32_t warMomentumBlitzkriegTime;
  // int32_t warMomentumFlashTime;
  // float gunnerPitchOffset;
  // int32_t cameraLinkedEntitiesCount;
  // int32_t cameraLinkedEntities[4];
  // anim::link_type_e cameraLinkedEntitiesType[4];
  // vec3_t cameraLinkedEntitiesOffset[4];
  // int32_t groundTiltEntNum;
  // bool poisoned;
  // db::xasset::Material *genericMaterialMap[64];
  // bool genericOverlayEnabled[3];
  // db::xasset::Material *genericOverlayMaterial[3];
  // float genericOverlayParam[3][16];
  // int32_t genericOverlaySortIndex[3];
  // float genericOverlayParamADS[8];
  // bool genericOverlayEnableADS;
  // bool needThermalDraw;
  // bool underwater;
  // bool swimming;
  // int32_t swimmingStartTime;
  // int32_t missileViewWobbleLastTime;
  // vec3_t missileViewWobbleFreq;
  // vec3_t missileViewWobbleAmp;
  // int32_t counterSpinTime;
  // vec_t counterSpinTarget;
  // vec_t counterSpinAngle;
  // int32_t cameraSpikeCamEntity;
  // bool cameraSpikeCamActive;
  // bool cameraSpikeCamStatic;
  // vec3_t cameraSpikeCamOrigin;
  // vec3_t cameraSpikeCamAngles;
  // bool hasLocalRadar;
  // vec2_t localRadarPos;
  // float localRadarProgress;
  // bool globalScramblerActive;
  // float scramblerEnemyAlpha;
  // float scramblerFriendlyAlpha;
  // float lockOnHeatRatio;
  // int32_t popUpSwayStartTime;
  // float popupRotationAngle;
  // float popUpAngleDelta;
  // int32_t swimState;
  // CompassScrambler compassScrambler[16];
  // centity_t *closestEnemyScrambler;
  // CompassSensor compassSensors[2];
  // bool alliesFlagAway;
  // bool axisFlagAway;
  // bool resetWeaponStateFlag;
  // int32_t delayedShieldHitEventsCount;
  // ShieldHitDelayedEvent delayedShieldHitEvents[2];
  // int32_t recentKilledByClientNum;
  // int32_t recentKilledByTime;
  // int32_t recentVictimClientNums[8];
  // int32_t recentVictimTimes[8];
  // int32_t recentVictimIndex;
  // bool talkingToEnemy;
  // FogSettings serverFogSettings;
  // bool forceInfraredMaterialsStreaming;
  // bool hudOutlineOn;
  // weapon::DroppedWeaponDobjInfo droppedWeaponDobjInfo[64];
  // vec3_t traceHitPos;
  // int32_t traceHitDistance;
  // bool traceHitEnemy;
  // int32_t rangeFinderStartTime;
  // bool rangeFinderTimerActive;
  // bool rangeFinderPerformTrace;
  // bool rangeFinderPerformFxVisTest;
  // int32_t playerVehicleEntity;
  // int32_t respawnTime;
  // bool clientCollisionLinked;
  // float altScopeFrac;
  // int32_t lastAltScopeFracUpdateTime;
  // bool useAlternateAimAssistParams;
  // int32_t empStartTime;
  // int32_t lastEmpTime;
  // footstepEvent_t footstepEvents[16];
  // uint32_t numFootstepEvents;
  // delayedNotetrackEvent_t delayedNotetrackEvents[32];
  // uint32_t numDelayedNotetrackEvents;
  // int32_t movers[256];
  // int32_t moversCount;
  // int32_t hit_charge;
  // bool hit_possible;
  // bool bClientNeedsConnectCallback;
  // team_t shoutcastListenInTeam;
  // int32_t shoutcastListenInBits;
  // bool shoutcastThirdPersonMode;
  // bool shoutcasterFlipScoreSettings;
  // weapon::Weapon selectedClassWeapon;
  // bool resetLuiNeeded;
  // bool hostMigrationOccured;
  // int32_t safeSpectatorCamTime;
  // vec3_t safeSpectatorCamPos;
  // vec3_t safeSpectatorCamOffset;
  // team_t team;
  // gfx::GfxDuplicateRenderOption duplicateRenderOptions[128];
  // volatile int32_t numDuplicateRenderOptions;
  // clientHudMenu_t clientHudMenus[64];
  // ui::CurrentWeaponModel currentWeaponModels;
  // ui::CurrentWeaponModel stowedWeaponModels;
  // ui::UIModelIndex hudItemsModel;
  // ui::UIModelIndex hudItemPulseNoAmmoModel;
  // ui::UIModelIndex hudItemPulseNoLethalModel;
  // ui::UIModelIndex hudItemPulseNoTacticalModel;
  // ui::UIModelIndex hudItemHeroHoldProgressModel;
  // ui::UIModelIndex tabletLocationSelectorTextModel;
  // ui::UIModelIndex shouldUpdateHUDLeveUpModel;
  // ui::UIModelIndex playerRoleUpdatedModel;
  // ui::UIModelIndex primaryWeapon;
  // ui::UIModelIndex primaryWeaponIndex;
  // ui::UIModelIndex primaryWeaponItem;
  // ui::UIModelIndex primaryAttachments[8];
  // ui::UIModelIndex secondaryWeapon;
  // ui::UIModelIndex secondaryWeaponIndex;
  // ui::UIModelIndex secondaryWeaponItem;
  // ui::UIModelIndex secondaryAttachments[8];
  // ui::UIModelIndex currentPrimaryOffhandModel;
  // ui::UIModelIndex currentSecondaryOffhandModel;
  // ui::UIModelIndex currentSpecialOffhandModel;
  // ui::UIModelIndex specialOffhandModel;
  // ui::UIModelIndex specialOffhandCountModel;
  // ui::UIModelIndex currentHeroWeaponOffhandModel;
  // ui::UIModelIndex heroWeaponOffhandModel;
  // ui::UIModelIndex heroWeaponOffhandCountModel;
  // ui::UIModelIndex primaryOffhandModel;
  // ui::UIModelIndex primaryOffhandCountModel;
  // ui::UIModelIndex primaryOffhandRefModel;
  // ui::UIModelIndex secondaryOffhandModel;
  // ui::UIModelIndex secondaryOffhandCountModel;
  // ui::UIModelIndex secondaryOffhandRefModel;
  // ui::UIModelIndex secondaryOffhandAspectRatioModel;
  // RigUIModel rig1Model;
  // RigUIModel rig2Model;
  // ui::UIModelIndex playerSpawnedModel;
  // ui::UIModelIndex scriptNotifyModel;
  // ui::UIModelIndex yawModel;
  // ui::UIModelIndex playerConnectedModel;
  // ui::UIModelIndex playerDisconnectedModel;
  // ui::UIModelIndex playerBotStatusChangedModel;
  // ui::UIModelIndex colorBlindSensitiveFriendlyColorModel;
  // ui::UIModelIndex colorBlindSensitiveEnemyColorModel;
  // ui::UIModelIndex colorBlindSensitiveEnemyScorestreakTargetColorModel;
  // ui::UIModelIndex noNeedToHealModel;
  // bool wasInSmoke;
  // int32_t lastSmokeChangeTime;
  // ui::UIModelIndex smokeEffectModel;
  // ui::VehicleModel vehicleUIModel;
  // ui::UIModelIndex killstreaksModel;
  // ui::KillstreakRewardModel killstreakRewardInfoModel[4];
  // ui::UIModelIndex rewardMomentumModel;
  // ui::UIModelIndex distanceToClosestEnemyEmpKillstreak;
  // ui::UIModelIndex gameScoreModel;
  // ui::UIModelIndex roundLimitModel;
  // ui::UIModelIndex roundWinLimitModel;
  // ui::UIModelIndex roundsPlayedModel;
  // ui::UIModelIndex scoreLimitModel;
  // ui::UIModelIndex playerScoreModel;
  // ui::UIModelIndex enemyScoreModel;
  // ui::UIModelIndex alliesAliveModel;
  // ui::UIModelIndex enemiesAliveModel;
  // ui::UIModelIndex factionListModel;
  // ui::UIModelIndex factionModel;
  // ui::UIModelIndex playerFactionTeamEnumModel;
  // ui::UIModelIndex enemyFactionTeamEnumModel;
  // ui::UIModelIndex playerFactionNameModel;
  // ui::UIModelIndex enemyFactionNameModel;
  // ui::UIModelIndex isCoDCaster;
  // ui::UIModelIndex gameTimeEndModel;
  // ui::UIModelIndex gameTimeLowModel;
  // ui::UIModelIndex gameTimeLimitModel;
  // ui::UIModelIndex highestScoreModel;
  // ui::UIModelIndex highestScoreNameModel;
  // ui::UIModelIndex firstPlaceXUIDModel;
  // ui::UIModelIndex inGameNotificationModel;
  // ui::UIModelIndex earnedKillstreakNameModel;
  // ui::UIModelIndex earnedKillstreakImageModel;
  // ui::UIModelIndex earnedMedalNameModel;
  // ui::UIModelIndex earnedMedalImageModel;
  // ui::UIModelIndex earnedGunLevelWeaponNameModel;
  // ui::UIModelIndex earnedGunLevelAttachRewardTextModel;
  // ui::UIModelIndex earnedGunLevelWeaponImageModel;
  // ui::UIModelIndex earnedRankUpNameModel;
  // ui::UIModelIndex earnedRankUpImageModel;
  // ui::UIModelIndex scorePopupNameModel;
  // ui::UIModelIndex scorePopupAmountModel;
  // ui::UIModelIndex playerCalloutModel;
  // ui::UIModelIndex playerCalloutHeadingModel;
  // ui::UIModelIndex playerCalloutPlayerNameModel;
  // ui::UIModelIndex playerCalloutClanTagModel;
  // ui::UIModelIndex playerCalloutRankModel;
  // ui::UIModelIndex playerCalloutRankIconModel;
  // ui::UIModelIndex playerCalloutPrestigeModel;
  // ui::UIModelIndex playerCalloutLeagueRankModel;
  // ui::UIModelIndex playerCalloutLeagueIconModel;
  // ui::UIModelIndex playerCalloutSelectedBgModel;
  // ui::UIModelIndex playerCalloutXuidModel;
  // ui::UIModelIndex playerObitModel;
  // ui::UIModelIndex playerObitCalloutModel;
  // ui::UIModelIndex playerObitCalloutClientNumModel;
  // ui::UIModelIndex perksModel;
  // ui::PerkModel perkInfoModel[3][2];
  // ui::UIModelIndex perkCountModel[3];
  // ui::UIModelIndex anyPerkChangedModel;
  // ui::UIModelIndex bonusCardCountModel;
  // ui::UIModelIndex bonusCardModel[3];
  // ui::UIModelIndex specialtiesModel;
  // ui::UIModelIndex specialtiesModel_combat_efficiency;
  // ui::UIModelIndex playerAbilitiesModel;
  // ui::UIModelIndex playerEnergyRatioModel;
  // ui::UIModelIndex playerSprintRatioModel;
  // ui::UIModelIndex playerSprintMaxSpeedRatio;
  // ui::UIModelIndex playerSprintDecaySpeedRatio;
  // ui::UIModelIndex playerSprintBoostingModel;
  // ui::UIModelIndex playerSprintExhaustedModel;
  // ui::PlayerGadgetModel playerGadgetInfoModel[4];
  // ui::UIModelIndex cscDebugMenuModel;
  // ui::UIModelIndex cscDebugMenuListItemModel;
  // ui::UIModelIndex cscDebugMenuItemNameModel;
  // ui::UIModelIndex cscDebugMenuItemColorModel;
  // ui::UIModelIndex cscDebugMenuCountModel;
  // ui::UIModelIndex deadSpectatePlayerIndex;
  // ui::UIModelIndex deadSpectatePlayerTeam;
  // ui::UIModelIndex killCamHideEmpRebootHud;
  // ui::UIModelIndex clientCharacterSelectionUpdateModel;
  // ui::UIModelIndex clientCharacterIndex;
  // ui::PlayerRoleModel clientRoleModel[19];
  // anim::ObjectiveAnimData objectiveAnimData[128];
  // int32_t activeSkiptos;
  // lobby::LobbyClientPool<GrappleTargetInfo> grappleInfo;
  // SpeedBlur speedBlur;
  // vec3_t AnimScriptStartOrigin;
  // float AnimScriptStartYaw;
  // float AnimScriptErrorYaw;
  // lobby::LobbyClientPool<bool> wasPlayingAnimScript;
  // int32_t currentPlayerAnimScript;
  // uint8_t SceneHolsterWeaponState;
  // db::xasset::MaterialHandle zodBeastIconMaterial[4];
  // CentityPool<int32_t> zodBeastIconType;
  // CentityPool<vec3_t> zodBeastIconPos;
  // CentityPool<int32_t> zodBeastIconTime;
  // bool checkPointRestored;
  // uint32_t centInCheckpointSnapshot[32];
  // bool aggressiveCullAllowed;
  // XUID showcaseWeaponPaintshopXUID;
  // bool isContentScreenFilterPlaying;
  // zmWeaponInfo zmweapons;
  // phys::objcamCameraTable objectiveCameras;
  // phys::objcamCameraState objectiveCameraState;
  // entityMinimapVisionInfo entityMinimapVision;
});
// TODO: Correct size is 0x342720. This struct needs corrected.
ASSERT_SIZE(cg_t, CG_T_SIZE);

struct cgPool {
  LocalClientPool<cg_t> pool;
};

struct __attribute__((aligned(8))) cgs_t {
  int viewX;
  int viewY;
  int viewWidth;
  int viewHeight;
  float viewAspect;
  float sceneViewportX;
  float sceneViewportY;
  float sceneViewportWidth;
  float sceneViewportHeight;
  int serverCommandSequence;
  int processedSnapshotNum;
  bool localServer;
  char gametype[32];
  char szHostName[256];
  int maxclients;
  int privateClients;
  char mappath[64];
  name_t mapname;
  int gameEndTime;
  int voteTime;
  int voteYes;
  int voteNo;
  char voteString[256];
  int redCrosshair;
  db::xasset::FxEffectDefHandle fxs[1280];
  db::xasset::FxEffectDefHandle killcamRewindFx[75];
  int killcamRewindFxCount;
  db::xasset::FxEffectDefHandle playerFireFx[3];
  ShellshockParams *holdBreathParams;
  float compassWidth;
  float compassHeight;
  float compassY;
  sv::clientInfo_t corpseinfo[6];
  user::actorInfo_t actorCorpseInfo[32];
  bool entUpdateToggleContextKey;
};
struct cgsPool {
  LocalClientPool<cgs_t> pool;
};

struct LocalClientCentityPools {
  LocalClientPool<centityPool_t *> pools;
};

typedef AtomicLocalClientPool<centityPool_t *> AtomicLocalClientCentityPools;

} // namespace cl
} // namespace level
} // namespace game