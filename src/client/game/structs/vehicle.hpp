#pragma once

#include "asm.hpp"
#include "ai.hpp"
#include "math.hpp"
#include "phys.hpp"
#include "snd.hpp"
#include "gfx.hpp"
#include "core.hpp"
#include "user.hpp"
namespace game {
namespace vechicle {
struct gentity_s;
#pragma pack(push, 1)

// sizeof=0x18
struct WheelState {
  int32_t m_state;
  int32_t m_last_state;
  float m_rate[4];
};
static_assert(sizeof(WheelState) == 0x18, "WheelState size must be 24 bytes");

enum VehicleCategory : int32_t {
  VEHICLE_CATEGORY_NONE = 0x0,
  VEHICLE_CATEGORY_QUAD_TANK = 0x1,
  VEHICLE_CATEGORY_AMWS = 0x2,
  VEHICLE_CATEGORY_WASP = 0x3,
  VEHICLE_CATEGORY_ROCKET_WASP = 0x4,
  VEHICLE_CATEGORY_HUNTER = 0x5,
  VEHICLE_CATEGORY_SIEGE_BOT = 0x6,
  VEHICLE_CATEGORY_COUNT = 0x7,
};

// sizeof=0x20
struct view_limits_t {
  float horizSpanLeft;
  float horizSpanRight;
  float vertSpanUp;
  float vertSpanDown;
  float horizResistLeft;
  float horizResistRight;
  float vertResistUp;
  float vertResistDown;
};
static_assert(sizeof(view_limits_t) == 0x20,
              "view_limits_t size must be 32 bytes");

// sizeof=0xC
struct VehicleGearData {
  float minRPM;
  float maxRPM;
  float ratio;
};
static_assert(sizeof(VehicleGearData) == 0xC,
              "VehicleGearData size must be 12 bytes");

// sizeof=0x28
struct VehicleEngineSound {
  const char *name;
  snd::SndAliasId alias;
  float params[7];
};
static_assert(sizeof(VehicleEngineSound) == 0x28,
              "VehicleEngineSound size must be 40 bytes");

struct XModel; // TODO
typedef XModel *XModelPtr;

// sizeof=0x2C0
struct VehicleEngine {
  bool simpleEngine;
  uint8_t _padding01[3];
  float idleRpms;
  float maxRpms;
  float maxTorque;
  float brakingCoeff;
  vec4_t loadFadeParams;
  vec4_t loadVolumeRange;
  float loadScale;
  float loadSmoothing;
  float throttleLag;
  float pitchScale;
  float rpmSmoothing;
  float differentialRatio;
  uint8_t _padding4C[4];
  VehicleEngineSound onload[5];
  VehicleEngineSound offload[5];
  int32_t numGears;
  bool loopLastGear;
  uint8_t _padding1E5[3];
  VehicleGearData gears[10];
  bool useAudioMaxSpeed;
  uint8_t _padding261[3];
  float audioMaxSpeed;
  const char *eng_idle_snd_name;
  snd::SndAliasId eng_idle_snd;
  float eng_idle_start_speed;
  float eng_idle_end_speed;
  float eng_idle_start_pitch;
  float eng_idle_end_pitch;
  uint8_t _padding284[4];
  const char *eng_move_snd_name;
  snd::SndAliasId eng_move_snd;
  float eng_move_start_speed;
  float eng_move_end_speed;
  float eng_move_start_pitch;
  float eng_move_end_pitch;
  uint8_t _padding2A4[4];
  const char *eng_offload_snd_name;
  snd::SndAliasId eng_offload_snd;
  float eng_offload_start_pitch;
  float eng_offload_end_pitch;
  uint8_t _padding2BC[4];
};
static_assert(sizeof(VehicleEngine) == 0x2C0,
              "VehicleEngine size must be 704 bytes");

struct DevGraph;

enum DevEventType : int32_t {
  EVENT_ACTIVATE = 0x0,
  EVENT_DEACTIVATE = 0x1,
  EVENT_ACCEPT = 0x2,
  EVENT_UPDATE = 0x3,
  EVENT_DRAW = 0x4,
  EVENT_SAVE = 0x5,
};

typedef void (*DevGraphEventCallback)(const DevGraph *, DevEventType,
                                      LocalClientNum_t);
typedef void (*DevGraphTextCallback)(const DevGraph *, const float, const float,
                                     char *, const int);

// sizeof=0x38
struct DevGraph {
  vec2_t *knots;
  int32_t *knotCount;
  int32_t knotCountMax;
  int32_t selectedKnot;
  DevGraphEventCallback eventCallback;
  DevGraphTextCallback textCallback;
  void *data;
  bool disableEditingEndPoints;
  uint8_t _padding31[7];
};
static_assert(sizeof(DevGraph) == 0x38, "DevGraph size must be 56 bytes");

// sizeof=0x180
struct GraphFloat {
  char name[64];
  vec2_t knots[32];
  int32_t knotCount;
  float scale;
  DevGraph devguiGraph; // TODO: is this used in release?
};
static_assert(sizeof(GraphFloat) == 0x180, "GraphFloat size must be 384 bytes");

// sizeof=0x28
struct AircraftPreset {
  const char *presetName;
  int32_t maxSpeedVol;
  float minVol;
  float maxVol;
  int32_t maxSpeedPitch;
  float minPitch;
  float maxPitch;
  bool xAxis;
  bool yAxis;
  bool zAxis;
  uint8_t _padding23[5];
};
static_assert(sizeof(AircraftPreset) == 0x28,
              "AircraftPreset size must be 40 bytes");

// sizeof=0x30
struct AircraftSound {
  const char *presetName;
  AircraftPreset *preset;
  const char *aliasName;
  snd::SndAliasId alias;
  uint8_t _padding1C[4];
  const char *tagName;
  scr::ScrString_t tagScriptString;
  uint8_t _padding2C[4];
};
static_assert(sizeof(AircraftSound) == 0x30,
              "AircraftSound size must be 48 bytes");

// sizeof=0x10
struct VehicleSurfaceSound {
  const char *surfaceSoundName;
  snd::SndAliasId alias;
  uint8_t _padding0C[4];
};
static_assert(sizeof(VehicleSurfaceSound) == 0x10,
              "VehicleSurfaceSound size must be 16 bytes");

// sizeof=0x18
struct VehicleDriveBySound {
  int32_t apex;
  uint8_t _padding04[4];
  const char *name;
  snd::SndAliasId alias;
  uint8_t _padding14[4];
};

// sizeof=0x1000
struct VehicleSoundDef {
  const char *name;
  VehicleEngine engine;
  XString engineGraphName;
  GraphFloat *engineGraph;
  float driveBySoundRadius[2];
  VehicleDriveBySound driveBySounds[40];
  AircraftPreset aircraftPresets[20];
  AircraftSound aircraftSounds[20];
  VehicleSurfaceSound surfaceSounds[40];
};
static_assert(sizeof(VehicleSoundDef) == 0x1000,
              "VehicleSoundDef size must be 4096 bytes");

typedef VehicleSoundDef *VehicleSoundDefPtr;

enum VehicleFxDefType : int32_t {
  VEHICLEFX_DEF_NITROUS = 0x0,
  VEHICLEFX_DEF_GROUND = 0x1,
  VEHICLEFX_DEF_AIRCRAFT = 0x2,
  VEHICLEFX_DEF_BOAT = 0x3,
  VEHICLEFX_DEF_TANK = 0x4,
  NUM_VEHICLEFX_DEF_TYPES = 0x5,
};

struct VehicleFxDef {
  const char *name;
  char *csvInclude;
  VehicleFxDefType type;
  phys::FxEffectDefHandle treadFx[40];
  phys::FxEffectDefHandle peelFx[40];
  phys::FxEffectDefHandle skidFx[40];
};

typedef VehicleFxDef *VehicleFxDefPtr;
typedef phys::Material *MaterialHandle;

enum TractionType : int32_t {
  TRACTION_TYPE_FRONT = 0x0,
  TRACTION_TYPE_BACK = 0x1,
  TRACTION_TYPE_ALL_WD = 0x2,
  NUM_TRACTION_TYPES = 0x3,
};

// sizeof=0x178
struct VehicleParameter {
  float m_speed_max;
  float m_accel_max;
  float m_reverse_scale;
  float m_steer_angle_max;
  float m_steer_angle_speed_scale;
  float m_steer_speed;
  float m_wheel_radius;
  float m_susp_spring_k;
  float m_susp_damp_k;
  float m_susp_adj;
  float m_susp_hard_limit;
  float m_susp_min_height;
  float m_tire_fric;
  float m_tire_fric_assist;
  float m_tire_fric_assist_threshold;
  float m_tire_fric_hand_brake_fwd;
  float m_tire_fric_hand_brake_side;
  float m_tire_fric_load_factor_min;
  float m_tire_fric_load_factor_max;
  float m_hand_brake_fake_rotation_force;
  float m_hand_brake_redirect_force;
  float m_body_mass;
  float m_roll_stability;
  float m_pitch_stability;
  float m_pitch_roll_resistance;
  float m_yaw_resistance;
  float m_upright_strength;
  float m_tire_damp_coast;
  float m_tire_damp_brake;
  float m_tire_damp_hand;
  float m_auto_hand_brake_min_speed;
  float m_max_climb_angle;
  float m_hand_brake_slide_time;
  float m_brake_to_reverse_time;
  float m_hover_trace_start_offset;
  float m_hover_trace_length_from_ground;
  float m_hover_trace_length_from_wall;
  float m_hover_to_wall_gravity_scale;
  float m_hover_to_wall_upright_strength_multiplier;
  float m_hover_to_wall_gravity_lerp_rate;
  float m_hover_to_ground_gravity_lerp_rate;
  float m_hover_to_ground_upright_strength_multiplier;
  float m_hover_min_speed_to_apply_brakes;
  float m_hover_brake_scale_factor;
  TractionType m_traction_type;
  uint8_t _paddingB4[4];
  const char *m_name;
  vec3_t m_wheel_offset;
  vec3_t m_bbox_min;
  vec3_t m_bbox_max;
  vec3_t m_mass_center_offset;
  vec3_t m_buoyancybox_min;
  vec3_t m_buoyancybox_max;
  float m_water_speed_max;
  float m_water_accel_max;
  float m_water_turn_accel;
  float m_water_turn_speed_max;
  float m_water_ebrake_power;
  vec3_t m_boat_motor_offset;
  float m_boat_speed_rise;
  float m_boat_speed_tilt;
  float m_boat_side_fric_scale;
  float m_boat_forward_fric_scale;
  float m_boat_vertical_fric_scale;
  float m_motorcycle_max_lean;
  float m_jump_force;
  float m_jump_force_in_air;
  float m_jump_force_from_wall;
  float m_jump_force_from_wall_max;
  float m_jump_force_from_wall_extra_z;
  float m_jump_gravity_scale_upwards;
  float m_jump_gravity_scale_transition_z;
  float m_jump_gravity_scale_downwards;
  float m_jump_from_wall_stick_angle;
  bool m_jumps_require_button_release;
  bool m_drive_on_walls;
  bool m_charge_jump;
  uint8_t _padding167[1];
  float m_linear_drag_scale;
  float m_angular_drag_scale;
  float m_gravity_scale;
  uint8_t _padding174[4];
};
static_assert(sizeof(VehicleParameter) == 0x178,
              "VehicleParameter size must be 376 bytes");

// sizeof=0x14
struct VehicleAntenna {
  float springK;
  float damp;
  float length;
  float gravity;
  bool useXAxis;
  uint8_t _padding11[3];
};
static_assert(sizeof(VehicleAntenna) == 0x14,
              "VehicleAntenna size must be 20 bytes");

// sizeof=0xAA0
struct VehicleDef {
  const char *name;
  int16_t type;
  uint8_t _padding0A[2];
  scr::ScrString_t scriptVehicleType;
  scr::ScrString_t archeType;
  scr::ScrString_t scoreType;
  scr::ScrString_t playerDrivenVersion;
  VehicleCategory category;
  qboolean nonstick;
  qboolean remoteControl;
  qboolean bulletDamage;
  qboolean armorPiercingDamage;
  qboolean grenadeDamage;
  qboolean projectileDamage;
  qboolean projectileSplashDamage;
  qboolean heavyExplosiveDamage;
  float physicsLaunchGrenadeScale;
  float physicsLaunchProjectileScale;
  float physicsLaunchExplosiveScale;
  float physicsLaunchDeathScale;
  float grenadeDamageMultiplier;
  float projectileDamageMultiplier;
  float explosiveDamageMultiplier;
  float bulletDamageMultiplier;
  float meleeDamageMultiplier;
  int16_t cameraMode;
  uint8_t _padding66[2];
  qboolean autoRecenterOnAccel;
  qboolean thirdPersonDriver;
  qboolean thirdPersonUseVehicleRoll;
  qboolean thirdPersonCameraPitchVehicleRelative;
  qboolean thirdPersonCameraHeightWorldRelative;
  float thirdPersonCameraRange;
  float thirdPersonCameraMinPitchClamp;
  float thirdPersonCameraMaxPitchClamp;
  float thirdPersonCameraHeight[2];
  float thirdPersonCameraPitch[2];
  float thirdPersonCameraDriveOnWallHeight;
  qboolean cameraAlwaysAutoCenter;
  float cameraAutoCenterLerpRate;
  float cameraAutoCenterMaxLerpRate;
  qboolean lodNoCull;
  float thirdPersonCameraSpringDistance;
  float thirdPersonCameraSpringTime;
  float thirdPersonCameraHandbrakeTurnRateInc;
  float cameraRollFraction;
  scr::ScrString_t cameraTag;
  vec3_t tagPlayerOffset;
  float cameraMinFOV;
  float cameraMaxFOV;
  float cameraMinFOVSpeed;
  float cameraMaxFOVSpeed;
  float blurMin;
  float blurMax;
  float blurMinSpeed;
  float blurMaxSpeed;
  float blurInnerRadius;
  float blurOuterRadius;
  qboolean killcamCollision;
  float killcamDist;
  float killcamZDist;
  float killcamMinDist;
  float killcamZTargetOffset;
  float killcamFOV;
  float killcamNearBlur;
  float killcamNearBlurStart;
  float killcamNearBlurEnd;
  float killcamFarBlur;
  float killcamFarBlurStart;
  float killcamFarBlurEnd;
  qboolean isDrivable;
  int32_t numberOfSeats;
  int32_t numberOfGunners;
  int32_t seatSwitchOrder[11];
  int32_t driverControlledGunPos;
  float entryPointRadius[5];
  float texScrollScale;
  float wheelRotRate;
  float extraWheelRotScale;
  qboolean wheelChildTakesSteerYaw;
  qboolean useHeliBoneControllers;
  float minSpeed;
  float maxSpeed;
  float defaultSpeed;
  float maxSpeedVertical;
  float accel;
  float accelVertical;
  float rotRate;
  float rotAccel;
  float angDampening;
  float maxAngVelocityPitch;
  float maxTorquePitch;
  float rotMomentum;
  qboolean rotYawFromInput;
  float maxBodyPitch;
  float maxBodyRoll;
  float collisionDamage;
  float collisionSpeed;
  float suspensionTravel;
  qboolean aiSlidingTurn;
  qboolean meleeAvoidance;
  float heliCollisionScalar;
  float viewPitchOffset;
  float viewInfluence;
  float tiltFromAcceleration[2];
  float tiltFromDeceleration[2];
  float tiltFromVelocity[2];
  float tiltFromPrevious[2];
  float tiltSpeed[2];
  float vehHelicopterHoverRadius;
  float vehHelicopterHoverSpeed;
  float vehHelicopterHoverAccel;
  const char *turretWeapon;
  view_limits_t turretViewLimits;
  qboolean turretClampPlayerView;
  qboolean turretLockTurretToPlayerView;
  qboolean turretFireFromCamera;
  uint8_t _padding24C[4];
  const char *gunnerWeapon[4];
  uint16_t gunnerWeaponIndex[4];
  view_limits_t passengerViewLimits[6];
  const char *sndNames[7];
  snd::SndAliasId sndIndices[7];
  uint8_t _padding38C[4];
  const char *sndMaterialNames[3];
  float skidSpeedMin;
  float skidSpeedMax;
  float peelSpeedMin;
  float peelSpeedMax;
  const char *futzName;
  float futzBlend;
  uint8_t _padding3C4[4];
  VehicleSoundDefPtr soundDef;
  int32_t animType;
  uint8_t _padding3D4[4];
  const char *animSet;
  qboolean scriptedAnimationEntry;
  float mantleAngles[4];
  scr::ScrString_t extraWheelTags[4];
  scr::ScrString_t rotorArmTags[4];
  int32_t rotorArmFlapType[8];
  float rotorArmFlapInfluence[8];
  scr::ScrString_t driverHideTag;
  scr::ScrString_t driverOtherHideTags[4];
  XModelPtr attachmentModels[4];
  scr::ScrString_t attachmentTags[4];
  XModelPtr deathAttachmentModels[4];
  scr::ScrString_t deathAttachmentTags[4];
  uint32_t targetingImmunePerk;
  scr::ScrString_t targetTags[4];
  vec3_t aimAssistMins;
  vec3_t aimAssistMaxs;
  float tracerOffset[2];
  uint8_t _padding4FC[4];
  XModelPtr model;
  XModelPtr viewModel;
  XModelPtr deathModel;
  XModelPtr enemyModel;
  float modelSwapDelay;
  uint8_t _padding524[4];
  phys::FxEffectDefHandle exhaustFx;
  scr::ScrString_t exhaustFxTag1;
  scr::ScrString_t exhaustFxTag2;
  VehicleFxDefPtr surfaceFx;
  phys::FxEffectDefHandle deathFx;
  scr::ScrString_t deathFxTag;
  uint8_t _padding54C[4];
  const char *deathFxSound;
  phys::FxEffectDefHandle lightFx[4];
  scr::ScrString_t lightFxTag[4];
  float radiusDamageMin;
  float radiusDamageMax;
  float radiusDamageRadius;
  uint8_t _padding594[4];
  const char *shootShock;
  float deathQuakeScale;
  float deathQuakeDuration;
  float deathQuakeRadius;
  uint8_t _deathQuakeRadious[4];
  const char *rumbleType;
  float rumbleScale;
  float rumbleDuration;
  float rumbleRadius;
  float rumbleBaseTime;
  float rumbleAdditionalTime;
  int32_t healthDefault;
  int32_t eTeam;
  int32_t boostAccelMultiplier;
  float boostDuration;
  float boostDurationMin;
  float boostRecoveryScalar;
  float boostSpeedIncrease;
  qboolean addToCompass;
  qboolean addToCompassEnemy;
  qboolean addToCompassEnemyFiring;
  uint8_t _padding5F4[4];
  const char *compassIcon;
  MaterialHandle compassIconMaterial;
  scr::ScrString_t compassIconTag;
  float compassScale;
  const char *steerAxisName;
  int32_t steerAxis;
  uint8_t _padding61C[4];
  const char *gasAxisName;
  int32_t gasAxis;
  uint8_t _padding62C[4];
  const char *gasButtonName;
  int32_t gasButton;
  uint8_t _padding63C[4];
  const char *reverseBrakeButtonName;
  int32_t reverseBrakeButton;
  uint8_t _padding64C[4];
  const char *handBrakeButtonName;
  int32_t handBrakeButton;
  uint8_t _padding65C[4];
  const char *attackButtonName;
  int32_t attackButton;
  uint8_t _padding66C[4];
  const char *attackSecondaryButtonName;
  int32_t attackSecondaryButton;
  uint8_t _padding67C[4];
  const char *boostButtonName;
  int32_t boostButton;
  uint8_t _padding68C[4];
  const char *moveUpButtonName;
  int32_t moveUpButton;
  uint8_t _padding69C[4];
  const char *moveDownButtonName;
  int32_t moveDownButton;
  uint8_t _padding6AC[4];
  const char *switchSeatButtonName;
  int32_t switchSeatButton;
  qboolean noButtonAutoRemap;
  XString steerGraphName;
  GraphFloat *steerGraph;
  XString accelGraphName;
  GraphFloat *accelGraph;
  qboolean isNitrous;
  qboolean isFourWheelSteering;
  qboolean isMiddleWheelSteeringEnabled;
  qboolean useCollmap;
  float radius;
  float minHeight;
  float maxHeight;
  float lightCollisionSpeed;
  anim::RumbleInfoPtr lightCollisionRumble;
  float heavyCollisionSpeed;
  uint8_t _padding70C[4];
  anim::RumbleInfoPtr heavyCollisionRumble;
  anim::RumbleInfoPtr jumpLandingRumble;
  qboolean noDirectionalDamage;
  qboolean fakeBodyStabilizer;
  qboolean turnInPlace;
  qboolean alwaysDriveForward;
  qboolean smallVehicleCollision;
  float vehHelicopterBoundsRadius;
  float vehHelicopterDecelerationFwd;
  float vehHelicopterDecelerationSide;
  float vehHelicopterDecelerationUp;
  float vehHelicopterTiltFromControllerAxes;
  qboolean vehHelicopterPitchFromLookAxis;
  float vehHelicopterTiltFromFwdAndYaw;
  float vehHelicopterTiltFromFwdAndYaw_VelAtMaxTilt;
  float vehHelicopterTiltMomentum;
  float vehHelicopterJitterMinTime;
  float vehHelicopterJitterMaxTime;
  vec3_t vehHelicopterJitterMinAccel;
  vec3_t vehHelicopterJitterMaxAccel;
  float vehHelicopterJitterVelocityThreshold;
  float vehHelicopterJitterTiltPitchScale;
  float vehHelicopterJitterTiltRollScale;
  float vehHelicopterDogfightRollRate;
  vec3_t vehHelicopterDogfightLocalOffset;
  float vehHelicopterDogfightVelocityOffsetScalar;
  float vehHelicopterDogfightOvershootScalar;
  float vehHelicopterDogfightDesiredLocationLerpRate;
  float vehHelicopterDogfightSpeedFalloffDistance;
  float vehHelicopterDogfightSpeedFalloffDistanceExponent;
  float vehHelicopterDogfightControlLeewayAngle;
  float vehHelicopterDogfightFocusDampingAngle;
  float vehHelicopterDogfightFocusDampingAngleExponent;
  float vehHelicopterDogfightFocusAngleDeltaMaxRoll;
  float vehHelicopterDogfightCameraShakeScalar;
  vec3_t vehHelicopterDogfightCameraOffset;
  float vehHelicopterDogfightCameraLerpTime;
  qboolean vehHelicopterFlapRotor;
  qboolean vehHelicopterQuadRotor;
  qboolean vehHelicopterAccelTwardsView;
  qboolean vehHelicopterAccelTwardsViewWhenFiring;
  float maxRotorRotationSpeed;
  float idleRotorRotationSpeed;
  qboolean rotorArmRotateAroundY;
  float maxRotorArmMovementAngle;
  float maxRotorArmRotationAngle;
  vec2_t rotorArmMovementInfluence[4];
  qboolean vehHelicopterMaintainHeight;
  qboolean vehHelicopterMaintainMaxHeight;
  float vehHelicopterMaintainHeightLimit;
  float vehHelicopterMaintainHeightAccel;
  float vehHelicopterMaintainHeightMinimum;
  float vehHelicopterMaintainHeightMaximum;
  float vehHelicopterMaintainCeilingMinimum;
  qboolean joltVehicle;
  qboolean joltVehicleDriver;
  float joltMaxTime;
  float joltTime;
  float joltWaves;
  float joltIntensity;
  uint8_t _padding844[4];
  VehicleParameter nitrousVehParams;
  qboolean doFootSteps;
  qboolean ignoreVortices;
  qboolean isSentient;
  qboolean isPathfinder;
  scr::ScriptBundlePtr scriptBundleSettings;
  scr::ScriptBundlePtr vehicleRidersBundle;
  scr::ScriptBundlePtr vehicleRidersRobotBundle;
  scr::ScriptBundlePtr assassinationBundle;
  anim::AnimStateMachinePtr animStateMachine;
  anim::AnimSelectorTableSetPtr animSelectorTable;
  anim::AnimMappingTablePtr animMappingTable;
  VehicleAntenna antenna[4];
  char *csvInclude;
  float customFloat0;
  float customFloat1;
  float customFloat2;
  qboolean customBool0;
  qboolean customBool1;
  qboolean customBool2;
  snd::SurfaceSoundDefPtr vehicleFootstepTable;
  phys::SurfaceFXTableDefPtr vehicleFootstepFXTable;
  phys::DestructibleDefPtr destructibleDef;
  gfx::GfxImage *tacticalModeIcon;
  float tacticalModeHeight;
  uint8_t _paddingA9C[4];
};
static_assert(sizeof(VehicleDef) == 0xAA0,
              "VehicleDef size must be 2720 bytes");

// sizeof=0x18
struct VehicleStun {
  bool m_changed_this_frame;
  uint8_t _padding01[3];
  float m_stun_time;
  float m_last_throttle;
  float m_throttle_stun_time;
  float m_last_brake;
  float m_brake_stun_time;
};
static_assert(sizeof(VehicleStun) == 0x18, "VehicleStun size must be 24 bytes");

// sizeof=0x110
class NitrousVehicleController {
public:
  enum class VehicleHorseControlState : int32_t {
    LEFT_STICK_STEERS_HORSE_AND_CAMERA = 0x0,
    LEFT_STICK_STEERS_HORSE_RIGHT_STICK_CAMERA = 0x1,
    RIGHT_STICK_STEERS_HORSE_AND_CAMERA = 0x2,
  };

  math::Dir3 m_script_goal_position;
  float m_script_goal_radius;
  float m_script_goal_speed;
  float m_desired_yaw;
  bool m_has_desired_yaw;
  uint8_t _padding1D[3];
  float m_horse_target_speed;
  NitrousVehicleController::VehicleHorseControlState m_horse_control_state;
  float m_stopped_time;
  float m_braking_time;
  float m_stuck_time;
  uint8_t _padding34[12];
  math::Dir3 m_stuck_position;
  user::usercmd_t m_cmd;
  vec3_t m_cmd_viewangles;
  uint8_t _paddingAC[4];
  uint64_t m_pm_flags;
  bool m_cmd_valid;
  bool m_stop_at_goal;
  uint8_t _paddingBA[2];
  float m_jump_held_time;
  float m_stick_off_wall_time;
  vec3_t m_last_jump_normal;
  vec2_t m_jump_input;
  bool m_jump_awaiting_button_release;
  bool m_last_jump_button_press_initiated;
  bool m_last_jump_initiated_in_air;
  uint8_t _paddingDB[1];
  VehicleStun m_stun;
  bool m_hover_wall_targetting;
  uint8_t _paddingF5[3];
  game_button_bits_t m_last_update_button_bits;
  uint8_t _padding104[12];
};
static_assert(sizeof(NitrousVehicleController) == 0x110,
              "NitrousVehicleController size must be 272 bytes");

// sizeof=0x1C
class mover_record_t {
public:
  vec3_t m_origin;
  vec3_t m_angles;
  int32_t m_time;
};
static_assert(sizeof(mover_record_t) == 0x1C,
              "mover_record_t size must be 28 bytes");

class hkaiNavMeshInstance; // TODO
                           // sizeof=0x390
struct mover_info_t {
  mover_record_t m_records[32];
  int32_t m_last_record;
  uint8_t _padding384[4];
  hkaiNavMeshInstance *m_meshInstance;
};
static_assert(sizeof(mover_info_t) == 0x390,
              "mover_info_t size must be 912 bytes");

// sizeof=0x4
class minspec_mutex {
public:
  volatile uint32_t m_token;
};
static_assert(sizeof(minspec_mutex) == 0x4,
              "minspec_mutex size must be 4 bytes");

struct NitrousVehicle {
  phys::PhysObjUserData *m_phys_user_data;
  WheelState m_wheel_state[6];
  math::RotTranMat43 m_wheel_orig_relpo[6];
  uint8_t _padding98[8];
  int32_t m_wheel_damage[6];
  int32_t m_wheel_surf_types[6];
  float m_wheel_yaw[6];
  float m_steer_factor;
  float m_steer_scale;
  const VehicleParameter *m_parameter;
  float m_throttle;
  float m_brake;
  float m_hand_brake;
  float m_script_brake;
  int64_t m_boost_start_time;
  float m_boost_time_pool;
  float m_forward_vel;
  uint8_t _padding298[8];
  math::Dir3 m_ground_vel;
  math::Dir3 m_last_known_velocity;
  float m_hand_brake_friction_time;
  uint8_t _padding2c4[4];
  gentity_s *m_owner;
  int32_t m_entnum;
  uint8_t _padding2d4[4];
  const VehicleDef *m_vehicle_def;
  XModel *m_xmodel;
  phys::rigid_body_constraint_custom_orientation *m_orientation_constraint;
  phys::rigid_body_constraint_custom_path *m_vpc;
  int32_t m_flags;
  int32_t m_notify_flags;
  int32_t m_server_notify_flags;
  vec3_t m_collision_hitp;
  vec3_t m_collision_hitn;
  float m_collision_intensity;
  int32_t m_collision_stype;
  int32_t m_collision_entnum;
  uint8_t _padding328[8];
  math::RotTranMat43 m_mat;
  NitrousVehicleController mVehicleController;
  float m_fake_rpm;
  int32_t m_num_colliding_wheels;
  float m_current_side_fric_scale;
  float m_current_fwd_fric_scale;
  float m_stuck_time;
  int32_t m_lastNetworkTime;
  int32_t m_lastErrorReductionTime;
  vec3_t m_networkErrorOrigin;
  vec3_t m_networkErrorAngles;
  int32_t m_trackDistAccumLeftInches;
  int32_t m_trackDistAccumRightInches;
  int32_t m_sfx_state_flags;
  float m_speed_scale;
  float m_drivepath_scale;
  float m_accel_scale;
  int32_t m_throttle_held_down;
  bool m_throttle_time_started;
  bool m_last_all_wheel_contact_was_wall;
  bool m_is_being_drop_deployed;
  uint8_t _padding4D3[1];
  vec3_t m_last_origin;
  vec3_t m_last_angles;
  float m_buoyancyOffset;
  mover_info_t m_mover_info;
  minspec_mutex m_mutex;
  int32_t id;
  int32_t refcount;
  uint8_t _padding88C[4];
  phys::rigid_body_constraint_wheel *m_wheels[6];
  float m_desired_speed_factor;
  float m_acceleration_factor;
  float m_power_braking_factor;
  float m_braking_factor;
  float m_coasting_factor;
  float m_reference_wheel_radius;
  float m_steer_current_angle;
  float m_steer_max_angle;
  float m_steer_speed;
  uint8_t _padding8E4[12];
  math::Dir3 m_steer_front_pt_loc;
  float m_steer_front_back_length;
  float m_min_turning_radius;
  uint32_t m_state_flags;
  uint8_t _padding90C[4];
};

// sizeof=0x18
struct vehicle_spline_node_t {
  int16_t nextIdx;
  int16_t prevIdx;
  float length;
  vec3_t dir;
  float t;
};
static_assert(sizeof(vehicle_spline_node_t) == 0x18,
              "vehicle_spline_node_t size must be 24 bytes");

// sizeof=0x5C
struct vehicle_node_t {
  scr::ScrString_t name;
  scr::ScrString_t target;
  scr::ScrString_t target2;
  scr::ScrString_t script_linkname;
  scr::ScrString_t script_noteworthy;
  int16_t index;
  uint8_t _padding16[2];
  int32_t flags;
  float speed;
  float lookAhead;
  vec3_t origin;
  vec3_t angles;
  float radius;
  float tension;
  vehicle_spline_node_t splineNode;
};
static_assert(sizeof(vehicle_node_t) == 0x5C,
              "vehicle_node_t size must be 92 bytes");

enum path_type_e : int32_t {
  PT_NONE = 0x0,
  PT_OLD_SPLINE = 0x1,
  PT_SPLINE = 0x2,
  PT_AI_PATH = 0x3,
};

// sizeof=0x164
class vehicle_pathpos_t {
public:
  int16_t nodeIdx;
  int16_t prevIdx;
  int16_t endOfPath;
  float frac;
  float speed;
  float lookAhead;
  float slide;
  float width;
  float manualTime;
  float widthLookAheadFrac;
  float distanceTraveled;
  vec3_t origin;
  vec3_t angles;
  vec3_t lookPos;
  vec3_t origin_on_spline;
  vec3_t angles_on_spline;
  vec3_t lookPos_on_spline;
  qboolean use_spline_speed;
  float manualSpeed;
  vehicle_node_t switchNode[2];
  int32_t flags;
  float customGoalLength;
  vec3_t customGoalDir;
  vec3_t aiGoalPos;
  path_type_e path_type;
  int16_t startNodeIdx;
  float t;
  float tot_len;
  float tot_time;
};

// sizeof=0x118
struct vehicle_physic_t {
  vec3_t origin;
  vec3_t prevOrigin;
  vec3_t angles;
  vec3_t prevAngles;
  vec3_t maxAngleVel;
  float yawAccel;
  float yawDecel;
  vec3_t mins;
  vec3_t maxs;
  vec3_t vel;
  vec3_t bodyVel;
  vec3_t rotVel;
  vec3_t accel;
  float maxPitchAngle;
  float maxRollAngle;
  float wheelZVel[6];
  float wheelZPos[6];
  int32_t wheelSurfType[6];
  vec3_t bodyTilt;
  vec3_t worldTilt;
  vec3_t worldTiltVel;
  float heliLockHeight;
  float curveLength;
  int32_t curveID;
  float curveStep;
  float curveTime;
  float timeStep;
};
static_assert(sizeof(vehicle_physic_t) == 0x118,
              "vehicle_physic_t size must be 280 bytes");

enum class VehicleTargetType : int32_t {

  VEHICLE_TARGET_NONE = 0x0,
  VEHICLE_TARGET_ENTITY = 0x1,
  VEHICLE_TARGET_POSITION = 0x2,
  VEHICLE_TARGET_RELATIVE_ANGLES = 0x3,
};

enum class VehicleTurretState : int32_t {

  VEH_TURRET_STOPPED = 0x0,
  VEH_TURRET_STOPPING = 0x1,
  VEH_TURRET_MOVING_CW = 0x2,
  VEH_TURRET_MOVING_CCW = 0x3,
};

enum class VehicleMoveState : int32_t {
  VEH_MOVESTATE_STOP = 0x0,
  VEH_MOVESTATE_MOVE = 0x1,
  VEH_MOVESTATE_HOVER = 0x2,
  VEH_MOVESTATE_PLANE_ONCURVE = 0x3,
  VEH_MOVESTATE_PLANE_FREE = 0x4,
};

enum class AwarenessLevel : int32_t {

  AI_AWARENESS_UNAWARE = 0x0,
  AI_AWARENESS_LOW_ALERT = 0x1,
  AI_AWARENESS_HIGH_ALERT = 0x2,
  AI_AWARENESS_COMBAT = 0x3,
  NUM_AWARENESS_LEVELS = 0x4,
};

enum class behaviorTreeNodeResult_t : int32_t {

  BHTN_INVALID = 0x0,
  BHTN_DEFAULT = 0x0,
  BHTN_TERMINATE = 0x1,
  BHTN_TERMINATED = 0x2,
  BHTN_FAILURE = 0x3,
  BHTN_SUCCESS = 0x4,
  BHTN_RUNNING = 0x5,
  BHTN_NODE_RESULT_COUNT = 0x6,
};

enum class CombatMode : int32_t {
  AI_COMBAT_COVER = 0x0,
  AI_COMBAT_NO_COVER = 0x1,
  AI_COMBAT_COVER_ONLY = 0x2,
  AI_COMBAT_AMBUSH = 0x3,
  AI_COMBAT_AMBUSH_NODES_ONLY = 0x4,
  AI_COMBAT_EXPOSED_NODES_ONLY = 0x5,
  AI_COMBAT_ANY_EXPOSED_NODES_ONLY = 0x6,
  NUM_COMBAT_MODES = 0x7,
};

enum class AlertLevel : int32_t {
  AI_ALERTNESS_ASLEEP = 0x0,
  AI_ALERTNESS_NONCOMBAT = 0x1,
  AI_ALERTNESS_LOW_ALERT = 0x2,
  AI_ALERTNESS_HIGH_ALERT = 0x3,
  AI_ALERTNESS_COMBAT = 0x4,
  NUM_ALERT_LEVELS = 0x5,
};

enum class ai_badplace_t : int32_t {
  AI_BADPLACE_NONE = 0x0,
  AI_BADPLACE_NORMAL = 0x1,
  AI_BADPLACE_REALLYBAD = 0x2,
};

enum class ai_animmode_t : int32_t {

  AI_ANIM_UNKNOWN = 0x0,
  AI_ANIM_MOVE_CODE = 0x1,
  AI_ANIM_MOVE_CODE_NOGRAVITY = 0x2,
  AI_ANIM_USE_POS_DELTAS = 0x3,
  AI_ANIM_USE_ANGLE_DELTAS = 0x4,
  AI_ANIM_PROCEDURAL_MODES = 0x4,
  AI_ANIM_USE_ANGLE_DELTAS_NOCLIP = 0x5,
  AI_ANIM_USE_BOTH_DELTAS = 0x6,
  AI_ANIM_USE_BOTH_DELTAS_NOCLIP_STAIRS = 0x7,
  AI_ANIM_USE_BOTH_DELTAS_NOCLIP = 0x8,
  AI_ANIM_USE_BOTH_DELTAS_NOGRAVITY = 0x9,
  AI_ANIM_USE_BOTH_DELTAS_ZONLY_PHYSICS = 0xA,
  AI_ANIM_NOPHYSICS = 0xB,
  AI_ANIM_POINT_RELATIVE = 0xC,
  AI_ANIM_SLIDE = 0xD,
  AI_ANIM_PRETRAVERSAL = 0xE,
  AI_ANIM_MODE_COUNT = 0xF,
};

enum class ai_traverse_mode_t : int32_t {
  AI_TRAVERSE_INVALID = 0x0,
  AI_TRAVERSE_GRAVITY = 0x1,
  AI_TRAVERSE_NOGRAVITY = 0x2,
  AI_TRAVERSE_NOCLIP = 0x3,
  AI_TRAVERSE_COUNT = 0x4,
};

enum class ai_stance_t : int32_t {

  STANCE_BAD = 0x0,
  STANCE_STAND = 0x1,
  STANCE_CROUCH = 0x2,
  STANCE_PRONE = 0x4,
  STANCE_ANY = 0x7,
  STANCE_NONE = 0x0,
};

enum class DamageMode : int32_t {
  AI_DAMAGE_NORMAL = 0x0,
  AI_DAMAGE_NEXT_SHOT_KILLS = 0x1,
  NUM_DAMAGE_MODES = 0x2,
};

enum class ai_orient_mode_t : int32_t {
  AI_ORIENT_INVALID = 0x0,
  AI_ORIENT_DONT_CHANGE = 0x1,
  AI_ORIENT_TO_MOTION = 0x2,
  AI_ORIENT_TO_ENEMY = 0x3,
  AI_ORIENT_TO_ENEMY_OR_MOTION_SIDESTEP = 0x4,
  AI_ORIENT_TO_GOAL = 0x5,
  AI_ORIENT_COUNT = 0x6,
};

enum class ai_state_transition_t : uint32_t {

  AIS_TRANSITION_CANONICAL = 0xFFFFFFFF,

  AIS_TRANSITION_NONE = 0x0,

  AIS_TRANSITION_SET = 0x1,

  AIS_TRANSITION_PUSH = 0x2,
  AIS_TRANSITION_POP = 0x3,

};

enum class ai_substate_t : int32_t {

  STATE_EXPOSED_BEHAVE = 0x64,
  STATE_EXPOSED_COMBAT = 0x65,
  STATE_EXPOSED_NONCOMBAT = 0x66,
  STATE_EXPOSED_REACQUIRE_MOVE = 0x67,
  STATE_EXPOSED_REACQUIRE_DONE = 0x68,
  STATE_EXPOSED_FLASHBANGED = 0x69,
  STATE_EXPOSED_REACTION = 0x6A,
  STATE_DEATH_PRECLEANUP = 0xC8,
  STATE_DEATH_POSTCLEANUP = 0xC9,
  STATE_GRENADE_FLEE = 0x12C,
  STATE_GRENADE_TAKECOVER = 0x12D,
  STATE_GRENADE_COWER = 0x12E,
  STATE_GRENADE_COMBAT = 0x12F,
  STATE_GRENADE_COVERATTACK = 0x130,
  STATE_GRENADE_ACQUIRE = 0x131,
  STATE_GRENADE_THROWBACK = 0x132,
};

enum class ai_state_t : int32_t {

  AIS_INVALID = 0x0,

  AIS_KEEPCURRENT = 0x0,
  AIS_BEHAVE = 0x1,
  AIS_DEFAULT = 0x1,
  AIS_SETABLE_FIRST = 0x1,
  AIS_SETABLE_LAST = 0x1,
  AIS_SCRIPTEDANIM = 0x2,

  AIS_PUSHABLE_FIRST = 0x2,
  AIS_PUSHABLE_LAST = 0x2,
  AIS_COUNT = 0x3,
};

enum class AISpecies : int32_t {

  AI_SPECIES_HUMAN = 0x0,
  AI_SPECIES_DOG = 0x1,
  AI_SPECIES_ZOMBIE = 0x2,
  AI_SPECIES_ZOMBIE_DOG = 0x3,
  AI_SPECIES_ROBOT = 0x4,
  MAX_AI_SPECIES = 0x5,
  AI_SPECIES_ALL = 0x5,
};

// sizeof=0x24
struct VehicleTurret {
  int32_t fireTime;
  int32_t fireBarrel;
  float barrelOffset;
  int32_t flags;
  float heatVal;
  float turretOnTargetRange;
  int32_t weaponShotCount;
  float accuracy;
  VehicleTurretState turretState;
};
static_assert(sizeof(VehicleTurret) == 0x24,
              "VehicleTurret size must be 36 bytes");

// sizeof=0x3C
struct VehicleJitter {
  int32_t jitterPeriodMin;
  int32_t jitterPeriodMax;
  int32_t jitterEndTime;
  vec3_t jitterOffsetRange;
  vec3_t jitterDeltaAccel;
  vec3_t jitterAccel;
  vec3_t jitterPos;
};
static_assert(sizeof(VehicleJitter) == 0x3C,
              "VehicleJitter size must be 60 bytes");

// sizeof=0x1C
struct VehicleHover {
  float hoverRadius;
  float hoverSpeed;
  float hoverAccel;
  vec3_t hoverGoalPos;
  qboolean useHoverAccelForAngles;
};
static_assert(sizeof(VehicleHover) == 0x1C,
              "VehicleHover size must be 28 bytes");

// sizeof=0x10
struct VehicleGunnerTags {
  BoneIndex turretParent;
  BoneIndex turret;
  BoneIndex barrel;
  BoneIndex aim;
  BoneIndex flash[4];
};
static_assert(sizeof(VehicleGunnerTags) == 0x10,
              "VehicleGunnerTags size must be 16 bytes");

// sizeof=0x84
struct VehicleTags {
  BoneIndex player;
  BoneIndex popout;
  BoneIndex body;
  BoneIndex eye;
  VehicleGunnerTags turretTags;
  VehicleGunnerTags gunnerTags[4];
  BoneIndex wheel[6];
  BoneIndex seats[11];
  BoneIndex entryPoints[5];
};
static_assert(sizeof(VehicleTags) == 0x84,
              "VehicleTags size must be 132 bytes");

// sizeof=0x3C
struct VehicleCornerCache {
  vec3_t start;
  vec3_t end;
  vec3_t center;
  vec3_t travelVector;
  int32_t lastUpdateTime;
  int32_t startSlidingTime;
  float travelDistance;
};
static_assert(sizeof(VehicleCornerCache) == 0x3C,
              "VehicleCornerCache size must be 60 bytes");

// sizeof=0x2C
struct VehicleTarget {
  VehicleTargetType targetType;
  int32_t targetEnt;
  vec3_t targetOrigin;
  vec3_t targetOffset;
  vec3_t targetRelativeAngles;
};
static_assert(sizeof(VehicleTarget) == 0x2C,
              "VehicleTarget size must be 44 bytes");

enum class ClientFallSpeed : int32_t {

  FALL_SPEED_NONE = 0x0,
  FALL_SPEED_DEFAULT = 0x0,
  FALL_SPEED_SLOW = 0x1,
  FALL_SPEED_MED = 0x2,
  FALL_SPEED_FAST = 0x3,
  FALL_SPEED_RAPID = 0x4,
  FALL_SPEED_COUNT = 0x5,
};

enum class ClientVisibleBodyMode : int32_t {

  CLIENT_VISIBLEBODYMODE_UNKNOWN = 0x0,
  CLIENT_VISIBLEBODYMODE_FULL = 0x1,
  CLIENT_VISIBLEBODYMODE_FIRST_PERSON_CINEMATIC = 0x2,
  CLIENT_VISIBLEBODYMODE_FIRST_PERSON_LEGS = 0x3,
};

enum class ShockViewTypes : int32_t {
  SHELLSHOCK_VIEWTYPE_BLURRED = 0x0,
  SHELLSHOCK_VIEWTYPE_FLASHED = 0x1,
  SHELLSHOCK_VIEWTYPE_CONCUSSED = 0x2,
  SHELLSHOCK_VIEWTYPE_SHOCKED = 0x3,
  SHELLSHOCK_VIEWTYPE_NONE = 0x4,
  SHELLSHOCK_VIEWTYPE_COUNT = 0x5,
};

enum class CameraMode : int32_t {

  CAM_NORMAL = 0x0,
  CAM_LINKED = 0x1,
  CAM_VEHICLE = 0x2,
  CAM_VEHICLE_THIRDPERSON = 0x3,
  CAM_VEHICLE_GUNNER = 0x4,
  CAM_TURRET = 0x5,
  CAM_MISSILE = 0x6,
  CAM_SPIKECAMERA = 0x7,
  CAM_SCRIPTED_EXTRACAM = 0x8,
  CAM_SCOPE = 0x9,
  CAM_PLAYER_BODY = 0xA,
  CAM_XCAM = 0xB,
  CAM_SCRIPTED = 0xC,
  CAM_RADIANT = 0xD,
};

enum class VehicleAnimState : int32_t {
  VEHICLEANIMSTATE_IDLE = 0x0,
  VEHICLEANIMSTATE_ENTRY = 0x1,
  VEHICLEANIMSTATE_CHANGEPOS = 0x2,
  VEHICLEANIMSTATE_EXIT = 0x3,
  VEHICLEANIMSTATECOUNT = 0x4,
};

#pragma pack(pop)

#pragma pack(push, 1)

// sizeof=0x4
union CollisionAabbTreeIndex {
  int32_t firstChildIndex;
  int32_t partitionIndex;
};
static_assert(sizeof(CollisionAabbTreeIndex) == 0x4,
              "CollisionAabbTreeIndex size must be 4 bytes");

// sizeof=0x20
struct CollisionAabbTree {
  vec3_t origin;
  uint16_t materialIndex;
  uint16_t childCount;
  vec3_t halfSize;
  CollisionAabbTreeIndex u;
};
static_assert(sizeof(CollisionAabbTree) == 0x20,
              "CollisionAabbTree size must be 32 bytes");
struct cbrush_t; // TODO

// sizeof=0x10
struct col_prim_t {
  int32_t type;
  uint8_t _padding04[4];
  union {
    const CollisionAabbTree *tree;
    const cbrush_t *brush;
  };
};
static_assert(sizeof(col_prim_t) == 0x10, "col_prim_t size must be 16 bytes");

class float4 {
public:
  uint8_t gap0[16];
};

// sizeof=10
class hybrid_vector {
public:
  float4 vec;
};
static_assert(sizeof(hybrid_vector) == 0x10,
              "hybrid_vector size must be 16 bytes");

struct TraceThreadInfo; // TODO
class visitor_base_t {
public:
  // int32_t (**_vptr$visitor_base_t)(void);
  void *_vptr$visitor_base_t;
};

class colgeom_visitor_t : visitor_base_t {
public:
  uint8_t _padding0[0xF];
  hybrid_vector m_mn;
  hybrid_vector m_mx;
  hybrid_vector m_p0;
  hybrid_vector m_p1;
  hybrid_vector m_delta;
  hybrid_vector m_rvec;
  float m_radius;
  phys::contents_t m_mask;
  TraceThreadInfo *m_threadInfo;
};

#pragma pack(pop)
#pragma pack(push, 16)
template <size_t T> class colgeom_visitor_inlined_t : colgeom_visitor_t {
  int32_t nprims;
  bool overflow;
  col_prim_t prims[T];
};

// sizeof=0x1350
typedef colgeom_visitor_inlined_t<300> vehicle_proximity_data_t;
static_assert(sizeof(vehicle_proximity_data_t) == 0x1350,
              "vehicle_proximity_data_t size must be 4944 bytes");

#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x1430
struct vehicle_cache_t {
  vec3_t lastOrigin;
  vec3_t lastAngles;
  int32_t hit_indices[6];
  int32_t hit_sflags[6];
  vec3_t hit_normals[6];
  float hit_fractions[6];
  TraceHitType hit_type[6];
  uint16_t hit_id[6];
  uint8_t _paddingCC[4];
  vehicle_proximity_data_t proximity_data;
  int32_t wheel_mask;
  uint8_t _padding1424[12];
};
static_assert(sizeof(vehicle_cache_t) == 0x1430,
              "vehicle_cache_t size must be 5168 bytes");

// sizeof=0x8
struct VehicleSeat {
  int32_t occupantEntNum;
  bool scriptOccupied;
  uint8_t _padding5[3];
};
static_assert(sizeof(VehicleSeat) == 0x8, "VehicleSeat size must be 8 bytes");

// sizeof=0x690
struct AIBody {
  int32_t randSeed;
  uint8_t _padding04[4];
  anim::AnimStateMachineRuntime asmRuntime;
  const anim::AnimSelectorTableSet *animTableSet;
  const anim::AnimMappingTable *animMappingTables[3];
  int32_t currentAnimMappingTableIndex;
  uint8_t _padding35C[4];
  const anim::AimTable *aimTable;
  struct {
    union {
      anim::AnimSelectorTableCache animationSelectorCache;
      anim::AnimSelectorTableCache transitionDecoratorCache;
    };
    anim::AnimSelectorTableCache caches[2];
  };
  uint8_t _padding68C[4];
};
static_assert(sizeof(AIBody) == 0x690, "AIBody size must be 1680 bytes");

struct vehicle_t {
  vehicle_pathpos_t pathPos;
  int32_t pathOffsetFlags;
  vec3_t pathOffsetTransform[4];
  vec3_t pathFixedOffset;
  vec3_t pathVariableOffset;
  vec3_t pathVariableOffsetTargetPrev;
  vec3_t pathVariableOffsetTarget;
  float pathVariableOffsetRate;
  float pathVariableOffsetTime;
  vehicle_physic_t phys;
  int32_t entNum;
  int16_t defIndex;
  uint8_t _padding2EE[2];
  int32_t flags;
  VehicleMoveState moveState;
  VehicleTurret turret;
  float turretRotScale;
  VehicleJitter jitter;
  VehicleHover hover;
  VehicleTurret gunnerTurrets[4];
  VehicleJitter gunnerJitter[4];
  float minigunRotationSpeed;
  int32_t manualMode;
  float manualSpeed;
  float manualAccel;
  float manualDecel;
  float speed;
  float maxSpeedOverride;
  float maxDragSpeed;
  float turningAbility;
  qboolean hasGoalYaw;
  qboolean stopAtGoal;
  qboolean stopping;
  qboolean hasLookAtOrigin;
  EntHandle lookAtEnt;
  vec3_t lookAtOrigin;
  vec3_t goalPosition;
  float goalYaw;
  VehicleCornerCache cornerCache;
  qboolean hasTargetYaw;
  float targetYaw;
  float minDesiredTurnYaw;
  float prevDesiredYaw;
  float pathTransitionTime;
  float maxPathTransitionTime;
  vec3_t pathTransitionOffset;
  vec3_t pathTransitionAngleOffset;
  qboolean hasDefaultPitch;
  float defaultPitch;
  float hasGoalRoll;
  float goalRoll;
  float goalRollTime;
  float currentRollTime;
  int32_t numRolls;
  VehicleTarget mainTurretTarget;
  VehicleTarget gunnerTargets[4];
  float nearGoalNotifyDist;
  vec2_t joltDir;
  float joltTime;
  float joltWave;
  float joltSpeed;
  float joltDecel;
  VehicleTags boneIndex;
  int32_t turretHitNum;
  VehicleSeat seats[11];
  qboolean secTurretAiControlled;
  qboolean addToCompass;
  uint8_t _padding7B4[4];
  NitrousVehicle *nitrousVehicle;
  vehicle_cache_t vehicle_cache;
  float heliHeightLockOffset;
  float lastViewOffset;
  int32_t driveBySoundIndex[2];
  float driveBySoundDelay[2];
  float driveBySoundTimeout[2];
  ai::sentient_t *sentient;
  AIBody aiBody;
  uint8_t tmodeVehicleVisibility;
  uint8_t _padding22A9[3];
  float predictedCollisionTime;
};

// sizeof=0x684
struct vehicleAntilagFrame_t {
  vec3_t position[64];
  vec3_t angles[64];
  uint8_t useCount[64];
  uint8_t inUse[64];
  int time;
};
static_assert(sizeof(vehicleAntilagFrame_t) == 0x684,
              "vehicleAntilagFrame_t size must be 1668 bytes");

#pragma pack(pop)
} // namespace vechicle
} // namespace game