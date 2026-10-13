#pragma once

#include "core.hpp"
#include "db/xasset/xmodel.hpp"
#include <cstdint>
namespace game {
namespace scr {
typedef uint32_t ScrString_t;
}

namespace weapon {

typedef uint64_t AttachmentMask;

enum class GadgetTargetResult : uint32_t {
  NONE = 0x0,
  HIT = 0x1,
  MISS = 0x2,
  MAX = 0x3,
};

inline constexpr const char *serialize(const GadgetTargetResult result) {
  switch (result) {
  case GadgetTargetResult::NONE:
    return "GADGETTARGET_NONE";
  case GadgetTargetResult::HIT:
    return "GADGETTARGET_HIT";
  case GadgetTargetResult::MISS:
    return "GADGETTARGET_MISS";
  default:
    return "GADGETTARGET_INVALID";
  }
}

enum class gadgetPulseShareType_e : int32_t {
  DEFAULT = 0x0,
  NONE = 0x1,
  MINIMAP = 0x2,
  VIEWPORT = 0x3,
  BOTH = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const gadgetPulseShareType_e type) {
  switch (type) {
  case gadgetPulseShareType_e::DEFAULT:
    return "GADGETPULSESHARE_DEFAULT";
  case gadgetPulseShareType_e::NONE:
    return "GADGETPULSESHARE_NONE";
  case gadgetPulseShareType_e::MINIMAP:
    return "GADGETPULSESHARE_MINIMAP";
  case gadgetPulseShareType_e::VIEWPORT:
    return "GADGETPULSESHARE_VIEWPORT";
  case gadgetPulseShareType_e::BOTH:
    return "GADGETPULSESHARE_BOTH";
  default:
    return "GADGETPULSESHARE_INVALID";
  }
}

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
  };

  uint64_t weaponData;
  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"weaponIdx\": {}, \"attachment1\": {}, \"attachment2\": {}, "
        "\"attachment3\": {}, \"attachment4\": {}, \"attachment5\": {}, "
        "\"attachment6\": {}, \"attachment7\": {}, \"attachment8\": {} }}",
        weaponIdx, attachment1, attachment2, attachment3, attachment4,
        attachment5, attachment6, attachment7, attachment8);
  }
};
ASSERT_SIZE(Weapon, 0x8);

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"attachmentCosmeticVariantIndex1\": {}, "
        "\"attachmentCosmeticVariantIndex2\": {}, "
        "\"attachmentCosmeticVariantIndex3\": {}, "
        "\"attachmentCosmeticVariantIndex4\": {}, "
        "\"attachmentCosmeticVariantIndex5\": {}, "
        "\"attachmentCosmeticVariantIndex6\": {}, "
        "\"attachmentCosmeticVariantIndex7\": {}, "
        "\"attachmentCosmeticVariantIndex8\": {} }}",
        attachmentCosmeticVariantIndex1, attachmentCosmeticVariantIndex2,
        attachmentCosmeticVariantIndex2, attachmentCosmeticVariantIndex3,
        attachmentCosmeticVariantIndex4, attachmentCosmeticVariantIndex5,
        attachmentCosmeticVariantIndex6, attachmentCosmeticVariantIndex7,
        attachmentCosmeticVariantIndex8);
  }
};
ASSERT_SIZE(AttachmentCosmeticVariantIndexes, 0x4);

PACKED(struct entityGadgetState_t {
  Weapon gadgetWeapon;
  int32_t gadgetActivateTime;
  int16_t gadgetEntNum;
  uint8_t gadgetEntityStateFlags;
  uint8_t _padding0F[1];
});

enum class gadgetType_e : int32_t {
  NONE = 0x0,
  OTHER = 0x1,
  OPTIC_CAMO = 0x2,
  ARMOR_REGEN = 0x3,
  ARMOR = 0x4,
  DRONE = 0x5,
  VISION_PULSE = 0x6,
  MULTI_ROCKET = 0x7,
  TURRET_DEPLOY = 0x8,
  GRENADE = 0x9,
  JUKE = 0xA,
  HACKER = 0xB,
  INFRARED = 0xC,
  SPEED_BURST = 0xD,
  HERO_WEAPON = 0xE,
  COMBAT_EFFICIENCY = 0xF,
  FLASHBACK = 0x10,
  CLEANSE = 0x11,
  SYSTEM_OVERLOAD = 0x12,
  SERVO_SHORTOUT = 0x13,
  EXO_BREAKDOWN = 0x14,
  SURGE = 0x15,
  RAVAGE_CORE = 0x16,
  REMOTE_HIJACK = 0x17,
  IFF_OVERRIDE = 0x18,
  CACOPHONY = 0x19,
  FORCED_MALFUNCTION = 0x1A,
  CONCUSSIVE_WAVE = 0x1B,
  OVERDRIVE = 0x1C,
  UNSTOPPABLE_FORCE = 0x1D,
  RAPID_STRIKE = 0x1E,
  ACTIVE_CAMO = 0x1F,
  SENSORY_OVERLOAD = 0x20,
  ES_STRIKE = 0x21,
  IMMOLATION = 0x22,
  FIREFLY_SWARM = 0x23,
  SMOKESCREEN = 0x24,
  MISDIRECTION = 0x25,
  MRPUKEY = 0x26,
  SHOCK_FIELD = 0x27,
  RESURRECT = 0x28,
  HEAT_WAVE = 0x29,
  CLONE = 0x2A,
  ROULETTE = 0x2B,
  THIEF = 0x2C,
  DISRUPTOR = 0x2D,
  HORNET_SWARM = 0x2E,
  GROUP_REVIVE = 0x2F,
  XRAY_EYES = 0x30,
  EMERGENCY_MELEE = 0x31,
  SHOULDER_GUN = 0x32,
  GRAPPLE = 0x33,
  INVULNERABLE = 0x34,
  SPRINT_BOOST = 0x35,
  HEALTH_REGEN = 0x36,
  SELF_DESTRUCT = 0x37,
  BARRIER_BUILDER = 0x38,
  SPAWN_BEACON = 0x39,
  COUNT = 0x3A,
};

inline constexpr const char *serialize(const gadgetType_e type) {
  switch (type) {
  case gadgetType_e::NONE:
    return "GADGET_NONE";
  case gadgetType_e::OTHER:
    return "GADGET_OTHER";
  case gadgetType_e::OPTIC_CAMO:
    return "GADGET_OPTIC_CAMO";
  case gadgetType_e::ARMOR_REGEN:
    return "GADGET_ARMOR_REGEN";
  case gadgetType_e::ARMOR:
    return "GADGET_ARMOR";
  case gadgetType_e::DRONE:
    return "GADGET_DRONE";
  case gadgetType_e::VISION_PULSE:
    return "GADGET_VISION_PULSE";
  case gadgetType_e::MULTI_ROCKET:
    return "GADGET_MULTI_ROCKET";
  case gadgetType_e::TURRET_DEPLOY:
    return "GADGET_TURRET_DEPLOY";
  case gadgetType_e::GRENADE:
    return "GADGET_GRENADE";
  case gadgetType_e::JUKE:
    return "GADGET_JUKE";
  case gadgetType_e::HACKER:
    return "GADGET_HACKER";
  case gadgetType_e::INFRARED:
    return "GADGET_INFRARED";
  case gadgetType_e::SPEED_BURST:
    return "GADGET_SPEED_BURST";
  case gadgetType_e::HERO_WEAPON:
    return "GADGET_HERO_WEAPON";
  case gadgetType_e::COMBAT_EFFICIENCY:
    return "GADGET_COMBAT_EFFICIENCY";
  case gadgetType_e::FLASHBACK:
    return "GADGET_FLASHBACK";
  case gadgetType_e::CLEANSE:
    return "GADGET_CLEANSE";
  case gadgetType_e::SYSTEM_OVERLOAD:
    return "GADGET_SYSTEM_OVERLOAD";
  case gadgetType_e::SERVO_SHORTOUT:
    return "GADGET_SERVO_SHORTOUT";
  case gadgetType_e::EXO_BREAKDOWN:
    return "GADGET_EXO_BREAKDOWN";
  case gadgetType_e::SURGE:
    return "GADGET_SURGE";
  case gadgetType_e::RAVAGE_CORE:
    return "GADGET_RAVAGE_CORE";
  case gadgetType_e::REMOTE_HIJACK:
    return "GADGET_REMOTE_HIJACK";
  case gadgetType_e::IFF_OVERRIDE:
    return "GADGET_IFF_OVERRIDE";
  case gadgetType_e::CACOPHONY:
    return "GADGET_CACOPHONY";
  case gadgetType_e::FORCED_MALFUNCTION:
    return "GADGET_FORCED_MALFUNCTION";
  case gadgetType_e::CONCUSSIVE_WAVE:
    return "GADGET_CONCUSSIVE_WAVE";
  case gadgetType_e::OVERDRIVE:
    return "GADGET_OVERDRIVE";
  case gadgetType_e::UNSTOPPABLE_FORCE:
    return "GADGET_UNSTOPPABLE_FORCE";
  case gadgetType_e::RAPID_STRIKE:
    return "GADGET_RAPID_STRIKE";
  case gadgetType_e::ACTIVE_CAMO:
    return "GADGET_ACTIVE_CAMO";
  case gadgetType_e::SENSORY_OVERLOAD:
    return "GADGET_SENSORY_OVERLOAD";
  case gadgetType_e::ES_STRIKE:
    return "GADGET_ES_STRIKE";
  case gadgetType_e::IMMOLATION:
    return "GADGET_IMMOLATION";
  case gadgetType_e::FIREFLY_SWARM:
    return "GADGET_FIREFLY_SWARM";
  case gadgetType_e::SMOKESCREEN:
    return "GADGET_SMOKESCREEN";
  case gadgetType_e::MISDIRECTION:
    return "GADGET_MISDIRECTION";
  case gadgetType_e::MRPUKEY:
    return "GADGET_MRPUKEY";
  case gadgetType_e::SHOCK_FIELD:
    return "GADGET_SHOCK_FIELD";
  case gadgetType_e::RESURRECT:
    return "GADGET_RESURRECT";
  case gadgetType_e::HEAT_WAVE:
    return "GADGET_HEAT_WAVE";
  case gadgetType_e::CLONE:
    return "GADGET_CLONE";
  case gadgetType_e::ROULETTE:
    return "GADGET_ROULETTE";
  case gadgetType_e::THIEF:
    return "GADGET_THIEF";
  case gadgetType_e::DISRUPTOR:
    return "GADGET_DISRUPTOR";
  case gadgetType_e::HORNET_SWARM:
    return "GADGET_HORNET_SWARM";
  case gadgetType_e::GROUP_REVIVE:
    return "GADGET_GROUP_REVIVE";
  case gadgetType_e::XRAY_EYES:
    return "GADGET_XRAY_EYES";
  case gadgetType_e::EMERGENCY_MELEE:
    return "GADGET_EMERGENCY_MELEE";
  case gadgetType_e::SHOULDER_GUN:
    return "GADGET_SHOULDER_GUN";
  case gadgetType_e::GRAPPLE:
    return "GADGET_GRAPPLE";
  case gadgetType_e::INVULNERABLE:
    return "GADGET_INVULNERABLE";
  case gadgetType_e::SPRINT_BOOST:
    return "GADGET_SPRINT_BOOST";
  case gadgetType_e::HEALTH_REGEN:
    return "GADGET_HEALTH_REGEN";
  case gadgetType_e::SELF_DESTRUCT:
    return "GADGET_SELF_DESTRUCT";
  case gadgetType_e::BARRIER_BUILDER:
    return "GADGET_BARRIER_BUILDER";
  case gadgetType_e::SPAWN_BEACON:
    return "GADGET_SPAWN_BEACON";
  default:
    return "GADGET_INVALID";
  }
}

enum class gadgetHoldMode_e : int32_t {
  WIELD = 0x0,
  OFFHAND = 0x1,
  THROW = 0x2,
  HANDS_FREE = 0x3,
  OFFHAND_PRIME = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const gadgetHoldMode_e mode) {
  switch (mode) {
  case gadgetHoldMode_e::WIELD:
    return "HOLDMODE_WIELD";
  case gadgetHoldMode_e::OFFHAND:
    return "HOLDMODE_OFFHAND";
  case gadgetHoldMode_e::THROW:
    return "HOLDMODE_THROW";
  case gadgetHoldMode_e::HANDS_FREE:
    return "HOLDMODE_HANDS_FREE";
  case gadgetHoldMode_e::OFFHAND_PRIME:
    return "HOLDMODE_OFFHAND_PRIME";
  default:
    return "HOLDMODE_INVALID";
  }
}

enum class gadgetPowerUseType_e : int32_t {
  AMMO = 0x0,
  COOLDOWN = 0x1,
  POWER_BAR = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const gadgetPowerUseType_e type) {
  switch (type) {
  case gadgetPowerUseType_e::AMMO:
    return "POWERUSE_AMMO";
  case gadgetPowerUseType_e::COOLDOWN:
    return "POWERUSE_COOLDOWN";
  case gadgetPowerUseType_e::POWER_BAR:
    return "POWERUSE_POWER_BAR";
  default:
    return "POWERUSE_INVALID";
  }
}

enum class gadgetActivateMode_e : int32_t {
  HOLD = 0x0,
  TOGGLE = 0x1,
  SINGLE = 0x2,
  TOGGLE_AND_ATTACK = 0x3,
  TOGGLE_AND_HOLD = 0x4,
  SINGLE_STAY_ON = 0x5,
  AUTO = 0x6,
  PRIME_ON_HOLD_ACTIVATE_ON_RELEASE = 0x7,
  PRIME_ON_HOLD_ACTIVATE_ON_RELEASE_STAY_ON = 0x8,
  COUNT = 0x9,
};

inline constexpr const char *serialize(const gadgetActivateMode_e mode) {
  switch (mode) {
  case gadgetActivateMode_e::HOLD:
    return "ACTIVATE_HOLD";
  case gadgetActivateMode_e::TOGGLE:
    return "ACTIVATE_TOGGLE";
  case gadgetActivateMode_e::SINGLE:
    return "ACTIVATE_SINGLE";
  case gadgetActivateMode_e::TOGGLE_AND_ATTACK:
    return "ACTIVATE_TOGGLE_AND_ATTACK";
  case gadgetActivateMode_e::TOGGLE_AND_HOLD:
    return "ACTIVATE_TOGGLE_AND_HOLD";
  case gadgetActivateMode_e::SINGLE_STAY_ON:
    return "ACTIVATE_SINGLE_STAY_ON";
  case gadgetActivateMode_e::AUTO:
    return "ACTIVATE_AUTO";
  case gadgetActivateMode_e::PRIME_ON_HOLD_ACTIVATE_ON_RELEASE:
    return "ACTIVATE_PRIME_ON_HOLD_ACTIVATE_ON_RELEASE";
  case gadgetActivateMode_e::PRIME_ON_HOLD_ACTIVATE_ON_RELEASE_STAY_ON:
    return "ACTIVATE_PRIME_ON_HOLD_ACTIVATE_ON_RELEASE_STAY_ON";
  default:
    return "ACTIVATE_INVALID";
  }
}

enum class weapOverlayReticle_t : int32_t {
  NONE = 0x0,
  CROSSHAIR = 0x1,
  COUNT = 0x2,
};

inline constexpr const char *serialize(const weapOverlayReticle_t reticle) {
  switch (reticle) {
  case weapOverlayReticle_t::NONE:
    return "RETICLE_NONE";
  case weapOverlayReticle_t::CROSSHAIR:
    return "RETICLE_CROSSHAIR";
  default:
    return "RETICLE_INVALID";
  }
}

enum class WeapStickinessType : int32_t {
  NONE = 0x0,
  ALL = 0x1,
  ALL_NO_SENTIENTS = 0x2,
  ALL_NO_VEHICLES = 0x3,
  GROUND = 0x4,
  GROUND_WITH_YAW = 0x5,
  FLESH = 0x6,
  WEAPSTICKINESS_IGNORE = 0x7,
  COUNT = 0x8,
};

inline constexpr const char *serialize(const WeapStickinessType type) {
  switch (type) {
  case WeapStickinessType::NONE:
    return "WEAPSTICKINESS_NONE";
  case WeapStickinessType::ALL:
    return "WEAPSTICKINESS_ALL";
  case WeapStickinessType::ALL_NO_SENTIENTS:
    return "WEAPSTICKINESS_ALL_NO_SENTIENTS";
  case WeapStickinessType::ALL_NO_VEHICLES:
    return "WEAPSTICKINESS_ALL_NO_VEHICLES";
  case WeapStickinessType::GROUND:
    return "WEAPSTICKINESS_GROUND";
  case WeapStickinessType::GROUND_WITH_YAW:
    return "WEAPSTICKINESS_GROUND_WITH_YAW";
  case WeapStickinessType::FLESH:
    return "WEAPSTICKINESS_FLESH";
  case WeapStickinessType::WEAPSTICKINESS_IGNORE:
    return "WEAPSTICKINESS_IGNORE";
  default:
    return "WEAPSTICKINESS_INVALID";
  }
}

enum class eAttachment : int32_t {
  NOT_FOUND = -1,
  NONE = 0x0,
  TYPE_FIRST = 0x1,
  ACOG = 0x1,
  CLANTAG = 0x2,
  DAMAGE = 0x3,
  DUALCLIP = 0x4,
  DUALOPTIC = 0x5,
  DW = 0x6,
  DYNZOOM = 0x7,
  EXTBARREL = 0x8,
  EXTCLIP = 0x9,
  FASTADS = 0xA,
  FASTRELOAD = 0xB,
  FMJ = 0xC,
  GL = 0xD,
  GADGET_MOD_0 = 0xE,
  GADGET_MOD_TAKE_TWO = 0xE,
  GADGET_MOD_1 = 0xF,
  GADGET_MOD_2 = 0x10,
  GADGET_MOD_3 = 0x11,
  GADGET_MOD_4 = 0x12,
  GADGET_MOD_5 = 0x13,
  GRIP = 0x14,
  HOLO = 0x15,
  IR = 0x16,
  IRONSIGHTS = 0x17,
  KILLCOUNTER = 0x18,
  MK = 0x19,
  MMS = 0x1A,
  NOTRACER = 0x1B,
  PRECISION = 0x1C,
  QUICKDRAW = 0x1D,
  RANGEFINDER = 0x1E,
  RECON = 0x1F,
  REDDOT = 0x20,
  REFLEX = 0x21,
  RF = 0x22,
  SELECTFIRE = 0x23,
  STACKFIRE = 0x24,
  STALKER = 0x25,
  STEADYAIM = 0x26,
  SUPPLY = 0x27,
  SUPPRESSED = 0x28,
  SWAYREDUC = 0x29,
  TACKNIFE = 0x2A,
  VZOOM = 0x2B,
  TYPE_LAST = 0x2B,
  TYPE_COUNT = 0x2C,
  INVALID = 0x2C,
};

inline constexpr const char *serialize(const eAttachment att) {
  switch (att) {
  case eAttachment::NOT_FOUND:
    return "ATTACHMENT_NOT_FOUND";
  case eAttachment::NONE:
    return "ATTACHMENT_NONE";
  case eAttachment::ACOG:
    return "ATTACHMENT_ACOG";
  case eAttachment::CLANTAG:
    return "ATTACHMENT_CLANTAG";
  case eAttachment::DAMAGE:
    return "ATTACHMENT_DAMAGE";
  case eAttachment::DUALCLIP:
    return "ATTACHMENT_DUALCLIP";
  case eAttachment::DUALOPTIC:
    return "ATTACHMENT_DUALOPTIC";
  case eAttachment::DW:
    return "ATTACHMENT_DW";
  case eAttachment::DYNZOOM:
    return "ATTACHMENT_DYNZOOM";
  case eAttachment::EXTBARREL:
    return "ATTACHMENT_EXTBARREL";
  case eAttachment::EXTCLIP:
    return "ATTACHMENT_EXTCLIP";
  case eAttachment::FASTADS:
    return "ATTACHMENT_FASTADS";
  case eAttachment::FASTRELOAD:
    return "ATTACHMENT_FASTRELOAD";
  case eAttachment::FMJ:
    return "ATTACHMENT_FMJ";
  case eAttachment::GL:
    return "ATTACHMENT_GL";
  case eAttachment::GADGET_MOD_0:
    return "ATTACHMENT_GADGET_MOD_0";
  case eAttachment::GADGET_MOD_1:
    return "ATTACHMENT_GADGET_MOD_1";
  case eAttachment::GADGET_MOD_2:
    return "ATTACHMENT_GADGET_MOD_2";
  case eAttachment::GADGET_MOD_3:
    return "ATTACHMENT_GADGET_MOD_3";
  case eAttachment::GADGET_MOD_4:
    return "ATTACHMENT_GADGET_MOD_4";
  case eAttachment::GADGET_MOD_5:
    return "ATTACHMENT_GADGET_MOD_5";
  case eAttachment::GRIP:
    return "ATTACHMENT_GRIP";
  case eAttachment::HOLO:
    return "ATTACHMENT_HOLO";
  case eAttachment::IR:
    return "ATTACHMENT_IR";
  case eAttachment::IRONSIGHTS:
    return "ATTACHMENT_IRONSIGHTS";
  case eAttachment::KILLCOUNTER:
    return "ATTACHMENT_KILLCOUNTER";
  case eAttachment::MK:
    return "ATTACHMENT_MK";
  case eAttachment::MMS:
    return "ATTACHMENT_MMS";
  case eAttachment::NOTRACER:
    return "ATTACHMENT_NOTRACER";
  case eAttachment::PRECISION:
    return "ATTACHMENT_PRECISION";
  case eAttachment::QUICKDRAW:
    return "ATTACHMENT_QUICKDRAW";
  case eAttachment::RANGEFINDER:
    return "ATTACHMENT_RANGEFINDER";
  case eAttachment::RECON:
    return "ATTACHMENT_RECON";
  case eAttachment::REDDOT:
    return "ATTACHMENT_REDDOT";
  case eAttachment::REFLEX:
    return "ATTACHMENT_REFLEX";
  case eAttachment::RF:
    return "ATTACHMENT_RF";
  case eAttachment::SELECTFIRE:
    return "ATTACHMENT_SELECTFIRE";
  case eAttachment::STACKFIRE:
    return "ATTACHMENT_STACKFIRE";
  case eAttachment::STALKER:
    return "ATTACHMENT_STALKER";
  case eAttachment::STEADYAIM:
    return "ATTACHMENT_STEADYAIM";
  case eAttachment::SUPPLY:
    return "ATTACHMENT_SUPPLY";
  case eAttachment::SUPPRESSED:
    return "ATTACHMENT_SUPPRESSED";
  case eAttachment::SWAYREDUC:
    return "ATTACHMENT_SWAYREDUC";
  case eAttachment::TACKNIFE:
    return "ATTACHMENT_TACKNIFE";
  case eAttachment::VZOOM:
    return "ATTACHMENT_VZOOM";
  default:
    return "ATTACHMENT_INVALID";
  }
}

PACKED(struct gadgetProperties_t {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"activate_mode\": {}, \"gadget_type\": {}, \"weapon_hold_mode\": "
        "{}, "
        "\"power_use_type\": {}, \"activate_with_primed_delay\": {}, "
        "\"movementMultipliers\": {}, "
        "\"shock_field_radius\": {}, \"shock_field_damage\": {}, "
        "\"camo_bread_crumb_duration\": {}, "
        "\"camo_invisibility_alert_time\": {}, "
        "\"camo_invisibility_flicker_extension_time\": {}, "
        "\"camo_invisibility_flicker_radius_extension\": {}, "
        "\"camo_invisibility_radius\": {}, "
        "\"camo_invisibility_takedown_response_radius\": {}, "
        "\"camo_invisibility_takedown_reveal_time\": {}, "
        "\"camo_takedown_power_gain\": {}, "
        "\"escort_drone_bullet_dmg_power_loss\": {}, "
        "\"escort_drone_burst_count_max\": {}, "
        "\"escort_drone_burst_count_min\": {}, "
        "\"escort_drone_burst_power_loss\": {}, "
        "\"escort_drone_burst_wait_time\": {}, "
        "\"escort_drone_exp_dmg_power_loss\": {}, \"escort_drone_hover_dist\": "
        "{}, "
        "\"escort_drone_launch_dist\": {}, "
        "\"escort_drone_misc_dmg_power_loss\": {}, "
        "\"escort_drone_target_acquire_time\": {}, "
        "\"escort_drone_tether_max_dist\": {}, "
        "\"escort_drone_tether_min_dist\": {}, \"flicker_on_damage\": {}, "
        "\"flicker_on_power_loss\": {}, "
        "\"flicker_on_power_low\": {}, \"flicker_on_whizby\": {}, "
        "\"heroVersion_2_0\": {}, "
        "\"multiRocket_acquisition_time\": {}, \"multiRocket_fire_interval\": "
        "{}, "
        "\"multiRocket_fire_power_loss\": {}, \"multirocket_target_number\": "
        "{}, "
        "\"multiRocket_target_radius\": {}, \"gadget_max_hitpoints\": {}, "
        "\"blurAmount\": {}, "
        "\"blurRadiusInner\": {}, \"blurRadiusOuter\": {}, \"blurOutScale\": "
        "{}, "
        "\"blurInTime\": {}, \"blurOutTime\": {}, \"blur_screen\": {}, "
        "\"changeLens\": {}, "
        "\"lensScaleInitial\": {}, \"lensScaleFinal\": {}, "
        "\"lensMinFocalLength\": {}, "
        "\"lensMaxFocalLength\": {}, \"lensTransTimeIn\": {}, "
        "\"lensTransTimeOut\": {}, "
        "\"power_shared\": {}, \"power_attack_loss\": {}, "
        "\"power_consume_on_ammo_use\": {}, "
        "\"power_damage_factor\": {}, \"power_flicker_chance\": {}, "
        "\"power_flicker_frequency\": {}, "
        "\"power_flicker_threshold\": {}, \"power_gain_on_retrieve\": {}, "
        "\"power_gain_score_factor\": {}, "
        "\"power_gain_score_ignore_self\": {}, "
        "\"power_gain_score_ignore_when_active\": {}, "
        "\"power_init_empty\": {}, \"power_init_amount\": {}, "
        "\"power_juke_loss\": {}, "
        "\"power_jump_loss\": {}, \"power_melee_loss\": {}, "
        "\"power_move_loss\": {}, "
        "\"power_move_loss_speed\": {}, \"power_on_damage_factor\": {}, "
        "\"power_power_bonus\": {}, "
        "\"power_recharge_delay\": {}, \"power_recharge_delay_max\": {}, "
        "\"power_recharge_rate\": {}, "
        "\"power_recharge_disable_on_emp\": {}, \"power_replenish_factor\": "
        "{}, "
        "\"power_reset_on_class_change\": {}, "
        "\"power_reset_on_death_if_active\": {}, "
        "\"power_reset_on_spawn\": {}, \"power_reset_on_team_change\": {}, "
        "\"power_reset_on_round_switch\": {}, "
        "\"power_round_end_active_penalty\": {}, "
        "\"power_sprint_loss\": {}, \"power_shut_off_on_death_penalty\": {}, "
        "\"power_shut_off_penalty\": {}, \"power_turn_off_penalty\": {}, "
        "\"power_usable_threshold\": {}, "
        "\"power_usage_rate\": {}, \"pulse_duration\": {}, \"pulse_margin\": "
        "{}, \"pulse_reveal_time\": {}, "
        "\"pulse_reveal_camo_time\": {}, \"pulse_reveal_time_viewModel\": {}, "
        "\"pulse_share_radius\": {}, "
        "\"pulse_enemy_share_type\": {}, \"pulse_share_type\": {}, "
        "\"pulse_type\": {}, "
        "\"pulse_max_range\": {}, \"shield_blast_protection_120\": {}, "
        "\"shield_blast_protection_180\": {}, "
        "\"shield_blast_protection_30\": {}, \"shield_blast_protection_60\": "
        "{}, "
        "\"shield_reflect_actor_accuracy_multiplier\": {}, "
        "\"shield_reflect_aim_assist_lerp\": {}, "
        "\"shield_reflect_damage_multiplier\": {}, "
        "\"shield_reflect_power_gain\": {}, "
        "\"shield_reflect_power_loss\": {}, \"speed_max_power_usage_rate\": "
        "{}, "
        "\"speed_max_power_speed_threshold\": {}, \"speed_sprint_out_scale\": "
        "{}, "
        "\"speed_wallrun_out_scale\": {}, \"cleanseRadius\": {}, "
        "\"turnoff_onEmpJammed\": {}, "
        "\"turnoff_onHeldKillstreak\": {}, \"turnoff_onAttack\": {}, "
        "\"can_activate_whenEmpJammed\": {}, "
        "\"turret_fire_power_loss\": {}, \"wielded_stayOn_onEmpJammed\": {}, "
        "\"wielded_stayOn_onHeldKillstreak\": {}, "
        "\"wielded_stayOn_onOffhandThrow\": {}, \"flashback_rewindTime\": {}, "
        "\"flashback_screenFlashShotFadeTime\": {}, "
        "\"flashback_screenFlashWhiteFadeTime\": {}, "
        "\"flashback_rewindDistance\": {}, \"fastFire\": {}, \"fastReload\": "
        "{}, \"fastSwitch\": {}, "
        "\"fastMelee\": {}, \"fastToss\": {}, \"fastEquipmentUse\": {}, "
        "\"fastADS\": {}, "
        "\"activatesCast\": {}, \"targetedCast\": {} }}",
        game::weapon::serialize(activate_mode),
        game::weapon::serialize(gadget_type),
        game::weapon::serialize(weapon_hold_mode),
        game::weapon::serialize(power_use_type), activate_with_primed_delay,
        ::serialize(movementMultipliers), shock_field_radius,
        shock_field_damage, camo_bread_crumb_duration,
        camo_invisibility_alert_time, camo_invisibility_flicker_extension_time,
        camo_invisibility_flicker_radius_extension, camo_invisibility_radius,
        camo_invisibility_takedown_response_radius,
        camo_invisibility_takedown_reveal_time, camo_takedown_power_gain,
        escort_drone_bullet_dmg_power_loss, escort_drone_burst_count_max,
        escort_drone_burst_count_min, escort_drone_burst_power_loss,
        escort_drone_burst_wait_time, escort_drone_exp_dmg_power_loss,
        escort_drone_hover_dist, escort_drone_launch_dist,
        escort_drone_misc_dmg_power_loss, escort_drone_target_acquire_time,
        escort_drone_tether_max_dist, escort_drone_tether_min_dist,
        flicker_on_damage, flicker_on_power_loss, flicker_on_power_low,
        flicker_on_whizby, heroVersion_2_0.serialize(),
        multiRocket_acquisition_time, multiRocket_fire_interval,
        multiRocket_fire_power_loss, multirocket_target_number,
        multiRocket_target_radius, gadget_max_hitpoints, blurAmount,
        blurRadiusInner, blurRadiusOuter, blurOutScale, blurInTime, blurOutTime,
        ::serialize(blur_screen), ::serialize(changeLens), lensScaleInitial,
        lensScaleFinal, lensMinFocalLength, lensMaxFocalLength, lensTransTimeIn,
        lensTransTimeOut, power_shared.serialize(), power_attack_loss,
        power_consume_on_ammo_use.serialize(), power_damage_factor,
        power_flicker_chance, power_flicker_frequency, power_flicker_threshold,
        power_gain_on_retrieve, power_gain_score_factor,
        ::serialize(power_gain_score_ignore_self),
        ::serialize(power_gain_score_ignore_when_active),
        ::serialize(power_init_empty), power_init_amount, power_juke_loss,
        power_jump_loss, power_melee_loss, power_move_loss,
        power_move_loss_speed, power_on_damage_factor, power_power_bonus,
        power_recharge_delay, power_recharge_delay_max, power_recharge_rate,
        power_recharge_disable_on_emp.serialize(), power_replenish_factor,
        ::serialize(power_reset_on_class_change),
        ::serialize(power_reset_on_death_if_active),
        ::serialize(power_reset_on_spawn),
        ::serialize(power_reset_on_team_change),
        power_reset_on_round_switch.serialize(), power_round_end_active_penalty,
        power_sprint_loss, power_shut_off_on_death_penalty,
        power_shut_off_penalty, power_turn_off_penalty, power_usable_threshold,
        power_usage_rate, pulse_duration, pulse_margin, pulse_reveal_time,
        pulse_reveal_camo_time, pulse_reveal_time_viewModel, pulse_share_radius,
        game::weapon::serialize(pulse_enemy_share_type),
        game::weapon::serialize(pulse_share_type),
        game::weapon::serialize(pulse_type), pulse_max_range,
        shield_blast_protection_120, shield_blast_protection_180,
        shield_blast_protection_30, shield_blast_protection_60,
        shield_reflect_actor_accuracy_multiplier,
        shield_reflect_aim_assist_lerp, shield_reflect_damage_multiplier,
        shield_reflect_power_gain, shield_reflect_power_loss,
        speed_max_power_usage_rate, speed_max_power_speed_threshold,
        speed_sprint_out_scale, speed_wallrun_out_scale, cleanseRadius,
        ::serialize(turnoff_onEmpJammed), ::serialize(turnoff_onHeldKillstreak),
        ::serialize(turnoff_onAttack), ::serialize(can_activate_whenEmpJammed),
        turret_fire_power_loss, ::serialize(wielded_stayOn_onEmpJammed),
        ::serialize(wielded_stayOn_onHeldKillstreak),
        ::serialize(wielded_stayOn_onOffhandThrow), flashback_rewindTime,
        flashback_screenFlashShotFadeTime, flashback_screenFlashWhiteFadeTime,
        flashback_rewindDistance, ::serialize(fastFire),
        ::serialize(fastReload), ::serialize(fastSwitch),
        ::serialize(fastMelee), ::serialize(fastToss),
        ::serialize(fastEquipmentUse), ::serialize(fastADS),
        ::serialize(activatesCast), targetedCast.serialize());
  }
});

struct WeaponAttachmentUniqueModelInfo {
  db::xasset::xmodel::XModelPtr model;
  db::xasset::xmodel::XModelPtr modelADS;
  XString tag;
  vec3_t offset;
  vec3_t rotation;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"model\": {}, \"modelADS\": {}, \"tag\": {}, "
                       "\"offset\": {}, \"rotation\": {} }}",
                       ::serialize(model), ::serialize(modelADS),
                       ::serialize(tag), offset.serialize(),
                       rotation.serialize());
  }
};

struct AttachmentCosmeticVariantModelInfo {
  db::xasset::xmodel::XModelPtr model;
  db::xasset::xmodel::XModelPtr modelADS;
  XString tag;
  vec3_t offset;
  vec3_t rotation;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"model\": {}, \"modelADS\": {}, \"tag\": {}, "
                       "\"offset\": {}, \"rotation\": {} }}",
                       ::serialize(model), ::serialize(modelADS),
                       ::serialize(tag), offset.serialize(),
                       rotation.serialize());
  }
};

struct AttachmentCosmeticVariantSet {
  AttachmentCosmeticVariantModelInfo viewModelInfo[2];
  AttachmentCosmeticVariantModelInfo worldModelInfo[2];
  const char *displayNameShort;
  const char *displayNameLong;
  const char *description;
  gfx::GfxImagePtr uiMaterial;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"viewModelInfo\": {}, \"worldModelInfo\": {}, "
        "\"displayNameShort\": {}, "
        "\"displayNameLong\": {}, \"description\": {}, \"uiMaterial\": {:p} }}",
        ::serialize(viewModelInfo), ::serialize(worldModelInfo),
        ::serialize(displayNameShort), ::serialize(displayNameLong),
        ::serialize(description), reinterpret_cast<const void *>(uiMaterial));
  }
};

PACKED(struct AttachmentCosmeticVariant {
  const char *name;
  AttachmentCosmeticVariantSet *variantSets;
  int32_t numVariantSets;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"variantSets\": {}, \"numVariantSets\": {} }}",
        ::serialize(name), ::serialize(variantSets, numVariantSets),
        numVariantSets);
  }
});
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"attachmentModelAssociation\": {}, "
        "\"attachmentCosmeticVariantOverride\": {:p}, "
        "\"viewModelInfo\": {}, \"worldModelInfo\": {}, \"displayNameShort\": "
        "{}, "
        "\"displayNameLong\": {}, \"description\": {}, \"uiMaterial\": {:p} }}",
        game::weapon::serialize(attachmentModelAssociation),
        reinterpret_cast<const void *>(attachmentCosmeticVariantOverride),
        ::serialize(viewModelInfo), ::serialize(worldModelInfo),
        ::serialize(displayNameShort), ::serialize(displayNameLong),
        ::serialize(description), reinterpret_cast<const void *>(uiMaterial));
  }
};

PACKED(struct CamoMaterialChannel {
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"replaceFlags\": {:04X}, \"camoMaterial\": {}, "
                       "\"translationX\": {}, "
                       "\"translationY\": {}, \"scaleX\": {}, \"scaleY\": {}, "
                       "\"rotation\": {}, "
                       "\"normalBlend\": {}, \"glossBlend\": {}, "
                       "\"albedoTint\": {}, \"detailMap\": {:p}, "
                       "\"detailHeight\": {}, \"detailScale\": {} }}",
                       replaceFlags, ::serialize(camoMaterial), translationX,
                       translationY, scaleX, scaleY, rotation, normalBlend,
                       glossBlend, albedoTint.serialize(),
                       reinterpret_cast<const void *>(detailMap), detailHeight,
                       detailScale.serialize());
  }
});

struct CamoBaseMaterial {
  db::xasset::MaterialDeferredHandle material;
  gfx::GfxImage *mask;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"material\": {}, \"mask\": {:p} }}",
                       ::serialize(material),
                       reinterpret_cast<const void *>(mask));
  }
};

PACKED(struct CamoMaterial {
  uint16_t numBaseMaterials;
  uint8_t _padding02[6];
  CamoBaseMaterial *baseMaterials;
  uint16_t activeChannels;
  uint8_t _padding12[6];
  CamoMaterialChannel camoMaterialChannels[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numBaseMaterials\": {}, \"baseMaterials\": {}, "
                       "\"activeChannels\": {:04X}, "
                       "\"camoMaterialChannels\": {} }}",
                       numBaseMaterials,
                       ::serialize(baseMaterials, numBaseMaterials),
                       activeChannels, ::serialize(camoMaterialChannels));
  }
});

PACKED(struct WeaponCamoMaterialSet {
  uint32_t numMaterials;
  uint8_t _padding04[4];
  CamoMaterial *materials;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numMaterials\": {}, \"materials\": {} }}",
                       numMaterials, ::serialize(materials, numMaterials));
  }
});

PACKED(struct WeaponCamo {
  const char *name;
  WeaponCamoMaterialSet *camoMaterials;
  uint32_t numCamoMaterials;
  uint8_t _padding014[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"camoMaterials\": {}, \"numCamoMaterials\": {} }}",
        ::serialize(name), ::serialize(camoMaterials, numCamoMaterials),
        numCamoMaterials);
  }
});

PACKED(struct WeaponAttachmentUnique {
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
  db::xasset::xmodel::XModelPtr *attachViewModel;
  db::xasset::xmodel::XModelPtr *attachWorldModel;
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"szInternalName\": {}, \"szDisplayName\": {}, \"szDisplayDesc\": "
        "{}, "
        "\"modHudIcon\": {}, \"attachmentImage\": {}, \"attachmentType\": {}, "
        "\"siblingLink\": {}, \"childLink\": {}, "
        "\"combinedAttachmentTypeMask\": {}, "
        "\"szAltWeaponName\": {}, \"altWeaponIndex\": {}, "
        "\"szDualWieldWeaponName\": {}, "
        "\"dualWieldWeaponIndex\": {}, \"hideTags\": {:p}, "
        "\"attachmentModelSets\": {}, "
        "\"attachViewModel\": {:p}, \"attachWorldModel\": {:p}, "
        "\"attachViewModelTag\": {:p}, "
        "\"attachWorldModelTag\": {:p}, \"attachViewModelOffsets\": {}, "
        "\"attachWorldModelOffsets\": {}, "
        "\"attachViewModelRotations\": {}, \"attachWorldModelRotations\": {}, "
        "\"weaponCamo\": {:p}, "
        "\"disableBaseWeaponAttachment\": {}, \"disableBaseWeaponClip\": {}, "
        "\"overrideBaseWeaponAttachmentOffsets\": {}, "
        "\"viewModelOffsetBaseAttachment\": {}, "
        "\"worldModelOffsetBaseAttachment\": {}, \"overlayMaterial\": {}, "
        "\"overlayMaterialLowRes\": {}, "
        "\"overlayReticle\": {}, \"iFirstRaiseTime\": {}, \"iAltRaiseTime\": "
        "{}, \"iAltDropTime\": {}, "
        "\"iAdsAltRaiseTime\": {}, \"iAdsAltDropTime\": {}, "
        "\"iReloadAmmoAdd\": {}, \"iReloadStartAdd\": {}, "
        "\"bSegmentedReload\": {}, \"szXAnims\": {:p}, \"animationOverrides\": "
        "{}, "
        "\"locationDamageMultipliers\": {:p}, \"damage\": {}, \"damageRange\": "
        "{}, \"soundOverrides\": {}, "
        "\"fireSound\": {}, \"fireSoundPlayer\": {}, \"fireBurstSound\": {}, "
        "\"fireBurstSoundPlayer\": {}, "
        "\"fireLoopSound\": {}, \"fireLoopSoundPlayer\": {}, "
        "\"fireLoopSoundLeft\": {}, "
        "\"fireLoopSoundPlayerLeft\": {}, \"fireLoopEndSound\": {}, "
        "\"fireLoopEndSoundPlayer\": {}, "
        "\"fireStartSound\": {}, \"fireStopSound\": {}, "
        "\"fireStartSoundPlayer\": {}, \"fireStopSoundPlayer\": {}, "
        "\"fireLastSound\": {}, \"fireLastSoundPlayer\": {}, "
        "\"fireKillcamSound\": {}, \"fireKillcamSoundPlayer\": {}, "
        "\"crackSound\": {}, \"whizbySound\": {}, \"emptyFireSound\": {}, "
        "\"emptyFireSoundPlayer\": {}, "
        "\"spinLoopSound\": {}, \"spinLoopSoundPlayer\": {}, "
        "\"startSpinSound\": {}, \"startSpinSoundPlayer\": {}, "
        "\"stopSpinSound\": {}, \"stopSpinSoundPlayer\": {}, "
        "\"rotateLoopSound\": {}, \"rotateLoopSoundPlayer\": {}, "
        "\"rotateStopSound\": {}, \"rotateStopSoundPlayer\": {}, "
        "\"effectOverrides\": {}, \"viewFlashEffect\": {}, "
        "\"worldFlashEffect\": {}, \"vViewFlashOffset\": {}, "
        "\"vWorldFlashOffset\": {}, \"tracerType\": {}, "
        "\"enemyTracerType\": {}, \"laserTypeViewModel\": {}, "
        "\"laserTypeWorld\": {}, \"adsDofStart\": {}, "
        "\"adsDofEnd\": {}, \"iAmmoIndex\": {}, \"iClipIndex\": {}, "
        "\"bOverrideLeftHandIK\": {}, "
        "\"bOverrideLeftHandProneIK\": {}, \"ikLeftHandOffset\": {}, "
        "\"ikLeftHandRotation\": {}, "
        "\"ikLeftHandProneOffset\": {}, \"ikLeftHandProneRotation\": {}, "
        "\"iCanDoQuickToss\": {}, "
        "\"stickiness\": {}, \"ammoCountEquipment\": {}, \"iStartAmmo\": {}, "
        "\"iMaxAmmo\": {}, "
        "\"iExplosionRadius\": {}, \"iExplosionRadiusMultiplier\": {}, "
        "\"fuseTime\": {}, "
        "\"proximityDetonation\": {}, \"proximityAlarmInnerRadius\": {}, "
        "\"proximityAlarmOuterRadius\": {}, "
        "\"proximityAlarmActivationDelay\": {}, \"chainEventRadius\": {}, "
        "\"chainEventTime\": {}, "
        "\"chainEventMax\": {}, \"multiDetonation\": {}, "
        "\"multiDetonationFragmentSpeed\": {}, "
        "\"curveballForce\": {}, \"curveballMaxRadius\": {}, "
        "\"lockOnMaxRange\": {}, "
        "\"lockOnMaxRangeNoLineOfSight\": {}, \"lockOnSpeed\": {}, "
        "\"bProjExplodeWhenStationary\": {}, "
        "\"gadgetProperty\": {}, \"customFloat0\": {}, \"customFloat1\": {}, "
        "\"customFloat2\": {}, "
        "\"customFloat3\": {}, \"customFloat4\": {}, \"customFloat5\": {}, "
        "\"customBool0\": {}, "
        "\"customBool1\": {}, \"customBool2\": {}, \"customBool3\": {}, "
        "\"customBool4\": {}, "
        "\"customBool5\": {} }}",
        ::serialize(szInternalName), ::serialize(szDisplayName),
        ::serialize(szDisplayDesc), ::serialize(modHudIcon),
        ::serialize(attachmentImage), game::weapon::serialize(attachmentType),
        siblingLink, childLink, ::serialize(combinedAttachmentTypeMask),
        ::serialize(szAltWeaponName), altWeaponIndex,
        ::serialize(szDualWieldWeaponName), dualWieldWeaponIndex,
        reinterpret_cast<const void *>(hideTags),
        ::serialize(attachmentModelSets),
        reinterpret_cast<const void *>(attachViewModel),
        reinterpret_cast<const void *>(attachWorldModel),
        reinterpret_cast<const void *>(attachViewModelTag),
        reinterpret_cast<const void *>(attachWorldModelTag),
        ::serialize(attachViewModelOffsets),
        ::serialize(attachWorldModelOffsets),
        ::serialize(attachViewModelRotations),
        ::serialize(attachWorldModelRotations),
        reinterpret_cast<const void *>(weaponCamo),
        ::serialize(disableBaseWeaponAttachment),
        ::serialize(disableBaseWeaponClip),
        ::serialize(overrideBaseWeaponAttachmentOffsets),
        viewModelOffsetBaseAttachment.serialize(),
        worldModelOffsetBaseAttachment.serialize(),
        ::serialize(overlayMaterial), ::serialize(overlayMaterialLowRes),
        game::weapon::serialize(overlayReticle), iFirstRaiseTime, iAltRaiseTime,
        iAltDropTime, iAdsAltRaiseTime, iAdsAltDropTime, iReloadAmmoAdd,
        iReloadStartAdd, ::serialize(bSegmentedReload),
        reinterpret_cast<const void *>(szXAnims),
        ::serialize(animationOverrides),
        reinterpret_cast<const void *>(locationDamageMultipliers),
        ::serialize(damage), ::serialize(damageRange), soundOverrides,
        ::serialize(fireSound), ::serialize(fireSoundPlayer),
        ::serialize(fireBurstSound), ::serialize(fireBurstSoundPlayer),
        ::serialize(fireLoopSound), ::serialize(fireLoopSoundPlayer),
        ::serialize(fireLoopSoundLeft), ::serialize(fireLoopSoundPlayerLeft),
        ::serialize(fireLoopEndSound), ::serialize(fireLoopEndSoundPlayer),
        ::serialize(fireStartSound), ::serialize(fireStopSound),
        ::serialize(fireStartSoundPlayer), ::serialize(fireStopSoundPlayer),
        ::serialize(fireLastSound), ::serialize(fireLastSoundPlayer),
        ::serialize(fireKillcamSound), ::serialize(fireKillcamSoundPlayer),
        ::serialize(crackSound), ::serialize(whizbySound),
        ::serialize(emptyFireSound), ::serialize(emptyFireSoundPlayer),
        ::serialize(spinLoopSound), ::serialize(spinLoopSoundPlayer),
        ::serialize(startSpinSound), ::serialize(startSpinSoundPlayer),
        ::serialize(stopSpinSound), ::serialize(stopSpinSoundPlayer),
        ::serialize(rotateLoopSound), ::serialize(rotateLoopSoundPlayer),
        ::serialize(rotateStopSound), ::serialize(rotateStopSoundPlayer),
        effectOverrides, ::serialize(viewFlashEffect),
        ::serialize(worldFlashEffect), vViewFlashOffset.serialize(),
        vWorldFlashOffset.serialize(), ::serialize(tracerType),
        ::serialize(enemyTracerType), ::serialize(laserTypeViewModel),
        ::serialize(laserTypeWorld), adsDofStart, adsDofEnd, iAmmoIndex,
        iClipIndex, ::serialize(bOverrideLeftHandIK),
        ::serialize(bOverrideLeftHandProneIK), ikLeftHandOffset.serialize(),
        ikLeftHandRotation.serialize(), ikLeftHandProneOffset.serialize(),
        ikLeftHandProneRotation.serialize(), iCanDoQuickToss,
        game::weapon::serialize(stickiness), ammoCountEquipment, iStartAmmo,
        iMaxAmmo, iExplosionRadius, iExplosionRadiusMultiplier, fuseTime,
        proximityDetonation, proximityAlarmInnerRadius,
        proximityAlarmOuterRadius, proximityAlarmActivationDelay,
        chainEventRadius, chainEventTime, chainEventMax, multiDetonation,
        multiDetonationFragmentSpeed, curveballForce, curveballMaxRadius,
        lockOnMaxRange, lockOnMaxRangeNoLineOfSight, lockOnSpeed,
        bProjExplodeWhenStationary, gadgetProperty.serialize(), customFloat0,
        customFloat1, customFloat2, customFloat3, customFloat4, customFloat5,
        ::serialize(customBool0), ::serialize(customBool1),
        ::serialize(customBool2), ::serialize(customBool3),
        ::serialize(customBool4), ::serialize(customBool5));
  }
});
typedef WeaponAttachmentUnique *WeaponAttachmentUniquePtr;

enum class WeaponDobjInfoSlot : int32_t {
  HELD = 0x0,
  STOWED = 0x1,
  MELEE = 0x2,
  OFFHAND = 0x4,
};

inline constexpr const char *serialize(const WeaponDobjInfoSlot slot) {
  switch (slot) {
  case WeaponDobjInfoSlot::HELD:
    return "SLOT_HELD";
  case WeaponDobjInfoSlot::STOWED:
    return "SLOT_STOWED";
  case WeaponDobjInfoSlot::MELEE:
    return "SLOT_MELEE";
  case WeaponDobjInfoSlot::OFFHAND:
    return "SLOT_OFFHAND";
  default:
    return "SLOT_INVALID";
  }
}

struct ExplosionDamage {
  int32_t radius;
  int32_t height;
  int32_t damage;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"radius\": {}, \"height\": {}, \"damage\": {} }}",
                       radius, height, damage);
  }
};

enum class weapLockType_t : int32_t {
  NONE = 0x0,
  TARGETED = 0x1,
  SINGLEAP = 0x2,
  MULTIAP = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const weapLockType_t type) {
  switch (type) {
  case weapLockType_t::NONE:
    return "LOCKTYPE_NONE";
  case weapLockType_t::TARGETED:
    return "LOCKTYPE_TARGETED";
  case weapLockType_t::SINGLEAP:
    return "LOCKTYPE_SINGLEAP";
  case weapLockType_t::MULTIAP:
    return "LOCKTYPE_MULTIAP";
  default:
    return "LOCKTYPE_INVALID";
  }
}

enum class WeapStickOrientation : int32_t {
  NONE = 0x0,
  FLAT = 0x1,
  FLAT_MAINTAIN_DIRECTION = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const WeapStickOrientation orient) {
  switch (orient) {
  case WeapStickOrientation::NONE:
    return "STICKORIENT_NONE";
  case WeapStickOrientation::FLAT:
    return "STICKORIENT_FLAT";
  case WeapStickOrientation::FLAT_MAINTAIN_DIRECTION:
    return "STICKORIENT_FLAT_MAINTAIN_DIRECTION";
  default:
    return "STICKORIENT_INVALID";
  }
}

struct RumbleInfo;
typedef RumbleInfo *RumbleInfoPtr;

enum class DamageType : int32_t {
  NORMAL = 0x0,
  NORMAL_SHOTGUN = 0x1,
  BURNED = 0x2,
  MELEE = 0x3,
  MELEE_ASSASSINATE = 0x4,
  MELEE_CHARGE = 0x5,
  MELEE_BASH = 0x6,
  MELEE_ARMBLADE = 0x7,
  EXPLOSIVE = 0x8,
  SUICIDE = 0x9,
  FALLING = 0xA,
  BLOWBACK = 0xB,
  ELECTRIFIED = 0xC,
  ANNIHILATOR = 0xD,
  FIRST_USER_SPECIFIED = 0xD,
  FIREFLIES = 0xE,
  BOW_PARTIAL_CHARGE = 0xF,
  BOW_FULL_CHARGE = 0x10,
  MELEE_BAT = 0x11,
  ENERGY_WEAPON = 0x12,
  COUNT = 0x13,
};

inline constexpr const char *serialize(const DamageType dtype) {
  switch (dtype) {
  case DamageType::NORMAL:
    return "DAMAGETYPE_NORMAL";
  case DamageType::NORMAL_SHOTGUN:
    return "DAMAGETYPE_NORMAL_SHOTGUN";
  case DamageType::BURNED:
    return "DAMAGETYPE_BURNED";
  case DamageType::MELEE:
    return "DAMAGETYPE_MELEE";
  case DamageType::MELEE_ASSASSINATE:
    return "DAMAGETYPE_MELEE_ASSASSINATE";
  case DamageType::MELEE_CHARGE:
    return "DAMAGETYPE_MELEE_CHARGE";
  case DamageType::MELEE_BASH:
    return "DAMAGETYPE_MELEE_BASH";
  case DamageType::MELEE_ARMBLADE:
    return "DAMAGETYPE_MELEE_ARMBLADE";
  case DamageType::EXPLOSIVE:
    return "DAMAGETYPE_EXPLOSIVE";
  case DamageType::SUICIDE:
    return "DAMAGETYPE_SUICIDE";
  case DamageType::FALLING:
    return "DAMAGETYPE_FALLING";
  case DamageType::BLOWBACK:
    return "DAMAGETYPE_BLOWBACK";
  case DamageType::ELECTRIFIED:
    return "DAMAGETYPE_ELECTRIFIED";
  case DamageType::ANNIHILATOR:
    return "DAMAGETYPE_ANNIHILATOR";
  case DamageType::FIREFLIES:
    return "DAMAGETYPE_FIREFLIES";
  case DamageType::BOW_PARTIAL_CHARGE:
    return "DAMAGETYPE_BOW_PARTIAL_CHARGE";
  case DamageType::BOW_FULL_CHARGE:
    return "DAMAGETYPE_BOW_FULL_CHARGE";
  case DamageType::MELEE_BAT:
    return "DAMAGETYPE_MELEE_BAT";
  case DamageType::ENERGY_WEAPON:
    return "DAMAGETYPE_ENERGY_WEAPON";
  default:
    return "DAMAGETYPE_INVALID";
  }
}

enum class weapClass_t : int32_t {
  RIFLE = 0x0,
  MG = 0x1,
  SMG = 0x2,
  SPREAD = 0x3,
  PISTOL = 0x4,
  GRENADE = 0x5,
  ROCKETLAUNCHER = 0x6,
  TURRET = 0x7,
  NON_PLAYER = 0x8,
  GAS = 0x9,
  ITEM = 0xA,
  MELEE = 0xB,
  KILLSTREAK_ALT_STORED_WEAPON = 0xC,
  PISTOL_SPREAD = 0xD,
  BALL = 0xE,
  COUNT = 0xF,
};

inline constexpr const char *serialize(const weapClass_t wclass) {
  switch (wclass) {
  case weapClass_t::RIFLE:
    return "WEAPCLASS_RIFLE";
  case weapClass_t::MG:
    return "WEAPCLASS_MG";
  case weapClass_t::SMG:
    return "WEAPCLASS_SMG";
  case weapClass_t::SPREAD:
    return "WEAPCLASS_SPREAD";
  case weapClass_t::PISTOL:
    return "WEAPCLASS_PISTOL";
  case weapClass_t::GRENADE:
    return "WEAPCLASS_GRENADE";
  case weapClass_t::ROCKETLAUNCHER:
    return "WEAPCLASS_ROCKETLAUNCHER";
  case weapClass_t::TURRET:
    return "WEAPCLASS_TURRET";
  case weapClass_t::NON_PLAYER:
    return "WEAPCLASS_NON_PLAYER";
  case weapClass_t::GAS:
    return "WEAPCLASS_GAS";
  case weapClass_t::ITEM:
    return "WEAPCLASS_ITEM";
  case weapClass_t::MELEE:
    return "WEAPCLASS_MELEE";
  case weapClass_t::KILLSTREAK_ALT_STORED_WEAPON:
    return "WEAPCLASS_KILLSTREAK_ALT_STORED_WEAPON";
  case weapClass_t::PISTOL_SPREAD:
    return "WEAPCLASS_PISTOL_SPREAD";
  case weapClass_t::BALL:
    return "WEAPCLASS_BALL";
  default:
    return "WEAPCLASS_INVALID";
  }
}

enum class weapType_t : int32_t {
  BULLET = 0x0,
  GRENADE = 0x1,
  PROJECTILE = 0x2,
  BINOCULARS = 0x3,
  GAS = 0x4,
  BOMB = 0x5,
  MINE = 0x6,
  MELEE = 0x7,
  RIOTSHIELD = 0x8,
  COUNT = 0x9,
};

inline constexpr const char *serialize(const weapType_t wtype) {
  switch (wtype) {
  case weapType_t::BULLET:
    return "WEAPTYPE_BULLET";
  case weapType_t::GRENADE:
    return "WEAPTYPE_GRENADE";
  case weapType_t::PROJECTILE:
    return "WEAPTYPE_PROJECTILE";
  case weapType_t::BINOCULARS:
    return "WEAPTYPE_BINOCULARS";
  case weapType_t::GAS:
    return "WEAPTYPE_GAS";
  case weapType_t::BOMB:
    return "WEAPTYPE_BOMB";
  case weapType_t::MINE:
    return "WEAPTYPE_MINE";
  case weapType_t::MELEE:
    return "WEAPTYPE_MELEE";
  case weapType_t::RIOTSHIELD:
    return "WEAPTYPE_RIOTSHIELD";
  default:
    return "WEAPTYPE_INVALID";
  }
}

enum class PenetrateType : int32_t {
  NONE = 0x0,
  SMALL = 0x1,
  MEDIUM = 0x2,
  LARGE = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const PenetrateType ptype) {
  switch (ptype) {
  case PenetrateType::NONE:
    return "PENTYPE_NONE";
  case PenetrateType::SMALL:
    return "PENTYPE_SMALL";
  case PenetrateType::MEDIUM:
    return "PENTYPE_MEDIUM";
  case PenetrateType::LARGE:
    return "PENTYPE_LARGE";
  default:
    return "PENTYPE_INVALID";
  }
}

enum class ImpactType : int32_t {
  NONE = 0x0,
  BULLET_SMALL = 0x1,
  BULLET_LARGE = 0x2,
  BULLET_AP = 0x3,
  BULLET_XTREME = 0x4,
  SHOTGUN = 0x5,
  GRENADE_BOUNCE = 0x6,
  GRENADE_EXPLODE = 0x7,
  RIFLE_GRENADE = 0x8,
  ROCKET_EXPLODE = 0x9,
  ROCKET_EXPLODE_XTREME = 0xA,
  PROJECTILE_DUD = 0xB,
  MORTAR_SHELL = 0xC,
  TANK_SHELL = 0xD,
  BOLT = 0xE,
  BLADE = 0xF,
  COUNT = 0x10,
};

inline constexpr const char *serialize(const ImpactType itype) {
  switch (itype) {
  case ImpactType::NONE:
    return "IMPACT_NONE";
  case ImpactType::BULLET_SMALL:
    return "IMPACT_BULLET_SMALL";
  case ImpactType::BULLET_LARGE:
    return "IMPACT_BULLET_LARGE";
  case ImpactType::BULLET_AP:
    return "IMPACT_BULLET_AP";
  case ImpactType::BULLET_XTREME:
    return "IMPACT_BULLET_XTREME";
  case ImpactType::SHOTGUN:
    return "IMPACT_SHOTGUN";
  case ImpactType::GRENADE_BOUNCE:
    return "IMPACT_GRENADE_BOUNCE";
  case ImpactType::GRENADE_EXPLODE:
    return "IMPACT_GRENADE_EXPLODE";
  case ImpactType::RIFLE_GRENADE:
    return "IMPACT_RIFLE_GRENADE";
  case ImpactType::ROCKET_EXPLODE:
    return "IMPACT_ROCKET_EXPLODE";
  case ImpactType::ROCKET_EXPLODE_XTREME:
    return "IMPACT_ROCKET_EXPLODE_XTREME";
  case ImpactType::PROJECTILE_DUD:
    return "IMPACT_PROJECTILE_DUD";
  case ImpactType::MORTAR_SHELL:
    return "IMPACT_MORTAR_SHELL";
  case ImpactType::TANK_SHELL:
    return "IMPACT_TANK_SHELL";
  case ImpactType::BOLT:
    return "IMPACT_BOLT";
  case ImpactType::BLADE:
    return "IMPACT_BLADE";
  default:
    return "IMPACT_INVALID";
  }
}

enum class weapInventoryType_t : int32_t {
  PRIMARY = 0x0,
  OFFHAND = 0x1,
  ITEM = 0x2,
  ALTMODE = 0x3,
  MELEE = 0x4,
  DWLEFTHAND = 0x5,
  ABILITY = 0x6,
  HEAVY = 0x7,
  COUNT = 0x8,
};

inline constexpr const char *serialize(const weapInventoryType_t inv) {
  switch (inv) {
  case weapInventoryType_t::PRIMARY:
    return "INV_PRIMARY";
  case weapInventoryType_t::OFFHAND:
    return "INV_OFFHAND";
  case weapInventoryType_t::ITEM:
    return "INV_ITEM";
  case weapInventoryType_t::ALTMODE:
    return "INV_ALTMODE";
  case weapInventoryType_t::MELEE:
    return "INV_MELEE";
  case weapInventoryType_t::DWLEFTHAND:
    return "INV_DWLEFTHAND";
  case weapInventoryType_t::ABILITY:
    return "INV_ABILITY";
  case weapInventoryType_t::HEAVY:
    return "INV_HEAVY";
  default:
    return "INV_INVALID";
  }
}

enum class weapFireType_t : int32_t {
  FULLAUTO = 0x0,
  SINGLESHOT = 0x1,
  BURSTFIRE = 0x2,
  AUTOBURSTFIRE = 0x3,
  STACKED = 0x4,
  ALLLOCKED = 0x5,
  MINIGUN = 0x6,
  CHARGESHOT = 0x7,
  CHARGESHOTBURST = 0x8,
  CHARGESHOTAUTOBURST = 0x9,
  JETGUN = 0xA,
  MELEE = 0xB,
  AIMONLY = 0xC,
  COUNT = 0xD,
};

inline constexpr const char *serialize(const weapFireType_t ftype) {
  switch (ftype) {
  case weapFireType_t::FULLAUTO:
    return "FIRETYPE_FULLAUTO";
  case weapFireType_t::SINGLESHOT:
    return "FIRETYPE_SINGLESHOT";
  case weapFireType_t::BURSTFIRE:
    return "FIRETYPE_BURSTFIRE";
  case weapFireType_t::AUTOBURSTFIRE:
    return "FIRETYPE_AUTOBURSTFIRE";
  case weapFireType_t::STACKED:
    return "FIRETYPE_STACKED";
  case weapFireType_t::ALLLOCKED:
    return "FIRETYPE_ALLLOCKED";
  case weapFireType_t::MINIGUN:
    return "FIRETYPE_MINIGUN";
  case weapFireType_t::CHARGESHOT:
    return "FIRETYPE_CHARGESHOT";
  case weapFireType_t::CHARGESHOTBURST:
    return "FIRETYPE_CHARGESHOTBURST";
  case weapFireType_t::CHARGESHOTAUTOBURST:
    return "FIRETYPE_CHARGESHOTAUTOBURST";
  case weapFireType_t::JETGUN:
    return "FIRETYPE_JETGUN";
  case weapFireType_t::MELEE:
    return "FIRETYPE_MELEE";
  case weapFireType_t::AIMONLY:
    return "FIRETYPE_AIMONLY";
  default:
    return "FIRETYPE_INVALID";
  }
}

enum class weapClipType_t : int32_t {
  BOTTOM = 0x0,
  TOP = 0x1,
  LEFT = 0x2,
  DP28 = 0x3,
  PTRS = 0x4,
  LMG = 0x5,
  COUNT = 0x6,
};

enum class barrelType_t : int32_t {
  SINGLE = 0x0,
  DUAL = 0x1,
  DUAL_ALTERNATE = 0x2,
  QUAD = 0x3,
  QUAD_ALTERNATE = 0x4,
  QUAD_DOUBLE_ALTERNATE = 0x5,
  OFFSET_DUAL = 0x6,
  OFFSET_DUAL_ALTERNATE = 0x7,
  COUNT = 0x8,
};

inline constexpr const char *serialize(const barrelType_t btype) {
  switch (btype) {
  case barrelType_t::SINGLE:
    return "BARREL_SINGLE";
  case barrelType_t::DUAL:
    return "BARREL_DUAL";
  case barrelType_t::DUAL_ALTERNATE:
    return "BARREL_DUAL_ALTERNATE";
  case barrelType_t::QUAD:
    return "BARREL_QUAD";
  case barrelType_t::QUAD_ALTERNATE:
    return "BARREL_QUAD_ALTERNATE";
  case barrelType_t::QUAD_DOUBLE_ALTERNATE:
    return "BARREL_QUAD_DOUBLE_ALTERNATE";
  case barrelType_t::OFFSET_DUAL:
    return "BARREL_OFFSET_DUAL";
  case barrelType_t::OFFSET_DUAL_ALTERNATE:
    return "BARREL_OFFSET_DUAL_ALTERNATE";
  default:
    return "BARREL_INVALID";
  }
}

enum class OffhandClass : int32_t {
  NONE = 0x0,
  FRAG_GRENADE = 0x1,
  SMOKE_GRENADE = 0x2,
  FLASH_GRENADE = 0x3,
  GEAR = 0x4,
  SUPPLYDROP_MARKER = 0x5,
  GADGET = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const OffhandClass oclass) {
  switch (oclass) {
  case OffhandClass::NONE:
    return "OFFHANDCLASS_NONE";
  case OffhandClass::FRAG_GRENADE:
    return "OFFHANDCLASS_FRAG_GRENADE";
  case OffhandClass::SMOKE_GRENADE:
    return "OFFHANDCLASS_SMOKE_GRENADE";
  case OffhandClass::FLASH_GRENADE:
    return "OFFHANDCLASS_FLASH_GRENADE";
  case OffhandClass::GEAR:
    return "OFFHANDCLASS_GEAR";
  case OffhandClass::SUPPLYDROP_MARKER:
    return "OFFHANDCLASS_SUPPLYDROP_MARKER";
  case OffhandClass::GADGET:
    return "OFFHANDCLASS_GADGET";
  default:
    return "OFFHANDCLASS_INVALID";
  }
}

enum class OffhandSlot : int32_t {
  NONE = 0x0,
  LETHAL_GRENADE = 0x1,
  TACTICAL_GRENADE = 0x2,
  EQUIPMENT = 0x3,
  SPECIFIC_USE = 0x4,
  GADGET = 0x5,
  SPECIAL = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const OffhandSlot oslot) {
  switch (oslot) {
  case OffhandSlot::NONE:
    return "OFFHANDSLOT_NONE";
  case OffhandSlot::LETHAL_GRENADE:
    return "OFFHANDSLOT_LETHAL_GRENADE";
  case OffhandSlot::TACTICAL_GRENADE:
    return "OFFHANDSLOT_TACTICAL_GRENADE";
  case OffhandSlot::EQUIPMENT:
    return "OFFHANDSLOT_EQUIPMENT";
  case OffhandSlot::SPECIFIC_USE:
    return "OFFHANDSLOT_SPECIFIC_USE";
  case OffhandSlot::GADGET:
    return "OFFHANDSLOT_GADGET";
  case OffhandSlot::SPECIAL:
    return "OFFHANDSLOT_SPECIAL";
  default:
    return "OFFHANDSLOT_INVALID";
  }
}

enum class weapStance_t : int32_t {
  STAND = 0x0,
  DUCK = 0x1,
  PRONE = 0x2,
  NUM = 0x3,
};

inline constexpr const char *serialize(const weapStance_t stance) {
  switch (stance) {
  case weapStance_t::STAND:
    return "STANCE_STAND";
  case weapStance_t::DUCK:
    return "STANCE_DUCK";
  case weapStance_t::PRONE:
    return "STANCE_PRONE";
  default:
    return "STANCE_INVALID";
  }
}

enum class activeReticleType_t : int32_t {
  NONE = 0x0,
  PIP_ON_A_STICK = 0x1,
  BOUNCING_DIAMOND = 0x2,
  MISSILE_LOCK = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const activeReticleType_t art) {
  switch (art) {
  case activeReticleType_t::NONE:
    return "ACTIVERETICLE_NONE";
  case activeReticleType_t::PIP_ON_A_STICK:
    return "ACTIVERETICLE_PIP_ON_A_STICK";
  case activeReticleType_t::BOUNCING_DIAMOND:
    return "ACTIVERETICLE_BOUNCING_DIAMOND";
  case activeReticleType_t::MISSILE_LOCK:
    return "ACTIVERETICLE_MISSILE_LOCK";
  default:
    return "ACTIVERETICLE_INVALID";
  }
}

enum class weaponIconRatioType_t : int32_t {
  WEAPON_ICON_RATIO_1TO1 = 0x0,
  WEAPON_ICON_RATIO_2TO1 = 0x1,
  WEAPON_ICON_RATIO_4TO1 = 0x2,
  WEAPON_ICON_RATIO_COUNT = 0x3,
};

inline constexpr const char *serialize(const weaponIconRatioType_t ratio) {
  switch (ratio) {
  case weaponIconRatioType_t::WEAPON_ICON_RATIO_1TO1:
    return "RATIO_1TO1";
  case weaponIconRatioType_t::WEAPON_ICON_RATIO_2TO1:
    return "RATIO_2TO1";
  case weaponIconRatioType_t::WEAPON_ICON_RATIO_4TO1:
    return "RATIO_4TO1";
  default:
    return "RATIO_INVALID";
  }
}

enum class ammoCounterClipType_t : int32_t {
  NONE = 0x0,
  MAGAZINE = 0x1,
  SHORTMAGAZINE = 0x2,
  SHOTGUN = 0x3,
  ROCKET = 0x4,
  BELTFED = 0x5,
  ALTWEAPON = 0x6,
  COUNT = 0x7,
};

inline constexpr const char *serialize(const ammoCounterClipType_t act) {
  switch (act) {
  case ammoCounterClipType_t::NONE:
    return "AMMOCOUNTER_NONE";
  case ammoCounterClipType_t::MAGAZINE:
    return "AMMOCOUNTER_MAGAZINE";
  case ammoCounterClipType_t::SHORTMAGAZINE:
    return "AMMOCOUNTER_SHORTMAGAZINE";
  case ammoCounterClipType_t::SHOTGUN:
    return "AMMOCOUNTER_SHOTGUN";
  case ammoCounterClipType_t::ROCKET:
    return "AMMOCOUNTER_ROCKET";
  case ammoCounterClipType_t::BELTFED:
    return "AMMOCOUNTER_BELTFED";
  case ammoCounterClipType_t::ALTWEAPON:
    return "AMMOCOUNTER_ALTWEAPON";
  default:
    return "AMMOCOUNTER_INVALID";
  }
}

enum class WeapRotateType : int32_t {
  GRENADE = 0x0,
  BLADE = 0x1,
  CYLINDER = 0x2,
  ANIMATION = 0x3,
  COUNT = 0x4,
};

inline constexpr const char *serialize(const WeapRotateType rtype) {
  switch (rtype) {
  case WeapRotateType::GRENADE:
    return "ROTATETYPE_GRENADE";
  case WeapRotateType::BLADE:
    return "ROTATETYPE_BLADE";
  case WeapRotateType::CYLINDER:
    return "ROTATETYPE_CYLINDER";
  case WeapRotateType::ANIMATION:
    return "ROTATETYPE_ANIMATION";
  default:
    return "ROTATETYPE_INVALID";
  }
}

enum class weapProjExposion_t : int32_t {
  NONE = 0x0,
  GRENADE = 0x1,
  ROCKET = 0x2,
  FLASHBANG = 0x3,
  DUD = 0x4,
  SMOKE = 0x5,
  HEAVY = 0x6,
  FIRE = 0x7,
  NAPALMBLOB = 0x8,
  BOLT = 0x9,
  SHRAPNELSPAN = 0xA,
  COUNT = 0xB,
};

inline constexpr const char *serialize(const weapProjExposion_t exp) {
  switch (exp) {
  case weapProjExposion_t::NONE:
    return "PROJEXP_NONE";
  case weapProjExposion_t::GRENADE:
    return "PROJEXP_GRENADE";
  case weapProjExposion_t::ROCKET:
    return "PROJEXP_ROCKET";
  case weapProjExposion_t::FLASHBANG:
    return "PROJEXP_FLASHBANG";
  case weapProjExposion_t::DUD:
    return "PROJEXP_DUD";
  case weapProjExposion_t::SMOKE:
    return "PROJEXP_SMOKE";
  case weapProjExposion_t::HEAVY:
    return "PROJEXP_HEAVY";
  case weapProjExposion_t::FIRE:
    return "PROJEXP_FIRE";
  case weapProjExposion_t::NAPALMBLOB:
    return "PROJEXP_NAPALMBLOB";
  case weapProjExposion_t::BOLT:
    return "PROJEXP_BOLT";
  case weapProjExposion_t::SHRAPNELSPAN:
    return "PROJEXP_SHRAPNELSPAN";
  default:
    return "PROJEXP_INVALID";
  }
}

enum class eAttachmentPoint : int32_t {
  NONE = 0x0,
  TOP = 0x1,
  FIRST = 0x1,
  BOTTOM = 0x2,
  TRIGGER = 0x3,
  MUZZLE = 0x4,
  GUNPERK = 0x5,
  RIG = 0x6,
  COUNT = 0x7,
  INVALID = 0x7,
};

inline constexpr const char *serialize(const eAttachmentPoint pt) {
  switch (pt) {
  case eAttachmentPoint::NONE:
    return "ATTACHMENTPOINT_NONE";
  case eAttachmentPoint::TOP:
    return "ATTACHMENTPOINT_TOP";
  case eAttachmentPoint::BOTTOM:
    return "ATTACHMENTPOINT_BOTTOM";
  case eAttachmentPoint::TRIGGER:
    return "ATTACHMENTPOINT_TRIGGER";
  case eAttachmentPoint::MUZZLE:
    return "ATTACHMENTPOINT_MUZZLE";
  case eAttachmentPoint::GUNPERK:
    return "ATTACHMENTPOINT_GUNPERK";
  case eAttachmentPoint::RIG:
    return "ATTACHMENTPOINT_RIG";
  case eAttachmentPoint::INVALID:
    return "ATTACHMENTPOINT_INVALID";
  default:
    return "ATTACHMENTPOINT_INVALID";
  }
}

enum class WeapOverlayInteface_t : int32_t {
  NONE = 0x0,
  JAVELIN = 0x1,
  TURRETSCOPE = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const WeapOverlayInteface_t iface) {
  switch (iface) {
  case WeapOverlayInteface_t::NONE:
    return "OVERLAYINT_NONE";
  case WeapOverlayInteface_t::JAVELIN:
    return "OVERLAYINT_JAVELIN";
  case WeapOverlayInteface_t::TURRETSCOPE:
    return "OVERLAYINT_TURRETSCOPE";
  default:
    return "OVERLAYINT_INVALID";
  }
}

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"szInternalName\": {}, \"szDisplayName\": {}, \"attachmentType\": "
        "{}, "
        "\"attachmentPoint\": {}, \"penetrateType\": {}, \"fireType\": {}, "
        "\"firstRaisePriority\": {}, \"fHipIdleAmount\": {}, "
        "\"bAltWeaponAdsOnly\": {}, "
        "\"bAltWeaponDisableSwitching\": {}, \"altScopeADSTransInTime\": {}, "
        "\"altScopeADSTransOutTime\": {}, \"bSilenced\": {}, \"bDualMag\": {}, "
        "\"laserSight\": {}, \"bInfraRed\": {}, \"bUseAsMelee\": {}, "
        "\"bDualWield\": {}, "
        "\"sharedAmmo\": {}, \"fMasterDamageRangeScale\": {}, \"damageScale\": "
        "{}, "
        "\"damageRangeScale\": {}, \"multishotBaseDamageScale\": {}, "
        "\"multishotBaseDamageRangeScale\": {}, "
        "\"aimAssistRangeScale\": {}, \"aimAssistRangeAdsScale\": {}, "
        "\"fAdsFlinchScalar\": {}, "
        "\"fAdsFiringFlinchScalar\": {}, \"fAdsTurnRateScalar\": {}, "
        "\"fAdsWallRunBobScalar\": {}, "
        "\"fAdsAdditiveFallScalar\": {}, \"fAdsAdditiveJumpScalar\": {}, "
        "\"fAdsAdditiveJumpLandScalar\": {}, "
        "\"fAdsZoom1_focalLength\": {}, \"fAdsZoom1_fStop\": {}, "
        "\"fAdsZoom2_focalLength\": {}, "
        "\"fAdsZoom2_fStop\": {}, \"fAdsZoom3_focalLength\": {}, "
        "\"fAdsZoom3_fStop\": {}, "
        "\"fAdsZoomFov1\": {}, \"fAdsZoomFov2\": {}, \"fAdsZoomFov3\": {}, "
        "\"fAdsZoomInFrac\": {}, "
        "\"fAdsZoomOutFrac\": {}, \"fAdsTransInTimeScale\": {}, "
        "\"fAdsTransOutTimeScale\": {}, "
        "\"fAdsRecoilReductionRate\": {}, \"fAdsRecoilReductionLimit\": {}, "
        "\"fAdsViewKickCenterSpeedScale\": {}, "
        "\"fAdsIdleAmountScale\": {}, \"idleSpeedFromFireTransitionTime\": {}, "
        "\"fAdsScopeBlurAmount\": {}, "
        "\"fAdsScopeBlurStart\": {}, \"adsSwayViewInsteadOfGun\": {}, "
        "\"fkickAlignedInputScalar\": {}, "
        "\"fkickOpposedInputScalar\": {}, \"swayOverride\": {}, "
        "\"swayMaxAngle\": {}, "
        "\"swayLerpSpeed\": {}, \"swayPitchScale\": {}, \"swayYawScale\": {}, "
        "\"swayHorizScale\": {}, "
        "\"swayVertScale\": {}, \"adsSwayOverride\": {}, \"adsSwayMaxAngle\": "
        "{}, \"adsSwayLerpSpeed\": {}, "
        "\"adsSwayTransitionLerpSpeed\": {}, \"adsSwayPitchScale\": {}, "
        "\"adsSwayYawScale\": {}, "
        "\"fAdsSwayHorizScale\": {}, \"fAdsSwayVertScale\": {}, "
        "\"adsMoveSpeedScale\": {}, "
        "\"adsFiringSpeedScale\": {}, \"adsMoveSpeedTransitionTimeIn\": {}, "
        "\"adsMoveSpeedTransitionTimeOut\": {}, "
        "\"fHipSpreadMinScale\": {}, \"fHipSpreadMaxScale\": {}, "
        "\"strafeRotR\": {}, \"standMoveF\": {}, "
        "\"vStandRot\": {}, \"fFireTimeScale\": {}, \"fBurstDelayTimeScale\": "
        "{}, \"fReloadTimeScale\": {}, "
        "\"fReloadEmptyTimeScale\": {}, \"fReloadAddTimeScale\": {}, "
        "\"fReloadEmptyAddTimeScale\": {}, "
        "\"fReloadQuickTimeScale\": {}, \"fReloadQuickEmptyTimeScale\": {}, "
        "\"fReloadQuickAddTimeScale\": {}, "
        "\"fReloadQuickEmptyAddTimeScale\": {}, \"mmsWeapon\": {}, "
        "\"mmsInScope\": {}, \"mmsFOV\": {}, "
        "\"mmsAspect\": {}, \"mmsMaxDist\": {}, \"clipSizeScale\": {}, "
        "\"iClipSize\": {}, "
        "\"stackFire\": {:08X}, \"stackFireSpread\": {}, "
        "\"stackFireAccuracyDecay\": {}, "
        "\"luiReticle\": {}, \"perks\": {}, \"customFloat0\": {}, "
        "\"customFloat1\": {}, "
        "\"customFloat2\": {}, \"customFloat3\": {}, \"customFloat4\": {}, "
        "\"customFloat5\": {}, "
        "\"customBool0\": {}, \"customBool1\": {}, \"customBool2\": {}, "
        "\"customBool3\": {}, "
        "\"customBool4\": {}, \"customBool5\": {} }}",
        ::serialize(szInternalName), ::serialize(szDisplayName),
        game::weapon::serialize(attachmentType),
        game::weapon::serialize(attachmentPoint),
        game::weapon::serialize(penetrateType),
        game::weapon::serialize(fireType), firstRaisePriority, fHipIdleAmount,
        ::serialize(bAltWeaponAdsOnly), ::serialize(bAltWeaponDisableSwitching),
        altScopeADSTransInTime, altScopeADSTransOutTime, ::serialize(bSilenced),
        ::serialize(bDualMag), ::serialize(laserSight), ::serialize(bInfraRed),
        ::serialize(bUseAsMelee), ::serialize(bDualWield),
        ::serialize(sharedAmmo), fMasterDamageRangeScale,
        ::serialize(damageScale), ::serialize(damageRangeScale),
        ::serialize(multishotBaseDamageScale),
        ::serialize(multishotBaseDamageRangeScale), aimAssistRangeScale,
        aimAssistRangeAdsScale, fAdsFlinchScalar, fAdsFiringFlinchScalar,
        fAdsTurnRateScalar, fAdsWallRunBobScalar, fAdsAdditiveFallScalar,
        fAdsAdditiveJumpScalar, fAdsAdditiveJumpLandScalar,
        fAdsZoom1_focalLength, fAdsZoom1_fStop, fAdsZoom2_focalLength,
        fAdsZoom2_fStop, fAdsZoom3_focalLength, fAdsZoom3_fStop, fAdsZoomFov1,
        fAdsZoomFov2, fAdsZoomFov3, fAdsZoomInFrac, fAdsZoomOutFrac,
        fAdsTransInTimeScale, fAdsTransOutTimeScale, fAdsRecoilReductionRate,
        fAdsRecoilReductionLimit, fAdsViewKickCenterSpeedScale,
        fAdsIdleAmountScale, idleSpeedFromFireTransitionTime,
        fAdsScopeBlurAmount, fAdsScopeBlurStart,
        ::serialize(adsSwayViewInsteadOfGun), fkickAlignedInputScalar,
        fkickOpposedInputScalar, ::serialize(swayOverride), swayMaxAngle,
        swayLerpSpeed, swayPitchScale, swayYawScale, swayHorizScale,
        swayVertScale, ::serialize(adsSwayOverride), adsSwayMaxAngle,
        adsSwayLerpSpeed, adsSwayTransitionLerpSpeed, adsSwayPitchScale,
        adsSwayYawScale, fAdsSwayHorizScale, fAdsSwayVertScale,
        adsMoveSpeedScale, adsFiringSpeedScale, adsMoveSpeedTransitionTimeIn,
        adsMoveSpeedTransitionTimeOut, fHipSpreadMinScale, fHipSpreadMaxScale,
        strafeRotR, standMoveF, vStandRot.serialize(), fFireTimeScale,
        fBurstDelayTimeScale, fReloadTimeScale, fReloadEmptyTimeScale,
        fReloadAddTimeScale, fReloadEmptyAddTimeScale, fReloadQuickTimeScale,
        fReloadQuickEmptyTimeScale, fReloadQuickAddTimeScale,
        fReloadQuickEmptyAddTimeScale, ::serialize(mmsWeapon),
        ::serialize(mmsInScope), mmsFOV, mmsAspect, mmsMaxDist, clipSizeScale,
        iClipSize, stackFire, stackFireSpread, stackFireAccuracyDecay,
        ::serialize(luiReticle), ::serialize(perks), customFloat0, customFloat1,
        customFloat2, customFloat3, customFloat4, customFloat5,
        ::serialize(customBool0), ::serialize(customBool1),
        ::serialize(customBool2), ::serialize(customBool3),
        ::serialize(customBool4), ::serialize(customBool5));
  }
};
typedef WeaponAttachment *WeaponAttachmentPtr;

enum class guidedMissileType_t : int32_t {
  NONE = 0x0,
  SIDEWINDER = 0x1,
  HELLFIRE = 0x2,
  JAVELIN = 0x3,
  BALLISTIC = 0x4,
  WIREGUIDED = 0x5,
  TVGUIDED = 0x6,
  DRONE = 0x7,
  HEATSEEKING = 0x8,
  ROBOTECH = 0x9,
  DYNAMICIMPACTPOINT = 0xA,
  COUNT = 0xB,
};

inline constexpr const char *serialize(const guidedMissileType_t gtype) {
  switch (gtype) {
  case guidedMissileType_t::NONE:
    return "GUIDEDMISSILE_NONE";
  case guidedMissileType_t::SIDEWINDER:
    return "GUIDEDMISSILE_SIDEWINDER";
  case guidedMissileType_t::HELLFIRE:
    return "GUIDEDMISSILE_HELLFIRE";
  case guidedMissileType_t::JAVELIN:
    return "GUIDEDMISSILE_JAVELIN";
  case guidedMissileType_t::BALLISTIC:
    return "GUIDEDMISSILE_BALLISTIC";
  case guidedMissileType_t::WIREGUIDED:
    return "GUIDEDMISSILE_WIREGUIDED";
  case guidedMissileType_t::TVGUIDED:
    return "GUIDEDMISSILE_TVGUIDED";
  case guidedMissileType_t::DRONE:
    return "GUIDEDMISSILE_DRONE";
  case guidedMissileType_t::HEATSEEKING:
    return "GUIDEDMISSILE_HEATSEEKING";
  case guidedMissileType_t::ROBOTECH:
    return "GUIDEDMISSILE_ROBOTECH";
  case guidedMissileType_t::DYNAMICIMPACTPOINT:
    return "GUIDEDMISSILE_DYNAMICIMPACTPOINT";
  default:
    return "GUIDEDMISSILE_INVALID";
  }
}

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
  game::db::xasset::xmodel::XModelPtr viewModel;
  game::db::xasset::xmodel::XModelPtr viewDamagedModel1;
  game::db::xasset::xmodel::XModelPtr viewDamagedModel2;
  game::db::xasset::xmodel::XModelPtr viewDamagedModel3;
  game::db::xasset::xmodel::XModelPtr frontendModel;
  scr::ScrString_t viewmodelTag;
  scr::ScrString_t persistentViewModelTag;
  scr::ScrString_t persistentWorldModelTag;
  scr::ScrString_t stowedModelTag;
  scr::ScrString_t *hideTags;
  game::db::xasset::xmodel::XModelPtr *attachViewModel;
  game::db::xasset::xmodel::XModelPtr *attachWorldModel;
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
  game::db::xasset::xmodel::XModelPtr handModel;
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
  game::db::xasset::xmodel::XModelPtr worldModel;
  game::db::xasset::xmodel::XModelPtr worldModelLeft;
  game::db::xasset::xmodel::XModelPtr worldDamagedModel1;
  game::db::xasset::xmodel::XModelPtr worldDamagedModel2;
  game::db::xasset::xmodel::XModelPtr worldDamagedModel3;
  game::db::xasset::xmodel::XModelPtr stowedModel;
  game::db::xasset::xmodel::XModelPtr worldClipModel;
  game::db::xasset::xmodel::XModelPtr rocketModel;
  game::db::xasset::xmodel::XModelPtr mountedModel;
  game::db::xasset::xmodel::XModelPtr worldGlowingModel;
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
  game::db::xasset::xmodel::XModelPtr projectileModel;
  game::db::xasset::xmodel::XModelPtr projectileModelEnemy;
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

PACKED(struct WeaponDobjInfo {
  enum class Slot : uint32_t {
    HELD = 0x0,
    STOWED = 0x1,
    MELEE = 0x2,
    OFFHAND = 0x4,
  };

  static inline constexpr const char *serialize(const Slot slot) {
    switch (slot) {
    case WeaponDobjInfo::Slot::HELD:
      return "SLOT_HELD";
    case WeaponDobjInfo::Slot::STOWED:
      return "SLOT_STOWED";
    case WeaponDobjInfo::Slot::MELEE:
      return "SLOT_MELEE";
    case WeaponDobjInfo::Slot::OFFHAND:
      return "SLOT_OFFHAND";
    default:
      return "SLOT_INVALID";
    }
  }

  struct ModelMask {
    Slot slot;
    qboolean weapon;
    union {
      const weapon::WeaponDef *weapDef;
      const WeaponAttachmentUnique *attachment;
    };
    uint32_t dobjModelMask;
    uint8_t _padding14[4];

    inline std::string serialize() const noexcept {
      return std::format("{{ \"slot\": {}, \"weapon\": {}, \"target\": {:p}, "
                         "\"dobjModelMask\": {:08X} }}",
                         WeaponDobjInfo::serialize(slot), weapon.serialize(),
                         static_cast<bool>(weapon) ? (const void *)weapDef
                                                   : (const void *)attachment,
                         dobjModelMask);
    }
  };

  uint8_t numModelMasks;
  uint8_t _padding01[3];
  Slot addingSlot;
  int16_t lastAddedModel;
  uint8_t _padding0A[6];
  ModelMask modelMasks[32];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numModelMasks\": {}, \"addingSlot\": {}, "
                       "\"lastAddedModel\": {}, \"modelMasks\": {} }}",
                       numModelMasks, serialize(addingSlot), lastAddedModel,
                       ::serialize(modelMasks));
  }
});

PACKED(struct DroppedWeaponDobjInfo {
  int32_t entnum;
  uint8_t _padding04[4];
  weapon::WeaponDobjInfo weaponDobjInfo;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"entnum\": {}, \"weaponDobjInfo\": {} }}", entnum,
                       weaponDobjInfo.serialize());
  }
});

enum class QueuedActionState : uint32_t {
  NONE = 0x0,
  FIRE_WEAPON = 0x1,
  RELOAD_WEAPON = 0x2,
  MELEE = 0x3,
  SPECIAL_MELEE = 0x4,
  COUNT = 0x5,
};

inline constexpr const char *serialize(const QueuedActionState state) {
  switch (state) {
  case QueuedActionState::NONE:
    return "ACTION_NONE";
  case QueuedActionState::FIRE_WEAPON:
    return "ACTION_FIRE_WEAPON";
  case QueuedActionState::RELOAD_WEAPON:
    return "ACTION_RELOAD_WEAPON";
  case QueuedActionState::MELEE:
    return "ACTION_MELEE";
  case QueuedActionState::SPECIAL_MELEE:
    return "ACTION_SPECIAL_MELEE";
  default:
    return "ACTION_INVALID";
  }
}

PACKED(struct PlayerWeaponHandState {
  int32_t weaponTime;
  int32_t weaponDelay;
  int32_t weaponState;
  uint32_t weaponShotCount;
  animationNumber_t weapAnim;
  uint8_t _padding12[2];
  int32_t weaponFrameTimeRemainder;
  QueuedActionState queuedActionState;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"weaponTime\": {}, \"weaponDelay\": {}, \"weaponState\": {}, "
        "\"weaponShotCount\": {}, \"weapAnim\": {}, "
        "\"weaponFrameTimeRemainder\": {}, "
        "\"queuedActionState\": {} }}",
        weaponTime, weaponDelay, weaponState, weaponShotCount,
        weapAnim.serialize(), weaponFrameTimeRemainder,
        game::weapon::serialize(queuedActionState));
  }
});
ASSERT_SIZE(PlayerWeaponHandState, 0x1C);

struct AmmoPool {
  int32_t count;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"count\": {} }}", count);
  }
};
ASSERT_SIZE(AmmoPool, 0x4);

struct AmmoClip {
  int32_t count;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"count\": {} }}", count);
  }
};
ASSERT_SIZE(AmmoClip, 0x4);

PACKED(struct PlayerHeldWeapon {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"weapon\": {}, \"options\": {}, "
        "\"attachmentCosmeticVariantIndexes\": {}, "
        "\"percent\": {}, \"time\": {}, \"adsZoomSelect\": {}, \"flags\": "
        "{:02X}, "
        "\"overHeating\": {}, \"heldBefore\": {}, \"quickReload\": {}, "
        "\"blockWeaponPickup\": {}, \"needsRechamber\": {}, "
        "\"friendlyTargeting\": {}, "
        "\"friendlyTargetLocked\": {}, \"friendlyHacking\": {} }}",
        weapon.serialize(), options.serialize(),
        attachmentCosmeticVariantIndexes.serialize(), percent.value, time.value,
        adsZoomSelect, flags, ::serialize(static_cast<bool>(overHeating)),
        ::serialize(static_cast<bool>(heldBefore)),
        ::serialize(static_cast<bool>(quickReload)),
        ::serialize(static_cast<bool>(blockWeaponPickup)),
        ::serialize(static_cast<bool>(needsRechamber)),
        ::serialize(static_cast<bool>(friendlyTargeting)),
        ::serialize(static_cast<bool>(friendlyTargetLocked)),
        ::serialize(static_cast<bool>(friendlyHacking)));
  }
});
ASSERT_SIZE(PlayerHeldWeapon, 0x30);

PACKED(struct visWeapon_s {
  Weapon weapon;
  int32_t streamerOptions;
  uint8_t _padding0C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"weapon\": {}, \"streamerOptions\": {:08X} }}",
                       weapon.serialize(), streamerOptions);
  }
});

struct visLoadOut_s {
  visWeapon_s primary;
  visWeapon_s secondary;
  int32_t primaryGadgetIdx;
  int32_t secondaryGadgetIdx;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"primary\": {}, \"secondary\": {}, \"primaryGadgetIdx\": {}, "
        "\"secondaryGadgetIdx\": {} }}",
        primary.serialize(), secondary.serialize(), primaryGadgetIdx,
        secondaryGadgetIdx);
  }
};

PACKED(struct DelayedWeaponFire {
  int32_t time;
  uint8_t _padding04[4];
  Weapon weapon;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"time\": {}, \"weapon\": {} }}", time,
                       weapon.serialize());
  }
});

enum class gadgetHeldSlot_e : uint32_t {
  GADGET_HELD_0 = 0x0,
  GADGET_HELD_1 = 0x1,
  GADGET_HELD_2 = 0x2,
  GADGET_HELD_3 = 0x3,
  GADGET_HELD_COUNT = 0x4,
};

inline constexpr const char *serialize(const gadgetHeldSlot_e slot) {
  switch (slot) {
  case gadgetHeldSlot_e::GADGET_HELD_0:
    return "GADGET_HELD_0";
  case gadgetHeldSlot_e::GADGET_HELD_1:
    return "GADGET_HELD_1";
  case gadgetHeldSlot_e::GADGET_HELD_2:
    return "GADGET_HELD_2";
  case gadgetHeldSlot_e::GADGET_HELD_3:
    return "GADGET_HELD_3";
  default:
    return "GADGET_HELD_INVALID";
  }
}

PACKED(struct WeaponVariantDef {
  const char *szInternalName;
  const char *szModeIndependentName;
  int sessionMode;
  int iVariantCount;
  WeaponDef *weapDef;
  const char *szDisplayName;
  const char *szAltWeaponName;
  const char *szAttachmentUnique;
  WeaponAttachmentPtr *attachments;
  WeaponAttachmentUniquePtr *attachmentUniques;
  const char **szXAnims;
  scr::ScrString_t *hideTags;
  db::xasset::xmodel::XModelPtr *attachViewModel;
  db::xasset::xmodel::XModelPtr *attachWorldModel;
  const char **attachViewModelTag;
  const char **attachWorldModelTag;
  float attachViewModelOffsets[15];
  float attachWorldModelOffsets[15];
  float attachViewModelRotations[15];
  float attachWorldModelRotations[15];
  vec3_t stowedModelOffsets;
  vec3_t stowedModelRotations;
  uint32_t altWeaponIndex;
  uint8_t _padding184[4];
  AttachmentMask iAttachments;
  bool bIgnoreAttachments;
  uint8_t _padding191[3];
  int iClipSize;
  int iReloadTime;
  int iReloadEmptyTime;
  int iReloadQuickTime;
  int iReloadQuickEmptyTime;
  int iReloadSpecialComboTime;
  int iReloadSpecialComboEmptyTime;
  int iReloadSpecialComboQuickTime;
  int iReloadSpecialComboQuickEmptyTime;
  int iAdsTransInTime;
  int iAdsTransOutTime;
  int iAltRaiseTime;
  int iAdsAltRaiseTime;
  int meleeAssassinationStateTimeTransInTime;
  int meleeAssassinationStateTimeTransOutTime;
  const char *szAmmoDisplayName;
  const char *szAmmoName;
  int iAmmoIndex;
  uint8_t _padding1E4[4];
  const char *szClipName;
  int iClipIndex;
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
  bool bSilenced;
  bool bDualMag;
  bool bInfraRed;
  bool bTVGuided;
  uint32_t perks[4];
  bool bAntiQuickScope;
  uint8_t _padding281[7];
  gfx::GfxImagePtr overlayMaterial;
  gfx::GfxImagePtr overlayMaterialLowRes;
  gfx::GfxImagePtr dpadIcon;
  weaponIconRatioType_t dpadIconRatio;
  bool noAmmoOnDpadIcon;
  uint8_t _padding2A5[3];
  vec3_t ikLeftHandIdlePos;
  vec3_t ikLeftHandOffset;
  vec3_t ikLeftHandRotation;
  bool bUsingLeftHandProneIK;
  uint8_t _padding2CD[3];
  vec3_t ikLeftHandProneOffset;
  vec3_t ikLeftHandProneRotation;
  vec3_t ikLeftHandUiViewerOffset;
  vec3_t ikLeftHandUiViewerRotation;
});
// Verified
ASSERT_SIZE(WeaponVariantDef, 0x300);

PACKED(struct weaponParms {
  vec3_t forward;
  vec3_t right;
  vec3_t up;
  vec3_t muzzleTrace;
  vec3_t gunForward;
  uint8_t _padding3C[4];
  Weapon weapon;
  const WeaponVariantDef *weapVariantDef;
  const WeaponDef *weapDef;
  bool reflected;
  uint8_t _padding59[7];
});

} // namespace weapon
} // namespace game
