#pragma once

#include "core.hpp"
#include "quake/core.hpp"
#include "db/xasset/core.hpp"
#include "dyn/ent.hpp"
#include "asm.hpp"
#include "rope.hpp"

namespace game {
namespace cm {

typedef vec4_t float4;

class hybrid_vector {
public:
  float4 vec;
};

struct TraceExtents {
  hybrid_vector start;
  hybrid_vector end;
  hybrid_vector invDelta;
};
struct TraceCheckCount {
  uint16_t global;
  uint16_t *partitions;
  uint16_t *brushes;
};

struct cplane_t {
  vec3_t normal;
  float dist;
  uint8_t type;
  uint8_t signbits;
  uint8_t pad[2];
};

struct cbrushside_t {
  cplane_t *plane;
  contents_t cflags;
  int32_t sflags;
};

struct __attribute__((aligned(16))) cbrush_t {
  vec3_t mins;
  contents_t contents;
  vec3_t maxs;
  uint32_t numsides;
  cbrushside_t *sides;
  contents_t axial_cflags[2][3];
  int32_t axial_sflags[2][3];
  uint32_t numverts;
  vec3_t *verts;
};

struct ClipMaterial {
  const char *name;
  int32_t surfaceFlags;
  contents_t contentFlags;
};

typedef uint32_t LeafBrush;

struct cLeafBrushNodeLeaf_t {
  LeafBrush *brushes;
};

struct cLeafBrushNodeChildren_t {
  float dist;
  float range;
  uint16_t childOffset[2];
};

union __attribute__((aligned(8))) cLeafBrushNodeData_t {
  cLeafBrushNodeLeaf_t leaf;
  cLeafBrushNodeChildren_t children;
};

struct cLeafBrushNode_t {
  uint8_t axis;
  int32_t leafBrushCount;
  contents_t contents;
  cLeafBrushNodeData_t data;
};

struct Bounds {
  vec3_t midPoint;
  vec3_t halfSize;
};

struct ClipInfo {
  int32_t planeCount;
  cplane_t *planes;
  uint32_t numMaterials;
  ClipMaterial *materials;
  uint32_t numBrushSides;
  cbrushside_t *brushsides;
  uint32_t leafbrushNodesCount;
  cLeafBrushNode_t *leafbrushNodes;
  uint32_t numLeafBrushes;
  LeafBrush *leafbrushes;
  uint32_t numBrushVerts;
  vec3_t *brushVerts;
  uint32_t nuinds;
  uint32_t *uinds;
  uint32_t numBrushes;
  cbrush_t *brushes;
  Bounds *brushBounds;
  contents_t *brushContents;
};

struct __attribute__((aligned(8))) cLeaf_t {
  uint32_t firstCollAabbIndex;
  uint32_t collAabbCount;
  contents_t brushContents;
  contents_t terrainContents;
  vec3_t mins;
  vec3_t maxs;
  intptr_t leafBrushNode;
  int32_t cluster;
};

struct cmodel_t {
  vec3_t mins;
  vec3_t maxs;
  float radius;
  ClipInfo *info;
  cLeaf_t leaf;
};

struct BrushWrapper {
  vec3_t mins;
  contents_t contents;
  vec3_t maxs;
  uint32_t numsides;
  cbrushside_t *sides;
  contents_t axial_cflags[2][3];
  int32_t axial_sflags[2][3];
  uint32_t numverts;
  vec3_t *verts;
  cplane_t *planes;
};

struct PhysGeomInfo {
  BrushWrapper *brush;
  int32_t type;
  vec3_t orientation[3];
  vec3_t offset;
  vec3_t halfLengths;
};

struct __attribute__((aligned(8))) PhysGeomList {
  uint32_t count;
  PhysGeomInfo *geoms;
  contents_t contents;
};

struct TraceThreadInfo {
  TraceCheckCount checkcount;
  cbrush_t *box_brush;
  cmodel_t *box_model;
  PhysGeomList **geoms;
};

struct traceWork_t {
  hybrid_vector delta;
  hybrid_vector midpoint;
  hybrid_vector halfDelta;
  hybrid_vector halfDeltaAbs;
  hybrid_vector size;
  hybrid_vector bounds[2];
  hybrid_vector radiusOffset;
  TraceExtents extents;
  vec_t deltaLen;
  vec_t deltaLenSq;
  contents_t contents;
  bool isPoint;
  bool axialCullOnly;
  float radius;
  float offsetZ;
  float boundingRadius;
  TraceThreadInfo threadInfo;
};

struct TriggerHull {
  Bounds bounds;
  contents_t contents;
  uint16_t slabCount;
  uint16_t firstSlab;
};

struct TriggerSlab {
  vec3_t dir;
  float midPoint;
  float halfSize;
};

struct TriggerModel {
  contents_t contents;
  uint16_t hullCount;
  uint16_t firstHull;
};

#pragma pack(push, 1)
struct MapTriggers {
  uint32_t count;
  uint8_t _padding4[4];
  TriggerModel *models;
  uint32_t hullCount;
  uint8_t _padding14[4];
  TriggerHull *hulls;
  uint32_t slabCount;
  uint8_t _padding24[4];
  TriggerSlab *slabs;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MapEnts {
  const char *name;
  char *entityString;
  int32_t numEntityChars;
  uint8_t _padding14[4];
  MapTriggers trigger;
};
#pragma pack(pop)

struct cStaticModelWritable {
  uint16_t nextModelInWorldSector;
};

#pragma pack(push, 1)
struct cStaticModel_s {
  cStaticModelWritable writable;
  uint8_t _padding02[6];
  db::xasset::XModel *xmodel;
  contents_t contents;
  vec3_t origin;
  vec3_t invScaledAxis[3];
  vec3_t absmin;
  vec3_t absmax;
  uint8_t _padding5C[4];
  anim::DObjAnimMat *boneMtxs;
  int32_t numBoneMtxs;
  scr::ScrString_t targetname;
};
#pragma pack(pop)

typedef cStaticModel_s cStaticModel_t;

struct cNode_t {
  cplane_t *plane;
  int32_t children[2];
};

#pragma pack(push, 1)
struct clipMap_t {
  const char *name;
  qboolean isInUse;
  uint8_t _padding0C[4];
  ClipInfo info;
  ClipInfo *pInfo;
  uint32_t numStaticModels;
  uint8_t _paddingAC[4];
  cStaticModel_t *staticModelList;
  uint32_t numNodes;
  uint8_t _paddingBC[4];
  cNode_t *nodes;
  uint32_t numLeafs;
  uint8_t _paddingCC[4];
  cLeaf_t *leafs;
  uint32_t vertCount;
  uint8_t _paddingDC[4];
  vec3_t *verts;
  int32_t triCount;
  uint8_t _paddingEC[4];
  uint32_t *triIndices;
  uint8_t *triEdgeIsWalkable;
  int32_t partitionCount;
  uint8_t _padding104[4];
  phys::CollisionPartition *partitions;
  int32_t aabbTreeCount;
  uint8_t _padding114[4];
  phys::CollisionAabbTree *aabbTrees;
  uint32_t numSubModels;
  uint8_t _padding124[4];
  cmodel_t *cmodels;
  int32_t numClusters;
  int32_t clusterBytes;
  uint8_t *visibility;
  qboolean vised;
  uint8_t _padding144[4];
  MapEnts *mapEnts;
  cbrush_t *box_brush;
  cmodel_t box_model;
  dyn::ent::DynEntityId originalDynEntCount;
  dyn::ent::DynEntityId dynEntCount[4];
  uint8_t _padding1C2[6];
  dyn::ent::DynEntityDef *dynEntDefList[2];
  dyn::ent::DynEntityPose *dynEntPoseList[2];
  dyn::ent::DynEntityClient *dynEntClientList[2];
  dyn::ent::DynEntityServer *dynEntServerList[2];
  dyn::ent::DynEntityColl *dynEntCollList[4];
  int32_t num_constraints;
  uint8_t _padding22C[4];
  phys::PhysConstraint *constraints;
  int32_t max_ropes;
  uint8_t _padding23C[4];
  rope::rope_t *ropes;
  scr::ScrString_t miscModelTargetnameGroups[32];
  uint32_t checksum;
  uint8_t _padding2CC[4];
};
ASSERT_SIZE(clipMap_t, 0x2D0);
ASSERT_OFFSET(clipMap_t, nodes, 0xC0);
#pragma pack(pop)

#pragma pack(push, 1)
struct trace_t {
  hybrid_vector normal;
  float fraction;
  int sflags;
  contents_t cflags;
  TraceBrushType brushType;
  uint16_t brushId;
  uint8_t _padding22[2];
  TraceHitType hitType;
  uint16_t hitId;
  uint16_t modelIndex;
  BoneIndex subModelBoneIndex;
  uint8_t _padding2E[2];
  scr::ScrString_t partName;
  BoneIndex boneIndex;
  uint16_t partGroup;
  bool allsolid;
  bool startsolid;
  bool walkable;
  uint8_t _padding3A[5];
  cStaticModel_s *staticModel;
  int hitPartition;
  uint8_t _padding4C[4];
};
ASSERT_SIZE(trace_t, 0x50);
#pragma pack(pop)

} // namespace cm
} // namespace game