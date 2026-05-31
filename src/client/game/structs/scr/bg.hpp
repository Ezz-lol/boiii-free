#pragma once

#include <cstdint>
#include "core.hpp"
namespace game {

namespace snd {
struct SndAliasList;
}

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

struct __attribute__((aligned(8))) animation_t {
  char name[64];
  int32_t initialLerp;
  int32_t finalLerp;
  float forceAnimRate;
  float moveSpeed;
  float rotSpeed;
  int32_t duration;
  int32_t nameHash;
  int32_t flag;
  int64_t movetype;
  int32_t stance;
  int32_t noteType;
  int32_t locBlendIndex;
  float locBlendAlignTime;
  uint16_t localOverrideAnim;
};

struct locBlendPairAnim_t {
  int32_t animIndex;
  float extent;
};
struct __attribute__((aligned(8))) locBlendPair_t {
  locBlendPairAnim_t *start;
  locBlendPairAnim_t *end;
  int32_t strideType;
};
struct locBlendSet_t {
  char name[64];
  int32_t startIndex;
  int32_t count;
};

enum class locBlendType_t : uint32_t {
  DIRECTIONAL = 0x0,
  TURN_LEAN = 0x1,
  COUNT = 0x2,
};

struct __attribute__((aligned(8))) locBlend_t {
  char name[64];
  locBlendType_t type;
  locBlendSet_t *sets[4];
  int32_t numSets;
};

struct __attribute__((aligned(4))) XAnimSearchEntry {
  ScrString_t name;
  uint16_t animIndex;
};

struct XAnimParam {
  char strName[16];
  float fValue;
};

struct XAnimParent {
  uint16_t flags;
  uint16_t children;
};

struct XAnimEntry {
  uint16_t bCreated;
  uint16_t numAnims;
  uint16_t firstParamIxPlusOne;
  uint16_t paramCount;
  uint16_t parent;
  union {
    XAnimParts *parts;
    XAnimParent animParent;
  };
};

struct XAnimBlendTargetWeights {
  uint16_t numKeys;
  uint16_t *frames;
  float *values;
};

typedef uint8_t ByteVec[3];
typedef uint16_t uint16_tVec[3];

union XAnimDynamicFrames {
  ByteVec *_1;
  uint16_tVec *_2;
};
union XAnimDynamicIndices {
  uint8_t _1[1];
  uint16_t _2[1];
};

struct __attribute__((aligned(8))) XAnimPartTransFrames {
  vec3_t mins;
  vec3_t size;
  XAnimDynamicFrames frames;
  XAnimDynamicIndices indices;
};

union XAnimPartTransData {
  XAnimPartTransFrames frames;
  vec3_t frame0;
};

struct XAnimPartTrans {
  uint16_t size;
  uint8_t smallTrans;
  XAnimPartTransData u;
};

namespace fp16 {
union half {
  uint16_t u;
  struct {
    uint16_t Mantissa : 10;
    uint16_t Exponent : 5;
    uint16_t Sign : 1;
  };
};

struct half2_pack {
  union {
    half h[2];
    struct {
      int16_t x;
      int16_t y;
    };
    struct {
      uint32_t xy;
    };
  };
};

struct half4_pack {
  union {
    half h[4];
    struct {
      int16_t x;
      int16_t y;
      int16_t z;
      int16_t w;
    };
    struct {
      uint32_t xy;
      uint32_t zw;
    };
    struct {
      uint64_t xyzw;
    };
  };
};

} // namespace fp16
typedef fp16::half2_pack XQuat2;

struct __attribute__((aligned(8))) XAnimDeltaPartQuatDataFrames2 {
  XQuat2 *frames;
  XAnimDynamicIndices indices;
};

union XAnimDeltaPartQuatData2 {
  XAnimDeltaPartQuatDataFrames2 frames;
  XQuat2 frame0;
};

struct XAnimDeltaPartQuat2 {
  uint16_t size;
  XAnimDeltaPartQuatData2 u;
};

typedef fp16::half4_pack XQuat;

struct __attribute__((aligned(8))) XAnimDeltaPartQuatDataFrames {
  XQuat *frames;
  XAnimDynamicIndices indices;
};

union XAnimDeltaPartQuatData {
  XAnimDeltaPartQuatDataFrames frames;
  XQuat frame0;
};

struct XAnimDeltaPartQuat {
  uint16_t size;
  XAnimDeltaPartQuatData u;
};

struct XAnimDeltaPart {
  XAnimPartTrans *trans;
  XAnimDeltaPartQuat2 *quat2;
  XAnimDeltaPartQuat *quat;
};

struct XAnimIKPitchBone {
  uint16_t index;
  vec3_t rotation;
  vec3_t translation;
};

struct XAnimIKPitchLayer {
  float angle;
  uint16_t numBones;
  uint16_t bonesStart;
};

struct XAnimMarkup {
  ScrString_t type;
  float startTime;
  float endTime;
  uint8_t persistent;
  ScrString_t param1;
  ScrString_t param2;
};

struct XAnimNotifyInfo {
  ScrString_t type;
  float time;
  ScrString_t param1;
  ScrString_t param2;
  ScrString_t param3;
};

struct __attribute__((aligned(4))) BlendTargetRemap {
  ScrString_t name;
  uint16_t index;
};

union XAnimIndices {
  uint8_t *_1;
  uint16_t *_2;
  void *data;
};

struct XAnimParts {
  BlendTargetRemap *blendTargetRemap;
  ScrString_t *blendTargetNames;
  ScrString_t *names;
  XAnimBlendTargetWeights *blendTargetWeights;
  XAnimDeltaPart *deltaPart;
  XAnimIKPitchBone *ikPitchBones;
  XAnimIKPitchLayer *ikPitchLayers;
  XAnimMarkup *markup;
  XAnimNotifyInfo *notify;
  XAnimNotifyInfo *shutdownNotify;
  XAnimNotifyInfo *startupNotify;
  uint8_t *dataByte;
  uint8_t *extraChannelData;
  uint8_t *randomDataByte;
  const char *name;
  int32_t *dataInt;
  int32_t *randomDataInt;
  int16_t *dataShort;
  int16_t *randomDataShort;
  XAnimIndices indices;
  ScrString_t branchName;
  float framerate;
  float frequency;
  float loopEntryTime;
  float primedLength;
  uint32_t indexCount;
  uint32_t numIKPitchBones;
  uint32_t numIKPitchLayers;
  uint32_t randomDataShortCount;
  uint32_t streamedFileSize;
  uint32_t dataByteCount;
  uint32_t dataIntCount;
  uint32_t dataShortCount;
  uint32_t extraChannelDataCount;
  uint32_t randomDataByteCount;
  uint32_t randomDataIntCount;
  uint16_t boneCount[10];
  uint16_t numBones;
  uint16_t numframes;
  uint16_t numBlendTargets;
  bool bDelta;
  bool bDelta3D;
  bool bExtraChannel;
  bool bHasBoneMask;
  bool bLeftHandGripIK;
  bool bLoop;
  bool bLoopSync;
  uint8_t assetType;
  uint8_t extraFlags;
  uint8_t markupCount;
  uint8_t notifyCount;
  uint8_t shutdownNotifyCount;
  uint8_t startupNotifyCount;
  bool pad;
};

struct XAnim {
  ScrString_t name;
  int32_t size;
  uint32_t paramSize;
  XAnimParam *params;
  ScrString_t *animNames;
  XAnimSearchEntry *sortedSearchList;
  XAnimEntry entries[1];
};

struct scr_animtree_t {
  XAnim *anims;
};

struct animScriptCondition_t {
  int32_t index;
  uint32_t value;
};

enum class animBodyPart_t : uint32_t {
  UNUSED = 0x0,
  LEGS = 0x1,
  TORSO = 0x2,
  BOTH = 0x3,
  TORSO_2 = 0x4,
  TORSO_FULL = 0x5,
  MOTION_MATCHING = 0x6,
  COUNT = 0x7,
};

struct __attribute__((aligned(4))) animScriptCommand_t {
  snd::SndAliasList *soundAlias;
  ScrString_t tagName;
  animBodyPart_t bodyPart[2];
  int16_t animIndex[2];
  uint16_t animDuration[2];
  union {
    uint8_t flags;
    struct {
      uint8_t motionMatchingEnabled : 1;
    };
  };
};

struct animScriptItem_t {
  int32_t numConditions;
  animScriptCondition_t conditions[5];
  int32_t numCommands;
  animScriptCommand_t commands[8];
};

struct animScript_t {
  int32_t numItems;
  animScriptItem_t *items[192];
};

struct animScriptData_t {
  animation_t animations[6000];
  uint32_t numAnimations;
  animScript_t scriptAnims[1][45];
  animScript_t scriptCannedAnims[1][45];
  animScript_t scriptStateChange[1][1];
  animScript_t scriptEvents[101];
  animScriptItem_t scriptItems[6272];
  int32_t numScriptItems;
  scr_animtree_t animTree;
  uint16_t torsoAnim;
  uint16_t torso2Anim;
  uint16_t legsAnim;
  uint16_t motionMatchingAnim;
  uint16_t turningAnim;
  locBlendPairAnim_t locBlendPairAnims[2048];
  locBlendPair_t locBlendPairs[1024];
  locBlendSet_t locBlendSets[256];
  locBlend_t locBlends[128];
  int32_t numLocBlendPairAnims;
  int32_t numLocBlendPairs;
  int32_t numLocBlendDirSets;
  int32_t numLocBlends;
  snd::SndAliasList *(*soundAlias)(const char *);
  void (*playSoundAlias)(ClientNum_t, snd::SndAliasList *);
};

struct bgsAnim_t {
  animScriptData_t animScriptData;
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