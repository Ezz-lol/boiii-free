#pragma once

#include "core.hpp"
#include "weapon.hpp"
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

struct bot_move_t {
  vec_t angleDesired;
  vec_t magnitude;
};

struct bot_goal_t {
  vec3_t position;
  int32_t radius;
  bot_goal_state state;
};

// Entirely unspecified in BO3, BO4, BO2 alpha engines. Just 0x10 empty bytes.
// This is probably a marker class that's used with some kind of extension
// methods, as there are `hkVector4dComparison` and similar classes existing.
struct hkVector4f {
  uint8_t unknown[0x10];
};

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
#pragma pack(pop)

#pragma pack(push, 1)

// sizeof=0x50
struct usercmd_s {
  int32_t serverTime;
  game_button_bits_t button_bits;
  int32_t angles[3];
  uint8_t _padding1C[4];
  weapon::Weapon weapon;
  weapon::Weapon offHandWeapon;
  weapon::Weapon lastWeaponAltModeSwitch;
  char forwardmove;
  char rightmove;
  char upmove;
  char pitchmove;
  char yawmove;
  uint8_t _padding3D[1];
  uint16_t meleeChargeEnt;
  uint16_t meleeChargeDist;
  uint8_t _padding42[2];
  union {
    float rollmove;
    int16_t damageKick[2];
  };
  char selectedLocation[2];
  uint8_t selectedYaw;
  uint8_t _padding4B[1];
  uint16_t lastInput;
  uint16_t streamerState;
};
static_assert(sizeof(usercmd_s) == 0x50, "usercmd_s size must be 80 bytes");

typedef usercmd_s usercmd_t;

struct actor_t;     // TODO
struct objective_t; // TODO

#pragma pack(pop)
} // namespace user
} // namespace game