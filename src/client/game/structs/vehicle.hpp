#pragma once

#include "asm.hpp"
#include "ai.hpp"
#include "math.hpp"
#include "phys/core.hpp"
#include "snd/snd.hpp"
#include "gfx/gfx.hpp"
#include "core.hpp"
#include "user.hpp"
#include "hk/ai.hpp"
#include "cm.hpp"
#include "move.hpp"
#include <game/symbols/macros.hpp>
#include <game/symbol.hpp>

namespace game {
namespace level {
struct playerState_s;
typedef playerState_s playerState_t;

struct gentity_s;
typedef gentity_s gentity_t;
} // namespace level
namespace vehicle {

enum class vehicleFlags_e : uint32_t {
  PAUSED = 0x1,
  INITIALIZED = 0x2,
  SCRIPT_CONTROL = 0x4,
  DEBUG_RENDER = 0x8,
  PLAYERS_VEHICLE = 0x40,
  ATTACHED_PATH = 0x80,
  DRIVING_PATH = 0x100,
  ACTUATOR_DISABLED = 0x200,
  IN_WATER = 0x400,
  DISABLE_STABILIZERS = 0x800,
  STUNNED = 0x1000,
  NEEDS_UNPAUSE = 0x2000,
  NEEDS_ENTITIES_WHEELS_COLLISION = 0x4000,
  NEEDS_WHEEL_SETUP = 0x8000,
  BOOSTING = 0x10000,
  ANIMSCRIPTED = 0x20000,
};
IMPL_ENUM_OPERATORS(vehicleFlags_e);

template <typename T>
concept VehicleFlagLike = IntegralLike<T, uint32_t>;

#pragma pack(push, 1)
union VehicleFlags {
  uint32_t _raw;

  struct {
    uint32_t paused : 1;                          // B0
    uint32_t initialized : 1;                     // B1
    uint32_t script_control : 1;                  // B2
    uint32_t debug_render : 1;                    // B3
    uint32_t reserved_b4 : 1;                     // B4
    uint32_t reserved_b5 : 1;                     // B5
    uint32_t players_vehicle : 1;                 // B6
    uint32_t attached_path : 1;                   // B7
    uint32_t driving_path : 1;                    // B8
    uint32_t actuator_disabled : 1;               // B9
    uint32_t in_water : 1;                        // B10
    uint32_t disable_stabilizers : 1;             // B11
    uint32_t stunned : 1;                         // B12
    uint32_t needs_unpause : 1;                   // B13
    uint32_t needs_entities_wheels_collision : 1; // B14
    uint32_t needs_wheel_setup : 1;               // B15
    uint32_t boosting : 1;                        // B16
    uint32_t animscripted : 1;                    // B17
    uint32_t reserved_b18_31 : 14;
  };

  inline constexpr operator uint32_t() const noexcept { return _raw; }

  template <VehicleFlagLike T>
  static constexpr VehicleFlags from(T val) noexcept {
    return VehicleFlags{static_cast<uint32_t>(val)};
  }
  template <VehicleFlagLike T> inline constexpr void add(T flags) noexcept {
    _raw |= static_cast<uint32_t>(flags);
  }

  template <VehicleFlagLike T> inline constexpr void remove(T flags) noexcept {
    _raw &= static_cast<uint32_t>(~flags);
  }

  inline constexpr void clear() noexcept { _raw = 0; }

  template <VehicleFlagLike T> inline constexpr void set(T flags) noexcept {
    _raw = static_cast<uint32_t>(flags);
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags add(T flags) const noexcept {
    return VehicleFlags{_raw | static_cast<uint32_t>(flags)};
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags remove(T flags) const noexcept {
    return VehicleFlags{_raw & static_cast<uint32_t>(~flags)};
  }

  inline constexpr VehicleFlags clear() const noexcept {
    return VehicleFlags{0};
  }

  template <VehicleFlagLike T>
  inline constexpr VehicleFlags set(T flags) const noexcept {
    return VehicleFlags{static_cast<uint32_t>(flags)};
  }
};
#pragma pack(pop)

inline constexpr VehicleFlags operator~(VehicleFlags flag) noexcept {
  return VehicleFlags{~flag._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator|=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw |= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator&=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw &= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags &operator^=(VehicleFlags &lhs, T rhs) noexcept {
  lhs._raw ^= static_cast<uint32_t>(rhs);
  return lhs;
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator|(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw | static_cast<uint32_t>(rhs)};
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator|(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) | rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator&(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw & static_cast<uint32_t>(rhs)};
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator&(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) & rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr VehicleFlags operator^(VehicleFlags lhs, T rhs) noexcept {
  return VehicleFlags{lhs._raw ^ static_cast<uint32_t>(rhs)};
}
template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr VehicleFlags operator^(T lhs, VehicleFlags rhs) noexcept {
  return VehicleFlags{static_cast<uint32_t>(lhs) ^ rhs._raw};
}

template <VehicleFlagLike T>
inline constexpr bool operator==(VehicleFlags lhs, T rhs) noexcept {
  return lhs._raw == static_cast<uint32_t>(rhs);
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr bool operator==(T lhs, VehicleFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) == rhs._raw;
}

template <VehicleFlagLike T>
inline constexpr bool operator!=(VehicleFlags lhs, T rhs) noexcept {
  return lhs._raw != static_cast<uint32_t>(rhs);
}

template <
    VehicleFlagLike T,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, VehicleFlags>>>
inline constexpr bool operator!=(T lhs, VehicleFlags rhs) noexcept {
  return static_cast<uint32_t>(lhs) != rhs._raw;
};
ASSERT_SIZE(VehicleFlags, sizeof(uint32_t));
ASSERT_CPP03_POD(VehicleFlags);

PACKED(struct WheelState {
  int32_t m_state;
  int32_t m_last_state;
  float m_rate[4];
});
ASSERT_SIZE(WheelState, 0x18);

enum class VehicleCategory : int32_t {
  NONE = 0x0,
  QUAD_TANK = 0x1,
  AMWS = 0x2,
  WASP = 0x3,
  ROCKET_WASP = 0x4,
  HUNTER = 0x5,
  SIEGE_BOT = 0x6,
  COUNT = 0x7,
};

PACKED(struct view_limits_t {
  float horizSpanLeft;
  float horizSpanRight;
  float vertSpanUp;
  float vertSpanDown;
  float horizResistLeft;
  float horizResistRight;
  float vertResistUp;
  float vertResistDown;
});
ASSERT_SIZE(view_limits_t, 0x20);

PACKED(struct VehicleGearData {
  float minRPM;
  float maxRPM;
  float ratio;
});
ASSERT_SIZE(VehicleGearData, 0xC);

PACKED(struct VehicleEngineSound {
  const char *name;
  snd::SndAliasId alias;
  float params[7];
});
ASSERT_SIZE(VehicleEngineSound, 0x28);

struct XModel; // TODO
typedef XModel *XModelPtr;

PACKED(struct VehicleEngine {
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
});
ASSERT_SIZE(VehicleEngine, 0x2C0);

struct DevGraph;

enum class DevEventType : int32_t {
  ACTIVATE = 0x0,
  DEACTIVATE = 0x1,
  ACCEPT = 0x2,
  UPDATE = 0x3,
  DRAW = 0x4,
  SAVE = 0x5,
};

typedef thiscallPtr_t<void(const DevGraph *, DevEventType type,
                           LocalClientNum_t localClientNum)>
    DevGraphEventCallback;
typedef thiscallPtr_t<void(const DevGraph *, const float, const float,
                           char *text, const int)>
    DevGraphTextCallback;

PACKED(struct DevGraph {
  vec2_t *knots;
  int32_t *knotCount;
  int32_t knotCountMax;
  int32_t selectedKnot;
  DevGraphEventCallback eventCallback;
  DevGraphTextCallback textCallback;
  void *data;
  bool disableEditingEndPoints;
  uint8_t _padding31[7];
});
ASSERT_SIZE(DevGraph, 0x38);

PACKED(struct GraphFloat {
  char name[64];
  vec2_t knots[32];
  int32_t knotCount;
  float scale;
  DevGraph devguiGraph; // TODO: is this used in release?
});
ASSERT_SIZE(GraphFloat, 0x180);

PACKED(struct AircraftPreset {
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
});
ASSERT_SIZE(AircraftPreset, 0x28);

PACKED(struct AircraftSound {
  const char *presetName;
  AircraftPreset *preset;
  const char *aliasName;
  snd::SndAliasId alias;
  uint8_t _padding1C[4];
  const char *tagName;
  scr::ScrString_t tagScriptString;
  uint8_t _padding2C[4];
});
ASSERT_SIZE(AircraftSound, 0x30);

PACKED(struct VehicleSurfaceSound {
  const char *surfaceSoundName;
  snd::SndAliasId alias;
  uint8_t _padding0C[4];
});
ASSERT_SIZE(VehicleSurfaceSound, 0x10);

PACKED(struct VehicleDriveBySound {
  int32_t apex;
  uint8_t _padding04[4];
  const char *name;
  snd::SndAliasId alias;
  uint8_t _padding14[4];
});

PACKED(struct VehicleSoundDef {
  const char *name;
  VehicleEngine engine;
  XString engineGraphName;
  GraphFloat *engineGraph;
  float driveBySoundRadius[2];
  VehicleDriveBySound driveBySounds[40];
  AircraftPreset aircraftPresets[20];
  AircraftSound aircraftSounds[20];
  VehicleSurfaceSound surfaceSounds[40];
});
// Verified
ASSERT_SIZE(VehicleSoundDef, 0x1000);

typedef VehicleSoundDef *VehicleSoundDefPtr;

enum class VehicleFxDefType : int32_t {
  NITROUS = 0x0,
  GROUND = 0x1,
  AIRCRAFT = 0x2,
  BOAT = 0x3,
  TANK = 0x4,
  COUNT = 0x5,
};

PACKED(struct VehicleFxDef {
  const char *name;
  char *csvInclude;
  VehicleFxDefType type;
  uint8_t _padding14[4];
  db::xasset::FxEffectDefHandle treadFx[40];
  db::xasset::FxEffectDefHandle peelFx[40];
  db::xasset::FxEffectDefHandle skidFx[40];
});
// Verified
ASSERT_SIZE(VehicleFxDef, 0x3D8);

typedef VehicleFxDef *VehicleFxDefPtr;

enum class TractionType : int32_t {
  FRONT = 0x0,
  BACK = 0x1,
  ALL_WD = 0x2,
  COUNT = 0x3,
};

PACKED(struct VehicleParameter {
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
});
ASSERT_PACKED(VehicleParameter);
ASSERT_SIZE(VehicleParameter, 0x178);

PACKED(struct VehicleAntenna {
  float springK;
  float damp;
  float length;
  float gravity;
  bool useXAxis;
  uint8_t _padding11[3];
});
ASSERT_SIZE(VehicleAntenna, 0x14);

enum class VehicleType : uint16_t {
  WHEELS_4 = 0x0,
  MOTORCYCLE = 0x1,
  TANK = 0x2,
  PLANE = 0x3,
  BOAT = 0x4,
  ARTILLERY = 0x5,
  HELICOPTER = 0x6,
  COUNT = 0x7,
};

PACKED(struct VehicleDef {
  const char *name;
  VehicleType type;
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
  uint8_t _unknown184[4];
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
  uint8_t _padding21C[4];
  const char *turretWeapon;
  view_limits_t turretViewLimits;
  qboolean turretClampPlayerView;
  qboolean turretLockTurretToPlayerView;
  qboolean turretFireFromCamera;
  uint8_t _padding254[4];
  const char *gunnerWeapon[4];
  uint16_t gunnerWeaponIndex[4];
  view_limits_t passengerViewLimits[6];
  const char *sndNames[7];
  snd::SndAliasId sndIndices[7];
  uint8_t _padding394[4];
  const char *sndMaterialNames[3];
  float skidSpeedMin;
  float skidSpeedMax;
  float peelSpeedMin;
  float peelSpeedMax;
  const char *futzName;
  float futzBlend;
  uint8_t _padding3CC[4];
  VehicleSoundDefPtr soundDef;
  int32_t animType;
  uint8_t _padding3DC[4];
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
  uint8_t _padding504[4];
  XModelPtr model;
  XModelPtr viewModel;
  XModelPtr deathModel;
  XModelPtr enemyModel;
  float modelSwapDelay;
  uint8_t _padding52C[4];
  db::xasset::FxEffectDefHandle exhaustFx;
  scr::ScrString_t exhaustFxTag1;
  scr::ScrString_t exhaustFxTag2;
  VehicleFxDefPtr surfaceFx;
  db::xasset::FxEffectDefHandle deathFx;
  scr::ScrString_t deathFxTag;
  uint8_t _padding554[4];
  const char *deathFxSound;
  db::xasset::FxEffectDefHandle lightFx[4];
  scr::ScrString_t lightFxTag[4];
  float radiusDamageMin;
  float radiusDamageMax;
  float radiusDamageRadius;
  uint8_t _padding59C[4];
  const char *shootShock;
  float deathQuakeScale;
  float deathQuakeDuration;
  float deathQuakeRadius;
  uint8_t _padding5B4[4];
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
  uint8_t _padding5FC[4];
  const char *compassIcon;
  db::xasset::MaterialHandle compassIconMaterial;
  scr::ScrString_t compassIconTag;
  float compassScale;
  const char *steerAxisName;
  int32_t steerAxis;
  uint8_t _padding624[4];
  const char *gasAxisName;
  int32_t gasAxis;
  uint8_t _padding634[4];
  const char *gasButtonName;
  int32_t gasButton;
  uint8_t _padding644[4];
  const char *reverseBrakeButtonName;
  int32_t reverseBrakeButton;
  uint8_t _padding654[4];
  const char *handBrakeButtonName;
  int32_t handBrakeButton;
  uint8_t _padding664[4];
  const char *attackButtonName;
  int32_t attackButton;
  uint8_t _padding674[4];
  const char *attackSecondaryButtonName;
  int32_t attackSecondaryButton;
  uint8_t _padding684[4];
  const char *boostButtonName;
  int32_t boostButton;
  uint8_t _padding694[4];
  const char *moveUpButtonName;
  int32_t moveUpButton;
  uint8_t _padding6A4[4];
  const char *moveDownButtonName;
  int32_t moveDownButton;
  uint8_t _padding6B4[4];
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
  db::xasset::RumbleInfoPtr lightCollisionRumble;
  float heavyCollisionSpeed;
  uint8_t _padding714[4];
  db::xasset::RumbleInfoPtr heavyCollisionRumble;
  db::xasset::RumbleInfoPtr jumpLandingRumble;
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
  uint8_t _padding84C[4];
  VehicleParameter nitrousVehParams;
  qboolean doFootSteps;
  qboolean ignoreVortices;
  qboolean isSentient;
  qboolean isPathfinder;
  db::xasset::ScriptBundlePtr scriptBundleSettings;
  db::xasset::ScriptBundlePtr vehicleRidersBundle;
  db::xasset::ScriptBundlePtr vehicleRidersRobotBundle;
  db::xasset::ScriptBundlePtr assassinationBundle;
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
  db::xasset::SurfaceFXTableDefPtr vehicleFootstepFXTable;
  phys::DestructibleDefPtr destructibleDef;
  gfx::GfxImage *tacticalModeIcon;
  float tacticalModeHeight;
  uint8_t _paddingAA4[4];
});
ASSERT_OFFSET(VehicleDef, useHeliBoneControllers, 0x188);
ASSERT_OFFSET(VehicleDef, isSentient, 0x9D0);
// Verified
ASSERT_SIZE(VehicleDef, 0xAA8);

PACKED(struct VehicleStun {
  bool m_changed_this_frame;
  uint8_t _padding01[3];
  float m_stun_time;
  float m_last_throttle;
  float m_throttle_stun_time;
  float m_last_brake;
  float m_brake_stun_time;
});
ASSERT_SIZE(VehicleStun, 0x18);

PACKED(struct NitrousVehicleController {
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
  uint8_t _padding34[4];
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
  uint8_t _unknown106[0x2];
});
ASSERT_OFFSET(NitrousVehicleController, m_script_goal_speed, 0x14);
ASSERT_OFFSET(NitrousVehicleController, m_script_goal_radius, 0x10);
/*
   Server and client have different `NitrousVehicleController` structs :(.

   NitrousVehicleController is added to NitrousVehicle as (non-pointer) field,
   so we can't just use a simple `EngineDependentNitrousVehicleController` union
   to abstract this.

   NitrousVehicle can be abstracted this way though. This will be necessary
   if/when we begin using `NitrousVehicle`s in code executed in client.
*/
// Server
ASSERT_OFFSET(NitrousVehicleController, m_stop_at_goal, 0xC1);
// Client
// ASSERT_OFFSET(NitrousVehicleController, m_stop_at_goal, 0xB9);
ASSERT_SIZE(NitrousVehicleController, 0x10e);

PACKED(struct NitrousVehicleController_cl {
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
  uint8_t _padding34[8];
  math::Dir3 m_stuck_position;
  user::usercmd_t m_cmd;
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
  uint8_t _unknown106[28];
});
ASSERT_OFFSET(NitrousVehicleController_cl, m_script_goal_speed, 0x14);
ASSERT_OFFSET(NitrousVehicleController_cl, m_script_goal_radius, 0x10);
ASSERT_OFFSET(NitrousVehicleController_cl, m_stop_at_goal, 0xB9);
ASSERT_SIZE(NitrousVehicleController_cl, 0x120); // ?

PACKED(struct mover_record_t {
public:
  vec3_t m_origin;
  vec3_t m_angles;
  int32_t m_time;
});
ASSERT_SIZE(mover_record_t, 0x1C);

PACKED(struct mover_info_t {
  mover_record_t m_records[32];
  int32_t m_last_record;
  uint8_t _padding384[4];
  hk::ai::hkaiNavMeshInstance *m_meshInstance;
});
ASSERT_SIZE(mover_info_t, 0x390);

PACKED(struct minspec_mutex {
public:
  volatile uint32_t m_token;
});
ASSERT_SIZE(minspec_mutex, 0x4);

PACKED(struct NitrousVehicle {
  struct syms {
    static constexpr symbol<thiscall_t<void(NitrousVehicle *, float throttle)>>
        set_throttle{0x141BBD4D0, 0x140321840};
    static constexpr symbol<thiscall_t<float(NitrousVehicle *, double delta_t)>>
        get_stuck_accel_factor{0x142383B30, 0x1405C2030};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, LocalClientNum_t localClientNum)>>
        setup_wheels{0x142383C80, 0x1405C2180};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_boost{0x1423841D0, 0x1405C26D0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, float inTq, float spinVel0,
                        float spinVel1, float *outTq0, float *outTq1)>>
        update_differential{0x1423842C0, 0x1405C27C0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_fakey_stuff{0x1423843C0, 0x1405C2AA0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_gravity{0x1423849B0, 0x1405C3090};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>>
        update_orientation_constraint{0x142385640, 0x1405C3D20};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>> update_pause{
        0x142385810, 0x1405C3EF0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const double delta_t)>>
        update_prolog{0x142385970, 0x1405C4050};
    static constexpr symbol<fastcall_t<NitrousVehicle *(int32_t id)>>
        add_vehicle{0x142385E30, 0x1405C4510};
    static constexpr symbol<fastcall_t<void(double delta_t)>>
        frame_Epilog_All_Systems{0x142386EC0, 0x1405C55A0};
    static constexpr symbol<fastcall_t<void(const double delta_t)>>
        frame_prolog_all_systems{0x142387100, 0x1405C5610};
    static constexpr symbol<thiscall_t<float(const NitrousVehicle *)>>
        get_throttle{0x142387390, 0x1405C58B0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, level::gentity_t *owner,
                        const VehicleParameter *parameter)>>
        init{0x142387480, 0x1405C5980};
    static constexpr symbol<thiscall_t<bool(NitrousVehicle *)>> is_path_moving{
        0x142387860, 0x1405C5D60};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *, bool shutdown)>>
        pause_physics{0x14238AA20, 0x1405C5EF0};
    static constexpr symbol<thiscall_t<void(const NitrousVehicle *v)>>
        remove_vehicle{0x14238AD70, 0x1405C6240};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, int32_t attach_mode)>>
        start_path{0x14238B150, 0x1405C6470};
    static constexpr symbol<thiscall_t<void(NitrousVehicle *)>> unpause_physics{
        0x14238B2D0, 0x1405C65F0};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const float desired_speed_factor)>>
        update_braking_and_acceleration{0x14238EB00, 0x1405C69D0};
    static constexpr symbol<thiscall_t<void(
        NitrousVehicle *, const VehicleDef *vehicleDef, bool initialization)>>
        update_parms{0x14238F990, 0x1405C7860};
    static constexpr symbol<
        thiscall_t<void(NitrousVehicle *, const float acceleration_factor)>>
        update_steering{0x142390370, 0x1405C82B0};
  };

  inline void set_throttle(float throttle) {
    return syms::set_throttle(this, throttle);
  }
  inline float get_stuck_accel_factor(double delta_t) {
    return syms::get_stuck_accel_factor(this, delta_t);
  }
  inline void setup_wheels(LocalClientNum_t localClientNum) {
    return syms::setup_wheels(this, localClientNum);
  }
  inline void update_boost(const double delta_t) {
    return syms::update_boost(this, delta_t);
  }
  inline void update_differential(float inTq, float spinVel0, float spinVel1,
                                  float *outTq0, float *outTq1) {
    return syms::update_differential(this, inTq, spinVel0, spinVel1, outTq0,
                                     outTq1);
  }
  inline void update_fakey_stuff(const double delta_t) {
    return syms::update_fakey_stuff(this, delta_t);
  }
  inline void update_gravity(const double delta_t) {
    return syms::update_gravity(this, delta_t);
  }
  inline void update_orientation_constraint() {
    return syms::update_orientation_constraint(this);
  }
  inline void update_pause() { return syms::update_pause(this); }
  inline void update_prolog(const double delta_t) {
    return syms::update_prolog(this, delta_t);
  }
  static inline NitrousVehicle *add_vehicle(int32_t id) {
    return syms::add_vehicle(id);
  }
  static inline void frame_Epilog_All_Systems(double delta_t) {
    return syms::frame_Epilog_All_Systems(delta_t);
  }
  static inline void frame_prolog_all_systems(const double delta_t) {
    return syms::frame_prolog_all_systems(delta_t);
  }
  inline float get_throttle() { return syms::get_throttle(this); }
  inline void init(level::gentity_t * owner,
                   const VehicleParameter *parameter) {
    return syms::init(this, owner, parameter);
  }
  inline bool is_path_moving() { return syms::is_path_moving(this); }
  inline void pause_physics(bool shutdown) {
    return syms::pause_physics(this, shutdown);
  }
  static inline void remove_vehicle(const NitrousVehicle *v) {
    return syms::remove_vehicle(v);
  }
  inline void start_path(int32_t attach_mode) {
    return syms::start_path(this, attach_mode);
  }
  inline void unpause_physics() { return syms::unpause_physics(this); }
  inline void update_braking_and_acceleration(
      const float desired_speed_factor) {
    return syms::update_braking_and_acceleration(this, desired_speed_factor);
  }

  inline void update_parms(const VehicleDef *vehicleDef, bool initialization) {
    return syms::update_parms(this, vehicleDef, initialization);
  }
  inline void update_steering(const float acceleration_factor) {
    return syms::update_steering(this, acceleration_factor);
  }

  phys::PhysObjUserData *m_phys_user_data;
  WheelState m_wheel_state[6];
  uint8_t _padding98[8];
  math::RotTranMat43 m_wheel_orig_relpo[6];
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
  uint8_t _padding2C4[4];
  level::gentity_t *m_owner;
  int32_t m_entnum;
  uint8_t _padding2D4[4];
  const VehicleDef *m_vehicle_def;
  db::xasset::XModel *m_xmodel;
  phys::rigid_body_constraint_custom_orientation *m_orientation_constraint;
  phys::rigid_body_constraint_custom_path *m_vpc;
  VehicleFlags m_flags;
  int32_t m_notify_flags;
  int32_t m_server_notify_flags;
  vec3_t m_collision_hitp;
  vec3_t m_collision_hitn;
  float m_collision_intensity;
  int32_t m_collision_stype;
  int32_t m_collision_entnum;
  float m_fake_rpm;
  float m_current_side_fric_scale;
  float m_current_fwd_fric_scale;
  float m_stuck_time;
  uint8_t _padding328[8];
  math::RotTranMat43 m_mat;
  NitrousVehicleController mVehicleController;
  uint8_t _unknown494[6];
  int32_t m_num_colliding_wheels;
  uint8_t _unknown49C[12];
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
  uint8_t _padding4E3[1];
  vec3_t m_last_origin;
  vec3_t m_last_angles;
  float m_buoyancyOffset;
  mover_info_t m_mover_info;
  minspec_mutex m_mutex;
  int32_t id;
  int32_t refcount;
  uint8_t _padding89C[4];
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
  uint8_t _padding8F4[12];
  math::Dir3 m_steer_front_pt_loc;
  float m_steer_front_back_length;
  float m_min_turning_radius;
  uint32_t m_state_flags;
  uint8_t _padding91C[4];
});
ASSERT_OFFSET(NitrousVehicle, m_flags, 0x2F8);
ASSERT_OFFSET(NitrousVehicle, m_vehicle_def, 0x2D8);
ASSERT_OFFSET(NitrousVehicle, m_wheels, 0x8A0);
ASSERT_OFFSET(NitrousVehicle, m_mat, 0x340);
ASSERT_OFFSET(NitrousVehicle, m_num_colliding_wheels, 0x494);
ASSERT_OFFSET(NitrousVehicle, mVehicleController, 0x380);

PACKED(struct vehicle_spline_node_t {
  int16_t nextIdx;
  int16_t prevIdx;
  float length;
  vec3_t dir;
  float t;
});
ASSERT_SIZE(vehicle_spline_node_t, 0x18);

PACKED(struct vehicle_node_t {
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
});
ASSERT_SIZE(vehicle_node_t, 0x5C);

enum class path_type_e : int32_t {
  NONE = 0x0,
  OLD_SPLINE = 0x1,
  SPLINE = 0x2,
  AI_PATH = 0x3,
};
inline constexpr const char *serialize(const path_type_e type) {
  switch (type) {
  case path_type_e::NONE:
    return "path_type_e::NONE";
  case path_type_e::OLD_SPLINE:
    return "path_type_e::OLD_SPLINE";
  case path_type_e::SPLINE:
    return "path_type_e::SPLINE";
  case path_type_e::AI_PATH:
    return "path_type_e::AI_PATH";
  default:
    return "path_type_e::INVALID";
  }
}
PACKED(struct vehicle_pathpos_t {
public:
  int16_t nodeIdx;
  int16_t prevIdx;
  int16_t endOfPath;
  uint8_t _padding06[2];
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
  uint8_t _padding156[2];
  float t;
  float tot_len;
  float tot_time;
});
ASSERT_SIZE(vehicle_pathpos_t, 0x164);

PACKED(struct vehicle_physic_t {
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
});
ASSERT_SIZE(vehicle_physic_t, 0x118);

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
  STOP = 0x0,
  MOVE = 0x1,
  HOVER = 0x2,
  PLANE_ONCURVE = 0x3,
  PLANE_FREE = 0x4,
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

PACKED(struct VehicleTurret {
  int32_t fireTime;
  int32_t fireBarrel;
  float barrelOffset;
  int32_t flags;
  float heatVal;
  float turretOnTargetRange;
  int32_t weaponShotCount;
  float accuracy;
  VehicleTurretState turretState;
});
ASSERT_SIZE(VehicleTurret, 0x24);

PACKED(struct VehicleJitter {
  int32_t jitterPeriodMin;
  int32_t jitterPeriodMax;
  int32_t jitterEndTime;
  vec3_t jitterOffsetRange;
  vec3_t jitterDeltaAccel;
  vec3_t jitterAccel;
  vec3_t jitterPos;
});
ASSERT_SIZE(VehicleJitter, 0x3C);

PACKED(struct VehicleHover {
  float hoverRadius;
  float hoverSpeed;
  float hoverAccel;
  vec3_t hoverGoalPos;
  qboolean useHoverAccelForAngles;
});
ASSERT_SIZE(VehicleHover, 0x1C);

PACKED(struct VehicleGunnerTags {
  BoneIndex turretParent;
  BoneIndex turret;
  BoneIndex barrel;
  BoneIndex aim;
  BoneIndex flash[4];
});
ASSERT_SIZE(VehicleGunnerTags, 0x10);

PACKED(struct VehicleTags {
  BoneIndex player;
  BoneIndex popout;
  BoneIndex body;
  BoneIndex eye;
  VehicleGunnerTags turretTags;
  VehicleGunnerTags gunnerTags[4];
  BoneIndex wheel[6];
  BoneIndex seats[11];
  BoneIndex entryPoints[5];
});
ASSERT_SIZE(VehicleTags, 0x84);

PACKED(struct VehicleCornerCache {
  vec3_t start;
  vec3_t end;
  vec3_t center;
  vec3_t travelVector;
  int32_t lastUpdateTime;
  int32_t startSlidingTime;
  float travelDistance;
});
ASSERT_SIZE(VehicleCornerCache, 0x3C);

PACKED(struct VehicleTarget {
  VehicleTargetType targetType;
  int32_t targetEnt;
  vec3_t targetOrigin;
  vec3_t targetOffset;
  vec3_t targetRelativeAngles;
});
ASSERT_SIZE(VehicleTarget, 0x2C);

enum class ClientFallSpeed : int32_t {
  NONE = 0x0,
  DEFAULT = 0x0,
  SLOW = 0x1,
  MED = 0x2,
  FAST = 0x3,
  RAPID = 0x4,
  COUNT = 0x5,
};

enum class ClientVisibleBodyMode : int32_t {
  UNKNOWN = 0x0,
  FULL = 0x1,
  FIRST_PERSON_CINEMATIC = 0x2,
  FIRST_PERSON_LEGS = 0x3,
};

enum class ShockViewTypes : int32_t {
  BLURRED = 0x0,
  FLASHED = 0x1,
  CONCUSSED = 0x2,
  SHOCKED = 0x3,
  NONE = 0x4,
  COUNT = 0x5,
};

enum class VehicleAnimState : int32_t {
  IDLE = 0x0,
  ENTRY = 0x1,
  CHANGEPOS = 0x2,
  EXIT = 0x3,
  COUNT = 0x4,
};

typedef phys::colgeom_visitor_inlined_t<300> vehicle_proximity_data_t;
ASSERT_SIZE(vehicle_proximity_data_t, 0x1350);

PACKED(struct vehicle_cache_t {
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
});
ASSERT_SIZE(vehicle_cache_t, 0x1430);

PACKED(struct VehicleSeat {
  int32_t occupantEntNum;
  bool scriptOccupied;
  uint8_t _padding5[3];
});
ASSERT_SIZE(VehicleSeat, 0x8);

PACKED(struct vehicle_t {
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
  VehicleFlags flags;
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
  ai::AIBody aiBody;
  uint8_t tmodeVehicleVisibility;
  uint8_t _padding22A1[3];
  float predictedCollisionTime;
  uint8_t _padding22A8[8];
});
ASSERT_OFFSET(vehicle_t, nitrousVehicle, 0x7B8);
ASSERT_SIZE(vehicle_t, 0x22B8);

PACKED(struct vehicleAntilagFrame_t {
  vec3_t position[64];
  vec3_t angles[64];
  uint8_t useCount[64];
  uint8_t inUse[64];
  int32_t time;
});
ASSERT_SIZE(vehicleAntilagFrame_t, 0x684);

PACKED(struct PlayerVehicleState {
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
  bool fullPhysics;
  uint8_t _padding51[3];
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
  bool weaponOverheating;
  uint8_t _padding99[3];
  float weaponHeatPercent;
  float weaponLockonPercent;
  float damageMeter;
  int32_t lockedOnByEntNum;
  int32_t weaponWaitDuration;
  int32_t weaponWaitEndTime;
});
ASSERT_SIZE(PlayerVehicleState, 0xB4);

PACKED(struct vehicleState_t {
  VehicleFlags flags;
  int16_t animId;
  int16_t attachModelIndex[2];
  uint8_t attachTagIndex[2];
  uint8_t vehicleDefIndex;
  uint8_t _padding0D[3];
});

PACKED(struct VehicleLocalPhysics {
  cm::trace_t groundTrace;
  qboolean hasGround;
  qboolean onGround;
  uint8_t _unknown[0x2a8];
});
ASSERT_SIZE(VehicleLocalPhysics, 0x300);

typedef fastcallPtr_t<const VehicleDef *(LocalClientNum_t localClientNum,
                                         const level::playerState_t *ps)>
    vehicle_def;

typedef fastcallPtr_t<vehicle_proximity_data_t *(
    LocalClientNum_t localClientNum, const level::playerState_t *ps)>
    get_vehicle_proximity_cache;

typedef fastcallPtr_t<qboolean(const move::pmove_t *pm, int, vec3_t *)>
    get_wheel;
typedef fastcallPtr_t<const cm::PhysGeomList *(LocalClientNum_t localClientNum,
                                               const int)>
    get_vehicle_collmap;

typedef fastcallPtr_t<void(LocalClientNum_t localClientNum, const int,
                           const vec3_t *, const vec3_t *, const int)>
    vehicle_collision_notifies;

struct VehicleHandler {
  vehicle_def vehicle;
  get_wheel wheel;
  get_vehicle_proximity_cache proximityCache;
  get_vehicle_collmap collmap;
  vehicle_collision_notifies notifies;
};

PACKED(struct VehicleState {
  int32_t entity;
  vec3_t origin;
  vec3_t angles;
  vec3_t velocity;
  vec3_t angVelocity;
  vec3_t previousOrigin;
  vec3_t previousAngles;
  vec3_t bodyVelocity;
  vec2_t tilt;
  vec2_t tiltVelocity;
  vec3_t mins;
  vec3_t maxs;
  float targetHeightDelta;
  float lastGroundHeight;
  int32_t clipmask;
  vec3_t groundNormal;
});
ASSERT_SIZE(VehicleState, 0x98);

} // namespace vehicle
} // namespace game