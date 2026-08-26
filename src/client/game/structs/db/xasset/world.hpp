#pragma once

#include <cstdint>

#include <game/structs/quake/vec.hpp>
#include <game/structs/scr/primitives.hpp>

namespace game {
namespace db {
namespace xasset {
namespace world {
struct pathnode_tree_t;
typedef pathnode_tree_t *pathnode_tree_ptr;

struct pathnode_tree_nodes_t {
  int32_t nodeCount;
  uint16_t *nodes;
};

union pathnode_tree_info_t {
  pathnode_tree_ptr child[2];
  pathnode_tree_nodes_t s;
};

struct pathnode_tree_t {
  int32_t axis;
  vec_t dist;
  pathnode_tree_info_t u;
};

struct pathbasenode_t {
  vec3_t vOrigin;
  uint32_t type;
};

enum class nodeType : uint32_t {
  BADNODE = 0x0,
  PATHNODE = 0x1,
  COVER_STAND = 0x2,
  COVER_CROUCH = 0x3,
  COVER_CROUCH_WINDOW = 0x4,
  COVER_PRONE = 0x5,
  COVER_RIGHT = 0x6,
  COVER_LEFT = 0x7,
  COVER_PILLAR = 0x8,
  AMBUSH = 0x9,
  EXPOSED = 0xA,
  CONCEALMENT_STAND = 0xB,
  CONCEALMENT_CROUCH = 0xC,
  CONCEALMENT_PRONE = 0xD,
  REACQUIRE = 0xE,
  BALCONY = 0xF,
  SCRIPTED = 0x10,
  NEGOTIATION_BEGIN = 0x11,
  NEGOTIATION_END = 0x12,
  TURRET = 0x13,
  GUARD = 0x14,
  NUMTYPES = 0x15,
  DONTLINK = 0x15,
};

struct __declspec(align(4)) pathlink_s {
  float fDist;
  uint16_t nodeNum;
  uint8_t disconnectCount;
  uint8_t negotiationLink;
  uint8_t flags;
  uint8_t ubBadPlaceCount[5];
};

typedef pathlink_s pathlink_t;

struct __attribute__((aligned(8))) pathnode_constant_t {
  nodeType type;
  int32_t spawnflags;
  int32_t movementtype_ignore;
  int32_t movementtype_require;
  scr::ScrString_t targetname;
  scr::ScrString_t script_linkName;
  scr::ScrString_t script_noteworthy;
  scr::ScrString_t target;
  scr::ScrString_t animscript;
  scr::scr_funcptr_t animscriptfunc;
  vec3_t vOrigin;
  float fAngle;
  vec2_t forward;
  short wOverlapNode[4];
  uint16_t totalLinkCount;
  pathlink_t *Links;
  int32_t navFaceIndex;
  int32_t navRegionIndex;
  int32_t negotiationLinkedNodeIndex;
  float negotiationCostModifier;
  float width;
};

class SentientHandle {
public:
  uint16_t number;
  uint16_t infoIndex;
  uint32_t pad;
};

struct pathnode_dynamic_t {
  SentientHandle pOwner;
  int32_t iFreeTime;
  int32_t iValidTime[3];
  int32_t dangerousNodeTime[3];
  int32_t inPlayerLOSTime;
  short wLinkCount;
  short wOverlapCount;
  short turretEntNumber;
  short userCount;
  int32_t flags;
};

struct pathnode_t;

struct __attribute__((aligned(8))) pathnode_transient_t {
  int32_t iSearchFrame;
  pathnode_t *pNextOpen;
  pathnode_t *pPrevOpen;
  pathnode_t *pParent;
  float fCost;
  float fHeuristic;
  union {
    float nodeCost;
    int32_t linkIndex;
  };
};

struct pathnode_t {
  pathnode_constant_t constant;
  pathnode_dynamic_t dynamic;
  pathnode_transient_t transient;
};

struct PathData {
  uint32_t nodeCount;
  uint32_t originalNodeCount;
  pathnode_t *nodes;
  pathbasenode_t *basenodes;
  int32_t visBytes;
  uint8_t *pathVis;
  int32_t smoothBytes;
  uint8_t *smoothCache;
  int32_t nodeTreeCount;
  pathnode_tree_t *nodeTree;
};

// Verified
struct GameWorld {
  const char *name;
  PathData path;
};
ASSERT_SIZE(GameWorld, 0x50);
} // namespace world
} // namespace xasset
} // namespace db
} // namespace game