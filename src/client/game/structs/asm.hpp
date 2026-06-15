#pragma once
#include "core.hpp"
#include "quake.hpp"
#include "scr/core.hpp"
#include "fp16.hpp"

namespace game {

namespace snd {
struct SndAliasList;
}

// asm reserved
namespace anim {

#pragma pack(push, 1)

enum class StateMachineStatus : int32_t {

  BSM_INVALID = 0x0,
  BSM_DEFAULT = 0x0,
  BSM_UNUSED1 = 0x1,
  BSM_UNUSED2 = 0x2,
  BSM_FAILURE = 0x3,
  BSM_SUCCESS = 0x4,
  BSM_RUNNING = 0x5,
  BSM_VALID = 0x6,
  BSM_TERMINATE = 0x7,
  BSM_STATUS_COUNT = 0x8,
};

enum class aiPathMode : int32_t {

  AI_PATH_MODE_INVALID = 0x0,
  AI_PATH_DONT_MOVE = 0x1,
  AI_PATH_MOVE_ALLOWED = 0x2,
  AI_PATH_MOVE_DELAYED = 0x3,
};

enum class aiGoalSources : int32_t {
  AI_GOAL_SRC_SCRIPT_GOAL = 0x0,
  AI_GOAL_SRC_SCRIPT_ENTITY_GOAL = 0x1,
  AI_GOAL_SRC_SCRIPT_FORCED_GOAL = 0x2,
  AI_GOAL_SRC_ENEMY = 0x3,
};

enum class asmAnimSource_t : int32_t {
  ASM_ANIM_SOURCE_NONE = 0x0,
  ASM_ANIM_SOURCE_SUBSTATE = 0x1,
  ASM_ANIM_SOURCE_TRANSITION = 0x2,
  ASM_ANIM_SOURCE_TRANSDEC = 0x3,
  ASM_ANIM_SOURCE_EXTERNAL = 0x4,
  ASM_ANIM_SOURCE_COUNT = 0x5,
};

enum class asmSubstateEndReason_t : int32_t {
  END_REASON_NONE = 0x0,
  END_REASON_SUBSTATE = 0x1,
  END_REASON_TRANSITION = 0x2,
  END_REASON_TRANSDEC = 0x3,
  END_REASON_NEW_REQUEST = 0x4,
  END_REASON_EXTERNAL = 0x5,
  END_REASON_COUNT = 0x6,
};

enum class asmStatus_t : int32_t {

  ASM_STATUS_INACTIVE = 0x0,
  ASM_STATUS_RUNNING = 0x1,
  ASM_STATUS_COMPLETE = 0x2,
  ASM_STATUS_TERMINATED = 0x3,
  ASM_STATUS_COUNT = 0x4,
};
// sizeof=0x90
struct RumbleGraph {
  const char *name;
  int32_t knotCount;
  vec2_t knots[16];
  uint8_t _padding8C[4];
};
ASSERT_SIZE(RumbleGraph, 0x90);

// sizeof=0x40
struct RumbleInfo {
  const char *name;
  int32_t duration;
  float range;
  RumbleGraph *highRumbleGraph;
  RumbleGraph *lowRumbleGraph;
  qboolean fadeWithDistance;
  qboolean broadcast;
  float camShakeScale;
  int32_t camShakeDuration;
  float camShakeRange;
  float pulseRadiusOuter;
  float pulseScale;
  scr::ScrString_t pulseBoneTag;
};
ASSERT_SIZE(RumbleInfo, 0x40);

typedef RumbleInfo *RumbleInfoPtr;

// sizeof=0x18
struct AliasToAnimsEntry {
  scr::ScrString_t aliasName;
  uint8_t _padding04[4];
  scr::ScrString_t *anims;
  int32_t numAnims;
  uint8_t _padding14[4];
};
ASSERT_SIZE(AliasToAnimsEntry, 0x18);

// sizeof=0x8
struct AnimSelectorTableEntry {
  scr::ScrString_t name;
  union {
    uint32_t _data;
    int32_t integer;
    float v;
  };
};
ASSERT_SIZE(AnimSelectorTableEntry, 0x8);

// sizeof=0x18
struct AnimMappingTable {
  const char *name;
  AliasToAnimsEntry *entries;
  int32_t numAnimAlias;
  uint8_t _padding14[4];
};
ASSERT_SIZE(AnimMappingTable, 0x18);
typedef AnimMappingTable *AnimMappingTablePtr;

// sizeof=0x10C
struct AnimSelectorTableCache {
  int32_t tableIndex;
  int32_t rowIndex;
  AnimSelectorTableEntry searchCriteria[32];
  int32_t searchCriteriaSize;
};
ASSERT_SIZE(AnimSelectorTableCache, 0x10C);

struct AnimSelectorOutParamsGroup1 {
  int32_t aliasColumn;
  int32_t aimUpAliasColumn;
  int32_t aimDownAliasColumn;
  int32_t aimLeftAliasColumn;
  int32_t aimRightAliasColumn;
  int32_t mocompColumn;
};

struct AnimSelectorOutParamsGroup2 {
  int32_t shootSemiAliasColumn;
  int32_t shootSingleAliasColumn;
  int32_t shootBurst3AliasColumn;
  int32_t shootBurst4AliasColumn;
  int32_t shootBurst5AliasColumn;
  int32_t shootBurst6AliasColumn;
};

struct AnimSelectorOutParamsGroup3 {
  int32_t paramForwardAliasColumn;
  int32_t paramBackwardAliasColumn;
  int32_t paramLeftAliasColumn;
  int32_t paramRightAliasColumn;
  int32_t paramBalanceAliasColumn;
  int32_t paramTurnLeftAliasColumn;
  int32_t paramTurnRightAliasColumn;
  int32_t paramIdleBlendDropoffColumn;
};

enum class animSelectorColumnDataType_t : int32_t {

  AST_ENUM_TYPE = 0x0,
  AST_FLOAT_TYPE = 0x1,
  AST_INT_TYPE = 0x2,
  AST_MIN_FLOAT_TYPE = 0x3,
  AST_MAX_FLOAT_TYPE = 0x4,
  AST_RANGE_TYPE = 0x5,
  AST_STRING_TYPE = 0x6,
  AST_DATA_TYPE_COUNT = 0x7,
  AST_INVALID_DATA_TYPE = 0x7,
};

struct AnimSelectorTableColumn {
  scr::ScrString_t name;
  scr::ScrString_t parentName;
  bool desired;
  bool outputColumn;
  bool aliasColumn;
  animSelectorColumnDataType_t dataType;
  scr::ScrVarCanonicalName_t nameCanonicalStringIndexCache;
  scr::ScrVarCanonicalName_t parentNameCanonicalStringIndexCache;
};

struct AnimSelectorTableEntry;

struct AnimSelectorTableRow {
  AnimSelectorTableEntry *entries;
  int32_t numEntries;
  uint8_t _padding0C[4];
};
ASSERT_SIZE(AnimSelectorTableRow, 0x10);

struct AnimSelectorTable {
  scr::ScrString_t name;
  uint8_t _padding04[4];
  AnimSelectorTableColumn *columns;
  int32_t numColumns;
  uint8_t _padding14[4];
  AnimSelectorTableRow *rows;
  int32_t numRows;
  union {
    AnimSelectorOutParamsGroup1 outParamsGroup1;
    AnimSelectorOutParamsGroup2 outParamsGroup2;
    AnimSelectorOutParamsGroup3 outParamsGroup3;
  };
  int32_t blendInColumn;
  int32_t blendOutColumn;
  int32_t aimTableColumn;
};
ASSERT_SIZE(AnimSelectorTable, 0x50);

struct AnimSelectorTableSet {
  const char *name;
  AnimSelectorTable *tables;
  int32_t numTables;
  uint8_t _padding14[4];
};
ASSERT_SIZE(AnimSelectorTableSet, 0x18);

typedef AnimSelectorTableSet *AnimSelectorTableSetPtr;

struct AimTableEntry {
  const char *name;
  scr::ScrString_t entryName;
  int32_t right;
  int32_t left;
  int32_t up;
  int32_t down;
  uint8_t _padding1C[4];
};
ASSERT_SIZE(AimTableEntry, 0x20);

// sizeof=0x8
struct PrimaryDeltaFootstep_t {
  float time;
  scr::ScrString_t notetrackName;
};
ASSERT_SIZE(PrimaryDeltaFootstep_t, 0x8);

typedef int32_t paramAnimIndexes[7];
typedef scr::ScrString_t paramAnimAliases[7];
typedef int32_t aimAnimationsArray_t[4];
typedef float aimLimits_t[4];

// sizeof=10
struct aimAnimations_t {
  union {
    struct {
      int32_t xanimUpAimIndex;
      int32_t xanimDownAimIndex;
      int32_t xanimLeftAimIndex;
      int32_t xanimRightAimIndex;
    };
    aimAnimationsArray_t aimArray;
  };
};
ASSERT_SIZE(aimAnimations_t, 0x10);

// sizeof=0x98
struct AimLayer {
  bool active;
  uint8_t _padding01[3];
  scr::ScrString_t tableName;
  scr::ScrString_t searchedAimTableName;
  int32_t tableIndex;
  int32_t rowIndex;
  int32_t lastValidEnemyTime;
  vec3_t targetPos;
  vec3_t prevAimTagAngles;
  vec3_t prevAimDeltaAngles;
  vec3_t currAimDeltaAngles;
  bool prevAimsMultipleDelta;
  uint8_t _padding49[3];
  aimAnimations_t aimAnimIndexes;
  aimAnimations_t prevAimAnimIndexes;
  aimAnimations_t aimBlendIndexes;
  aimLimits_t aimLimits;
  uint8_t _padding8C[4];
  const AimTableEntry *aimTableEntry;
};
ASSERT_SIZE(AimLayer, 0x98);

// sizeof=0x20C
struct PrimaryDeltaLayer_t {
  asmAnimSource_t deltaAnimSource;
  scr::ScrString_t currDeltaTableName;
  scr::ScrString_t currDeltaTableEvalFunc;
  int32_t currDeltaTableIndex;
  int32_t currDeltaTableRow;
  float currDeltaRate;
  union {
    int32_t currDeltaAnimIndex;
    paramAnimIndexes currDeltaParamAnimIndex;
  };
  union {
    scr::ScrString_t currDeltaAnimAlias;
    paramAnimAliases currDeltaParamAnimAlias;
  };
  float currDeltaParamWeights[7];
  float blendOutTime;
  float aimStartTime;
  float aimStopTime;
  float mocompStartTime;
  float mocompStopTime;
  int32_t footstepCount;
  PrimaryDeltaFootstep_t footsteps[48];
  int32_t deltaRequestTime;
  uint8_t toggleCounter;
  uint8_t _padding209[3];
};
ASSERT_SIZE(PrimaryDeltaLayer_t, 0x20C);

// sizeof=0x18
struct shootAnimations_t {
  int32_t xanimSemiIndex;
  int32_t xanimSingleShotIndex;
  int32_t xanimBurstIndexes[4];
};
ASSERT_SIZE(shootAnimations_t, 0x18);

// sizeof=0x44
struct ShootLayer {
  bool active;
  uint8_t _padding1[3];
  scr::ScrString_t tableName;
  int32_t tableIndex;
  int32_t rowIndex;
  shootAnimations_t shootAnimationIndexes;
  int32_t currentShootAnimationIndex;
  bool waitingForFirstShot;
  uint8_t _padding2D[3];
  int32_t activationDelayedTime;
  int32_t firingFlagStartTime;
  int32_t firingFlagStopTime;
  int32_t nextShootLayerStartTime;
  int32_t currentBurstCount;
};
ASSERT_SIZE(ShootLayer, 0x44);

// sizeof=0x18
struct TransDecLayer {
  bool active;
  uint8_t _padding1[3];
  scr::ScrString_t tableName;
  scr::ScrString_t tableEvalFunc;
  int32_t tableIndex;
  int32_t rowIndex;
  int32_t animIndex;
};
ASSERT_SIZE(TransDecLayer, 0x18);

struct MocompLayer {
  bool active;
  uint8_t _padding1[3];
  scr::ScrString_t mocompName;
};
ASSERT_SIZE(MocompLayer, 0x8);

// sizeof=0x290
struct AimTable {
  const char *name;
  scr::ScrString_t tableName;
  uint8_t _padding0C[4];
  AimTableEntry atEntries[20];
};
ASSERT_SIZE(AimTable, 0x290);

typedef AimLayer AimLayer_t;
typedef ShootLayer ShootLayer_t;
typedef TransDecLayer TransDecLayer_t;
typedef MocompLayer MocompLayer_t;

// sizeof=0x40
struct AnimSubState {
  scr::ScrString_t name;
  scr::ScrString_t fullname;
  int32_t validationFlags;
  int32_t subStateFlags;
  int32_t ownerStateIndex;
  scr::ScrString_t deltaSelector;
  scr::ScrString_t aimSelector;
  scr::ScrString_t shootSelector;
  scr::ScrString_t transitionSelector;
  scr::ScrString_t deltaSelectorEvaluator;
  scr::ScrString_t transitionSelectorEvaluator;
  uint8_t _padding2C[4];
  int32_t *substateTransitionIndices;
  int32_t numSubStateTransitions;
  uint8_t _padding3C[4];
};
ASSERT_SIZE(AnimSubState, 0x40);

// sizeof=0x18
struct AnimState {
  scr::ScrString_t name;
  int32_t validationFlags;
  int32_t *subStatesIndices;
  int32_t numSubStates;
  uint8_t _padding14[4];
};
ASSERT_SIZE(AnimState, 0x18);

// sizeof=0x2C
struct AnimTransition {
  scr::ScrString_t name;
  scr::ScrString_t fullname;
  int32_t validationFlags;
  int32_t targetStateIndex;
  int32_t targetSubStateIndex;
  scr::ScrString_t targetStateName;
  scr::ScrString_t targetSubStateName;
  scr::ScrString_t deltaSelector;
  scr::ScrString_t aimSelector;
  scr::ScrString_t shootSelector;
  scr::ScrString_t deltaSelectorEvaluator;
};
ASSERT_SIZE(AnimTransition, 0x2C);

// sizeof=0x38
struct AnimStateMachine {
  const char *name;
  AnimState *asmStates;
  int32_t numStates;
  uint8_t _padding14[4];
  AnimSubState *asmSubStates;
  int32_t numSubStates;
  uint8_t _padding24[4];
  AnimTransition *asmTransitions;
  int32_t numTransitions;
  uint8_t _padding34[4];
};
ASSERT_SIZE(AnimStateMachine, 0x38);
typedef AnimStateMachine *AnimStateMachinePtr;

// sizeof=0x330
struct AnimStateMachineRuntime {
  asmStatus_t status;
  asmSubstateEndReason_t endReason;
  const AnimStateMachine *animStateMachine;
  int32_t currSubStateIndex;
  int32_t currTransitionIndex;
  int32_t prevSubStateIndex;
  int32_t requestedSubStateIndex;
  float animationRate;
  PrimaryDeltaLayer_t primaryDeltaLayer;
  AimLayer_t aimLayer;
  ShootLayer_t shootLayer;
  TransDecLayer_t transDecLayer;
  MocompLayer_t mocompLayer;
  uint8_t _padding32C[4];
};
ASSERT_SIZE(AnimStateMachineRuntime, 0x330);

// sizeof = 0x14
struct XAnimParam {
  char strName[16];
  float fValue;
};
ASSERT_SIZE(XAnimParam, 0x14);

// sizeof=0x8
struct XAnimSearchEntry {
  scr::ScrString_t name;
  uint16_t animIndex;
  uint8_t _padding06[2];
};
ASSERT_SIZE(XAnimSearchEntry, 0x8);

struct XAnimParent {
  uint16_t flags;
  uint16_t children;
};

struct XAnimParts; // TODO

// sizeof=0x18
struct XAnimEntry {
  uint16_t bCreated;
  uint16_t numAnims;
  uint16_t firstParamIxPlusOne;
  uint16_t paramCount;
  uint16_t parent;
  uint8_t _padding0A[6];
  union {
    XAnimParts *parts;
    XAnimParent animParent;
  };
};
ASSERT_SIZE(XAnimEntry, 0x18);

struct XAnim {
  scr::ScrString_t name;
  int32_t size;
  uint32_t paramSize;
  XAnimParam *params;
  scr::ScrString_t *animNames;
  XAnimSearchEntry *sortedSearchList;
  XAnimEntry entries[1];
};

// sizeof=0x10
struct XAnimTree {
  XAnim *anims;
  uint32_t children;
  LocalClientNum_t localClientNum;
};
ASSERT_SIZE(XAnimTree, 0x10);

#pragma pack(pop)

#pragma pack(push, 1)
struct PlayerAnimBodyPartState {
  int32_t animStartTime;
  int32_t animDuration;
  animationNumber_t animNum;
  uint8_t _padding0A[2];
};
ASSERT_SIZE(PlayerAnimBodyPartState, 0xC);
#pragma pack(pop)

#pragma pack(push, 1)
struct AnimScriptedState {
  int32_t time;
  float rate;
  int32_t lerp;
  float goalTime;
  animationNumber_t anim;
  uint8_t _padding12[2];
};
ASSERT_SIZE(AnimScriptedState, 0x14);
#pragma pack(pop)

#pragma pack(push, 1)
struct motionMatchingInfo_t {
  qboolean active;
  float legOffsetAngle;
  int32_t kf_index;
  qboolean transitionState;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct locBlendInfo_t {
  bool bPlayedBlendLastFrame;
  bool bStraightenLegs;
  uint8_t _padding02[2];
  int32_t lastBlendCount;
  int32_t lastBlendAnims[4];
  int32_t lastStrideType;
  float blendLerpTime;
  animationNumber_t stopAnimNum;
  uint8_t _padding22[2];
  float stopBlendLerpTime;
  int32_t lastLocBlendAnimNum;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct animation_t {
  char name[64];
  int32_t initialLerp;
  int32_t finalLerp;
  float forceAnimRate;
  float moveSpeed;
  float rotSpeed;
  int32_t duration;
  int32_t nameHash;
  int32_t flags;
  int64_t movetype;
  int32_t stance;
  int32_t noteType;
  int32_t locBlendIndex;
  float locBlendAlignTime;
  uint16_t localOverrideAnim;
  uint8_t _padding7A[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct LerpFrameAnimation {
  animationNumber_t animationNumber;
  uint8_t _padding02[6];
  animation_t *animation;
  int32_t animationTime;
  vec3_t oldFramePos;
  float animSpeedScale;
  int32_t oldFrameSnapshotTime;
  qboolean scriptAnimFinished;
  int32_t syncAnimTime;
};
#pragma pack(pop)

struct BGModelAttachmentName {
  char model[64];
  char tag[64];
};

#pragma pack(push, 1)
struct LerpFrameAngles {
  float yawAngle;
  float yawAngleOld;
  qboolean yawing;
  float pitchAngle;
  qboolean pitching;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ModelAttachmentIndex {
  int16_t model;
  uint8_t tag;
  uint8_t _padding03[1];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct animState_t {
  int32_t deltaRequestTime;
  uint8_t deltaSource;
  uint8_t deltaRowLowBits;
  uint8_t deltaRowHighBits;
  uint8_t aimRow;
  uint8_t shootRow;
  uint8_t transdecActive;
  uint8_t currentAnimMappingTableIndex;
  uint8_t toggleCounter;
  float fAnimationRate;
  vec3_t traversalOrigin;
  float traversalYaw;
  int32_t traversalAnimIndex;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct animscripted_t {
  vec3_t axis[4];
  vec3_t originError;
  vec3_t originErrorReduction;
  vec3_t anglesError;
  vec3_t anglesErrorReduction;
  uint16_t anim;
  uint16_t root;
  uint8_t bStarted;
  uint8_t mode;
  uint8_t _padding66[2];
  int startTime;
  int lerpTime;
  float fHeightOfs;
  float fEndPitch;
  float fEndRoll;
  float fOrientLerp;
};
#pragma pack(pop)

struct TraversalAnim {
  int32_t animIndex;
  vec3_t origin;
  float yaw;
};

struct TraversalState {
  qboolean active;
  TraversalAnim anims[3];
  int32_t outroTimeLeft;
  vec3_t outroOrigin;
  vec3_t outroAngles;
};

struct ObjectiveAnimData {
  bool obscuredLastCheck;
  bool hiddenByGroupCulling;
};

struct DObjAnimMat {
  vec4_t quat;
  vec3_t trans;
  float transWeight;
};
struct BaseAnimBoneMat {
  BoneIndex boneIndex;
  bool ignoreParentRelativeData;
  bool primaryBone;
  vec4_t baseQuat;
  vec3_t baseTrans;
};
struct DSkelPartBits {
  int anim[12];
  int control[12];
  int skel[12];
  int worldQuat[12];
  int constrained[12];
};

struct __attribute__((aligned(4))) DSkel {
  DObjAnimMat *mat;
  BaseAnimBoneMat *baseAnimMat;
  float *blendTargetWeights;
  DSkelPartBits partBits;
  int timeStamp;
  bool dirty;
};

struct __attribute__((aligned(8))) DObj {
  XAnimTree *tree;
  uint8_t *ikState;
  gfx::GfxSiegeAnimState **siegeAnimations;
  db::xasset::XModel **models;
  volatile int64_t locked;
  DSkel skel;
  uint32_t hidePartBits[12];
  uint32_t hideSubModelBits;
  uint32_t hideSubModelShadowBits;
  scr::ScrString_t duplicatePartsRef;
  uint32_t ignoreCollision;
  float radius;
  float pitchAngle;
  float scale;
  uint16_t entnum;
  uint16_t numCosmeticBones;
  uint16_t numSiegeAnimations;
  uint16_t duplicatePartsSize;
  uint16_t flags;
  BoneIndex numBones;
  uint8_t numModels;
  uint8_t numConstrainedBones;
  char localClientIndex;
  uint8_t numRootAdjustMats;
};

struct DObjModel {
  db::xasset::XModel *model;
  scr::ScrString_t boneName;
  uint16_t parentModel;
  bool ignoreCollision;
  vec3_t trans;
  vec3_t rot;
};

enum class link_type_e : uint32_t {
  PITCH_CTRL = 0x0,
  YAW_CTRL = 0x1,
  ROLL_CTRL = 0x2,
  SWIM_CTRL = 0x3,
  FULL_CTRL = 0x4,
  COMPASS_CTRL = 0x5,
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
  scr::ScrString_t type;
  float startTime;
  float endTime;
  uint8_t persistent;
  scr::ScrString_t param1;
  scr::ScrString_t param2;
};

struct XAnimNotifyInfo {
  scr::ScrString_t type;
  float time;
  scr::ScrString_t param1;
  scr::ScrString_t param2;
  scr::ScrString_t param3;
};

struct __attribute__((aligned(4))) BlendTargetRemap {
  scr::ScrString_t name;
  uint16_t index;
};

union XAnimIndices {
  uint8_t *_1;
  uint16_t *_2;
  void *data;
};

struct XAnimParts {
  BlendTargetRemap *blendTargetRemap;
  scr::ScrString_t *blendTargetNames;
  scr::ScrString_t *names;
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
  scr::ScrString_t branchName;
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
  scr::ScrString_t tagName;
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

struct __attribute__((aligned(4))) ViewModelHand {
  int32_t iHandAnimIndex;
  animationNumber_t iPrevAnim;
};

struct __attribute__((aligned(8))) ViewModelInfo {
  DObj *viewModelDObj;
  DObj *viewModelDObjLegs;
  weapon::WeaponDobjInfo weaponDobjInfo;
  bool hasAnimTree;
  XAnimTree *tree;
  XAnim *anims;
  unsigned int partBits[12];
  ViewModelHand hand[2];
  db::xasset::XModelPtr altModels[4];
  uint32_t numAltModels;
};

} // namespace anim
} // namespace game