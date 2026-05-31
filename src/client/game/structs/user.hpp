#pragma once

#include "core.hpp"
#include "weapon.hpp"
#include "ai.hpp"
#include "asm.hpp"
#include <cstdint>
namespace game {
namespace user {

#pragma pack(push, 1)
enum BotLookType : int32_t {
  BOT_LOOK_NONE = 0x0,
  BOT_LOOK_FORWARD = 0x1,
  BOT_LOOK_POINT = 0x2,
  BOT_LOOK_ANGLES = 0x3,
};

enum bot_goal_state : int32_t {
  BOT_GOAL_INVALID = 0x0,
  BOT_GOAL_UNREACHABLE = 0x1,
  BOT_GOAL_SET = 0x2,
  BOT_GOAL_REACHED = 0x3,
};

struct bot_look_t {
  vec2_t anglesDesired;
  vec2_t anglesCurrent;
  vec3_t lookPoint;
  BotLookType lookType;
  vec3_t kickAngles;
  vec3_t kickAVel;
  weapon::Weapon grenadeWeapon;
  vec3_t grenadeOrigin;
  int32_t grenadeTime;
};
ASSERT_SIZE(bot_look_t, 0x50);

struct bot_move_t {
  vec_t angleDesired;
  vec_t magnitude;
};
ASSERT_SIZE(bot_move_t, 0x8);

struct bot_goal_t {
  vec3_t position;
  int32_t radius;
  bot_goal_state state;
};
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

struct bot_t {
  bot_look_t look;
  bot_move_t move;
  bot_goal_t goal;
  uint8_t _padding6C[4];
  hkVector4 lastCorner;
  game_button_bits_t tapButtons;
  game_button_bits_t buttons;
  weapon::Weapon weapon;
  int32_t lastMoveTime;
  char goal_callStack[1025];
  uint8_t _padding4A5[11];
};
ASSERT_SIZE(bot_t, 0x4B0);
#pragma pack(pop)

typedef bitarray::bitarray<32> serverFieldBits_t;
#pragma pack(push, 1)
struct usercmd_s {
  int32_t serverTime;
  game_button_bits_t button_bits;
  serverFieldBits_t serverFields;
  int32_t angles[3];
  uint16_t level;
  uint8_t _padding22[6];
  weapon::Weapon weapon;
  weapon::Weapon offHandWeapon;
  weapon::Weapon lastWeaponAltModeSwitch;
  int8_t forwardmove;
  int8_t rightmove;
  int8_t upmove;
  int8_t pitchmove;
  int8_t yawmove;
  uint8_t _padding45[1];
  uint16_t meleeChargeEnt;
  uint16_t meleeChargeDist;
  int16_t damageKick[2];
  int8_t selectedLocation[2];
  uint8_t selectedYaw;
  uint8_t _padding51[1];
  uint16_t lastInput;
  uint16_t streamerState;
  uint8_t _padding56[2];
  uint32_t kf_index;
  qboolean transition;
};
typedef usercmd_s usercmd_t;
static_assert(offsetof(usercmd_t, _padding45) == 0x45);
static_assert(offsetof(usercmd_t, _padding56) == 0x56);
ASSERT_SIZE(usercmd_t, 0x60);

struct actor_t; // TODO
#pragma pack(pop)

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
  char name[32];
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