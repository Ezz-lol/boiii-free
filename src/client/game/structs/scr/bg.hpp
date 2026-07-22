#pragma once

#include <cstdint>
#include "core.hpp"
#include "../asm.hpp"
namespace game {
namespace scr {
namespace bg {
struct bgsClientFieldCallbacks_t {
  clientFieldCallback_t clientFieldCallbacks[4][3072];
  uint32_t numCallbacks[4];
};

struct bgsClientFields_t {
  bool bRegistrationFinished;
  clientFieldSet_t clientFieldSets[15];
  uint32_t fieldHash;
  uint32_t highestVersionUsed;
  bgsClientFieldCallbacks_t *pPendingCallbacks;
};

struct bgsAnim_t {
  anim::animScriptData_t animScriptData;
  struct {
    scr_animtree_t tree;
    scr_anim_t body;
    scr_anim_t main;
    scr_anim_t torso;
    scr_anim_t torso2;
    scr_anim_t legs;
    scr_anim_t motion_matching;
  } generic_human;
  struct {
    scr_animtree_t tree;
  } ai_generic_human;
};

struct bgs_clientfieldapi_t {
  fastcallPtr_t<void(uint32_t)> CompareHashToGameState;
  fastcallPtr_t<bool(void)> AllowVersionReRegistration;
  fastcallPtr_t<bool(clientField_t *, uint32_t)> FieldVersionAllowsRegistration;
  fastcallPtr_t<bool(uint32_t)> ServerVersionAllowsRegistration;
};

} // namespace bg
} // namespace scr
} // namespace game