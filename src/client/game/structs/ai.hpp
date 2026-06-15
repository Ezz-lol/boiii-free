#pragma once

#include <cstdint>
#include "asm.hpp"
namespace game {
namespace ai {
struct sentient_t; // TODO

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

enum class AICategory : int32_t {
  NONE = 0x0,
  CIVILIAN = 0x1,
  ASSAULT = 0x2,
  CQB = 0x3,
  RPG = 0x4,
  SNIPER = 0x5,
  SUPPRESSOR = 0x6,
  RIOT_SHIELD = 0x7,
  COUNT = 0x8,
};

enum class AIWeaponFireType : int32_t {
  FULLAUTO = 0x0,
  BURST = 0x1,
  SINGLE_SHOT = 0x2,
  COUNT = 0x3,
  INVALID = 0x3,
};

} // namespace ai
} // namespace game