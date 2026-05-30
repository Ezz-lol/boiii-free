#pragma once

#include "core.hpp"
#include "db/db.hpp"
#include <cstdint>
namespace game {
namespace scr {
typedef uint32_t ScrString_t;
}

namespace weapon {

typedef uint64_t AttachmentMask;

enum class GadgetTargetResult : uint32_t {
  GTR_NONE = 0x0,
  GTR_HIT = 0x1,
  GTR_MISS = 0x2,
  GTR_MAX = 0x3,
};

enum class gadgetPulseShareType_e : int32_t {
  GADGET_PULSE_SHARE_DEFAULT = 0x0,
  GADGET_PULSE_SHARE_NONE = 0x1,
  GADGET_PULSE_SHARE_MINIMAP = 0x2,
  GADGET_PULSE_SHARE_VIEWPORT = 0x3,
  GADGET_PULSE_SHARE_BOTH = 0x4,
  GADGET_PULSE_SHARE_COUNT = 0x5,
};

// sizeof=0x8
union Weapon {
  struct {
    uint64_t weaponIdx : 9;
    uint64_t attachment1 : 6;
    uint64_t attachment2 : 6;
    uint64_t attachment3 : 6;
    uint64_t attachment4 : 6;
    uint64_t attachment5 : 6;
    uint64_t attachment6 : 6;
    uint64_t attachment7 : 6;
    uint64_t attachment8 : 6;
    uint64_t padding : 7;
  } _anon_0;

  uint64_t weaponData;
};
ASSERT_SIZE(Weapon, 0x8);

// sizeof=0x4
union AttachmentCosmeticVariantIndexes {
  struct {
    uint32_t attachmentCosmeticVariantIndex1 : 4;
    uint32_t attachmentCosmeticVariantIndex2 : 4;
    uint32_t attachmentCosmeticVariantIndex3 : 4;
    uint32_t attachmentCosmeticVariantIndex4 : 4;
    uint32_t attachmentCosmeticVariantIndex5 : 4;
    uint32_t attachmentCosmeticVariantIndex6 : 4;
    uint32_t attachmentCosmeticVariantIndex7 : 4;
    uint32_t attachmentCosmeticVariantIndex8 : 4;
  };
  uint32_t data;
};
ASSERT_SIZE(AttachmentCosmeticVariantIndexes, 0x4);

#pragma pack(push, 1)
struct entityGadgetState_t {
  Weapon gadgetWeapon;
  int32_t gadgetActivateTime;
  int16_t gadgetEntNum;
  uint8_t gadgetEntityStateFlags;
  uint8_t _padding0F[1];
};
#pragma pack(pop)

enum class gadgetType_e : int32_t {
  GADGET_TYPE_NONE = 0x0,
  GADGET_TYPE_OTHER = 0x1,
  GADGET_TYPE_OPTIC_CAMO = 0x2,
  GADGET_TYPE_ARMOR_REGEN = 0x3,
  GADGET_TYPE_ARMOR = 0x4,
  GADGET_TYPE_DRONE = 0x5,
  GADGET_TYPE_VISION_PULSE = 0x6,
  GADGET_TYPE_MULTI_ROCKET = 0x7,
  GADGET_TYPE_TURRET_DEPLOY = 0x8,
  GADGET_TYPE_GRENADE = 0x9,
  GADGET_TYPE_JUKE = 0xA,
  GADGET_TYPE_HACKER = 0xB,
  GADGET_TYPE_INFRARED = 0xC,
  GADGET_TYPE_SPEED_BURST = 0xD,
  GADGET_TYPE_HERO_WEAPON = 0xE,
  GADGET_TYPE_COMBAT_EFFICIENCY = 0xF,
  GADGET_TYPE_FLASHBACK = 0x10,
  GADGET_TYPE_CLEANSE = 0x11,
  GADGET_TYPE_SYSTEM_OVERLOAD = 0x12,
  GADGET_TYPE_SERVO_SHORTOUT = 0x13,
  GADGET_TYPE_EXO_BREAKDOWN = 0x14,
  GADGET_TYPE_SURGE = 0x15,
  GADGET_TYPE_RAVAGE_CORE = 0x16,
  GADGET_TYPE_REMOTE_HIJACK = 0x17,
  GADGET_TYPE_IFF_OVERRIDE = 0x18,
  GADGET_TYPE_CACOPHONY = 0x19,
  GADGET_TYPE_FORCED_MALFUNCTION = 0x1A,
  GADGET_TYPE_CONCUSSIVE_WAVE = 0x1B,
  GADGET_TYPE_OVERDRIVE = 0x1C,
  GADGET_TYPE_UNSTOPPABLE_FORCE = 0x1D,
  GADGET_TYPE_RAPID_STRIKE = 0x1E,
  GADGET_TYPE_ACTIVE_CAMO = 0x1F,
  GADGET_TYPE_SENSORY_OVERLOAD = 0x20,
  GADGET_TYPE_ES_STRIKE = 0x21,
  GADGET_TYPE_IMMOLATION = 0x22,
  GADGET_TYPE_FIREFLY_SWARM = 0x23,
  GADGET_TYPE_SMOKESCREEN = 0x24,
  GADGET_TYPE_MISDIRECTION = 0x25,
  GADGET_TYPE_MRPUKEY = 0x26,
  GADGET_TYPE_SHOCK_FIELD = 0x27,
  GADGET_TYPE_RESURRECT = 0x28,
  GADGET_TYPE_HEAT_WAVE = 0x29,
  GADGET_TYPE_CLONE = 0x2A,
  GADGET_TYPE_ROULETTE = 0x2B,
  GADGET_TYPE_THIEF = 0x2C,
  GADGET_TYPE_DISRUPTOR = 0x2D,
  GADGET_TYPE_HORNET_SWARM = 0x2E,
  GADGET_TYPE_GROUP_REVIVE = 0x2F,
  GADGET_TYPE_XRAY_EYES = 0x30,
  GADGET_TYPE_EMERGENCY_MELEE = 0x31,
  GADGET_TYPE_SHOULDER_GUN = 0x32,
  GADGET_TYPE_GRAPPLE = 0x33,
  GADGET_TYPE_INVULNERABLE = 0x34,
  GADGET_TYPE_SPRINT_BOOST = 0x35,
  GADGET_TYPE_HEALTH_REGEN = 0x36,
  GADGET_TYPE_SELF_DESTRUCT = 0x37,
  GADGET_TYPE_BARRIER_BUILDER = 0x38,
  GADGET_TYPE_SPAWN_BEACON = 0x39,
  GADGET_TYPE_COUNT = 0x3A,
};

enum class gadgetHoldMode_e : int32_t {
  GADGET_HOLD_MODE_WIELD = 0x0,
  GADGET_HOLD_MODE_OFFHAND = 0x1,
  GADGET_HOLD_MODE_THROW = 0x2,
  GADGET_HOLD_MODE_HANDS_FREE = 0x3,
  GADGET_HOLD_MODE_OFFHAND_PRIME = 0x4,
  GADGET_HOLD_MODE_COUNT = 0x5,
};

enum class gadgetPowerUseType_e : int32_t {
  GADGET_POWER_USE_AMMO = 0x0,
  GADGET_POWER_USE_COOLDOWN = 0x1,
  GADGET_POWER_USE_POWER_BAR = 0x2,
  GADGET_POWER_USE_COUNT = 0x3,
};

enum class gadgetActivateMode_e : int32_t {
  GADGET_ACTIVATE_MODE_HOLD = 0x0,
  GADGET_ACTIVATE_MODE_TOGGLE = 0x1,
  GADGET_ACTIVATE_MODE_SINGLE = 0x2,
  GADGET_ACTIVATE_MODE_TOGGLE_AND_ATTACK = 0x3,
  GADGET_ACTIVATE_MODE_TOGGLE_AND_HOLD = 0x4,
  GADGET_ACTIVATE_MODE_SINGLE_STAY_ON = 0x5,
  GADGET_ACTIVATE_MODE_AUTO = 0x6,
  GADGET_ACTIVATE_MODE_PRIME_ON_HOLD_ACTIVATE_ON_RELEASE = 0x7,
  GADGET_ACTIVATE_MODE_PRIME_ON_HOLD_ACTIVATE_ON_RELEASE_STAY_ON = 0x8,
  GADGET_ACTIVATE_MODE_COUNT = 0x9,
};

enum class weapOverlayReticle_t : int32_t {
  WEAPOVERLAYRETICLE_NONE = 0x0,
  WEAPOVERLAYRETICLE_CROSSHAIR = 0x1,
  WEAPOVERLAYRETICLE_NUM = 0x2,
};

enum class WeapStickinessType : int32_t {
  WEAPSTICKINESS_NONE = 0x0,
  WEAPSTICKINESS_ALL = 0x1,
  WEAPSTICKINESS_ALL_NO_SENTIENTS = 0x2,
  WEAPSTICKINESS_ALL_NO_VEHICLES = 0x3,
  WEAPSTICKINESS_GROUND = 0x4,
  WEAPSTICKINESS_GROUND_WITH_YAW = 0x5,
  WEAPSTICKINESS_FLESH = 0x6,
  WEAPSTICKINESS_IGNORE = 0x7,
  WEAPSTICKINESS_COUNT = 0x8,
};

enum class eAttachment : int32_t {
  ATTACHMENT_NOT_FOUND = -1,
  ATTACHMENT_NONE = 0x0,
  ATTACHMENT_TYPE_FIRST = 0x1,
  ATTACHMENT_ACOG = 0x1,
  ATTACHMENT_CLANTAG = 0x2,
  ATTACHMENT_DAMAGE = 0x3,
  ATTACHMENT_DUALCLIP = 0x4,
  ATTACHMENT_DUALOPTIC = 0x5,
  ATTACHMENT_DW = 0x6,
  ATTACHMENT_DYNZOOM = 0x7,
  ATTACHMENT_EXTBARREL = 0x8,
  ATTACHMENT_EXTCLIP = 0x9,
  ATTACHMENT_FASTADS = 0xA,
  ATTACHMENT_FASTRELOAD = 0xB,
  ATTACHMENT_FMJ = 0xC,
  ATTACHMENT_GL = 0xD,
  ATTACHMENT_GADGET_MOD_0 = 0xE,
  ATTACHMENT_GADGET_MOD_TAKE_TWO = 0xE,
  ATTACHMENT_GADGET_MOD_1 = 0xF,
  ATTACHMENT_GADGET_MOD_2 = 0x10,
  ATTACHMENT_GADGET_MOD_3 = 0x11,
  ATTACHMENT_GADGET_MOD_4 = 0x12,
  ATTACHMENT_GADGET_MOD_5 = 0x13,
  ATTACHMENT_GRIP = 0x14,
  ATTACHMENT_HOLO = 0x15,
  ATTACHMENT_IR = 0x16,
  ATTACHMENT_IRONSIGHTS = 0x17,
  ATTACHMENT_KILLCOUNTER = 0x18,
  ATTACHMENT_MK = 0x19,
  ATTACHMENT_MMS = 0x1A,
  ATTACHMENT_NOTRACER = 0x1B,
  ATTACHMENT_PRECISION = 0x1C,
  ATTACHMENT_QUICKDRAW = 0x1D,
  ATTACHMENT_RANGEFINDER = 0x1E,
  ATTACHMENT_RECON = 0x1F,
  ATTACHMENT_REDDOT = 0x20,
  ATTACHMENT_REFLEX = 0x21,
  ATTACHMENT_RF = 0x22,
  ATTACHMENT_SELECTFIRE = 0x23,
  ATTACHMENT_STACKFIRE = 0x24,
  ATTACHMENT_STALKER = 0x25,
  ATTACHMENT_STEADYAIM = 0x26,
  ATTACHMENT_SUPPLY = 0x27,
  ATTACHMENT_SUPPRESSED = 0x28,
  ATTACHMENT_SWAYREDUC = 0x29,
  ATTACHMENT_TACKNIFE = 0x2A,
  ATTACHMENT_VZOOM = 0x2B,
  ATTACHMENT_TYPE_LAST = 0x2B,
  ATTACHMENT_TYPE_COUNT = 0x2C,
  ATTACHMENT_INVALID = 0x2C,
};

#pragma pack(push, 1)
struct gadgetProperties_t {
  gadgetActivateMode_e activate_mode;
  gadgetType_e gadget_type;
  gadgetHoldMode_e weapon_hold_mode;
  gadgetPowerUseType_e power_use_type;
  int32_t activate_with_primed_delay;
  float movementMultipliers[14];
  float shock_field_radius;
  float shock_field_damage;
  int32_t camo_bread_crumb_duration;
  int32_t camo_invisibility_alert_time;
  int32_t camo_invisibility_flicker_extension_time;
  float camo_invisibility_flicker_radius_extension;
  int32_t camo_invisibility_radius;
  int32_t camo_invisibility_takedown_response_radius;
  int32_t camo_invisibility_takedown_reveal_time;
  int32_t camo_takedown_power_gain;
  float escort_drone_bullet_dmg_power_loss;
  float escort_drone_burst_count_max;
  float escort_drone_burst_count_min;
  int32_t escort_drone_burst_power_loss;
  int32_t escort_drone_burst_wait_time;
  float escort_drone_exp_dmg_power_loss;
  int32_t escort_drone_hover_dist;
  int32_t escort_drone_launch_dist;
  float escort_drone_misc_dmg_power_loss;
  int32_t escort_drone_target_acquire_time;
  int32_t escort_drone_tether_max_dist;
  int32_t escort_drone_tether_min_dist;
  int32_t flicker_on_damage;
  int32_t flicker_on_power_loss;
  int32_t flicker_on_power_low;
  int32_t flicker_on_whizby;
  qboolean heroVersion_2_0;
  int32_t multiRocket_acquisition_time;
  int32_t multiRocket_fire_interval;
  int32_t multiRocket_fire_power_loss;
  int32_t multirocket_target_number;
  int32_t multiRocket_target_radius;
  int32_t gadget_max_hitpoints;
  float blurAmount;
  float blurRadiusInner;
  float blurRadiusOuter;
  float blurOutScale;
  int32_t blurInTime;
  int32_t blurOutTime;
  bool blur_screen;
  bool changeLens;
  uint8_t _paddingEA[2];
  float lensScaleInitial;
  float lensScaleFinal;
  float lensMinFocalLength;
  float lensMaxFocalLength;
  int32_t lensTransTimeIn;
  int32_t lensTransTimeOut;
  qboolean power_shared;
  int32_t power_attack_loss;
  qboolean power_consume_on_ammo_use;
  float power_damage_factor;
  float power_flicker_chance;
  int32_t power_flicker_frequency;
  int32_t power_flicker_threshold;
  int32_t power_gain_on_retrieve;
  float power_gain_score_factor;
  bool power_gain_score_ignore_self;
  bool power_gain_score_ignore_when_active;
  bool power_init_empty;
  uint8_t _padding12B[1];
  int32_t power_init_amount;
  int32_t power_juke_loss;
  int32_t power_jump_loss;
  int32_t power_melee_loss;
  int32_t power_move_loss;
  int32_t power_move_loss_speed;
  float power_on_damage_factor;
  int32_t power_power_bonus;
  int32_t power_recharge_delay;
  int32_t power_recharge_delay_max;
  float power_recharge_rate;
  qboolean power_recharge_disable_on_emp;
  float power_replenish_factor;
  bool power_reset_on_class_change;
  bool power_reset_on_death_if_active;
  bool power_reset_on_spawn;
  bool power_reset_on_team_change;
  qboolean power_reset_on_round_switch;
  int32_t power_round_end_active_penalty;
  int32_t power_sprint_loss;
  int32_t power_shut_off_on_death_penalty;
  int32_t power_shut_off_penalty;
  int32_t power_turn_off_penalty;
  int32_t power_usable_threshold;
  float power_usage_rate;
  int32_t pulse_duration;
  int32_t pulse_margin;
  int32_t pulse_reveal_time;
  int32_t pulse_reveal_camo_time;
  int32_t pulse_reveal_time_viewModel;
  int32_t pulse_share_radius;
  gadgetPulseShareType_e pulse_enemy_share_type;
  gadgetPulseShareType_e pulse_share_type;
  gadgetPulseShareType_e pulse_type;
  int32_t pulse_max_range;
  float shield_blast_protection_120;
  float shield_blast_protection_180;
  float shield_blast_protection_30;
  float shield_blast_protection_60;
  float shield_reflect_actor_accuracy_multiplier;
  float shield_reflect_aim_assist_lerp;
  float shield_reflect_damage_multiplier;
  int32_t shield_reflect_power_gain;
  int32_t shield_reflect_power_loss;
  float speed_max_power_usage_rate;
  float speed_max_power_speed_threshold;
  float speed_sprint_out_scale;
  float speed_wallrun_out_scale;
  float cleanseRadius;
  bool turnoff_onEmpJammed;
  bool turnoff_onHeldKillstreak;
  bool turnoff_onAttack;
  bool can_activate_whenEmpJammed;
  int32_t turret_fire_power_loss;
  bool wielded_stayOn_onEmpJammed;
  bool wielded_stayOn_onHeldKillstreak;
  bool wielded_stayOn_onOffhandThrow;
  uint8_t _padding1EF[1];
  int32_t flashback_rewindTime;
  int32_t flashback_screenFlashShotFadeTime;
  int32_t flashback_screenFlashWhiteFadeTime;
  float flashback_rewindDistance;
  bool fastFire;
  bool fastReload;
  bool fastSwitch;
  bool fastMelee;
  bool fastToss;
  bool fastEquipmentUse;
  bool fastADS;
  bool activatesCast;
  qboolean targetedCast;
};
#pragma pack(pop)

struct WeaponAttachmentUniqueModelInfo {
  db::xasset::XModelPtr model;
  db::xasset::XModelPtr modelADS;
  XString tag;
  vec3_t offset;
  vec3_t rotation;
};

struct AttachmentCosmeticVariantModelInfo {
  db::xasset::XModelPtr model;
  db::xasset::XModelPtr modelADS;
  XString tag;
  vec3_t offset;
  vec3_t rotation;
};

struct AttachmentCosmeticVariantSet {
  AttachmentCosmeticVariantModelInfo viewModelInfo[2];
  AttachmentCosmeticVariantModelInfo worldModelInfo[2];
  const char *displayNameShort;
  const char *displayNameLong;
  const char *description;
  gfx::GfxImagePtr uiMaterial;
};

#pragma pack(push, 1)
struct AttachmentCosmeticVariant {
  const char *name;
  AttachmentCosmeticVariantSet *variantSets;
  int32_t numVariantSets;
  uint8_t _padding14[4];
};
#pragma pack(pop)
typedef AttachmentCosmeticVariant *AttachmentCosmeticVariantPtr;

struct WeaponAttachmentUniqueModelSet {
  eAttachment attachmentModelAssociation;
  AttachmentCosmeticVariantPtr attachmentCosmeticVariantOverride;
  WeaponAttachmentUniqueModelInfo viewModelInfo[2];
  WeaponAttachmentUniqueModelInfo worldModelInfo[2];
  const char *displayNameShort;
  const char *displayNameLong;
  const char *description;
  gfx::GfxImageHandle uiMaterial;
};

#pragma pack(push, 1)
struct CamoMaterialChannel {
  uint16_t replaceFlags;
  uint8_t _padding02[6];
  db::xasset::MaterialDeferredHandle camoMaterial;
  float translationX;
  float translationY;
  float scaleX;
  float scaleY;
  float rotation;
  float normalBlend;
  float glossBlend;
  gfx::GfxColor albedoTint;
  gfx::GfxImage *detailMap;
  float detailHeight;
  vec2_t detailScale;
  uint8_t _padding44[4];
};
#pragma pack(pop)

struct CamoBaseMaterial {
  db::xasset::MaterialDeferredHandle material;
  gfx::GfxImage *mask;
};

#pragma pack(push, 1)
struct CamoMaterial {
  uint16_t numBaseMaterials;
  uint8_t _padding02[6];
  CamoBaseMaterial *baseMaterials;
  uint16_t activeChannels;
  uint8_t _padding12[6];
  CamoMaterialChannel camoMaterialChannels[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WeaponCamoMaterialSet {
  uint32_t numMaterials;
  uint8_t _padding04[4];
  CamoMaterial *materials;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WeaponCamo {
  const char *name;
  WeaponCamoMaterialSet *camoMaterials;
  uint32_t numCamoMaterials;
  uint8_t _padding014[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WeaponAttachmentUnique {
  const char *szInternalName;
  const char *szDisplayName;
  const char *szDisplayDesc;
  gfx::GfxImageHandle modHudIcon;
  gfx::GfxImageHandle attachmentImage;
  eAttachment attachmentType;
  int32_t siblingLink;
  int32_t childLink;
  uint8_t _padding34[4];
  AttachmentMask combinedAttachmentTypeMask;
  const char *szAltWeaponName;
  uint32_t altWeaponIndex;
  uint8_t _padding4C[4];
  const char *szDualWieldWeaponName;
  uint32_t dualWieldWeaponIndex;
  uint8_t _padding5C[4];
  scr::ScrString_t *hideTags;
  WeaponAttachmentUniqueModelSet attachmentModelSets[2];
  db::xasset::XModelPtr *attachViewModel;
  db::xasset::XModelPtr *attachWorldModel;
  const char **attachViewModelTag;
  const char **attachWorldModelTag;
  float attachViewModelOffsets[15];
  float attachWorldModelOffsets[15];
  float attachViewModelRotations[15];
  float attachWorldModelRotations[15];
  WeaponCamo *weaponCamo;
  bool disableBaseWeaponAttachment;
  bool disableBaseWeaponClip;
  bool overrideBaseWeaponAttachmentOffsets;
  uint8_t _padding363[1];
  vec3_t viewModelOffsetBaseAttachment;
  vec3_t worldModelOffsetBaseAttachment;
  uint8_t _padding37C[4];
  gfx::GfxImageHandle overlayMaterial;
  gfx::GfxImageHandle overlayMaterialLowRes;
  weapOverlayReticle_t overlayReticle;
  int32_t iFirstRaiseTime;
  int32_t iAltRaiseTime;
  int32_t iAltDropTime;
  int32_t iAdsAltRaiseTime;
  int32_t iAdsAltDropTime;
  int32_t iReloadAmmoAdd;
  int32_t iReloadStartAdd;
  bool bSegmentedReload;
  uint8_t _padding3B1[7];
  const char **szXAnims;
  int32_t animationOverrides[6];
  float *locationDamageMultipliers;
  int32_t damage[6];
  float damageRange[6];
  int32_t soundOverrides;
  uint8_t _padding414[4];
  const char *fireSound;
  const char *fireSoundPlayer;
  const char *fireBurstSound;
  const char *fireBurstSoundPlayer;
  const char *fireLoopSound;
  const char *fireLoopSoundPlayer;
  const char *fireLoopSoundLeft;
  const char *fireLoopSoundPlayerLeft;
  const char *fireLoopEndSound;
  const char *fireLoopEndSoundPlayer;
  const char *fireStartSound;
  const char *fireStopSound;
  const char *fireStartSoundPlayer;
  const char *fireStopSoundPlayer;
  const char *fireLastSound;
  const char *fireLastSoundPlayer;
  const char *fireKillcamSound;
  const char *fireKillcamSoundPlayer;
  const char *crackSound;
  const char *whizbySound;
  const char *emptyFireSound;
  const char *emptyFireSoundPlayer;
  const char *spinLoopSound;
  const char *spinLoopSoundPlayer;
  const char *startSpinSound;
  const char *startSpinSoundPlayer;
  const char *stopSpinSound;
  const char *stopSpinSoundPlayer;
  const char *rotateLoopSound;
  const char *rotateLoopSoundPlayer;
  const char *rotateStopSound;
  const char *rotateStopSoundPlayer;
  int32_t effectOverrides;
  uint8_t _padding518[4];
  db::xasset::FxEffectDefHandle viewFlashEffect;
  db::xasset::FxEffectDefHandle worldFlashEffect;
  vec3_t vViewFlashOffset;
  vec3_t vWorldFlashOffset;
  db::xasset::TracerDefPtr tracerType;
  db::xasset::TracerDefPtr enemyTracerType;
  db::xasset::LaserDefPtr laserTypeViewModel;
  db::xasset::LaserDefPtr laserTypeWorld;
  float adsDofStart;
  float adsDofEnd;
  int32_t iAmmoIndex;
  int32_t iClipIndex;
  bool bOverrideLeftHandIK;
  bool bOverrideLeftHandProneIK;
  uint8_t _padding57A[2];
  vec3_t ikLeftHandOffset;
  vec3_t ikLeftHandRotation;
  vec3_t ikLeftHandProneOffset;
  vec3_t ikLeftHandProneRotation;
  int32_t iCanDoQuickToss;
  WeapStickinessType stickiness;
  int32_t ammoCountEquipment;
  int32_t iStartAmmo;
  int32_t iMaxAmmo;
  int32_t iExplosionRadius;
  float iExplosionRadiusMultiplier;
  int32_t fuseTime;
  int32_t proximityDetonation;
  int32_t proximityAlarmInnerRadius;
  int32_t proximityAlarmOuterRadius;
  int32_t proximityAlarmActivationDelay;
  int32_t chainEventRadius;
  int32_t chainEventTime;
  int32_t chainEventMax;
  int32_t multiDetonation;
  int32_t multiDetonationFragmentSpeed;
  float curveballForce;
  int32_t curveballMaxRadius;
  int32_t lockOnMaxRange;
  int32_t lockOnMaxRangeNoLineOfSight;
  int32_t lockOnSpeed;
  int32_t bProjExplodeWhenStationary;
  gadgetProperties_t gadgetProperty;
  float customFloat0;
  float customFloat1;
  float customFloat2;
  float customFloat3;
  float customFloat4;
  float customFloat5;
  bool customBool0;
  bool customBool1;
  bool customBool2;
  bool customBool3;
  bool customBool4;
  bool customBool5;
  uint8_t _padding832[6];
};
typedef WeaponAttachmentUnique *WeaponAttachmentUniquePtr;
#pragma pack(pop)

enum class WeaponDobjInfoSlot : int32_t {
  HELD = 0x0,
  STOWED = 0x1,
  MELEE = 0x2,
  OFFHAND = 0x4,
};

struct ExplosionDamage {
  int32_t radius;
  int32_t height;
  int32_t damage;
};

enum class weapLockType_t : int32_t {
  WEAPON_LOCKTYPE_NONE = 0x0,
  WEAPON_LOCKTYPE_TARGETED = 0x1,
  WEAPON_LOCKTYPE_SINGLEAP = 0x2,
  WEAPON_LOCKTYPE_MULTIAP = 0x3,
  WEAPON_LOCKTYPECOUNT = 0x4,
};

enum class WeapStickOrientation : int32_t {
  WEAPSTICKORIENTATION_NONE = 0x0,
  WEAPSTICKORIENTATION_FLAT = 0x1,
  WEAPSTICKORIENTATION_FLAT_MAINTAIN_DIRECTION = 0x2,
  WEAPSTICKORIENTATION_COUNT = 0x3,
};

struct RumbleInfo;
typedef RumbleInfo *RumbleInfoPtr;

enum class DamageType : int32_t {
  DMG_TYPE_NORMAL = 0x0,
  DMG_TYPE_NORMAL_SHOTGUN = 0x1,
  DMG_TYPE_BURNED = 0x2,
  DMG_TYPE_MELEE = 0x3,
  DMG_TYPE_MELEE_ASSASSINATE = 0x4,
  DMG_TYPE_MELEE_CHARGE = 0x5,
  DMG_TYPE_MELEE_BASH = 0x6,
  DMG_TYPE_MELEE_ARMBLADE = 0x7,
  DMG_TYPE_EXPLOSIVE = 0x8,
  DMG_TYPE_SUICIDE = 0x9,
  DMG_TYPE_FALLING = 0xA,
  DMG_TYPE_BLOWBACK = 0xB,
  DMG_TYPE_ELECTRIFIED = 0xC,
  DMG_TYPE_ANNIHILATOR = 0xD,
  DMG_TYPE_FIRST_USER_SPECIFIED = 0xD,
  DMG_TYPE_FIREFLIES = 0xE,
  DMG_TYPE_BOW_PARTIAL_CHARGE = 0xF,
  DMG_TYPE_BOW_FULL_CHARGE = 0x10,
  DMG_TYPE_MELEE_BAT = 0x11,
  DMG_TYPE_ENERGY_WEAPON = 0x12,
  DMG_TYPE_COUNT = 0x13,
};

enum class weapClass_t : int32_t {
  WEAPCLASS_RIFLE = 0x0,
  WEAPCLASS_MG = 0x1,
  WEAPCLASS_SMG = 0x2,
  WEAPCLASS_SPREAD = 0x3,
  WEAPCLASS_PISTOL = 0x4,
  WEAPCLASS_GRENADE = 0x5,
  WEAPCLASS_ROCKETLAUNCHER = 0x6,
  WEAPCLASS_TURRET = 0x7,
  WEAPCLASS_NON_PLAYER = 0x8,
  WEAPCLASS_GAS = 0x9,
  WEAPCLASS_ITEM = 0xA,
  WEAPCLASS_MELEE = 0xB,
  WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON = 0xC,
  WEAPCLASS_PISTOL_SPREAD = 0xD,
  WEAPCLASS_BALL = 0xE,
  WEAPCLASS_NUM = 0xF,
};

enum class weapType_t : int32_t {
  WEAPTYPE_BULLET = 0x0,
  WEAPTYPE_GRENADE = 0x1,
  WEAPTYPE_PROJECTILE = 0x2,
  WEAPTYPE_BINOCULARS = 0x3,
  WEAPTYPE_GAS = 0x4,
  WEAPTYPE_BOMB = 0x5,
  WEAPTYPE_MINE = 0x6,
  WEAPTYPE_MELEE = 0x7,
  WEAPTYPE_RIOTSHIELD = 0x8,
  WEAPTYPE_NUM = 0x9,
};

enum class PenetrateType : int32_t {
  PENETRATE_TYPE_NONE = 0x0,
  PENETRATE_TYPE_SMALL = 0x1,
  PENETRATE_TYPE_MEDIUM = 0x2,
  PENETRATE_TYPE_LARGE = 0x3,
  PENETRATE_TYPE_COUNT = 0x4,
};

enum class ImpactType : int32_t {
  IMPACT_TYPE_NONE = 0x0,
  IMPACT_TYPE_BULLET_SMALL = 0x1,
  IMPACT_TYPE_BULLET_LARGE = 0x2,
  IMPACT_TYPE_BULLET_AP = 0x3,
  IMPACT_TYPE_BULLET_XTREME = 0x4,
  IMPACT_TYPE_SHOTGUN = 0x5,
  IMPACT_TYPE_GRENADE_BOUNCE = 0x6,
  IMPACT_TYPE_GRENADE_EXPLODE = 0x7,
  IMPACT_TYPE_RIFLE_GRENADE = 0x8,
  IMPACT_TYPE_ROCKET_EXPLODE = 0x9,
  IMPACT_TYPE_ROCKET_EXPLODE_XTREME = 0xA,
  IMPACT_TYPE_PROJECTILE_DUD = 0xB,
  IMPACT_TYPE_MORTAR_SHELL = 0xC,
  IMPACT_TYPE_TANK_SHELL = 0xD,
  IMPACT_TYPE_BOLT = 0xE,
  IMPACT_TYPE_BLADE = 0xF,
  IMPACT_TYPE_COUNT = 0x10,
};

enum class weapInventoryType_t : int32_t {
  WEAPINVENTORY_PRIMARY = 0x0,
  WEAPINVENTORY_OFFHAND = 0x1,
  WEAPINVENTORY_ITEM = 0x2,
  WEAPINVENTORY_ALTMODE = 0x3,
  WEAPINVENTORY_MELEE = 0x4,
  WEAPINVENTORY_DWLEFTHAND = 0x5,
  WEAPINVENTORY_ABILITY = 0x6,
  WEAPINVENTORY_HEAVY = 0x7,
  WEAPINVENTORYCOUNT = 0x8,
};

enum class weapFireType_t : int32_t {
  WEAPON_FIRETYPE_FULLAUTO = 0x0,
  WEAPON_FIRETYPE_SINGLESHOT = 0x1,
  WEAPON_FIRETYPE_BURSTFIRE = 0x2,
  WEAPON_FIRETYPE_AUTOBURSTFIRE = 0x3,
  WEAPON_FIRETYPE_STACKED = 0x4,
  WEAPON_FIRETYPE_ALLLOCKED = 0x5,
  WEAPON_FIRETYPE_MINIGUN = 0x6,
  WEAPON_FIRETYPE_CHARGESHOT = 0x7,
  WEAPON_FIRETYPE_CHARGESHOTBURST = 0x8,
  WEAPON_FIRETYPE_CHARGESHOTAUTOBURST = 0x9,
  WEAPON_FIRETYPE_JETGUN = 0xA,
  WEAPON_FIRETYPE_MELEE = 0xB,
  WEAPON_FIRETYPE_AIMONLY = 0xC,
  WEAPON_FIRETYPECOUNT = 0xD,
};

enum class weapClipType_t : int32_t {
  WEAPON_CLIPTYPE_BOTTOM = 0x0,
  WEAPON_CLIPTYPE_TOP = 0x1,
  WEAPON_CLIPTYPE_LEFT = 0x2,
  WEAPON_CLIPTYPE_DP28 = 0x3,
  WEAPON_CLIPTYPE_PTRS = 0x4,
  WEAPON_CLIPTYPE_LMG = 0x5,
  WEAPON_CLIPTYPECOUNT = 0x6,
};

enum class barrelType_t : int32_t {
  BARREL_TYPE_SINGLE = 0x0,
  BARREL_TYPE_DUAL = 0x1,
  BARREL_TYPE_DUAL_ALTERNATE = 0x2,
  BARREL_TYPE_QUAD = 0x3,
  BARREL_TYPE_QUAD_ALTERNATE = 0x4,
  BARREL_TYPE_QUAD_DOUBLE_ALTERNATE = 0x5,
  BARREL_TYPE_OFFSET_DUAL = 0x6,
  BARREL_TYPE_OFFSET_DUAL_ALTERNATE = 0x7,
  BARREL_TYPE_COUNT = 0x8,
};

enum class OffhandClass : int32_t {
  OFFHAND_CLASS_NONE = 0x0,
  OFFHAND_CLASS_FRAG_GRENADE = 0x1,
  OFFHAND_CLASS_SMOKE_GRENADE = 0x2,
  OFFHAND_CLASS_FLASH_GRENADE = 0x3,
  OFFHAND_CLASS_GEAR = 0x4,
  OFFHAND_CLASS_SUPPLYDROP_MARKER = 0x5,
  OFFHAND_CLASS_GADGET = 0x6,
  OFFHAND_CLASS_COUNT = 0x7,
};

enum class OffhandSlot : int32_t {
  OFFHAND_SLOT_NONE = 0x0,
  OFFHAND_SLOT_LETHAL_GRENADE = 0x1,
  OFFHAND_SLOT_TACTICAL_GRENADE = 0x2,
  OFFHAND_SLOT_EQUIPMENT = 0x3,
  OFFHAND_SLOT_SPECIFIC_USE = 0x4,
  OFFHAND_SLOT_GADGET = 0x5,
  OFFHAND_SLOT_SPECIAL = 0x6,
  OFFHAND_SLOT_COUNT = 0x7,
};

enum class weapStance_t : int32_t {
  WEAPSTANCE_STAND = 0x0,
  WEAPSTANCE_DUCK = 0x1,
  WEAPSTANCE_PRONE = 0x2,
  WEAPSTANCE_NUM = 0x3,
};

enum class activeReticleType_t : int32_t {
  VEH_ACTIVE_RETICLE_NONE = 0x0,
  VEH_ACTIVE_RETICLE_PIP_ON_A_STICK = 0x1,
  VEH_ACTIVE_RETICLE_BOUNCING_DIAMOND = 0x2,
  VEH_ACTIVE_RETICLE_MISSILE_LOCK = 0x3,
  VEH_ACTIVE_RETICLE_COUNT = 0x4,
};

enum class weaponIconRatioType_t : int32_t {
  WEAPON_ICON_RATIO_1TO1 = 0x0,
  WEAPON_ICON_RATIO_2TO1 = 0x1,
  WEAPON_ICON_RATIO_4TO1 = 0x2,
  WEAPON_ICON_RATIO_COUNT = 0x3,
};

enum class ammoCounterClipType_t : int32_t {
  AMMO_COUNTER_CLIP_NONE = 0x0,
  AMMO_COUNTER_CLIP_MAGAZINE = 0x1,
  AMMO_COUNTER_CLIP_SHORTMAGAZINE = 0x2,
  AMMO_COUNTER_CLIP_SHOTGUN = 0x3,
  AMMO_COUNTER_CLIP_ROCKET = 0x4,
  AMMO_COUNTER_CLIP_BELTFED = 0x5,
  AMMO_COUNTER_CLIP_ALTWEAPON = 0x6,
  AMMO_COUNTER_CLIP_COUNT = 0x7,
};

enum class WeapRotateType : int32_t {
  WEAPROTATE_GRENADE_ROTATE = 0x0,
  WEAPROTATE_BLADE_ROTATE = 0x1,
  WEAPROTATE_CYLINDER_ROTATE = 0x2,
  WEAPROTATE_ANIMATION_ROTATE = 0x3,
  WEAPROTATE_COUNT = 0x4,
};

enum class weapProjExposion_t : int32_t {
  WEAPPROJEXP_NONE = 0x0,
  WEAPPROJEXP_GRENADE = 0x1,
  WEAPPROJEXP_ROCKET = 0x2,
  WEAPPROJEXP_FLASHBANG = 0x3,
  WEAPPROJEXP_DUD = 0x4,
  WEAPPROJEXP_SMOKE = 0x5,
  WEAPPROJEXP_HEAVY = 0x6,
  WEAPPROJEXP_FIRE = 0x7,
  WEAPPROJEXP_NAPALMBLOB = 0x8,
  WEAPPROJEXP_BOLT = 0x9,
  WEAPPROJEXP_SHRAPNELSPAN = 0xA,
  WEAPPROJEXP_NUM = 0xB,
};

enum class eAttachmentPoint : int32_t {
  ATTACHMENT_POINT_NONE = 0x0,
  ATTACHMENT_POINT_TOP = 0x1,
  ATTACHMENT_POINT_FIRST = 0x1,
  ATTACHMENT_POINT_BOTTOM = 0x2,
  ATTACHMENT_POINT_TRIGGER = 0x3,
  ATTACHMENT_POINT_MUZZLE = 0x4,
  ATTACHMENT_POINT_GUNPERK = 0x5,
  ATTACHMENT_POINT_RIG = 0x6,
  ATTACHMENT_POINT_COUNT = 0x7,
  ATTACHMENT_POINT_INVALID = 0x7,
};

enum class WeapOverlayInteface_t : int32_t {
  WEAPOVERLAYINTERFACE_NONE = 0x0,
  WEAPOVERLAYINTERFACE_JAVELIN = 0x1,
  WEAPOVERLAYINTERFACE_TURRETSCOPE = 0x2,
  WEAPOVERLAYINTERFACECOUNT = 0x3,
};

struct __attribute__((aligned(4))) WeaponAttachment {
  const char *szInternalName;
  const char *szDisplayName;
  eAttachment attachmentType;
  eAttachmentPoint attachmentPoint;
  PenetrateType penetrateType;
  weapFireType_t fireType;
  int32_t firstRaisePriority;
  float fHipIdleAmount;
  bool bAltWeaponAdsOnly;
  bool bAltWeaponDisableSwitching;
  float altScopeADSTransInTime;
  float altScopeADSTransOutTime;
  bool bSilenced;
  bool bDualMag;
  bool laserSight;
  bool bInfraRed;
  bool bUseAsMelee;
  bool bDualWield;
  bool sharedAmmo;
  float fMasterDamageRangeScale;
  float damageScale[6];
  float damageRangeScale[6];
  float multishotBaseDamageScale[6];
  float multishotBaseDamageRangeScale[6];
  float aimAssistRangeScale;
  float aimAssistRangeAdsScale;
  float fAdsFlinchScalar;
  float fAdsFiringFlinchScalar;
  float fAdsTurnRateScalar;
  float fAdsWallRunBobScalar;
  float fAdsAdditiveFallScalar;
  float fAdsAdditiveJumpScalar;
  float fAdsAdditiveJumpLandScalar;
  float fAdsZoom1_focalLength;
  float fAdsZoom1_fStop;
  float fAdsZoom2_focalLength;
  float fAdsZoom2_fStop;
  float fAdsZoom3_focalLength;
  float fAdsZoom3_fStop;
  float fAdsZoomFov1;
  float fAdsZoomFov2;
  float fAdsZoomFov3;
  float fAdsZoomInFrac;
  float fAdsZoomOutFrac;
  float fAdsTransInTimeScale;
  float fAdsTransOutTimeScale;
  float fAdsRecoilReductionRate;
  float fAdsRecoilReductionLimit;
  float fAdsViewKickCenterSpeedScale;
  float fAdsIdleAmountScale;
  int32_t idleSpeedFromFireTransitionTime;
  float fAdsScopeBlurAmount;
  float fAdsScopeBlurStart;
  bool adsSwayViewInsteadOfGun;
  float fkickAlignedInputScalar;
  float fkickOpposedInputScalar;
  bool swayOverride;
  float swayMaxAngle;
  float swayLerpSpeed;
  float swayPitchScale;
  float swayYawScale;
  float swayHorizScale;
  float swayVertScale;
  bool adsSwayOverride;
  float adsSwayMaxAngle;
  float adsSwayLerpSpeed;
  float adsSwayTransitionLerpSpeed;
  float adsSwayPitchScale;
  float adsSwayYawScale;
  float fAdsSwayHorizScale;
  float fAdsSwayVertScale;
  float adsMoveSpeedScale;
  float adsFiringSpeedScale;
  int32_t adsMoveSpeedTransitionTimeIn;
  int32_t adsMoveSpeedTransitionTimeOut;
  float fHipSpreadMinScale;
  float fHipSpreadMaxScale;
  float strafeRotR;
  float standMoveF;
  vec3_t vStandRot;
  float fFireTimeScale;
  float fBurstDelayTimeScale;
  float fReloadTimeScale;
  float fReloadEmptyTimeScale;
  float fReloadAddTimeScale;
  float fReloadEmptyAddTimeScale;
  float fReloadQuickTimeScale;
  float fReloadQuickEmptyTimeScale;
  float fReloadQuickAddTimeScale;
  float fReloadQuickEmptyAddTimeScale;
  bool mmsWeapon;
  bool mmsInScope;
  float mmsFOV;
  float mmsAspect;
  float mmsMaxDist;
  float clipSizeScale;
  int32_t iClipSize;
  uint32_t stackFire;
  float stackFireSpread;
  float stackFireAccuracyDecay;
  const char *luiReticle;
  uint32_t perks[4];
  float customFloat0;
  float customFloat1;
  float customFloat2;
  float customFloat3;
  float customFloat4;
  float customFloat5;
  bool customBool0;
  bool customBool1;
  bool customBool2;
  bool customBool3;
  bool customBool4;
  bool customBool5;
};
typedef WeaponAttachment *WeaponAttachmentPtr;

enum class guidedMissileType_t : int32_t {
  MISSILE_GUIDANCE_NONE = 0x0,
  MISSILE_GUIDANCE_SIDEWINDER = 0x1,
  MISSILE_GUIDANCE_HELLFIRE = 0x2,
  MISSILE_GUIDANCE_JAVELIN = 0x3,
  MISSILE_GUIDANCE_BALLISTIC = 0x4,
  MISSILE_GUIDANCE_WIREGUIDED = 0x5,
  MISSILE_GUIDANCE_TVGUIDED = 0x6,
  MISSILE_GUIDANCE_DRONE = 0x7,
  MISSILE_GUIDANCE_HEATSEEKING = 0x8,
  MISSILE_GUIDANCE_ROBOTECH = 0x9,
  MISSILE_GUIDANCE_DYNAMICIMPACTPOINT = 0xA,
  MISSILE_GUIDANCE_COUNT = 0xB,
};

struct WeaponDef {
  const char *szInternalName;
  const char *szModeIndependentName;
  int32_t sessionMode;
  int32_t iVariantCount;
  const char *szDisplayName;
  const char *szAltWeaponName;
  const char *szNextChargeLevelWeaponName;
  const char *szAttachmentUnique;
  const char *szOverlayName;
  const char **szXAnims;
  game::db::xasset::XModelPtr viewModel;
  game::db::xasset::XModelPtr viewDamagedModel1;
  game::db::xasset::XModelPtr viewDamagedModel2;
  game::db::xasset::XModelPtr viewDamagedModel3;
  game::db::xasset::XModelPtr frontendModel;
  scr::ScrString_t viewmodelTag;
  scr::ScrString_t persistentViewModelTag;
  scr::ScrString_t persistentWorldModelTag;
  scr::ScrString_t stowedModelTag;
  scr::ScrString_t *hideTags;
  game::db::xasset::XModelPtr *attachViewModel;
  game::db::xasset::XModelPtr *attachWorldModel;
  const char **attachViewModelTag;
  const char **attachWorldModelTag;
  float attachViewModelOffsets[15];
  float attachWorldModelOffsets[15];
  float attachViewModelRotations[15];
  float attachWorldModelRotations[15];
  vec3_t stowedModelOffsets;
  vec3_t stowedModelRotations;
  scr::ScrString_t worldModelTagRight;
  scr::ScrString_t worldModelTagLeft;
  bool worldModelNoTagAttach;
  game::db::xasset::XModelPtr handModel;
  const char *szModeName;
  AttachmentCosmeticVariantPtr *attachmentCosmeticVariants;
  int32_t playerAnimType;
  weapType_t weapType;
  weapClass_t weapClass;
  PenetrateType penetrateType;
  ImpactType impactType;
  weapInventoryType_t inventoryType;
  weapFireType_t fireType;
  weapClipType_t clipType;
  barrelType_t barrelType;
  gadgetProperties_t gadgetProperty;
  db::xasset::XCam *cacWeaponXCam;
  db::xasset::XCam *cacAttachmentsXCam;
  db::xasset::XCam *weaponIconXCam;
  bool isSignatureWeapon;
  bool isUtilityWeapon;
  game::db::xasset::ScriptBundlePtr statusEffectBundle;
  int32_t itemIndex;
  XString parentWeaponName;
  int32_t iJamFireTime;
  int32_t overheatWeapon;
  float overheatRate;
  float cooldownRate;
  float overheatEndVal;
  bool coolWhileFiring;
  bool fuelTankWeapon;
  int32_t iTankLifeTime;
  OffhandClass offhandClass;
  OffhandSlot offhandSlot;
  weapStance_t stance;
  db::xasset::FxEffectDefHandle viewFlashEffect;
  db::xasset::FxEffectDefHandle worldFlashEffect;
  db::xasset::FxEffectDefHandle barrelCooldownEffect;
  int32_t barrelCooldownMinCount;
  db::xasset::FxEffectDefHandle viewPersistentEffect;
  db::xasset::TaxFxSetPtr viewPersistentEffectSet;
  db::xasset::FxEffectDefHandle worldPersistentEffect;
  db::xasset::TaxFxSetPtr worldPersistentEffectSet;
  db::xasset::TaxFxSetPtr enemyDeathFxSet;
  vec3_t vViewFlashOffset;
  vec3_t vWorldFlashOffset;
  float slamJumpHeight;
  float slamJumpForwardSpeed;
  int32_t slamViewAngle;
  db::xasset::SharedWeaponSoundsPtr sharedWeaponSounds;
  const char *projectileLoopSound;
  const char *projectileFireSound;
  const char *pullbackSound;
  const char *pullbackSoundPlayer;
  const char *fireSoundDistant;
  const char *fireSound;
  const char *fireSoundPlayer;
  const char *fireBurstSound;
  const char *fireBurstSoundPlayer;
  const char *suppressedRapidFireBurstSound;
  const char *suppressedRapidFireBurstSoundPlayer;
  const char *fireLoopSound;
  const char *fireLoopSoundPlayer;
  const char *fireLoopSoundLeft;
  const char *fireLoopSoundPlayerLeft;
  const char *fireLoopEndSound;
  const char *fireLoopEndSoundPlayer;
  const char *suppressedRapidLoopFireSound;
  const char *suppressedRapidLoopFireSoundPlayer;
  const char *fireStartSound;
  const char *fireStopSound;
  const char *fireKillcamSound;
  const char *fireStartSoundPlayer;
  const char *fireStopSoundPlayer;
  const char *fireKillcamSoundPlayer;
  const char *fireLastSound;
  const char *fireLastSoundPlayer;
  const char *emptyFireSound;
  const char *emptyFireSoundPlayer;
  const char *crackSound;
  const char *whizbySound;
  const char *holdFireSound;
  const char *holdFireSoundPlayer;
  const char *chargeShotMaxSound;
  const char *chargeShotMaxSoundPlayer;
  float crackSoundRange;
  float crackSoundAngle;
  float whizbyRange;
  snd::SndAliasId gadgetLoopSound;
  snd::SndAliasId gadgetAltLoopSound;
  snd::SndAliasId gadgetOnSound;
  snd::SndAliasId gadgetOffSound;
  snd::SndAliasId gadgetPrimedLoopSound;
  snd::SndAliasId gadgetPrimedOnSound;
  snd::SndAliasId gadgetPrimedOffSound;
  snd::SndAliasId gadgetReadySound;
  snd::SndAliasId gadgetFlickerSound;
  snd::SndAliasId gadgetLoopSoundPlayer;
  snd::SndAliasId gadgetAltLoopSoundPlayer;
  snd::SndAliasId gadgetOnSoundPlayer;
  snd::SndAliasId gadgetOffSoundPlayer;
  snd::SndAliasId gadgetPrimedLoopSoundPlayer;
  snd::SndAliasId gadgetPrimedOnSoundPlayer;
  snd::SndAliasId gadgetPrimedOffSoundPlayer;
  snd::SndAliasId gadgetReadySoundPlayer;
  snd::SndAliasId gadgetFlickerSoundPlayer;
  snd::SndAliasId gadgetTakeSoundPlayer;
  snd::SndAliasId gadgetGiveSoundPlayer;
  snd::SndAliasId gadgetPingSoundPlayer;
  snd::SndAliasId gadgetEnemyPingSoundPlayer;
  snd::SndAliasId gadgetMissSoundPlayer;
  db::xasset::TaxFxSetPtr tagFXSetFirstPerson[4];
  db::xasset::TaxFxSetPtr tagFXSetThirdPerson[4];
  gfx::GfxImageHandle gadgetIconAvailable;
  gfx::GfxImageHandle gadgetIconUnavailable;
  gfx::GfxImageHandle gadgetEnemyTypeIcon;
  const char *fireDelayStartSound;
  const char *fireDelayStartSoundPlayer;
  const char *rechamberSound;
  const char *rechamberSoundPlayer;
  const char *reloadSound;
  const char *reloadSoundPlayer;
  const char *reloadEmptySound;
  const char *reloadEmptySoundPlayer;
  const char *reloadStartSound;
  const char *reloadStartSoundPlayer;
  const char *reloadEndSound;
  const char *reloadEndSoundPlayer;
  const char *rotateLoopSound;
  const char *rotateLoopSoundPlayer;
  const char *rotateStopSound;
  const char *rotateStopSoundPlayer;
  const char *deploySound;
  const char *deploySoundPlayer;
  const char *finishDeploySound;
  const char *finishDeploySoundPlayer;
  const char *breakdownSound;
  const char *breakdownSoundPlayer;
  const char *finishBreakdownSound;
  const char *finishBreakdownSoundPlayer;
  const char *detonateSound;
  const char *detonateSoundPlayer;
  const char *overheatSound;
  const char *overheatSoundPlayer;
  const char *adsZoomSound;
  const char *adsZoomOutSound;
  const char *shellCasing;
  const char *shellCasingPlayer;
  const char *lockOnStartSound;
  const char *lockOnFinishSound;
  const char *lockOnLostSound;
  const char *lockOnNeededFireSound;
  const char *weaponSwitchCancelSound;
  const char *standMountedWeapdef;
  const char *crouchMountedWeapdef;
  const char *proneMountedWeapdef;
  int32_t standMountedIndex;
  int32_t crouchMountedIndex;
  int32_t proneMountedIndex;
  db::xasset::FxEffectDefHandle viewShellEjectEffect;
  db::xasset::FxEffectDefHandle worldShellEjectEffect;
  db::xasset::FxEffectDefHandle viewLastShotEjectEffect;
  db::xasset::FxEffectDefHandle worldLastShotEjectEffect;
  db::xasset::FxEffectDefHandle viewFireDelayStartEffect;
  db::xasset::FxEffectDefHandle worldFireDelayStartEffect;
  vec3_t vViewShellEjectOffset;
  vec3_t vWorldShellEjectOffset;
  vec3_t vViewShellEjectRotation;
  vec3_t vWorldShellEjectRotation;
  gfx::GfxImageHandle reticleCenter;
  gfx::GfxImageHandle reticleSide;
  int32_t iReticleCenterSize;
  int32_t iReticleSideSize;
  int32_t iReticleMinOfs;
  activeReticleType_t activeReticleType;
  vec3_t vStandMove;
  vec3_t vStandRot;
  vec3_t vDuckedOfs;
  vec3_t vDuckedMove;
  vec3_t vDuckedSprintOfs;
  vec3_t vDuckedSprintRot;
  float fDuckedSprintCycleScale;
  vec3_t vSprintOfs;
  vec3_t vSprintRot;
  float fSprintCycleScale;
  vec3_t vLowReadyOfs;
  vec3_t vLowReadyRot;
  vec3_t vRideOfs;
  vec3_t vRideRot;
  vec3_t vMantleOfs;
  vec3_t vMantleRot;
  vec3_t vSlideOfs;
  vec3_t vSlideRot;
  vec3_t vWallRunOfs;
  vec3_t vWallRunRot;
  vec3_t vDuckedRot;
  vec3_t vProneOfs;
  vec3_t vProneMove;
  vec3_t vProneRot;
  vec3_t vStrafeMove;
  vec3_t vStrafeRot;
  float fPosMoveRate;
  float fPosProneMoveRate;
  float fStandMoveMinSpeed;
  float fDuckedMoveMinSpeed;
  float fProneMoveMinSpeed;
  float fPosRotRate;
  float fPosProneRotRate;
  float fStandRotMinSpeed;
  float fDuckedRotMinSpeed;
  float fProneRotMinSpeed;
  int32_t iWeaponStartHitPoints;
  int32_t iWeaponDamage1HitPoints;
  int32_t iWeaponDamage2HitPoints;
  int32_t iWeaponDamage3HitPoints;
  game::db::xasset::XModelPtr worldModel;
  game::db::xasset::XModelPtr worldModelLeft;
  game::db::xasset::XModelPtr worldDamagedModel1;
  game::db::xasset::XModelPtr worldDamagedModel2;
  game::db::xasset::XModelPtr worldDamagedModel3;
  game::db::xasset::XModelPtr stowedModel;
  game::db::xasset::XModelPtr worldClipModel;
  game::db::xasset::XModelPtr rocketModel;
  game::db::xasset::XModelPtr mountedModel;
  game::db::xasset::XModelPtr worldGlowingModel;
  bool bShownEnemyEquip;
  bool bShownEnemyExplo;
  bool bShownRetrievable;
  bool bHideClipWhenNoAmmo;
  const char *fireTypeString;
  gfx::GfxImageHandle hudIcon;
  weaponIconRatioType_t hudIconRatio;
  const char *szAmmoDisplayName;
  const char *szAmmoName;
  int32_t iAmmoIndex;
  const char *szClipName;
  int32_t iClipIndex;
  gfx::GfxImageHandle ammoCounterIcon;
  weaponIconRatioType_t ammoCounterIconRatio;
  ammoCounterClipType_t ammoCounterClip;
  int32_t iStartAmmo;
  int32_t iMaxAmmo;
  int32_t shotCount;
  const char *szSharedAmmoCapName;
  int32_t iSharedAmmoCapIndex;
  int32_t iSharedAmmoCap;
  bool unlimitedAmmo;
  bool ammoCountClipRelative;
  int32_t ammoCountEquipment;
  bool damageAlwaysKillsPlayer;
  float damageToOwnerScalar;
  int32_t damage[6];
  float damageRange[6];
  int32_t multishotBaseDamage[6];
  float multishotBaseDamageRange[6];
  int32_t minPlayerDamage;
  float damageDuration;
  float damageInterval;
  int32_t playerDamage;
  int32_t multishotBasePlayerDamage;
  int32_t multishotBaseMinPlayerDamage;
  int32_t iMeleeDamage;
  int32_t iMeleeFromBehindDamage;
  int32_t iMeleePowerDamage;
  int32_t iMeleePowerDamageLeft;
  bool doNotDamageOwner;
  bool destroysEquipment;
  scr::ScrString_t explosionTag;
  int32_t iFireDelay;
  int32_t iMeleeDelay;
  int32_t meleePowerDelay;
  int32_t meleePowerLoopDelay;
  int32_t meleePowerDelayLeft;
  int32_t meleeChargeDelay;
  int32_t meleeChargeDelayAbove;
  int32_t iDetonateDelay;
  int32_t noMeleeHint;
  int32_t iSpinUpTime;
  int32_t iSpinDownTime;
  float spinRate;
  const char *spinLoopSound;
  const char *spinLoopSoundPlayer;
  const char *startSpinSound;
  const char *startSpinSoundPlayer;
  const char *stopSpinSound;
  const char *stopSpinSoundPlayer;
  bool applySpinPitch;
  int32_t iClipSize;
  int32_t iReloadTime;
  int32_t iReloadEmptyTime;
  int32_t iReloadQuickTime;
  int32_t iReloadQuickEmptyTime;
  int32_t iReloadSpecialComboTime;
  int32_t iReloadSpecialComboEmptyTime;
  int32_t iReloadSpecialComboQuickTime;
  int32_t iReloadSpecialComboQuickEmptyTime;
  int32_t iAdsTransInTime;
  int32_t iAdsTransOutTime;
  int32_t iAltRaiseTime;
  int32_t iAdsAltRaiseTime;
  int32_t iFireTime;
  int32_t iLastFireTime;
  int32_t iRechamberTime;
  int32_t iRechamberBoltTime;
  int32_t iQuickTossTime;
  int32_t iHoldFireTime;
  int32_t iSlamTime;
  int32_t iCastInTime;
  int32_t iCastLoopTime;
  int32_t iCastOutTime;
  int32_t iCastOutHitTime;
  int32_t iCastTime;
  int32_t iDetonateTime;
  int32_t iMeleeTime;
  int32_t iMeleeAltTime;
  int32_t iBurstDelayTime;
  int32_t meleePowerTime;
  int32_t meleePowerInTime;
  int32_t meleePowerLoopTime;
  int32_t meleePowerOutTime;
  int32_t meleePowerTimeLeft;
  int32_t meleeChargeTime;
  int32_t meleeChargeTimeAbove;
  int32_t meleeChargeFatalTime;
  int32_t meleeChargeFatalCloseTime;
  int32_t meleeChargeMissTime;
  int32_t meleeCharge2Time;
  int32_t meleeCharge2FatalTime;
  int32_t meleeCharge2FatalCloseTime;
  int32_t meleeCharge2MissTime;
  int32_t meleeCharge3Time;
  int32_t meleeCharge3FatalTime;
  int32_t meleeCharge3FatalCloseTime;
  int32_t meleeCharge3MissTime;
  int32_t meleeAssassinationTimeFront;
  int32_t meleeAssassinationTimeBack;
  int32_t meleeAssassinationTimeLeft;
  int32_t meleeAssassinationTimeRight;
  int32_t meleeQueueMeleeEarlyTime;
  int32_t meleeLeftTime;
  int32_t meleeLeftChargeTime;
  int32_t meleeLeftChargeFatalTime;
  int32_t meleeAssassinationStateTimeTransInTime;
  int32_t meleeAssassinationStateTimeTransOutTime;
  int32_t iReloadTimeLeft;
  int32_t reloadShowRocketTime;
  int32_t iReloadEmptyTimeLeft;
  int32_t iReloadAddTime;
  int32_t iReloadEmptyAddTime;
  int32_t iReloadQuickAddTime;
  int32_t iReloadQuickEmptyAddTime;
  int32_t iReloadSpecialComboAddTime;
  int32_t iReloadSpecialComboEmptyAddTime;
  int32_t iReloadSpecialComboQuickAddTime;
  int32_t iReloadSpecialComboQuickEmptyAddTime;
  int32_t iReloadStartTime;
  int32_t iReloadStartAddTime;
  int32_t iReloadEndTime;
  int32_t iDropTime;
  int32_t iRaiseTime;
  int32_t iAltDropTime;
  int32_t iAdsAltDropTime;
  int32_t quickDropTime;
  int32_t quickRaiseTime;
  int32_t iFirstRaiseTime;
  int32_t iEmptyRaiseTime;
  int32_t iEmptyDropTime;
  int32_t chargeSprintInTime;
  int32_t chargeSprintLoopTime;
  int32_t chargeSprintOutTime;
  int32_t sprintInTime;
  int32_t sprintLoopTime;
  int32_t sprintLoopSlowTime;
  int32_t sprintOutTime;
  int32_t sprintCombatTime;
  int32_t sprintCombatCooldownTime;
  int32_t wallRunInTime;
  int32_t wallRunLoopTime;
  int32_t wallRunOutTime;
  int32_t lowReadyInTime;
  int32_t lowReadyLoopTime;
  int32_t lowReadyOutTime;
  int32_t contFireInTime;
  int32_t contFireLoopTime;
  int32_t contFireOutTime;
  int32_t crawlInTime;
  int32_t crawlForwardTime;
  int32_t crawlBackTime;
  int32_t crawlRightTime;
  int32_t crawlLeftTime;
  int32_t crawlOutFireTime;
  int32_t crawlOutTime;
  int32_t slideInTime;
  int32_t slideLoopTime;
  int32_t slideOutTime;
  int32_t leapInTime;
  int32_t leapLoopTime;
  int32_t leapCancelTime;
  int32_t leapOutTime;
  int32_t diveInTime;
  int32_t diveLoopTime;
  int32_t diveOutTime;
  int32_t swimFromLandTime;
  int32_t swimIdleLoopTime;
  int32_t swimCombatInTime;
  int32_t swimCombatCooldownTime;
  int32_t swimCombatOutTime;
  int32_t swimMovingInTime;
  int32_t swimMovingForwardTime;
  int32_t swimMovingBackwardTime;
  int32_t swimMovingLeftTime;
  int32_t swimMovingRightTime;
  int32_t swimMovingOutTime;
  int32_t swimSprintInTime;
  int32_t swimSprintLoopTime;
  int32_t swimSprintSurfaceLoopTime;
  int32_t swimSprintOutTime;
  int32_t swimToLandTime;
  int32_t swimDropTime;
  int32_t grappleNoTargetTime;
  int32_t grappleStartTime;
  int32_t grappleAnchorAscendTime;
  int32_t grappleAscendEndTime;
  int32_t grappleAnchorYankTime;
  int32_t grappleYankEndTime;
  int32_t doubleJumpInTime;
  int32_t doubleJumpLoopTime;
  int32_t doubleJumpCancelTime;
  int32_t doubleJumpOutTime;
  int32_t deployTime;
  int32_t breakdownTime;
  int32_t fuseTime;
  int32_t aiFuseTime;
  int32_t lockOnRadius;
  int32_t lockOnLossRadius;
  int32_t lockOnSpeed;
  bool requireLockonToFire;
  bool noAdsWhenMagEmpty;
  bool avoidDropCleanup;
  bool fuseStartsOnImpact;
  int32_t burnDuration;
  int32_t burnDamageInterval;
  int32_t burnDamage;
  int32_t lockOnRequiredMsgSuppressAfterFireTime;
  int32_t lockOnMinRange;
  int32_t lockOnMaxRange;
  int32_t lockOnMaxRangeNoLineOfSight;
  int32_t lockOnScreenRadius;
  float lockOnTimeMultInCover;
  int32_t lockOnLostSightPivotTimeout;
  float targetTetherHorzAng;
  float targetTetherVertAng;
  float targetLostHorzAng;
  float targetLostVertAng;
  const char *luigiReticleWidget;
  gfx::GfxImageHandle reticleSeekingLockOn;
  gfx::GfxImageHandle reticleLockOn;
  db::xasset::MaterialHandle reticlePivotLocked;
  db::xasset::MaterialHandle reticlePivotInvalid;
  db::xasset::MaterialHandle reticlePivotTarget;
  db::xasset::MaterialHandle reticlePivotSides;
  db::xasset::MaterialHandle reticlePivotLine;
  const char *luigiLockOnWidget;
  const char *lockonSeekerSearchSound;
  bool lockonSeekerSearchSoundLoops;
  const char *lockonSeekerLockedSound;
  bool lockonSeekerLockedSoundLoops;
  const char *lockonTargetLockedSound;
  bool lockonTargetLockedSoundLoops;
  const char *lockonTargetFiredOnSound;
  bool lockonTargetFiredOnSoundLoops;
  bool grappleWeapon;
  bool grappleWeaponHoldTrigger;
  float grappleReelOutSpeed;
  float grappleReelInSpeed;
  float grappleReelInAccelTime;
  float grappleReelInDecelTime;
  float grappleMagnetTetherHorzAng;
  float grappleMagnetTetherVertAng;
  gfx::GfxImageHandle reticleGrapple;
  scr::ScrString_t grappleStartTag;
  int32_t grappleWobbleTime;
  float grappleWobbleMaxAng;
  int32_t grappleWobbleAngTime;
  uint32_t stackFire;
  float stackFireSpread;
  float stackFireAccuracyDecay;
  const char *stackSound;
  bool chargeShotFireWhenQueueReachesMax;
  uint32_t chargeShotMaxLevel;
  uint32_t chargeShotMaxTime;
  uint32_t chargeShotMinTime;
  float autoAimRange;
  float aimAssistRange;
  float aimAssistRangeMin;
  float aimAssistRangeMinAds;
  uint32_t aimAssistAdsDelay;
  bool mountableWeapon;
  bool activatesSlam;
  bool activatesSlamAsMelee;
  bool activatesSlamAsPowerMelee;
  bool activatesSlamAsPowerMeleeLeft;
  bool disableSlamInAir;
  bool allowWhileTraversing;
  bool grenadeUseRigidBodyPhysics;
  bool dontTakeClipOnlyWeaponIfEmpty;
  int32_t additionalTracesOffset;
  float aimPadding;
  float enemyCrosshairRange;
  float enemyCrosshairADSRange;
  bool crosshairColorChange;
  float moveSpeedScale;
  float adsMoveSpeedScale;
  float jumpMoveSpeedScale;
  float firingSpeedScale;
  float adsFiringSpeedScale;
  float sprintFiringSpeedScale;
  int32_t adsMoveSpeedTransitionTimeIn;
  int32_t adsMoveSpeedTransitionTimeOut;
  float sprintDurationScale;
  float sprintRestoreDelay;
  weapOverlayReticle_t overlayReticle;
  WeapOverlayInteface_t overlayInterface;
  float overlayWidth;
  float overlayHeight;
  float fAdsBobFactor;
  float fAdsViewBobMult;
  bool bHoldBreathToSteady;
  float fHipSpreadStandMin;
  float fHipSpreadDuckedMin;
  float fHipSpreadProneMin;
  float fHipSpreadSlideMin;
  float hipSpreadStandMax;
  float hipSpreadDuckedMax;
  float hipSpreadProneMax;
  float hipSpreadSlideMax;
  float fHipSpreadDecayRate;
  float fHipSpreadFireAdd;
  float fHipSpreadTurnAdd;
  float fHipSpreadMoveAdd;
  float fHipSpreadSprintAdd;
  float fHipSpreadDuckedDecay;
  float fHipSpreadProneDecay;
  float fHipSpreadSlideDecay;
  float fHipReticleSidePos;
  float fAdsIdleAmount;
  float fHipIdleAmount;
  float adsIdleSpeed;
  float hipIdleSpeed;
  int32_t idleSpeedFromFireTransitionTime;
  float fIdleCrouchFactor;
  float fIdleProneFactor;
  float fGunMaxPitch;
  float fGunMaxYaw;
  float fAimAssistRangeAds;
  float fAdsSwayHorizScale;
  float fAdsSwayVertScale;
  float fkickAlignedInputScalar;
  float fkickOpposedInputScalar;
  float fAdsViewKickCenterSpeed;
  float fHipViewKickCenterSpeed;
  float fAdsFlinchScalar;
  float fAdsFiringFlinchScalar;
  float fAdsTurnRateScalar;
  float fAdsWallRunBobScalar;
  float fAdsAdditiveFallScalar;
  float fAdsAdditiveJumpScalar;
  float fAdsAdditiveJumpLandScalar;
  float fAdsZoom1_focalLength;
  float fAdsZoom1_fStop;
  float fAdsZoom2_focalLength;
  float fAdsZoom2_fStop;
  float fAdsZoom3_focalLength;
  float fAdsZoom3_fStop;
  float fAdsZoomFov1;
  float fAdsZoomFov2;
  float fAdsZoomFov3;
  float fAdsZoomInFrac;
  float fAdsZoomOutFrac;
  float fOverlayAlphaScale;
  float fOOPosAnimLength[4];
  float swayMaxAngle;
  float swayLerpSpeed;
  float swayPitchScale;
  float swayYawScale;
  float swayHorizScale;
  float swayVertScale;
  float swayShellShockScale;
  float adsSwayMaxAngle;
  float adsSwayLerpSpeed;
  float adsSwayTransitionLerpSpeed;
  float adsSwayPitchScale;
  float adsSwayYawScale;
  bool sharedAmmo;
  bool bRifleBullet;
  bool armorPiercing;
  bool bAirburstWeapon;
  bool bBoltAction;
  bool bUseAltTagFlash;
  bool bUseAntiLagRewind;
  bool bIsCarriedKillstreakWeapon;
  bool aimDownSight;
  bool dualRenderADS;
  bool bRechamberWhileAds;
  bool bReloadWhileAds;
  float adsViewErrorMin;
  float adsViewErrorMax;
  bool bCookOffHold;
  int32_t bCookOffHoldTime;
  bool bClipOnly;
  bool bCanUseInVehicle;
  bool bCanUseUnderWater;
  bool bDropGrenadeOnDeath;
  bool bWaterMovement;
  bool bNoDropsOrRaises;
  bool bAllowKnockback;
  bool adsFireOnly;
  bool cancelAutoHolsterWhenEmpty;
  bool suppressAmmoReserveDisplay;
  bool laserSight;
  bool bHideThirdPerson;
  bool bDualWield;
  bool bExplodeOnGround;
  bool bThrowBack;
  bool bThrowLeftHanded;
  bool bMagicMissileTrail;
  bool bRetrievable;
  bool bDieOnRespawn;
  bool bNoThirdPersonDropsOrRaises;
  bool bContinuousFire;
  bool bNoPing;
  bool bForceBounce;
  bool bNoBounce;
  bool bUseDroppedModelAsStowed;
  bool bNoQuickDropWhenEmpty;
  bool bKeepCrosshairWhenADS;
  bool bAltWeaponAdsOnly;
  bool bAltWeaponDisableSwitching;
  bool bAltWeaponOnADS;
  bool bSetUsedStat;
  bool bIsEmp;
  bool bIsFlash;
  bool bIsStun;
  bool bDoEmpDestroyFx;
  bool bDoStun;
  bool bDoDamageFeedback;
  bool bDoHackedStats;
  bool bDisableCamoOnFire;
  bool bAnyPlayerCanRetrieve;
  bool bDestroyableByTrophySystem;
  bool bDrawOffhandModelInHand;
  bool bNonStowedWeapon;
  bool bIsScavengable;
  bool bDoesFireDamage;
  bool bIgnoresFlakJacket;
  bool bNotKillstreak;
  bool bIsGameplayWeapon;
  bool bIsSupplyDropWeapon;
  bool bSkipBattlechatterKill;
  bool bSkipBattlechatterReload;
  bool bForceDamageShellshockAndRumble;
  bool bIsAIKillstreakDamage;
  bool bIgnoreTeamKills;
  bool bIsBallisticKnife;
  bool bIsPerkBottle;
  bool bSkipLowAmmoVox;
  bool bIsFlourishWeapon;
  bool bIsHybridWeapon;
  bool bDisableDeploy;
  bool noADSLockOnCheck;
  bool spinMinigunOnADS;
  bool firingCancelsSlide;
  bool bIsSniperWeapon;
  bool bIsHackToolWeapon;
  bool bIgnoresLightArmor;
  bool bMeleeIgnoresLightArmor;
  bool bIgnoresPowerArmor;
  bool bDisableWeaponRest;
  float fWeaponRestPitch;
  float fWeaponRestYaw;
  float fWeaponRestRoll;
  float fWeaponRestLookingUpPitch;
  float fWeaponRestLookingUpYaw;
  float fWeaponRestLookingUpRoll;
  float fWeaponRestLookingDownPitch;
  float fWeaponRestLookingDownYaw;
  float fWeaponRestLookingDownRoll;
  bool bTurretApplyRestPitch;
  bool bTurretApplyRestYaw;
  int32_t soundRattleRangeMin;
  int32_t soundRattleRangeMax;
  int32_t burstCount;
  float knockbackScaler;
  float weaponSwitchCancelTransitionTime;
  float teamKillPenaltyScale;
  gfx::GfxImageHandle killIcon;
  weaponIconRatioType_t killIconRatio;
  bool flipKillIcon;
  gfx::GfxImageHandle impactKillIcon;
  gfx::GfxImageHandle indicatorIcon;
  weaponIconRatioType_t indicatorIconRatio;
  db::xasset::ObjectivePtr weaponHeadObjective;
  db::xasset::ObjectivePtr crateObjective;
  float weaponHeadObjectiveHeight;
  bool bNoPartialReload;
  bool bSegmentedReload;
  bool bNoADSAutoReload;
  int32_t iReloadAmmoAdd;
  int32_t iReloadStartAdd;
  const char *szSpawnedGrenadeWeaponName;
  const char *szDualWieldWeaponName;
  uint32_t dualWieldWeaponIndex;
  uint32_t statWeaponIndex;
  int32_t iDropAmmoMin;
  int32_t iDropAmmoMax;
  int32_t iDropClipAmmoMin;
  int32_t iDropClipAmmoMax;
  int32_t iShotsBeforeRechamber;
  bool blocksProne;
  bool bShowIndicator;
  bool isRollingGrenade;
  bool showBallisticPrediction;
  bool showBallisticPredictionWhenAds;
  bool hideBallisticPrediction;
  bool isValuable;
  bool isTacticalInsertion;
  bool isReviveWeapon;
  bool bUseRigidBodyOnVehicle;
  int32_t totalCoverageDuration;
  int32_t totalCoverageMaxRadius;
  int32_t totalCoverageFadeInTime;
  int32_t totalCoverageFadeOutTime;
  ExplosionDamage explosionInner;
  ExplosionDamage explosionOuter;
  ExplosionDamage explosionInnerNoCollision;
  ExplosionDamage explosionOuterNoCollision;
  float explosionNormalOffset;
  bool iCanDoQuickToss;
  int32_t iIndicatorRadius;
  float damageConeAngle;
  int32_t iProjectileSpeed;
  int32_t iProjectileSpeedUp;
  int32_t iProjectileSpeedRelativeUp;
  int32_t iProjectileSpeedForward;
  float fProjectileTakeParentVelocity;
  int32_t iProjectileActivateDist;
  float projLifetime;
  float timeToAccelerate;
  float projectileCurvature;
  game::db::xasset::XModelPtr projectileModel;
  game::db::xasset::XModelPtr projectileModelEnemy;
  weapProjExposion_t projExplosion;
  db::xasset::FxEffectDefHandle projExplosionEffect;
  db::xasset::FxEffectDefHandle projExplosionEffectPlayer;
  bool projExplosionEffectForceNormalToProjAngles;
  bool projExplosionEffectForceNormalUp;
  db::xasset::FxEffectDefHandle projExplosionEffect2;
  db::xasset::FxEffectDefHandle projDudEffect;
  const char *projExplosionSound;
  const char *projExplosionSoundPlayer;
  const char *projSmokeStartSound;
  const char *projSmokeLoopSound;
  const char *projSmokeEndSound;
  const char *projDudSound;
  const char *mortarShellSound;
  const char *tankShellSound;
  bool bImpactDamageOnlyActorsAndPlayers;
  bool bProjImpactExplode;
  bool bProjImpactExplodeNoSentients;
  bool bProjSentientImpactExplode;
  bool bProjExplodeWhenStationary;
  bool bBulletImpactExplode;
  WeapStickinessType stickiness;
  WeapStickOrientation stickOrientation;
  WeapRotateType rotateType;
  bool plantable;
  bool hasDetonator;
  bool timedDetonation;
  bool allowsDetonateDuringReload;
  int32_t proximityDetonation;
  int32_t proximityAlarmInnerRadius;
  int32_t proximityAlarmOuterRadius;
  int32_t proximityAlarmActivationDelay;
  int32_t projectileKickback;
  int32_t chainEventRadius;
  int32_t chainEventTime;
  int32_t chainEventMax;
  int32_t multiDetonation;
  int32_t multiDetonationFragmentSpeed;
  float curveballForce;
  int32_t curveballMaxRadius;
  bool bNoCrumpleMissile;
  bool bNoCollidePlayer;
  bool bBoxTrace;
  bool bBoxTraceSymmetrical;
  bool bBoxTraceAgainstPlayersAI;
  int32_t boxTraceSize;
  bool bAuxiliaryTrace;
  float auxiliaryTraceRadius;
  int32_t auxiliaryTraceCount;
  bool rotate;
  bool bKeepRolling;
  bool holdButtonToThrow;
  bool offhandHoldIsCancelable;
  bool freezeMovementWhenFiring;
  float lowAmmoWarningThreshold;
  float lowTotalAmmoWarningThreshold;
  bool bDisallowAtMatchStart;
  bool projPassThroughFriendlyActors;
  bool projPassThroughFriendlyPlayers;
  bool projPassThroughEnemyActors;
  bool projPassThroughEnemyPlayers;
  bool overheatDial;
  bool projImpales;
  float impaleImpactDecel;
  float impaleImpactDecelTime;
  int32_t splitProjectileCount;
  float splitProjectileAngle;
  XString szSplitWeaponName;
  uint32_t splitWeaponIndex;
  float meleeChargeRange;
  float meleeLungeRange;
  float meleeChargeMinRange;
  float chainMeleeRange;
  float fMeleeAssassinationStateFOV;
  bool isNotDroppable;
  bool bUseAsMelee;
  bool bUseAsMeleeLunge;
  bool bUseAsMeleePowerLoop;
  bool bMeleeJuke;
  bool bMeleeCanAssassinate;
  bool meleeWithLeftHand;
  bool bMeleeServerResponse;
  bool bDisallowMeleeChargeOnPowerMelee;
  bool bDisallowMeleeChargeInAir;
  int32_t powerLevel;
  bool bNoHoldDelayAllowed;
  bool disallowUseAsOffhandMelee;
  bool cycleMeleeChargeAnims;
  int32_t meleeMaxChainKills;
  int32_t meleeJukeTime;
  int32_t meleeJukeAccelTime;
  int32_t meleeJukeDecelTime;
  float meleeJukeSpeed;
  bool meleeJukeIgnoreActors;
  bool meleeJukeGroundOnly;
  bool meleeAmmoJukeOnly;
  bool skipPostJukeMelee;
  bool isCameraSensor;
  bool isLaserSensor;
  bool isHoldUseGrenade;
  float *parallelBounce;
  float *perpendicularBounce;
  bool perpendicularBounceUpOnly;
  float bounceRandomness;
  bool stopTrailWhenStationary;
  db::xasset::FxEffectDefHandle projTrailEffect;
  guidedMissileType_t guidedMissileType;
  weapLockType_t lockonType;
  float maxSteeringAccel;
  float maxTurnDegreesPerSecond;
  int32_t projIgnitionDelay;
  db::xasset::FxEffectDefHandle projIgnitionEffect;
  const char *projIgnitionSound;
  int32_t iProjectileRepulsedSpeed;
  const char *projectileRepulsedSound;
  bool adsProneMove;
  float fAdsAimPitch;
  float fAdsCrosshairInFrac;
  float fAdsCrosshairOutFrac;
  int32_t adsGunKickReducedKickBullets;
  float adsGunKickReducedKickPercent;
  float fAdsGunKickPitchMin;
  float fAdsGunKickPitchMax;
  float fAdsGunKickYawMin;
  float fAdsGunKickYawMax;
  float fAdsGunKickAccel;
  float fAdsGunKickSpeedMax;
  float fAdsGunKickSpeedDecay;
  float fAdsGunKickStaticDecay;
  float fAdsViewKickPitchMin;
  float fAdsViewKickPitchMax;
  float fAdsViewKickMinMagnitude;
  float fAdsViewKickYawMin;
  float fAdsViewKickYawMax;
  float fAdsRecoilReductionRate;
  float fAdsRecoilReductionLimit;
  float fAdsRecoilReturnRate;
  float fAdsViewScatterMin;
  float fAdsViewScatterMax;
  float fAdsSpread;
  float fAdsScopeBlurAmount;
  float fAdsScopeBlurStart;
  bool adsSwayViewInsteadOfGun;
  int32_t hipGunKickReducedKickBullets;
  float hipGunKickReducedKickPercent;
  float fHipGunKickPitchMin;
  float fHipGunKickPitchMax;
  float fHipGunKickYawMin;
  float fHipGunKickYawMax;
  float fHipGunKickAccel;
  float fHipGunKickSpeedMax;
  float fHipGunKickSpeedDecay;
  float fHipGunKickStaticDecay;
  float fHipViewKickPitchMin;
  float fHipViewKickPitchMax;
  float fHipViewKickMinMagnitude;
  float fHipViewKickYawMin;
  float fHipViewKickYawMax;
  float fHipViewScatterMin;
  float fHipViewScatterMax;
  float fAdsViewKickCenterDuckedScale;
  float fAdsViewKickCenterProneScale;
  float fAntiQuickScopeTime;
  float fAntiQuickScopeScale;
  float fAntiQuickScopeSpreadMultiplier;
  float fAntiQuickScopeSpreadMax;
  float fAntiQuickScopeSwayFactor;
  float fightDist;
  float maxDist;
  const char *accuracyGraphName[2];
  vec2_t *accuracyGraphKnots[2];
  vec2_t *originalAccuracyGraphKnots[2];
  int32_t accuracyGraphKnotCount[2];
  int32_t originalAccuracyGraphKnotCount[2];
  int32_t iPositionReloadTransTime;
  float leftArc;
  float rightArc;
  float topArc;
  float bottomArc;
  float accuracy;
  float aiSpread;
  float playerSpread;
  float turretRotateRate;
  float minTurnSpeed[2];
  float maxTurnSpeed[2];
  const char *szUseHintString;
  const char *dropHintString;
  int32_t iUseHintStringIndex;
  int32_t dropHintStringIndex;
  bool drawPlayerFullBody;
  float horizViewJitter;
  float vertViewJitter;
  float cameraShakeScale;
  int32_t cameraShakeDuration;
  int32_t cameraShakeRadius;
  float explosionCameraShakeScale;
  int32_t explosionCameraShakeDuration;
  int32_t explosionCameraShakeRadius;
  const char *szScript;
  float destabilizationRateTime;
  float destabilizationCurvatureMax;
  int32_t destabilizeDistance;
  float robotechMaxPitch;
  float robotechMaxYaw;
  float robotechFrequency;
  float robotechVariancePitch;
  float robotechVarianceYaw;
  float robotechVarianceFrequency;
  float *locationDamageMultipliers;
  RumbleInfoPtr fireRumble;
  RumbleInfoPtr fireDelayRumble;
  RumbleInfoPtr meleeImpactRumble;
  RumbleInfoPtr reloadRumble;
  RumbleInfoPtr explosionRumble;
  RumbleInfoPtr holdFireRumble;
  RumbleInfoPtr chargeShotMaxLoopRumble;
  db::xasset::TracerDefPtr tracerType;
  db::xasset::TracerDefPtr enemyTracerType;
  db::xasset::TracerDefPtr underwaterTracerType;
  db::xasset::LaserDefPtr laserTypeViewModel;
  db::xasset::LaserDefPtr laserTypeWorld;
  float adsDofStart;
  float adsDofEnd;
  float hipDofStart;
  float hipDofEnd;
  float scanSpeed;
  float scanAccel;
  int32_t scanPauseTime;
  db::xasset::FlameTablePtr flameTableFirstPerson;
  db::xasset::FlameTablePtr flameTableThirdPerson;
  db::xasset::BeamDefPtr flameBeamFirstPerson;
  db::xasset::BeamDefPtr flameBeamThirdPerson;
  db::xasset::BeamDefPtr flameBeamFirstPersonUnderWater;
  db::xasset::BeamDefPtr flameBeamThirdPersonUnderWater;
  db::xasset::FxEffectDefHandle tagFx_preparationEffect;
  db::xasset::FxEffectDefHandle tagFlash_preparationEffect;
  db::xasset::FxEffectDefHandle underwaterExplosionSurfaceFX[3];
  int32_t underwaterExplosionSurfaceFXDepth[2];
  float waterSwellRadius;
  float waterSwellForceScale;
  float waterSwellMaxDepth;
  bool doGibbing;
  bool doGibbingOnMelee;
  bool doAnnihilate;
  bool doBlowback;
  DamageType damageType;
  float maxGibDistance;
  float gibChance;
  float gibChanceMultiple;
  float altScopeADSTransInTime;
  float altScopeADSTransOutTime;
  int32_t iDeathCamTime;
  int32_t iIntroFireTime;
  int32_t iIntroFireLength;
  db::xasset::FxEffectDefHandle meleeSwipeEffect;
  db::xasset::FxEffectDefHandle meleePowerEffect;
  db::xasset::FxEffectDefHandle meleePowerEffectLeft;
  db::xasset::FxEffectDefHandle meleeJukeViewEffect;
  db::xasset::FxEffectDefHandle meleeJukeWorldEffect;
  db::xasset::FxEffectDefHandle meleeImpactEffect;
  db::xasset::FxEffectDefHandle meleeImpactNoBloodEffect;
  const char *throwBackType;
  WeaponCamo *weaponCamo;
  db::xasset::FxEffectDefHandle movementDirectionFinalImpactEffect;
  db::xasset::FxImpactTablePtr normalImpactFX;
  db::xasset::FxImpactTablePtr exitImpactFX;
  db::xasset::FxImpactTablePtr bounceImpactFX;
  db::xasset::FxImpactTablePtr dudImpactFX;
  db::xasset::SoundsImpactTablePtr normalImpactSounds;
  db::xasset::SoundsImpactTablePtr exitImpactSounds;
  db::xasset::SoundsImpactTablePtr bounceImpactSounds;
  db::xasset::SoundsImpactTablePtr dudImpactSounds;
  bool doNotReflectImpactFX;
  int32_t maxInstancesAllowed;
  XString hackerTriggerOriginTag;
  float vehicleProjectileDamageScalar;
  float vehicleProjectileSplashDamageScalar;
  game::db::xasset::ScriptBundlePtr aiTurretAnims;
  game::db::xasset::ScriptBundlePtr playerTurretAnims;
  XString spawnInfluencer;
  bool bNoHitMarker;
  bool bSpecialPain;
  bool bDecoy;
  bool bAltOffhand;
  bool bDNIWeapon;
  bool bDynamicLadderSight;
  float fDynamicLadderSightDefaultOffset;
  float fDynamicLadderSightADSDistanceFromCamera;
  float fDynamicLadderSightMinPitchClamp;
  bool rechamberWhenFiringStops;
  bool forceUnderwaterBounceImpactFX;
  bool projectileSlowdownWhenFiredUnderwater;
  bool hideTagClipWhenEmpty;
  bool disableMeleeWhenEmpty;
  bool allowPowerMeleeRight;
  bool forceDamageHitLocation;
  bool ammoRegen;
  float sprintBoostRadius;
  float sprintBoostDuration;
  float customFloat0;
  float customFloat1;
  float customFloat2;
  float customFloat3;
  float customFloat4;
  float customFloat5;
  float customFloat6;
  float customFloat7;
  float customFloat8;
  float customFloat9;
  float customFloat10;
  float customFloat11;
  float customFloat12;
  float customFloat13;
  float customFloat14;
  float customFloat15;
  float customFloat16;
  float customFloat17;
  float customFloat18;
  float customFloat19;
  bool customBool0;
  bool customBool1;
  bool customBool2;
  bool customBool3;
  bool customBool4;
  bool customBool5;
  bool customBool6;
  bool customBool7;
  bool customBool8;
  bool customBool9;
  bool customBool10;
  bool customBool11;
  bool customBool12;
  bool customBool13;
  bool customBool14;
  bool customBool15;
  bool customBool16;
  bool customBool17;
  bool customBool18;
  bool customBool19;
  bool customBool20;
  bool customBool21;
  bool customBool22;
  bool customBool23;
  bool customBool24;
  bool customBool25;
  bool customBool26;
  bool customBool27;
  bool customBool28;
  bool customBool29;
  game::db::xasset::ScriptBundlePtr killcamSettings;
  WeaponAttachmentPtr *attachments;
  WeaponAttachmentUniquePtr *attachmentUniques;
  uint32_t altWeaponIndex;
  uint32_t nextChargeLevelWeaponIndex;
  AttachmentMask iAttachments;
  bool bIgnoreAttachments;
  bool bSilenced;
  bool bDualMag;
  bool bInfraRed;
  bool bTVGuided;
  uint32_t perks[4];
  bool bAntiQuickScope;
  gfx::GfxImageHandle overlayMaterial;
  gfx::GfxImageHandle overlayMaterialLowRes;
  gfx::GfxImageHandle dpadIcon;
  weaponIconRatioType_t dpadIconRatio;
  bool noAmmoOnDpadIcon;
  vec3_t ikLeftHandIdlePos;
  vec3_t ikLeftHandOffset;
  vec3_t ikLeftHandRotation;
  bool bUsingLeftHandProneIK;
  vec3_t ikLeftHandProneOffset;
  vec3_t ikLeftHandProneRotation;
  vec3_t ikLeftHandUiViewerOffset;
  vec3_t ikLeftHandUiViewerRotation;
};

#pragma pack(push, 1)
struct WeaponDobjInfoModelMask {
  WeaponDobjInfoSlot slot;
  qboolean weapon;
  union {
    const WeaponDef *weapDef;
    const WeaponAttachmentUnique *attachment;
  };
  uint32_t dobjModelMask;
  uint8_t _padding14[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WeaponDobjInfo {
  uint8_t numModelMasks;
  uint8_t _padding01[3];
  WeaponDobjInfoSlot addingSlot;
  int16_t lastAddedModel;
  uint8_t _padding0A[6];
  WeaponDobjInfoModelMask modelMasks[32];
};
#pragma pack(pop)

enum class QueuedActionState : uint32_t {
  QAS_STATE_NONE = 0x0,
  QAS_STATE_FIRE_WEAPON = 0x1,
  QAS_STATE_RELOAD_WEAPON = 0x2,
  QAS_STATE_MELEE = 0x3,
  QAS_STATE_SPECIAL_MELEE = 0x4,
  QAS_STATE_MAX = 0x5,
};

#pragma pack(push, 1)
struct PlayerWeaponHandState {
  int32_t weaponTime;
  int32_t weaponDelay;
  int32_t weaponState;
  uint32_t weaponShotCount;
  animationNumber_t weapAnim;
  uint8_t _padding12[2];
  int32_t weaponFrameTimeRemainder;
  QueuedActionState queuedActionState;
};
ASSERT_SIZE(PlayerWeaponHandState, 0x1C);
#pragma pack(pop)

struct AmmoPool {
  int32_t count;
};
static_assert(sizeof(AmmoPool) == 0x4, "sizeof(AmmoPool) != 0x4");

struct AmmoClip {
  int32_t count;
};
static_assert(sizeof(AmmoClip) == 0x4, "sizeof(AmmoClip) != 0x4");

#pragma pack(push, 1)
struct PlayerHeldWeapon {
  union percent_t {
    float value;
    float heatPercent;
    float lockOnPercent;
    float hackPercent;
  };

  union time_t {
    int32_t value;
    int32_t fuelTankTime;
    int32_t targetEndTime;
  };

  Weapon weapon;
  renderOptions_t options;
  AttachmentCosmeticVariantIndexes attachmentCosmeticVariantIndexes;
  percent_t percent;
  time_t time;
  int32_t adsZoomSelect;
  union {
    uint8_t flags;
    struct {
      uint8_t overHeating : 1;
      uint8_t heldBefore : 1;
      uint8_t quickReload : 1;
      uint8_t blockWeaponPickup : 1;
      uint8_t needsRechamber : 1;
      uint8_t friendlyTargeting : 1;
      uint8_t friendlyTargetLocked : 1;
      uint8_t friendlyHacking : 1;
    };
  };
  uint8_t _padding29[7];
};
static_assert(sizeof(PlayerHeldWeapon) == 0x30,
              "sizeof(PlayerHeldWeapon) != 0x30");
#pragma pack(pop)

#pragma pack(push, 1)
struct visWeapon_s {
  Weapon weapon;
  int32_t streamerOptions;
  uint8_t _padding0C[4];
};
#pragma pack(pop)

struct visLoadOut_s {
  visWeapon_s primary;
  visWeapon_s secondary;
  int32_t primaryGadgetIdx;
  int32_t secondaryGadgetIdx;
};

#pragma pack(push, 1)
struct DelayedWeaponFire {
  int32_t time;
  uint8_t _padding04[4];
  Weapon weapon;
};
#pragma pack(pop)

} // namespace weapon
} // namespace game