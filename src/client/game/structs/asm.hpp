#pragma once
#include "core.hpp"
#include "db/xasset/xmodel.hpp"
#include "fp16.hpp"
#include "quake/core.hpp"
#include "scr/primitives.hpp"
#include "weapon.hpp"

namespace game {

namespace snd {
struct SndAliasList;
}

// asm reserved
namespace anim {

#pragma pack(push, 1)

enum class StateMachineStatus : int32_t {
  INVALID = 0x0,
  DEFAULT = 0x0,
  UNUSED1 = 0x1,
  UNUSED2 = 0x2,
  FAILURE = 0x3,
  SUCCESS = 0x4,
  RUNNING = 0x5,
  VALID = 0x6,
  TERMINATE = 0x7,
  STATUS_COUNT = 0x8,
};
IMPL_ENUM_OPERATORS(StateMachineStatus);

inline constexpr const char *serialize(const StateMachineStatus status) {
  switch (status) {
  case StateMachineStatus::INVALID:
    return "STATUS_INVALID";
  case StateMachineStatus::UNUSED1:
    return "STATUS_UNUSED1";
  case StateMachineStatus::UNUSED2:
    return "STATUS_UNUSED2";
  case StateMachineStatus::FAILURE:
    return "STATUS_FAILURE";
  case StateMachineStatus::SUCCESS:
    return "STATUS_SUCCESS";
  case StateMachineStatus::RUNNING:
    return "STATUS_RUNNING";
  case StateMachineStatus::VALID:
    return "STATUS_VALID";
  case StateMachineStatus::TERMINATE:
    return "STATUS_TERMINATE";
  default:
    return "STATUS_UNKNOWN";
  }
}

enum class aiPathMode : int32_t {
  MODE_INVALID = 0x0,
  DONT_MOVE = 0x1,
  MOVE_ALLOWED = 0x2,
  MOVE_DELAYED = 0x3,
};
IMPL_ENUM_OPERATORS(aiPathMode);

inline constexpr const char *serialize(const aiPathMode mode) {
  switch (mode) {
  case aiPathMode::MODE_INVALID:
    return "AIMODE_INVALID";
  case aiPathMode::DONT_MOVE:
    return "AIMODE_DONT_MOVE";
  case aiPathMode::MOVE_ALLOWED:
    return "AIMODE_MOVE_ALLOWED";
  case aiPathMode::MOVE_DELAYED:
    return "AIMODE_MOVE_DELAYED";
  default:
    return "AIMODE_UNKNOWN";
  }
}

enum class aiGoalSources : int32_t {
  SCRIPT_GOAL = 0x0,
  SCRIPT_ENTITY_GOAL = 0x1,
  SCRIPT_FORCED_GOAL = 0x2,
  ENEMY = 0x3,
};
IMPL_ENUM_OPERATORS(aiGoalSources);

inline constexpr const char *serialize(const aiGoalSources source) {
  switch (source) {
  case aiGoalSources::SCRIPT_GOAL:
    return "GOALSRC_SCRIPT_GOAL";
  case aiGoalSources::SCRIPT_ENTITY_GOAL:
    return "GOALSRC_SCRIPT_ENTITY_GOAL";
  case aiGoalSources::SCRIPT_FORCED_GOAL:
    return "GOALSRC_SCRIPT_FORCED_GOAL";
  case aiGoalSources::ENEMY:
    return "GOALSRC_ENEMY";
  default:
    return "GOALSRC_UNKNOWN";
  }
}

enum class asmAnimSource_t : int32_t {
  NONE = 0x0,
  SUBSTATE = 0x1,
  TRANSITION = 0x2,
  TRANSDEC = 0x3,
  EXTERNAL = 0x4,
  COUNT = 0x5,
};
IMPL_ENUM_OPERATORS(asmAnimSource_t);

inline constexpr const char *serialize(const asmAnimSource_t src) {
  switch (src) {
  case asmAnimSource_t::NONE:
    return "ANIMSRC_NONE";
  case asmAnimSource_t::SUBSTATE:
    return "ANIMSRC_SUBSTATE";
  case asmAnimSource_t::TRANSITION:
    return "ANIMSRC_TRANSITION";
  case asmAnimSource_t::TRANSDEC:
    return "ANIMSRC_TRANSDEC";
  case asmAnimSource_t::EXTERNAL:
    return "ANIMSRC_EXTERNAL";
  default:
    return "ANIMSRC_UNKNOWN";
  }
}

enum class asmSubstateEndReason_t : int32_t {
  NONE = 0x0,
  SUBSTATE = 0x1,
  TRANSITION = 0x2,
  TRANSDEC = 0x3,
  NEW_REQUEST = 0x4,
  EXTERNAL = 0x5,
  COUNT = 0x6,
};
IMPL_ENUM_OPERATORS(asmSubstateEndReason_t);

inline constexpr const char *serialize(const asmSubstateEndReason_t reason) {
  switch (reason) {
  case asmSubstateEndReason_t::NONE:
    return "ENDREASON_NONE";
  case asmSubstateEndReason_t::SUBSTATE:
    return "ENDREASON_SUBSTATE";
  case asmSubstateEndReason_t::TRANSITION:
    return "ENDREASON_TRANSITION";
  case asmSubstateEndReason_t::TRANSDEC:
    return "ENDREASON_TRANSDEC";
  case asmSubstateEndReason_t::NEW_REQUEST:
    return "ENDREASON_NEW_REQUEST";
  case asmSubstateEndReason_t::EXTERNAL:
    return "ENDREASON_EXTERNAL";
  default:
    return "ENDREASON_UNKNOWN";
  }
}

enum class asmStatus_t : int32_t {
  INACTIVE = 0x0,
  RUNNING = 0x1,
  COMPLETE = 0x2,
  TERMINATED = 0x3,
  COUNT = 0x4,
};
IMPL_ENUM_OPERATORS(asmStatus_t);

inline constexpr const char *serialize(const asmStatus_t status) {
  switch (status) {
  case asmStatus_t::INACTIVE:
    return "ASMSTATUS_INACTIVE";
  case asmStatus_t::RUNNING:
    return "ASMSTATUS_RUNNING";
  case asmStatus_t::COMPLETE:
    return "ASMSTATUS_COMPLETE";
  case asmStatus_t::TERMINATED:
    return "ASMSTATUS_TERMINATED";
  default:
    return "ASMSTATUS_UNKNOWN";
  }
}

// sizeof=0x90
struct RumbleGraph {
  const char *name;
  int32_t knotCount;
  vec2_t knots[16];
  uint8_t _padding8C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"knotCount\": {}, \"knots\": {} }}",
                       ::serialize(name), knotCount, ::serialize(knots));
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"duration\": {}, \"range\": {}, "
        "\"highRumbleGraph\": {}, "
        "\"lowRumbleGraph\": {}, \"fadeWithDistance\": {}, \"broadcast\": "
        "{}, "
        "\"camShakeScale\": {}, \"camShakeDuration\": {}, \"camShakeRange\": "
        "{}, "
        "\"pulseRadiusOuter\": {}, \"pulseScale\": {}, \"pulseBoneTag\": "
        "{:08X} }}",
        ::serialize(name), duration, range, ::serialize(highRumbleGraph),
        ::serialize(lowRumbleGraph), fadeWithDistance.serialize(),
        broadcast.serialize(), camShakeScale, camShakeDuration, camShakeRange,
        pulseRadiusOuter, pulseScale, pulseBoneTag);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"aliasName\": 0x{:08X}, \"anims\": {}, \"numAnims\": {} }}",
        aliasName, ::serialize(anims, numAnims), numAnims);
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": 0x{:08X}, \"integer\": {}, \"v\": {} }}",
                       name, integer, v);
  }
};
ASSERT_SIZE(AnimSelectorTableEntry, 0x8);

// sizeof=0x18
struct AnimMappingTable {
  const char *name;
  AliasToAnimsEntry *entries;
  int32_t numAnimAlias;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"entries\": {}, \"numAnimAlias\": {} }}",
        ::serialize(name), ::serialize(entries, numAnimAlias), numAnimAlias);
  }
};
ASSERT_SIZE(AnimMappingTable, 0x18);
typedef AnimMappingTable *AnimMappingTablePtr;

// sizeof=0x10C
struct AnimSelectorTableCache {
  int32_t tableIndex;
  int32_t rowIndex;
  AnimSelectorTableEntry searchCriteria[32];
  int32_t searchCriteriaSize;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"tableIndex\": {}, \"rowIndex\": {}, "
                       "\"searchCriteria\": {}, \"searchCriteriaSize\": {} }}",
                       tableIndex, rowIndex, ::serialize(searchCriteria),
                       searchCriteriaSize);
  }
};
ASSERT_SIZE(AnimSelectorTableCache, 0x10C);

struct AnimSelectorOutParamsGroup1 {
  int32_t aliasColumn;
  int32_t aimUpAliasColumn;
  int32_t aimDownAliasColumn;
  int32_t aimLeftAliasColumn;
  int32_t aimRightAliasColumn;
  int32_t mocompColumn;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"aliasColumn\": {}, \"aimUpAliasColumn\": {}, "
                       "\"aimDownAliasColumn\": {}, "
                       "\"aimLeftAliasColumn\": {}, \"aimRightAliasColumn\": "
                       "{}, \"mocompColumn\": {} }}",
                       aliasColumn, aimUpAliasColumn, aimDownAliasColumn,
                       aimLeftAliasColumn, aimRightAliasColumn, mocompColumn);
  }
};

struct AnimSelectorOutParamsGroup2 {
  int32_t shootSemiAliasColumn;
  int32_t shootSingleAliasColumn;
  int32_t shootBurst3AliasColumn;
  int32_t shootBurst4AliasColumn;
  int32_t shootBurst5AliasColumn;
  int32_t shootBurst6AliasColumn;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"shootSemiAliasColumn\": {}, \"shootSingleAliasColumn\": {}, "
        "\"shootBurst3AliasColumn\": {}, "
        "\"shootBurst4AliasColumn\": {}, \"shootBurst5AliasColumn\": {}, "
        "\"shootBurst6AliasColumn\": {} }}",
        shootSemiAliasColumn, shootSingleAliasColumn, shootBurst3AliasColumn,
        shootBurst4AliasColumn, shootBurst5AliasColumn, shootBurst6AliasColumn);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"paramForwardAliasColumn\": {}, \"paramBackwardAliasColumn\": {}, "
        "\"paramLeftAliasColumn\": {}, "
        "\"paramRightAliasColumn\": {}, \"paramBalanceAliasColumn\": {}, "
        "\"paramTurnLeftAliasColumn\": {}, "
        "\"paramTurnRightAliasColumn\": {}, \"paramIdleBlendDropoffColumn\": "
        "{} }}",
        paramForwardAliasColumn, paramBackwardAliasColumn, paramLeftAliasColumn,
        paramRightAliasColumn, paramBalanceAliasColumn,
        paramTurnLeftAliasColumn, paramTurnRightAliasColumn,
        paramIdleBlendDropoffColumn);
  }
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

inline constexpr const char *
serialize(const animSelectorColumnDataType_t dtype) {
  switch (dtype) {
  case animSelectorColumnDataType_t::AST_ENUM_TYPE:
    return "AST_ENUM_TYPE";
  case animSelectorColumnDataType_t::AST_FLOAT_TYPE:
    return "AST_FLOAT_TYPE";
  case animSelectorColumnDataType_t::AST_INT_TYPE:
    return "AST_INT_TYPE";
  case animSelectorColumnDataType_t::AST_MIN_FLOAT_TYPE:
    return "AST_MIN_FLOAT_TYPE";
  case animSelectorColumnDataType_t::AST_MAX_FLOAT_TYPE:
    return "AST_MAX_FLOAT_TYPE";
  case animSelectorColumnDataType_t::AST_RANGE_TYPE:
    return "AST_RANGE_TYPE";
  case animSelectorColumnDataType_t::AST_STRING_TYPE:
    return "AST_STRING_TYPE";
  default:
    return "AST_INVALID_DATA_TYPE";
  }
}

struct AnimSelectorTableColumn {
  scr::ScrString_t name;
  scr::ScrString_t parentName;
  bool desired;
  bool outputColumn;
  bool aliasColumn;
  animSelectorColumnDataType_t dataType;
  scr::ScrVarCanonicalName_t nameCanonicalStringIndexCache;
  scr::ScrVarCanonicalName_t parentNameCanonicalStringIndexCache;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"parentName\": {}, \"desired\": {}, "
        "\"outputColumn\": {}, "
        "\"aliasColumn\": {}, \"dataType\": {}, "
        "\"nameCanonicalStringIndexCache\": 0x{:08X}, "
        "\"parentNameCanonicalStringIndexCache\": 0x{:08X} }}",
        name, parentName, ::serialize(desired), ::serialize(outputColumn),
        ::serialize(aliasColumn), game::anim::serialize(dataType),
        nameCanonicalStringIndexCache, parentNameCanonicalStringIndexCache);
  }
};

struct AnimSelectorTableEntry;

struct AnimSelectorTableRow {
  AnimSelectorTableEntry *entries;
  int32_t numEntries;
  uint8_t _padding0C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"entries\": {}, \"numEntries\": {} }}",
                       ::serialize(entries, numEntries), numEntries);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": 0x{:08X}, \"columns\": {}, \"numColumns\": {}, \"rows\": "
        "{:p}, "
        "\"numRows\": {}, \"outParamsGroup1\": {}, \"blendInColumn\": {}, "
        "\"blendOutColumn\": {}, \"aimTableColumn\": {} }}",
        name, ::serialize(columns, numColumns), numColumns, (const void *)rows,
        numRows, outParamsGroup1.serialize(), blendInColumn, blendOutColumn,
        aimTableColumn);
  }
};
ASSERT_SIZE(AnimSelectorTable, 0x50);

struct AnimSelectorTableSet {
  const char *name;
  AnimSelectorTable *tables;
  int32_t numTables;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"tables\": {}, \"numTables\": {} }}",
                       ::serialize(name), ::serialize(tables, numTables),
                       numTables);
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"entryName\": {}, \"right\": {}, "
                       "\"left\": {}, \"up\": {}, \"down\": {} }}",
                       ::serialize(name), entryName, right, left, up, down);
  }
};
ASSERT_SIZE(AimTableEntry, 0x20);

// sizeof=0x8
struct PrimaryDeltaFootstep_t {
  float time;
  scr::ScrString_t notetrackName;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"time\": {}, \"notetrackName\": {} }}", time,
                       notetrackName);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"xanimUpAimIndex\": {}, \"xanimDownAimIndex\": {}, "
        "\"xanimLeftAimIndex\": {}, \"xanimRightAimIndex\": {} }}",
        xanimUpAimIndex, xanimDownAimIndex, xanimLeftAimIndex,
        xanimRightAimIndex);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"active\": {}, \"tableName\": {}, \"searchedAimTableName\": {}, "
        "\"tableIndex\": {}, \"rowIndex\": {}, \"lastValidEnemyTime\": {}, "
        "\"targetPos\": {}, \"prevAimTagAngles\": {}, \"prevAimDeltaAngles\": "
        "{}, "
        "\"currAimDeltaAngles\": {}, \"prevAimsMultipleDelta\": {}, "
        "\"aimAnimIndexes\": {}, "
        "\"prevAimAnimIndexes\": {}, \"aimBlendIndexes\": {}, \"aimLimits\": "
        "{}, "
        "\"aimTableEntry\": {} }}",
        ::serialize(active), tableName, searchedAimTableName, tableIndex,
        rowIndex, lastValidEnemyTime, targetPos.serialize(),
        prevAimTagAngles.serialize(), prevAimDeltaAngles.serialize(),
        currAimDeltaAngles.serialize(), ::serialize(prevAimsMultipleDelta),
        aimAnimIndexes.serialize(), prevAimAnimIndexes.serialize(),
        aimBlendIndexes.serialize(), ::serialize(aimLimits),
        ::serialize(aimTableEntry));
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"deltaAnimSource\": {}, \"currDeltaTableName\": {}, "
        "\"currDeltaTableEvalFunc\": {}, "
        "\"currDeltaTableIndex\": {}, \"currDeltaTableRow\": {}, "
        "\"currDeltaRate\": {}, "
        "\"currDeltaAnimIndex\": {}, \"currDeltaAnimAlias\": 0x{:08X}, "
        "\"currDeltaParamWeights\": {}, "
        "\"blendOutTime\": {}, \"aimStartTime\": {}, \"aimStopTime\": {}, "
        "\"mocompStartTime\": {}, "
        "\"mocompStopTime\": {}, \"footstepCount\": {}, \"footsteps\": {}, "
        "\"deltaRequestTime\": {}, "
        "\"toggleCounter\": {} }}",
        game::anim::serialize(deltaAnimSource), currDeltaTableName,
        currDeltaTableEvalFunc, currDeltaTableIndex, currDeltaTableRow,
        currDeltaRate, currDeltaAnimIndex, currDeltaAnimAlias,
        ::serialize(currDeltaParamWeights), blendOutTime, aimStartTime,
        aimStopTime, mocompStartTime, mocompStopTime, footstepCount,
        ::serialize(footsteps), deltaRequestTime, toggleCounter);
  }
};
ASSERT_SIZE(PrimaryDeltaLayer_t, 0x20C);

// sizeof=0x18
struct shootAnimations_t {
  int32_t xanimSemiIndex;
  int32_t xanimSingleShotIndex;
  int32_t xanimBurstIndexes[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"xanimSemiIndex\": {}, \"xanimSingleShotIndex\": "
                       "{}, \"xanimBurstIndexes\": {} }}",
                       xanimSemiIndex, xanimSingleShotIndex,
                       ::serialize(xanimBurstIndexes));
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"active\": {}, \"tableName\": {}, \"tableIndex\": {}, "
        "\"rowIndex\": {}, "
        "\"shootAnimationIndexes\": {}, \"currentShootAnimationIndex\": {}, "
        "\"waitingForFirstShot\": {}, "
        "\"activationDelayedTime\": {}, \"firingFlagStartTime\": {}, "
        "\"firingFlagStopTime\": {}, "
        "\"nextShootLayerStartTime\": {}, \"currentBurstCount\": {} }}",
        ::serialize(active), tableName, tableIndex, rowIndex,
        shootAnimationIndexes.serialize(), currentShootAnimationIndex,
        ::serialize(waitingForFirstShot), activationDelayedTime,
        firingFlagStartTime, firingFlagStopTime, nextShootLayerStartTime,
        currentBurstCount);
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"active\": {}, \"tableName\": {}, "
                       "\"tableEvalFunc\": {}, \"tableIndex\": {}, "
                       "\"rowIndex\": {}, \"animIndex\": {} }}",
                       ::serialize(active), tableName, tableEvalFunc,
                       tableIndex, rowIndex, animIndex);
  }
};
ASSERT_SIZE(TransDecLayer, 0x18);

struct MocompLayer {
  bool active;
  uint8_t _padding1[3];
  scr::ScrString_t mocompName;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"active\": {}, \"mocompName\": 0x{:08X} }}",
                       ::serialize(active), mocompName);
  }
};
ASSERT_SIZE(MocompLayer, 0x8);

// sizeof=0x290
struct AimTable {
  const char *name;
  scr::ScrString_t tableName;
  uint8_t _padding0C[4];
  AimTableEntry atEntries[20];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"tableName\": 0x{:08X}, \"atEntries\": {} }}",
        ::serialize(name), tableName, ::serialize(atEntries));
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": 0x{:08X}, \"fullname\": 0x{:08X}, \"validationFlags\": "
        "0x{:08X}, "
        "\"subStateFlags\": 0x{:08X}, "
        "\"ownerStateIndex\": {}, \"deltaSelector\": 0x{:08X}, "
        "\"aimSelector\": "
        "{:08X}, "
        "\"shootSelector\": 0x{:08X}, "
        "\"transitionSelector\": 0x{:08X}, \"deltaSelectorEvaluator\": "
        "0x{:08X}, "
        "\"transitionSelectorEvaluator\": 0x{:08X}, "
        "\"substateTransitionIndices\": {}, \"numSubStateTransitions\": {} "
        "}}",
        name, fullname, validationFlags, subStateFlags, ownerStateIndex,
        deltaSelector, aimSelector, shootSelector, transitionSelector,
        deltaSelectorEvaluator, transitionSelectorEvaluator,
        ::serialize(substateTransitionIndices, numSubStateTransitions),
        numSubStateTransitions);
  }
};
ASSERT_SIZE(AnimSubState, 0x40);

// sizeof=0x18
struct AnimState {
  scr::ScrString_t name;
  int32_t validationFlags;
  int32_t *subStatesIndices;
  int32_t numSubStates;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": 0x{:08X}, \"validationFlags\": 0x{:08X}, "
                       "\"subStatesIndices\": {}, \"numSubStates\": {} }}",
                       name, validationFlags,
                       ::serialize(subStatesIndices, numSubStates),
                       numSubStates);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": 0x{:08X}, \"fullname\": 0x{:08X}, \"validationFlags\": "
        "{:08X}, "
        "\"targetStateIndex\": {}, "
        "\"targetSubStateIndex\": {}, \"targetStateName\": 0x{:08X}, "
        "\"targetSubStateName\": 0x{:08X}, "
        "\"deltaSelector\": 0x{:08X}, \"aimSelector\": 0x{:08X}, "
        "\"shootSelector\": 0x{:08X}, "
        "\"deltaSelectorEvaluator\": 0x{:08X} }}",
        name, fullname, validationFlags, targetStateIndex, targetSubStateIndex,
        targetStateName, targetSubStateName, deltaSelector, aimSelector,
        shootSelector, deltaSelectorEvaluator);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"asmStates\": {}, \"numStates\": "
        "{}, \"asmSubStates\": {}, "
        "\"numSubStates\": {}, \"asmTransitions\": {}, "
        "\"numTransitions\": {} }}",
        ::serialize(name), ::serialize(asmStates, numStates), numStates,
        ::serialize(asmSubStates, numSubStates), numSubStates,
        ::serialize(asmTransitions, numTransitions), numTransitions);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"status\": {}, \"endReason\": {}, "
        "\"animStateMachine\": {}, \"currSubStateIndex\": {}, "
        "\"currTransitionIndex\": {}, \"prevSubStateIndex\": "
        "{}, \"requestedSubStateIndex\": {}, "
        "\"animationRate\": {}, \"primaryDeltaLayer\": {}, "
        "\"aimLayer\": {}, \"shootLayer\": {}, "
        "\"transDecLayer\": {}, \"mocompLayer\": {} }}",
        game::anim::serialize(status), game::anim::serialize(endReason),
        ::serialize(animStateMachine), currSubStateIndex, currTransitionIndex,
        prevSubStateIndex, requestedSubStateIndex, animationRate,
        primaryDeltaLayer.serialize(), aimLayer.serialize(),
        shootLayer.serialize(), transDecLayer.serialize(),
        mocompLayer.serialize());
  }
};
ASSERT_SIZE(AnimStateMachineRuntime, 0x330);

// sizeof = 0x14
struct XAnimParam {
  char strName[16];
  float fValue;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"strName\": \"{}\", \"fValue\": {} }}",
                       ::serialize(strName), fValue);
  }
};
ASSERT_SIZE(XAnimParam, 0x14);

// sizeof=0x8
struct XAnimSearchEntry {
  scr::ScrString_t name;
  uint16_t animIndex;
  uint8_t _padding06[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": 0x{:08X}, \"animIndex\": {} }}", name,
                       animIndex);
  }
};
ASSERT_SIZE(XAnimSearchEntry, 0x8);

struct XAnimParent {
  uint16_t flags;
  uint16_t children;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"flags\": 0x{:04X}, \"children\": {} }}", flags,
                       children);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"bCreated\": {}, \"numAnims\": {}, \"firstParamIxPlusOne\": {}, "
        "\"paramCount\": {}, \"parent\": {}, \"animParent\": {} }}",
        bCreated, numAnims, firstParamIxPlusOne, paramCount, parent,
        animParent.serialize());
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": 0x{:08X}, \"size\": {}, \"paramSize\": {}, \"params\": "
        "{:p}, "
        "\"animNames\": {:p}, \"sortedSearchList\": {:p}, \"entries\": {} }}",
        name, size, paramSize, reinterpret_cast<const void *>(params),
        reinterpret_cast<const void *>(animNames),
        reinterpret_cast<const void *>(sortedSearchList), ::serialize(entries));
  }
};

// sizeof=0x10
struct XAnimTree {
  XAnim *anims;
  uint32_t children;
  LocalClientNum_t localClientNum;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"anims\": {:p}, \"children\": {}, \"localClientNum\": {} }}",
        reinterpret_cast<const void *>(anims), children,
        game::serialize(localClientNum));
  }
};
ASSERT_SIZE(XAnimTree, 0x10);

#pragma pack(pop)

PACKED(struct PlayerAnimBodyPartState {
  int32_t animStartTime;
  int32_t animDuration;
  animationNumber_t animNum;
  uint8_t _padding0A[2];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"animStartTime\": {}, \"animDuration\": {}, \"animNum\": {} }}",
        animStartTime, animDuration, animNum.serialize());
  }
});
ASSERT_SIZE(PlayerAnimBodyPartState, 0xC);

PACKED(struct AnimScriptedState {
  int32_t time;
  float rate;
  int32_t lerp;
  float goalTime;
  animationNumber_t anim;
  uint8_t _padding12[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"time\": {}, \"rate\": {}, \"lerp\": {}, "
                       "\"goalTime\": {}, \"anim\": {} }}",
                       time, rate, lerp, goalTime, anim.serialize());
  }
});
ASSERT_SIZE(AnimScriptedState, 0x14);

PACKED(struct motionMatchingInfo_t {
  qboolean active;
  float legOffsetAngle;
  int32_t kf_index;
  qboolean transitionState;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"active\": {}, \"legOffsetAngle\": {}, "
                       "\"kf_index\": {}, \"transitionState\": {} }}",
                       active.serialize(), legOffsetAngle, kf_index,
                       transitionState.serialize());
  }
});

PACKED(struct locBlendInfo_t {
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"bPlayedBlendLastFrame\": {}, \"bStraightenLegs\": "
                       "{}, \"lastBlendCount\": {}, "
                       "\"lastBlendAnims\": {}, \"lastStrideType\": {}, "
                       "\"blendLerpTime\": {}, "
                       "\"stopAnimNum\": {}, \"stopBlendLerpTime\": {}, "
                       "\"lastLocBlendAnimNum\": {} }}",
                       ::serialize(bPlayedBlendLastFrame),
                       ::serialize(bStraightenLegs), lastBlendCount,
                       ::serialize(lastBlendAnims, lastBlendCount),
                       lastStrideType, blendLerpTime, stopAnimNum.serialize(),
                       stopBlendLerpTime, lastLocBlendAnimNum);
  }
)};

PACKED(struct animation_t {
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": \"{}\", \"initialLerp\": {}, "
                       "\"finalLerp\": {}, \"forceAnimRate\": {}, "
                       "\"moveSpeed\": {}, \"rotSpeed\": {}, \"duration\": {}, "
                       "\"nameHash\": 0x{:08X}, "
                       "\"flags\": 0x{:08X}, \"movetype\": {}, \"stance\": {}, "
                       "\"noteType\": {}, "
                       "\"locBlendIndex\": {}, \"locBlendAlignTime\": {}, "
                       "\"localOverrideAnim\": {} }}",
                       name, initialLerp, finalLerp, forceAnimRate, moveSpeed,
                       rotSpeed, duration, nameHash, flags, movetype, stance,
                       noteType, locBlendIndex, locBlendAlignTime,
                       localOverrideAnim);
  }
});

PACKED(struct LerpFrameAnimation {
  animationNumber_t animationNumber;
  uint8_t _padding02[6];
  animation_t *animation;
  int32_t animationTime;
  vec3_t oldFramePos;
  float animSpeedScale;
  int32_t oldFrameSnapshotTime;
  qboolean scriptAnimFinished;
  int32_t syncAnimTime;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"animationNumber\": {}, \"animation\": {}, "
                       "\"animationTime\": {}, "
                       "\"oldFramePos\": {}, \"animSpeedScale\": {}, "
                       "\"oldFrameSnapshotTime\": {}, "
                       "\"scriptAnimFinished\": {}, \"syncAnimTime\": {} }}",
                       animationNumber.serialize(), ::serialize(animation),
                       animationTime, oldFramePos.serialize(), animSpeedScale,
                       oldFrameSnapshotTime, scriptAnimFinished.serialize(),
                       syncAnimTime);
  }
});

struct BGModelAttachmentName {
  char model[64];
  char tag[64];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"model\": \"{}\", \"tag\": \"{}\" }}",
                       ::serialize(model), ::serialize(tag));
  }
};

PACKED(struct LerpFrameAngles {
  float yawAngle;
  float yawAngleOld;
  qboolean yawing;
  float pitchAngle;
  qboolean pitching;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"yawAngle\": {}, \"yawAngleOld\": {}, \"yawing\": {}, "
        "\"pitchAngle\": {}, \"pitching\": {} }}",
        yawAngle, yawAngleOld, yawing.serialize(), pitchAngle,
        pitching.serialize());
  }
});

PACKED(struct ModelAttachmentIndex {
  int16_t model;
  uint8_t tag;
  uint8_t _padding03[1];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"model\": {}, \"tag\": {} }}", model, tag);
  }
});

PACKED(struct animState_t {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"deltaRequestTime\": {}, \"deltaSource\": {}, "
        "\"deltaRowLowBits\": {}, \"deltaRowHighBits\": {}, \"aimRow\": {}, "
        "\"shootRow\": {}, \"transdecActive\": {}, "
        "\"currentAnimMappingTableIndex\": {}, \"toggleCounter\": {}, "
        "\"fAnimationRate\": {}, \"traversalOrigin\": {}, \"traversalYaw\": "
        "{}, \"traversalAnimIndex\": {} }}",
        deltaRequestTime, deltaSource, deltaRowLowBits, deltaRowHighBits,
        aimRow, shootRow, transdecActive, currentAnimMappingTableIndex,
        toggleCounter, fAnimationRate, traversalOrigin.serialize(),
        traversalYaw, traversalAnimIndex);
  }
});

PACKED(struct animscripted_t {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"axis\": {}, \"originError\": {}, \"originErrorReduction\": {}, "
        "\"anglesError\": {}, \"anglesErrorReduction\": {}, \"anim\": {}, "
        "\"root\": {}, \"bStarted\": {}, \"mode\": {}, \"startTime\": {}, "
        "\"lerpTime\": {}, \"fHeightOfs\": {}, \"fEndPitch\": {}, "
        "\"fEndRoll\": {}, "
        "\"fOrientLerp\": {} }}",
        ::serialize(axis), originError.serialize(),
        originErrorReduction.serialize(), anglesError.serialize(),
        anglesErrorReduction.serialize(), anim, root, bStarted, mode, startTime,
        lerpTime, fHeightOfs, fEndPitch, fEndRoll, fOrientLerp);
  }
});

struct TraversalAnim {
  int32_t animIndex;
  vec3_t origin;
  float yaw;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"animIndex\": {}, \"origin\": {}, \"yaw\": {} }}",
                       animIndex, origin.serialize(), yaw);
  }
};

struct TraversalState {
  qboolean active;
  TraversalAnim anims[3];
  int32_t outroTimeLeft;
  vec3_t outroOrigin;
  vec3_t outroAngles;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"active\": {}, \"anims\": {}, \"outroTimeLeft\": {}, "
        "\"outroOrigin\": {}, \"outroAngles\": {} }}",
        active.serialize(), ::serialize(anims), outroTimeLeft,
        outroOrigin.serialize(), outroAngles.serialize());
  }
};

struct ObjectiveAnimData {
  bool obscuredLastCheck;
  bool hiddenByGroupCulling;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"obscuredLastCheck\": {}, \"hiddenByGroupCulling\": {} }}",
        ::serialize(obscuredLastCheck), ::serialize(hiddenByGroupCulling));
  }
};

struct DObjAnimMat {
  vec4_t quat;
  vec3_t trans;
  float transWeight;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"quat\": {}, \"trans\": {}, \"transWeight\": {} }}",
                       quat.serialize(), trans.serialize(), transWeight);
  }
};
struct BaseAnimBoneMat {
  BoneIndex boneIndex;
  bool ignoreParentRelativeData;
  bool primaryBone;
  vec4_t baseQuat;
  vec3_t baseTrans;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"boneIndex\": 0x{:04X}, \"ignoreParentRelativeData\": {}, "
        "\"primaryBone\": {}, "
        "\"baseQuat\": {}, \"baseTrans\": {} }}",
        boneIndex, ::serialize(ignoreParentRelativeData),
        ::serialize(primaryBone), baseQuat.serialize(), baseTrans.serialize());
  }
};
struct DSkelPartBits {
  int anim[12];
  int control[12];
  int skel[12];
  int worldQuat[12];
  int constrained[12];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"anim\": {}, \"control\": {}, \"skel\": {}, "
                       "\"worldQuat\": {}, \"constrained\": {} }}",
                       ::serialize(anim), ::serialize(control),
                       ::serialize(skel), ::serialize(worldQuat),
                       ::serialize(constrained));
  }
};

struct __attribute__((aligned(4))) DSkel {
  DObjAnimMat *mat;
  BaseAnimBoneMat *baseAnimMat;
  float *blendTargetWeights;
  DSkelPartBits partBits;
  int timeStamp;
  bool dirty;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"mat\": {}, \"baseAnimMat\": {}, "
                       "\"blendTargetWeights\": {}, "
                       "\"partBits\": {}, \"timeStamp\": {}, \"dirty\": {} }}",
                       ::serialize(mat), ::serialize(baseAnimMat),
                       ::serialize(blendTargetWeights), partBits.serialize(),
                       timeStamp, ::serialize(dirty));
  }
};

struct __attribute__((aligned(8))) DObj {
  XAnimTree *tree;
  uint8_t *ikState;
  gfx::GfxSiegeAnimState **siegeAnimations;
  db::xasset::xmodel::XModel **models;
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

  inline std::string serialize() const noexcept {
    const int64_t locked_copy = locked;
    return std::format(
        "{{ \"tree\": {:p}, \"ikState\": {:p}, \"siegeAnimations\": {:p}, "
        "\"models\": {:p}, "
        "\"locked\": {}, \"skel\": {}, \"hidePartBits\": {}, "
        "\"hideSubModelBits\": 0x{:08X}, "
        "\"hideSubModelShadowBits\": 0x{:08X}, \"duplicatePartsRef\": "
        "0x{:08X}, "
        "\"ignoreCollision\": 0x{:08X}, "
        "\"radius\": {}, \"pitchAngle\": {}, \"scale\": {}, \"entnum\": {}, "
        "\"numCosmeticBones\": {}, \"numSiegeAnimations\": {}, "
        "\"duplicatePartsSize\": {}, "
        "\"flags\": 0x{:04X}, \"numBones\": 0x{:04X}, \"numModels\": {}, "
        "\"numConstrainedBones\": {}, "
        "\"localClientIndex\": {}, \"numRootAdjustMats\": {} }}",
        reinterpret_cast<const void *>(tree),
        reinterpret_cast<const void *>(ikState),
        reinterpret_cast<const void *>(siegeAnimations),
        reinterpret_cast<const void *>(models), locked_copy, skel.serialize(),
        ::serialize(hidePartBits), hideSubModelBits, hideSubModelShadowBits,
        duplicatePartsRef, ignoreCollision, radius, pitchAngle, scale, entnum,
        numCosmeticBones, numSiegeAnimations, duplicatePartsSize, flags,
        numBones, numModels, numConstrainedBones, localClientIndex,
        numRootAdjustMats);
  }
};

struct DObjModel {
  db::xasset::xmodel::XModel *model;
  scr::ScrString_t boneName;
  uint16_t parentModel;
  bool ignoreCollision;
  vec3_t trans;
  vec3_t rot;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"model\": {}, \"boneName\": 0x{:08X}, \"parentModel\": {}, "
        "\"ignoreCollision\": {}, \"trans\": {}, \"rot\": {} }}",
        ::serialize(model), boneName, parentModel, ::serialize(ignoreCollision),
        trans.serialize(), rot.serialize());
  }
};

enum class link_type_e : uint32_t {
  PITCH_CTRL = 0x0,
  YAW_CTRL = 0x1,
  ROLL_CTRL = 0x2,
  SWIM_CTRL = 0x3,
  FULL_CTRL = 0x4,
  COMPASS_CTRL = 0x5,
};

inline constexpr const char *serialize(const link_type_e type) {
  switch (type) {
  case link_type_e::PITCH_CTRL:
    return "LINK_PITCH_CTRL";
  case link_type_e::YAW_CTRL:
    return "LINK_YAW_CTRL";
  case link_type_e::ROLL_CTRL:
    return "LINK_ROLL_CTRL";
  case link_type_e::SWIM_CTRL:
    return "LINK_SWIM_CTRL";
  case link_type_e::FULL_CTRL:
    return "LINK_FULL_CTRL";
  case link_type_e::COMPASS_CTRL:
    return "LINK_COMPASS_CTRL";
  default:
    return "LINK_INVALID";
  }
}

struct locBlendPairAnim_t {
  int32_t animIndex;
  float extent;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"animIndex\": {}, \"extent\": {} }}", animIndex,
                       extent);
  }
};
struct __attribute__((aligned(8))) locBlendPair_t {
  locBlendPairAnim_t *start;
  locBlendPairAnim_t *end;
  int32_t strideType;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"start\": {:p}, \"end\": {:p}, \"strideType\": {} }}",
        (const void *)start, (const void *)end, strideType);
  }
};

struct locBlendSet_t {
  char name[64];
  int32_t startIndex;
  int32_t count;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": \"{}\", \"startIndex\": {}, \"count\": {} }}", name,
        startIndex, count);
  }
};

enum class locBlendType_t : uint32_t {
  DIRECTIONAL = 0x0,
  TURN_LEAN = 0x1,
  COUNT = 0x2,
};

inline constexpr const char *serialize(const locBlendType_t type) {
  switch (type) {
  case locBlendType_t::DIRECTIONAL:
    return "LOCBLEND_DIRECTIONAL";
  case locBlendType_t::TURN_LEAN:
    return "LOCBLEND_TURN_LEAN";
  default:
    return "LOCBLEND_INVALID";
  }
}

struct __attribute__((aligned(8))) locBlend_t {
  char name[64];
  game::anim::locBlendType_t type;
  locBlendSet_t *sets[4];
  int32_t numSets;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": \"{}\", \"type\": {}, \"sets\": {:p}, \"numSets\": {} }}",
        name, game::anim::serialize(type), reinterpret_cast<const void *>(sets),
        numSets);
  }
};

struct XAnimBlendTargetWeights {
  uint16_t numKeys;
  uint16_t *frames;
  float *values;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"numKeys\": {}, \"frames\": {:p}, \"values\": {:p} }}", numKeys,
        reinterpret_cast<const void *>(frames),
        reinterpret_cast<const void *>(values));
  }
};

typedef uint8_t ByteVec[3];
typedef uint16_t uint16_tVec[3];

union XAnimDynamicFrames {
  ByteVec *_1;
  uint16_tVec *_2;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"_1\": {:p} }}",
                       reinterpret_cast<const void *>(_1));
  }
};
union XAnimDynamicIndices {
  uint8_t _1[1];
  uint16_t _2[1];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"_1\": 0x{:02X} }}", _1[0]);
  }
};

struct __attribute__((aligned(8))) XAnimPartTransFrames {
  vec3_t mins;
  vec3_t size;
  XAnimDynamicFrames frames;
  XAnimDynamicIndices indices;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"mins\": {}, \"size\": {}, \"frames\": {}, \"indices\": {} }}",
        mins.serialize(), size.serialize(), frames.serialize(),
        indices.serialize());
  }
};

union XAnimPartTransData {
  XAnimPartTransFrames frames;
  vec3_t frame0;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frame0\": {} }}", frame0.serialize());
  }
};

struct XAnimPartTrans {
  uint16_t size;
  uint8_t smallTrans;
  XAnimPartTransData u;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"size\": {}, \"smallTrans\": {}, \"u\": {} }}",
                       size, smallTrans, u.serialize());
  }
};

typedef fp16::half2_pack XQuat2;

struct __attribute__((aligned(8))) XAnimDeltaPartQuatDataFrames2 {
  XQuat2 *frames;
  XAnimDynamicIndices indices;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frames\": {:p}, \"indices\": {} }}",
                       reinterpret_cast<const void *>(frames),
                       indices.serialize());
  }
};

union XAnimDeltaPartQuatData2 {
  XAnimDeltaPartQuatDataFrames2 frames;
  XQuat2 frame0;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frames\": {} }}", frames.serialize());
  }
};

struct XAnimDeltaPartQuat2 {
  uint16_t size;
  XAnimDeltaPartQuatData2 u;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"size\": {}, \"u\": {} }}", size, u.serialize());
  }
};

typedef fp16::half4_pack XQuat;

struct __attribute__((aligned(8))) XAnimDeltaPartQuatDataFrames {
  XQuat *frames;
  XAnimDynamicIndices indices;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frames\": {:p}, \"indices\": {} }}",
                       reinterpret_cast<const void *>(frames),
                       indices.serialize());
  }
};

union XAnimDeltaPartQuatData {
  XAnimDeltaPartQuatDataFrames frames;
  XQuat frame0;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"frames\": {} }}", frames.serialize());
  }
};

struct XAnimDeltaPartQuat {
  uint16_t size;
  XAnimDeltaPartQuatData u;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"size\": {}, \"u\": {} }}", size, u.serialize());
  }
};

struct XAnimDeltaPart {
  XAnimPartTrans *trans;
  XAnimDeltaPartQuat2 *quat2;
  XAnimDeltaPartQuat *quat;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"trans\": {:p}, \"quat2\": {:p}, \"quat\": {:p} }}",
                       reinterpret_cast<const void *>(trans),
                       reinterpret_cast<const void *>(quat2),
                       reinterpret_cast<const void *>(quat));
  }
};

struct XAnimIKPitchBone {
  uint16_t index;
  vec3_t rotation;
  vec3_t translation;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"index\": {}, \"rotation\": {}, \"translation\": {} }}", index,
        rotation.serialize(), translation.serialize());
  }
};

struct XAnimIKPitchLayer {
  float angle;
  uint16_t numBones;
  uint16_t bonesStart;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"angle\": {}, \"numBones\": {}, \"bonesStart\": {} }}", angle,
        numBones, bonesStart);
  }
};

struct XAnimMarkup {
  scr::ScrString_t type;
  float startTime;
  float endTime;
  uint8_t persistent;
  scr::ScrString_t param1;
  scr::ScrString_t param2;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"type\": 0x{:08X}, \"startTime\": {}, \"endTime\": "
                       "{}, \"persistent\": {}, "
                       "\"param1\": 0x{:08X}, \"param2\": 0x{:08X} }}",
                       type, startTime, endTime, persistent, param1, param2);
  }
};

struct XAnimNotifyInfo {
  scr::ScrString_t type;
  float time;
  scr::ScrString_t param1;
  scr::ScrString_t param2;
  scr::ScrString_t param3;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"type\": 0x{:08X}, \"time\": {}, \"param1\": 0x{:08X}, "
        "\"param2\": 0x{:08X}, \"param3\": 0x{:08X} }}",
        type, time, param1, param2, param3);
  }
};

struct __attribute__((aligned(4))) BlendTargetRemap {
  scr::ScrString_t name;
  uint16_t index;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": 0x{:08X}, \"index\": {} }}", name, index);
  }
};

union XAnimIndices {
  uint8_t *_1;
  uint16_t *_2;
  void *data;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"data\": {:p} }}", data);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"blendTargetRemap\": {:p}, \"blendTargetNames\": {:p}, \"names\": "
        "{:p}, "
        "\"blendTargetWeights\": {:p}, \"deltaPart\": {:p}, \"ikPitchBones\": "
        "{:p}, "
        "\"ikPitchLayers\": {:p}, \"markup\": {:p}, \"notify\": {:p}, "
        "\"shutdownNotify\": {:p}, "
        "\"startupNotify\": {:p}, \"dataByte\": {:p}, \"extraChannelData\": "
        "{:p}, "
        "\"randomDataByte\": {:p}, \"name\": {}, \"dataInt\": {:p}, "
        "\"randomDataInt\": {:p}, "
        "\"dataShort\": {:p}, \"randomDataShort\": {:p}, \"indices\": {}, "
        "\"branchName\": 0x{:08X}, "
        "\"framerate\": {}, \"frequency\": {}, \"loopEntryTime\": {}, "
        "\"primedLength\": {}, "
        "\"indexCount\": {}, \"numIKPitchBones\": {}, \"numIKPitchLayers\": "
        "{}, "
        "\"randomDataShortCount\": {}, \"streamedFileSize\": {}, "
        "\"dataByteCount\": {}, "
        "\"dataIntCount\": {}, \"dataShortCount\": {}, "
        "\"extraChannelDataCount\": {}, "
        "\"randomDataByteCount\": {}, \"randomDataIntCount\": {}, "
        "\"boneCount\": {}, "
        "\"numBones\": {}, \"numframes\": {}, \"numBlendTargets\": {}, "
        "\"bDelta\": {}, "
        "\"bDelta3D\": {}, \"bExtraChannel\": {}, \"bHasBoneMask\": {}, "
        "\"bLeftHandGripIK\": {}, "
        "\"bLoop\": {}, \"bLoopSync\": {}, \"assetType\": {}, \"extraFlags\": "
        "{}, "
        "\"markupCount\": {}, \"notifyCount\": {}, \"shutdownNotifyCount\": "
        "{}, "
        "\"startupNotifyCount\": {}, \"pad\": {} }}",
        reinterpret_cast<const void *>(blendTargetRemap),
        reinterpret_cast<const void *>(blendTargetNames),
        reinterpret_cast<const void *>(names),
        reinterpret_cast<const void *>(blendTargetWeights),
        reinterpret_cast<const void *>(deltaPart),
        reinterpret_cast<const void *>(ikPitchBones),
        reinterpret_cast<const void *>(ikPitchLayers),
        reinterpret_cast<const void *>(markup),
        reinterpret_cast<const void *>(notify),
        reinterpret_cast<const void *>(shutdownNotify),
        reinterpret_cast<const void *>(startupNotify),
        reinterpret_cast<const void *>(dataByte),
        reinterpret_cast<const void *>(extraChannelData),
        reinterpret_cast<const void *>(randomDataByte), ::serialize(name),
        reinterpret_cast<const void *>(dataInt),
        reinterpret_cast<const void *>(randomDataInt),
        reinterpret_cast<const void *>(dataShort),
        reinterpret_cast<const void *>(randomDataShort), indices.serialize(),
        branchName, framerate, frequency, loopEntryTime, primedLength,
        indexCount, numIKPitchBones, numIKPitchLayers, randomDataShortCount,
        streamedFileSize, dataByteCount, dataIntCount, dataShortCount,
        extraChannelDataCount, randomDataByteCount, randomDataIntCount,
        ::serialize(boneCount), numBones, numframes, numBlendTargets,
        ::serialize(bDelta), ::serialize(bDelta3D), ::serialize(bExtraChannel),
        ::serialize(bHasBoneMask), ::serialize(bLeftHandGripIK),
        ::serialize(bLoop), ::serialize(bLoopSync), assetType, extraFlags,
        markupCount, notifyCount, shutdownNotifyCount, startupNotifyCount,
        ::serialize(pad));
  }
};

struct scr_animtree_t {
  XAnim *anims;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"anims\": {:p} }}",
                       reinterpret_cast<const void *>(anims));
  }
};

struct animScriptCondition_t {
  int32_t index;
  uint32_t value;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"index\": {}, \"value\": 0x{:08X} }}", index,
                       value);
  }
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

inline constexpr const char *serialize(const animBodyPart_t part) {
  switch (part) {
  case animBodyPart_t::UNUSED:
    return "BODYPART_UNUSED";
  case animBodyPart_t::LEGS:
    return "BODYPART_LEGS";
  case animBodyPart_t::TORSO:
    return "BODYPART_TORSO";
  case animBodyPart_t::BOTH:
    return "BODYPART_BOTH";
  case animBodyPart_t::TORSO_2:
    return "BODYPART_TORSO_2";
  case animBodyPart_t::TORSO_FULL:
    return "BODYPART_TORSO_FULL";
  case animBodyPart_t::MOTION_MATCHING:
    return "BODYPART_MOTION_MATCHING";
  default:
    return "BODYPART_INVALID";
  }
}

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

  std::string serialize() const noexcept;


};

struct animScriptItem_t {
  int32_t numConditions;
  animScriptCondition_t conditions[5];
  int32_t numCommands;
  animScriptCommand_t commands[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numConditions\": {}, \"conditions\": {}, "
                       "\"numCommands\": {}, \"commands\": {} }}",
                       numConditions, ::serialize(conditions), numCommands,
                       ::serialize(commands));
  }
};

struct animScript_t {
  int32_t numItems;
  animScriptItem_t *items[192];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numItems\": {}, \"items\": {:p} }}", numItems,
                       reinterpret_cast<const void *>(items));
  }
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
  fastcallPtr_t<snd::SndAliasList *(const char *)> soundAlias;
  fastcallPtr_t<void(ClientNum_t, snd::SndAliasList *)> playSoundAlias;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"animations\": {}, \"numAnimations\": {}, \"scriptAnims\": {}, "
        "\"scriptCannedAnims\": {}, \"scriptStateChange\": {}, "
        "\"scriptEvents\": {}, "
        "\"scriptItems\": {}, \"numScriptItems\": {}, \"animTree\": {}, "
        "\"torsoAnim\": {}, \"torso2Anim\": {}, \"legsAnim\": {}, "
        "\"motionMatchingAnim\": {}, "
        "\"turningAnim\": {}, \"locBlendPairAnims\": {}, \"locBlendPairs\": "
        "{}, "
        "\"locBlendSets\": {}, \"locBlends\": {}, \"numLocBlendPairAnims\": "
        "{}, "
        "\"numLocBlendPairs\": {}, \"numLocBlendDirSets\": {}, "
        "\"numLocBlends\": {} }}",
        ::serialize(&animations[0], numAnimations), numAnimations, "TODO",
        "TODO", "TODO", ::serialize(scriptEvents),
        ::serialize(&scriptItems[0], numScriptItems), numScriptItems,
        animTree.serialize(), torsoAnim, torso2Anim, legsAnim,
        motionMatchingAnim, turningAnim,
        ::serialize(&locBlendPairAnims[0], numLocBlendPairAnims),
        ::serialize(&locBlendPairs[0], numLocBlendPairs),
        ::serialize(&locBlendSets[0], numLocBlendDirSets),
        ::serialize(&locBlends[0], numLocBlends), numLocBlendPairAnims,
        numLocBlendPairs, numLocBlendDirSets, numLocBlends);
  }
};

struct __attribute__((aligned(4))) ViewModelHand {
  int32_t iHandAnimIndex;
  animationNumber_t iPrevAnim;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"iHandAnimIndex\": {}, \"iPrevAnim\": {} }}",
                       iHandAnimIndex, iPrevAnim.serialize());
  }
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
  db::xasset::xmodel::XModelPtr altModels[4];
  uint32_t numAltModels;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"viewModelDObj\": {:p}, \"viewModelDObjLegs\": {:p}, "
        "\"weaponDobjInfo\": {}, "
        "\"hasAnimTree\": {}, \"tree\": {:p}, \"anims\": {:p}, \"partBits\": "
        "{}, "
        "\"hand\": {}, \"altModels\": {}, \"numAltModels\": {} }}",
        (const void *)viewModelDObj, (const void *)viewModelDObjLegs,
        weaponDobjInfo.serialize(), ::serialize(hasAnimTree),
        (const void *)tree, (const void *)anims, ::serialize(partBits),
        ::serialize(hand),
        ::serialize(reinterpret_cast<const uintptr_t *const>(&altModels[0]),
                    std::size(altModels)),
        numAltModels);
  }
};
// Verified
ASSERT_SIZE(ViewModelInfo, 0x3A0);

struct ViewModelInfoPool {
  static constexpr size_t LEN = 64;
  LocalClientPool<ViewModelInfo> pool;

  static inline constexpr void assert_range(size_t index) {
    assert(index < LocalClientNum_t::LOCAL_CLIENT_COUNT &&
           "index to ViewModelInfoPool must be "
           "within range LOCAL_CLIENT_0 <= index < LOCAL_CLIENT_COUNT");
  }

  template <IntegralLike<size_t> Index>
  inline constexpr const ViewModelInfo &get(Index index_arg) const {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<size_t> Index>
  inline constexpr const ViewModelInfo &operator[](Index index) const {
    return get(index);
  }

  template <IntegralLike<size_t> Index>
  inline constexpr ViewModelInfo &get(Index index_arg) {
    const size_t index = static_cast<size_t>(index_arg);
    assert_range(index);
    return pool[index];
  }

  template <IntegralLike<size_t> Index>
  inline constexpr ViewModelInfo &operator[](Index index) {
    return get(index);
  }
};

} // namespace anim
} // namespace game
