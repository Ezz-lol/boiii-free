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
  void (*CompareHashToGameState)(uint32_t);
  bool (*AllowVersionReRegistration)(void);
  bool (*FieldVersionAllowsRegistration)(clientField_t *, uint32_t);
  bool (*ServerVersionAllowsRegistration)(uint32_t);
};

} // namespace bg
} // namespace scr
} // namespace game