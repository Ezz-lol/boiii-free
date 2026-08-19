#pragma once

#include "game/structs/scr/core.hpp"
#include <cstdint>
#include <game/structs/asm.hpp>
namespace game {
namespace scr {
namespace bg {

typedef fastcallPtr_t<void(LocalClientNum_t, uint32_t, float, float, bool, bool,
                           ScrString_t, bool, bool)>
    ClientFieldCodeCallbackFuncFloatVal;
typedef fastcallPtr_t<void(LocalClientNum_t, uint32_t, uint32_t, uint32_t, bool,
                           bool, ScrString_t, bool, bool)>
    ClientFieldCodeCallbackFuncUintVal;

struct clientFieldCodeCallback_t {
  struct {
    uint64_t bHasCodeCallback : 1;
    uint64_t bCodeCallbackIsFloatVal : 1;
  };
  union {
    ClientFieldCodeCallbackFuncFloatVal floatCallbackFunc;
    ClientFieldCodeCallbackFuncUintVal uintCallbackFunc;
  };
};

struct clientFieldUnionClient_t {
  uint8_t bSplitscreenHostOnly;
  uint8_t bCallbacksFor0WhenNew;
};

struct clientFieldUnionServer_t {
  uint8_t gap0;
};

union clientFieldUnion_t {
  clientFieldUnionClient_t client;
  clientFieldUnionServer_t server;
};

struct clientField_t {
  scr_func_t scriptCallbackFunc;
  clientFieldCodeCallback_t codeCallbackFunc;
  ScrString_t nameHash;
  uint32_t fieldOffset;
  uint32_t accessMask;
  uint8_t bitOffset;
  uint8_t fieldType;
  uint8_t obsolete;
  uint8_t clientFieldSet;
  uint32_t numBits;
  uint32_t version;
  clientFieldUnion_t u;
  uint8_t _padding13A[6];
};
ASSERT_SIZE(clientField_t, 0x140);

struct clientFieldCallback_t {
  clientField_t *pField;
  union {
    float oldFloat;
    uint32_t oldInt;
  };
  union {
    float newFloat;
    uint32_t newInt;
  };
  uint32_t localClientNum;
  uint32_t entNum;
  struct {
    uint64_t bInitialSnap : 1;
    uint64_t bNewEnt : 1;
    uint64_t bWasDemoJump : 1;
    uint64_t bWasKillcamTransition : 1;
  };
};

struct clientNetField_t {
  int32_t bitsUsed;
  int32_t netFieldOffset;
};

struct clientFieldSet_t {
  int32_t numFields;
  clientField_t *pFields[2048];
  int32_t numNetFields;
  clientNetField_t *pNetFields;
};

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
  fastcallPtr_t<bool(clientField_t *field, uint32_t)>
      FieldVersionAllowsRegistration;
  fastcallPtr_t<bool(uint32_t)> ServerVersionAllowsRegistration;
};

} // namespace bg
} // namespace scr
} // namespace game