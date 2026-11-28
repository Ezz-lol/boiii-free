#pragma once

#include "ai.hpp"
#include "asm.hpp"
#include "core.hpp"
#include "game/structs/db/xasset/world.hpp"
#include "game/structs/hk/core.hpp"
#include "weapon.hpp"

#include <cstdint>
namespace game {
namespace user {

enum class BotLookType : uint32_t {
  NONE = 0x0,
  FORWARD = 0x1,
  POINT = 0x2,
  ANGLES = 0x3,
};

inline constexpr const char *serialize(const BotLookType lookType) {
  switch (lookType) {
  case BotLookType::NONE:
    return "BOT_LOOK_TYPE_NONE";
  case BotLookType::FORWARD:
    return "BOT_LOOK_TYPE_FORWARD";
  case BotLookType::POINT:
    return "BOT_LOOK_TYPE_POINT";
  case BotLookType::ANGLES:
    return "BOT_LOOK_TYPE_ANGLES";
  default:
    return "BOT_LOOK_TYPE_INVALID";
  }
}

enum class bot_goal_state : uint32_t {
  INVALID = 0x0,
  UNREACHABLE = 0x1,
  SET = 0x2,
  REACHED = 0x3,
};

inline constexpr const char *serialize(const bot_goal_state goalState) {
  switch (goalState) {
  case bot_goal_state::INVALID:
    return "BOT_GOAL_INVALID";
  case bot_goal_state::UNREACHABLE:
    return "BOT_GOAL_UNREACHABLE";
  case bot_goal_state::SET:
    return "BOT_GOAL_SET";
  case bot_goal_state::REACHED:
    return "BOT_GOAL_REACHED";
  default:
    return "BOT_GOAL_INVALID";
  }
}

// Verified
PACKED(struct bot_look_t {
  vec3_t anglesDesired;
  vec3_t anglesCurrent;
  vec_t dot;
  vec3_t kickAngles;
  vec3_t kickAVel;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"anglesDesired\": {}, \"anglesCurrent\": {}, "
                       "\"dot\": {}, \"kickAngles\": {}, \"kickAVel\": {} }}",
                       anglesDesired.serialize(), anglesCurrent.serialize(),
                       dot, kickAngles.serialize(), kickAVel.serialize());
  }
});
ASSERT_SIZE(bot_look_t, 0x34);

PACKED(struct bot_move_t {
  vec_t angleDesired;
  vec_t magnitude;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"angleDesired\": {}, \"magnitude\": {} }}",
                       angleDesired, magnitude);
  }
});
ASSERT_SIZE(bot_move_t, 0x8);

PACKED(struct bot_goal_t {
  vec3_t position;
  int32_t radius;
  bot_goal_state state;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"position\": {}, radius: {}, state: {} }}",
                       position.serialize(), radius,
                       game::user::serialize(state));
  }
});
ASSERT_SIZE(bot_goal_t, 0x14);

/* Entirely unspecified in BO3, BO4, BO2 alpha engines. Just 0x10 empty bytes.
   This is probably a marker class that's used with some kind of extension
   methods, as there are `hkVector4dComparison` and similar classes existing.

   Given its name, I am assuming this is a vec4 of 4 floats. This matches
   vec4_t's type, so we will define it as such.

   Correct as needed.
*/
typedef vec4_t hkVector4f;

typedef hkVector4f hkVector4;

struct bot_settings_t {
  vec2_t lookSensitivity;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"lookSensitivity\": {} }}",
                       lookSensitivity.serialize());
  }
};

// Verified
PACKED(struct bot_t {
  bot_look_t look;
  bot_move_t move;
  vec3_t predictedCornerEntry;
  game_button_bits_t tapButtons;
  game_button_bits_t buttons;
  weapon::Weapon weapon;
  bool spawned;
  DEF_PAD(69, 7);
  hk::hkArray<vec3_t, hk::hkContainerHeapAllocator> path;
  int pathFailedCause;
  vec3_t pathFailedGoal;
  const db::xasset::world::pathnode_t *traversalStartNode;
  const db::xasset::world::pathnode_t *traversalEndNode;
  vec3_t traversalEnterPos;
  vec3_t traversalEndPos;
  bot_settings_t settings;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"look\": {}, \"move\": {}, \"predictedCornerEntry\": {}, "
        "\"tapButtons\": {}, \"buttons\": {}, \"weapon\": {}, \"spawned\": {}, "
        "\"path\": {}, \"pathFailedCause\": {}, \"pathFailedGoal\": {}, "
        "\"traversalStartNode\": {:p}, \"traversalEndNode\": {:p}, "
        "\"traversalEnterPos\": {}, \"traversalEndPos\": {}, \"settings\": {} "
        "}}",
        look.serialize(), move.serialize(), predictedCornerEntry.serialize(),
        tapButtons.serialize(), buttons.serialize(), weapon.serialize(),
        ::serialize(spawned), path.serialize(), pathFailedCause,
        pathFailedGoal.serialize(),
        static_cast<const void *>(traversalStartNode),
        static_cast<const void *>(traversalEndNode),
        traversalEnterPos.serialize(), traversalEndPos.serialize(),
        settings.serialize());
  }
});
ASSERT_SIZE(bot_t, 0xC0);

typedef bitarray<32> serverFieldBits_t;
PACKED(struct usercmd_s {
  int32_t serverTime;
  game_button_bits_t button_bits;
  serverFieldBits_t serverFields;
  int32_t angles[3];
  uint16_t level;
  DEF_PAD(22, 6);
  weapon::Weapon weapon;
  weapon::Weapon offHandWeapon;
  weapon::Weapon lastWeaponAltModeSwitch;
  int8_t forwardmove;
  int8_t rightmove;
  int8_t upmove;
  int8_t pitchmove;
  int8_t yawmove;
  DEF_PAD(45, 1);
  uint16_t meleeChargeEnt;
  uint16_t meleeChargeDist;
  int16_t damageKick[2];
  int8_t selectedLocation[2];
  uint8_t selectedYaw;
  DEF_PAD(51, 1);
  uint16_t lastInput;
  uint16_t streamerState;
  DEF_PAD(56, 2);
  uint32_t kf_index;
  qboolean transition;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"serverTime\": {}, \"button_bits\": {}, \"serverFields\": {}, "
        "\"angles\": {}, \"level\": {}, \"weapon\": {}, \"offHandWeapon\": "
        "{}, "
        "\"lastWeaponAltModeSwitch\": {}, \"forwardmove\": {}, "
        "\"rightmove\": "
        "{}, "
        "\"upmove\": {}, \"pitchmove\": {}, \"yawmove\": {}, "
        "\"meleeChargeEnt\": {}, "
        "\"meleeChargeDist\": {}, \"damageKick\": {}, \"selectedLocation\": "
        "{}, "
        "\"selectedYaw\": {}, \"lastInput\": {}, \"streamerState\": {}, "
        "\"kf_index\": {}, \"transition\": {} }}",
        serverTime, button_bits.serialize(), serverFields.serialize(),
        ::serialize(angles), level, weapon.serialize(),
        offHandWeapon.serialize(), lastWeaponAltModeSwitch.serialize(),
        forwardmove, rightmove, upmove, pitchmove, yawmove, meleeChargeEnt,
        meleeChargeDist, ::serialize(damageKick), ::serialize(selectedLocation),
        selectedYaw, lastInput, streamerState, kf_index,
        transition.serialize());
  }
});
typedef usercmd_s usercmd_t;
ASSERT_OFFSET(usercmd_t, _padding45, 0x45);
ASSERT_OFFSET(usercmd_t, _padding56, 0x56);
ASSERT_SIZE(usercmd_t, 0x60);

struct actor_t; // TODO

enum class gadgetPulseShareType_e : uint32_t {
  DEFAULT = 0x0,
  NONE = 0x1,
  MINIMAP = 0x2,
  VIEWPORT = 0x3,
  BOTH = 0x4,
  COUNT = 0x5,
};

struct __attribute__((aligned(8))) visionPulse_t {
  bool active;
  int32_t startTime;
  int32_t endTime;
  float radius;
  int32_t maxRadius;
  float pulseSpeed;
  gadgetPulseShareType_e revealShareType;
  gadgetPulseShareType_e orbShareType;
  vec3_t origin;
  bool friendly;
  int32_t ownerNum;
  int32_t pulseFadeOut;
  weapon::Weapon gadgetWeapon;
  int32_t numberEntsHit;
  snd::SndAliasId pulseSound;
  snd::SndAliasId pulseMissSound;
};

struct SonarAttachmentInfo {
  vec3_t prevOrigin;
  float movementDistance[60];
  int32_t nextMovementDistanceIndex;
};

struct __attribute__((aligned(4))) actorInfo_t {
  bool infoValid;
  bool nextValid;
  int32_t actorNum;
  int32_t entityNum;
  name_t name;
  uint32_t modelIndex;
  anim::ModelAttachmentIndex attachments[6];
  uint32_t attachIgnoreCollision;
  uint16_t aiType;
  ai::AIBody aiBody;
  int32_t lookAtEntNum;
  vec3_t lookAtPos;
  vec3_t lastLookAtPos;
  float lookAtBlend;
  int32_t lookAtTime;
  vec3_t lookAtAngles;
  bool lookAtNoTorso;
  struct {
    uint64_t lookAtTracking : 1;
    uint64_t lookAtAiming : 1;
    uint64_t gibbed : 1;
  };
  SonarAttachmentInfo sonarAttachmentInfo;
  weapon::WeaponDobjInfo weaponDobjInfo;
  float infraredFadeOut;
  bool dobjDirty;
  bool animsDirty;
  anim::XAnimTree *pXAnimTree;
  weapon::Weapon DObjWeapon;
  visionPulse_t revealPulse;
  bool wasFiring[5];
};
} // namespace user
} // namespace game
