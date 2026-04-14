#pragma once
#include "core.hpp"
#include "quake.hpp"
#include "scr.hpp"

namespace game {
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
static_assert(sizeof(RumbleGraph) == 0x90,
              "RumbleGraph size must be 144 bytes");

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
static_assert(sizeof(RumbleInfo) == 0x40, "RumbleInfo size must be 64 bytes");

typedef RumbleInfo *RumbleInfoPtr;

// sizeof=0x18
struct AliasToAnimsEntry {
  scr::ScrString_t aliasName;
  uint8_t _padding04[4];
  scr::ScrString_t *anims;
  int32_t numAnims;
  uint8_t _padding14[4];
};
static_assert(sizeof(AliasToAnimsEntry) == 0x18,
              "AliasToAnimsEntry size must be 24 bytes");

// sizeof=0x8
struct AnimSelectorTableEntry {
  scr::ScrString_t name;
  union {
    uint32_t _data;
    int32_t integer;
    float v;
  };
};
static_assert(sizeof(AnimSelectorTableEntry) == 0x8,
              "AnimSelectorTableEntry size must be 8 bytes");

// sizeof=0x18
struct AnimMappingTable {
  const char *name;
  AliasToAnimsEntry *entries;
  int32_t numAnimAlias;
  uint8_t _padding14[4];
};
static_assert(sizeof(AnimMappingTable) == 0x18,
              "AnimMappingTable size must be 24 bytes");
typedef AnimMappingTable *AnimMappingTablePtr;

// sizeof=0x10C
struct AnimSelectorTableCache {
  int32_t tableIndex;
  int32_t rowIndex;
  AnimSelectorTableEntry searchCriteria[32];
  int32_t searchCriteriaSize;
};
static_assert(sizeof(AnimSelectorTableCache) == 0x10C,
              "AnimSelectorTableCache size must be 268 bytes");

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

// sizeof=0x10
struct AnimSelectorTableRow {
  AnimSelectorTableEntry *entries;
  int32_t numEntries;
  uint8_t _padding0C[4];
};
static_assert(sizeof(AnimSelectorTableRow) == 0x10,
              "AnimSelectorTableRow size must be 16 bytes");

// sizeof=0x50
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
static_assert(sizeof(AnimSelectorTable) == 0x50,
              "AnimSelectorTable size must be 80 bytes");

// sizeof=0x18
struct AnimSelectorTableSet {
  const char *name;
  AnimSelectorTable *tables;
  int32_t numTables;
  uint8_t _padding14[4];
};
static_assert(sizeof(AnimSelectorTableSet) == 0x18,
              "AnimSelectorTableSet size must be 24 bytes");
typedef AnimSelectorTableSet *AnimSelectorTableSetPtr;

// sizeof=0x20
struct AimTableEntry {
  const char *name;
  scr::ScrString_t entryName;
  int32_t right;
  int32_t left;
  int32_t up;
  int32_t down;
  uint8_t _padding1C[4];
};
static_assert(sizeof(AimTableEntry) == 0x20,
              "AimTableEntry size must be 32 bytes");

// sizeof=0x8
struct PrimaryDeltaFootstep_t {
  float time;
  scr::ScrString_t notetrackName;
};
static_assert(sizeof(PrimaryDeltaFootstep_t) == 0x8,
              "PrimaryDeltaFootstep_t size must be 8 bytes");

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
static_assert(sizeof(aimAnimations_t) == 0x10,
              "aimAnimations_t size must be 16 bytes");

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
static_assert(sizeof(AimLayer) == 0x98, "AimLayer size must be 152 bytes");

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
static_assert(sizeof(PrimaryDeltaLayer_t) == 0x20C,
              "PrimaryDeltaLayer_t size must be 524 bytes");

// sizeof=0x18
struct shootAnimations_t {
  int32_t xanimSemiIndex;
  int32_t xanimSingleShotIndex;
  int32_t xanimBurstIndexes[4];
};
static_assert(sizeof(shootAnimations_t) == 0x18,
              "shootAnimations_t size must be 24 bytes");

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
static_assert(sizeof(ShootLayer) == 0x44, "ShootLayer size must be 68 bytes");

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
static_assert(sizeof(TransDecLayer) == 0x18,
              "TransDecLayer size must be 24 bytes");

struct MocompLayer {
  bool active;
  uint8_t _padding1[3];
  scr::ScrString_t mocompName;
};
static_assert(sizeof(MocompLayer) == 0x8, "MocompLayer size must be 8 bytes");

// sizeof=0x290
struct AimTable {
  const char *name;
  scr::ScrString_t tableName;
  uint8_t _padding0C[4];
  AimTableEntry atEntries[20];
};
static_assert(sizeof(AimTable) == 0x290, "AimTable size must be 656 bytes");

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
static_assert(sizeof(AnimSubState) == 0x40,
              "AnimSubState size must be 64 bytes");

// sizeof=0x18
struct AnimState {
  scr::ScrString_t name;
  int validationFlags;
  int *subStatesIndices;
  int numSubStates;
  uint8_t _padding14[4];
};
static_assert(sizeof(AnimState) == 0x18, "AnimState size must be 24 bytes");

// sizeof=0x2C
struct AnimTransition {
  scr::ScrString_t name;
  scr::ScrString_t fullname;
  int validationFlags;
  int targetStateIndex;
  int targetSubStateIndex;
  scr::ScrString_t targetStateName;
  scr::ScrString_t targetSubStateName;
  scr::ScrString_t deltaSelector;
  scr::ScrString_t aimSelector;
  scr::ScrString_t shootSelector;
  scr::ScrString_t deltaSelectorEvaluator;
};
static_assert(sizeof(AnimTransition) == 0x2C,
              "AnimTransition size must be 44 bytes");

// sizeof=0x38
struct AnimStateMachine {
  const char *name;
  AnimState *asmStates;
  int numStates;
  uint8_t _padding14[4];
  AnimSubState *asmSubStates;
  int numSubStates;
  uint8_t _padding24[4];
  AnimTransition *asmTransitions;
  int numTransitions;
  uint8_t _padding34[4];
};
static_assert(sizeof(AnimStateMachine) == 0x38,
              "AnimStateMachine size must be 56 bytes");
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
static_assert(sizeof(AnimStateMachineRuntime) == 0x330,
              "AnimStateMachineRuntime size must be 816 bytes");

// sizeof = 0x14
struct XAnimParam {
  char strName[16];
  float fValue;
};
static_assert(sizeof(XAnimParam) == 0x14, "XAnimParam size must be 20 bytes");

// sizeof=0x8
struct XAnimSearchEntry {
  scr::ScrString_t name;
  uint16_t animIndex;
  uint8_t _padding06[2];
};
static_assert(sizeof(XAnimSearchEntry) == 0x8,
              "XAnimSearchEntry size must be 8 bytes");

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
static_assert(sizeof(XAnimEntry) == 0x18, "XAnimEntry size must be 24 bytes");

struct XAnim {
  scr::ScrString_t name;
  int32_t size;
  unsigned int paramSize;
  XAnimParam *params;
  scr::ScrString_t *animNames;
  XAnimSearchEntry *sortedSearchList;
  XAnimEntry entries[1];
};

// sizeof=0x10
struct XAnimTree {
  XAnim *anims;
  unsigned int children;
  LocalClientNum_t localClientNum;
};
static_assert(sizeof(XAnimTree) == 0x10, "XAnimTree size must be 16 bytes");

#pragma pack(pop)
} // namespace anim
} // namespace game