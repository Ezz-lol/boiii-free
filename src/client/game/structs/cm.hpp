#pragma once

#include "asm.hpp"
#include "core.hpp"
#include "db/xasset/xmodel.hpp"
#include "dyn/ent.hpp"
#include "quake/core.hpp"
#include "rope.hpp"

namespace game {
namespace level {
namespace cl {
struct centity_t;
}
struct gentity_s;
typedef gentity_s gentity_t;

} // namespace level

namespace cm {

class hybrid_vector {
public:
  math::float4 vec;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"vec\": {} }}", vec.serialize());
  }
};

struct TraceExtents {
  hybrid_vector start;
  hybrid_vector end;
  hybrid_vector invDelta;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"start\": {}, \"end\": {}, \"invDelta\": {} }}",
                       start.serialize(), end.serialize(),
                       invDelta.serialize());
  }
};
struct TraceCheckCount {
  uint16_t global;
  uint16_t *partitions;
  uint16_t *brushes;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"global\": {}, \"partitions\": {:p}, \"brushes\": {:p} }}", global,
        reinterpret_cast<const void *>(partitions),
        reinterpret_cast<const void *>(brushes));
  }
};

struct ClipMaterial {
  const char *name;
  int32_t surfaceFlags;
  contents_t contentFlags;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"surfaceFlags\": 0x{:08X}, "
                       "\"contentFlags\": 0x{:08X} }}",
                       ::serialize(name), surfaceFlags, contentFlags);
  }
};

typedef uint32_t LeafBrush;

struct cLeafBrushNodeLeaf_t {
  LeafBrush *brushes;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"brushes\": {:p} }}",
                       reinterpret_cast<const void *>(brushes));
  }
};

struct cLeafBrushNodeChildren_t {
  float dist;
  float range;
  uint16_t childOffset[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"dist\": {}, \"range\": {}, \"childOffset\": {} }}",
                       dist, range, ::serialize(childOffset));
  }
};

union __attribute__((aligned(8))) cLeafBrushNodeData_t {
  cLeafBrushNodeLeaf_t leaf;
  cLeafBrushNodeChildren_t children;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"leaf\": {}, \"children\": {} }}", leaf.serialize(),
                       children.serialize());
  }
};

struct cLeafBrushNode_t {
  uint8_t axis;
  int32_t leafBrushCount;
  contents_t contents;
  cLeafBrushNodeData_t data;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"axis\": {}, \"leafBrushCount\": {}, \"contents\": "
                       "{:08X}, \"data\": {} }}",
                       axis, leafBrushCount, contents, data.serialize());
  }
};

struct Bounds {
  vec3_t midPoint;
  vec3_t halfSize;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"midPoint\": {}, \"halfSize\": {} }}",
                       midPoint.serialize(), halfSize.serialize());
  }
};

struct ClipInfo {
  int32_t planeCount;
  db::xasset::phys::cplane_t *planes;
  uint32_t numMaterials;
  ClipMaterial *materials;
  uint32_t numBrushSides;
  db::xasset::phys::cbrushside_t *brushsides;
  uint32_t leafbrushNodesCount;
  cLeafBrushNode_t *leafbrushNodes;
  uint32_t numLeafBrushes;
  LeafBrush *leafbrushes;
  uint32_t numBrushVerts;
  vec3_t *brushVerts;
  uint32_t nuinds;
  uint32_t *uinds;
  uint32_t numBrushes;
  db::xasset::phys::cbrush_t *brushes;
  Bounds *brushBounds;
  contents_t *brushContents;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"planeCount\": {}, \"planes\": {}, \"numMaterials\": {}, "
        "\"materials\": {}, \"numBrushSides\": {}, \"brushsides\": {}, "
        "\"leafbrushNodesCount\": {}, \"leafbrushNodes\": {}, "
        "\"numLeafBrushes\": {}, "
        "\"leafbrushes\": {}, \"numBrushVerts\": {}, \"brushVerts\": {}, "
        "\"nuinds\": {}, \"uinds\": {}, \"numBrushes\": {}, \"brushes\": "
        "{}, "
        "\"brushBounds\": {}, \"brushContents\": {} }}",
        planeCount, ::serialize(planes, planeCount), numMaterials,
        ::serialize(materials, numMaterials), numBrushSides,
        ::serialize(brushsides, numBrushSides), leafbrushNodesCount,
        ::serialize(leafbrushNodes, leafbrushNodesCount), numLeafBrushes,
        ::serialize(leafbrushes, numLeafBrushes), numBrushVerts,
        ::serialize(brushVerts, numBrushVerts), nuinds,
        ::serialize(uinds, nuinds), numBrushes,
        ::serialize(brushes, numBrushes), ::serialize(brushBounds, numBrushes),
        ::serialize(brushContents, numBrushes));
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"firstCollAabbIndex\": {}, \"collAabbCount\": {}, "
                       "\"brushContents\": 0x{:08X}, "
                       "\"terrainContents\": 0x{:08X}, \"mins\": {}, \"maxs\": "
                       "{}, \"leafBrushNode\": {}, "
                       "\"cluster\": {} }}",
                       firstCollAabbIndex, collAabbCount, brushContents,
                       terrainContents, mins.serialize(), maxs.serialize(),
                       leafBrushNode, cluster);
  }
};

struct cmodel_t {
  vec3_t mins;
  vec3_t maxs;
  float radius;
  ClipInfo *info;
  cLeaf_t leaf;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"mins\": {}, \"maxs\": {}, \"radius\": {}, "
                       "\"info\": {}, \"leaf\": {} }}",
                       mins.serialize(), maxs.serialize(), radius,
                       ::serialize(info), leaf.serialize());
  }
};

struct TraceThreadInfo {
  TraceCheckCount checkcount;
  db::xasset::phys::cbrush_t *box_brush;
  cmodel_t *box_model;
  db::xasset::phys::PhysGeomList **geoms;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"checkcount\": {}, \"box_brush\": {}, "
                       "\"box_model\": {}, \"geoms\": {:p} }}",
                       checkcount.serialize(), ::serialize(box_brush),
                       ::serialize(box_model),
                       reinterpret_cast<const void *>(geoms));
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"delta\": {}, \"midpoint\": {}, \"halfDelta\": {}, "
        "\"halfDeltaAbs\": {}, "
        "\"size\": {}, \"bounds\": {}, \"radiusOffset\": {}, \"extents\": {}, "
        "\"deltaLen\": {}, \"deltaLenSq\": {}, \"contents\": 0x{:08X}, "
        "\"isPoint\": {}, "
        "\"axialCullOnly\": {}, \"radius\": {}, \"offsetZ\": {}, "
        "\"boundingRadius\": {}, "
        "\"threadInfo\": {} }}",
        delta.serialize(), midpoint.serialize(), halfDelta.serialize(),
        halfDeltaAbs.serialize(), size.serialize(), ::serialize(bounds),
        radiusOffset.serialize(), extents.serialize(), deltaLen, deltaLenSq,
        contents, ::serialize(isPoint), ::serialize(axialCullOnly), radius,
        offsetZ, boundingRadius, threadInfo.serialize());
  }
};

struct TriggerHull {
  Bounds bounds;
  contents_t contents;
  uint16_t slabCount;
  uint16_t firstSlab;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"bounds\": {}, \"contents\": 0x{:08X}, "
                       "\"slabCount\": {}, \"firstSlab\": {} }}",
                       bounds.serialize(), contents, slabCount, firstSlab);
  }
};

struct TriggerSlab {
  vec3_t dir;
  float midPoint;
  float halfSize;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"dir\": {}, \"midPoint\": {}, \"halfSize\": {} }}",
                       dir.serialize(), midPoint, halfSize);
  }
};

struct TriggerModel {
  contents_t contents;
  uint16_t hullCount;
  uint16_t firstHull;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"contents\": 0x{:08X}, \"hullCount\": {}, \"firstHull\": {} }}",
        contents, hullCount, firstHull);
  }
};

PACKED(struct MapTriggers {
  uint32_t count;
  uint8_t _padding4[4];
  TriggerModel *models;
  uint32_t hullCount;
  uint8_t _padding14[4];
  TriggerHull *hulls;
  uint32_t slabCount;
  uint8_t _padding24[4];
  TriggerSlab *slabs;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"count\": {}, \"models\": {}, \"hullCount\": {}, "
                       "\"hulls\": {}, "
                       "\"slabCount\": {}, \"slabs\": {} }}",
                       count, ::serialize(models, count), hullCount,
                       ::serialize(hulls, hullCount), slabCount,
                       ::serialize(slabs, slabCount));
  }
});

PACKED(struct MapEnts {
  const char *name;
  char *entityString;
  int32_t numEntityChars;
  uint8_t _padding14[4];
  MapTriggers trigger;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"entityString\": {}, "
                       "\"numEntityChars\": {}, \"trigger\": {} }}",
                       ::serialize(name), ::serialize(entityString),
                       numEntityChars, trigger.serialize());
  }
});

struct cStaticModelWritable {
  uint16_t nextModelInWorldSector;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"nextModelInWorldSector\": {} }}",
                       nextModelInWorldSector);
  }
};

PACKED(struct cStaticModel_s {
  cStaticModelWritable writable;
  uint8_t _padding02[6];
  db::xasset::xmodel::XModel *xmodel;
  contents_t contents;
  vec3_t origin;
  vec3_t invScaledAxis[3];
  vec3_t absmin;
  vec3_t absmax;
  uint8_t _padding5C[4];
  anim::DObjAnimMat *boneMtxs;
  int32_t numBoneMtxs;
  scr::ScrString_t targetname;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"writable\": {}, \"xmodel\": {:p}, \"contents\": 0x{:08X}, "
        "\"origin\": {}, \"invScaledAxis\": {}, \"absmin\": {}, \"absmax\": "
        "{}, "
        "\"boneMtxs\": {}, \"numBoneMtxs\": {}, \"targetname\": 0x{:08X} }}",
        writable.serialize(), reinterpret_cast<const void *>(xmodel), contents,
        origin.serialize(), ::serialize(invScaledAxis), absmin.serialize(),
        absmax.serialize(), ::serialize(boneMtxs, numBoneMtxs), numBoneMtxs,
        targetname);
  }
});

typedef cStaticModel_s cStaticModel_t;

struct cNode_t {
  db::xasset::phys::cplane_t *plane;
  int32_t children[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"plane\": {}, \"children\": {} }}",
                       ::serialize(plane), ::serialize(children));
  }
};

PACKED(struct clipMap_t {
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
  db::xasset::phys::cbrush_t *box_brush;
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
  db::xasset::phys::PhysConstraint *constraints;
  int32_t max_ropes;
  uint8_t _padding23C[4];
  rope::rope_t *ropes;
  scr::ScrString_t miscModelTargetnameGroups[32];
  uint32_t checksum;
  uint8_t _padding2CC[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"isInUse\": {}, \"info\": {}, \"pInfo\": {:p}, "
        "\"numStaticModels\": {}, \"staticModelList\": {}, \"numNodes\": {}, "
        "\"nodes\": {}, \"numLeafs\": {}, \"leafs\": {}, \"vertCount\": "
        "{}, "
        "\"verts\": {}, \"triCount\": {}, \"triIndices\": {}, "
        "\"triEdgeIsWalkable\": {:p}, "
        "\"partitionCount\": {}, \"partitions\": {}, \"aabbTreeCount\": {}, "
        "\"aabbTrees\": {}, \"numSubModels\": {}, \"cmodels\": {}, "
        "\"numClusters\": {}, "
        "\"clusterBytes\": {}, \"visibility\": {:p}, \"vised\": {}, "
        "\"mapEnts\": {:p}, "
        "\"box_brush\": {}, \"box_model\": {}, \"originalDynEntCount\": {}, "
        "\"dynEntCount\": {}, \"dynEntDefList\": {}, \"dynEntPoseList\": "
        "{}, "
        "\"dynEntClientList\": {}, \"dynEntServerList\": {}, "
        "\"dynEntCollList\": {}, "
        "\"num_constraints\": {}, \"constraints\": {}, \"max_ropes\": {}, "
        "\"ropes\": {}, "
        "\"miscModelTargetnameGroups\": {}, \"checksum\": 0x{:08X} }}",
        ::serialize(name), isInUse.serialize(), info.serialize(),
        reinterpret_cast<const void *>(pInfo), numStaticModels,
        ::serialize(staticModelList, numStaticModels), numNodes,
        ::serialize(nodes, numNodes), numLeafs, ::serialize(leafs, numLeafs),
        vertCount, ::serialize(verts, vertCount), triCount,
        ::serialize(triIndices, triCount),
        reinterpret_cast<const void *>(triEdgeIsWalkable), partitionCount,
        ::serialize(partitions, partitionCount), aabbTreeCount,
        ::serialize(aabbTrees, aabbTreeCount), numSubModels,
        ::serialize(cmodels, numSubModels), numClusters, clusterBytes,
        reinterpret_cast<const void *>(visibility), vised.serialize(),
        reinterpret_cast<const void *>(mapEnts), ::serialize(box_brush),
        box_model.serialize(), originalDynEntCount, ::serialize(dynEntCount),
        ::serialize(dynEntDefList), ::serialize(dynEntPoseList),
        ::serialize(dynEntClientList), ::serialize(dynEntServerList),
        ::serialize(dynEntCollList), num_constraints,
        ::serialize(constraints, num_constraints), max_ropes,
        ::serialize(ropes, max_ropes), ::serialize(miscModelTargetnameGroups),
        checksum);
  }
});
ASSERT_SIZE(clipMap_t, 0x2D0);
ASSERT_OFFSET(clipMap_t, nodes, 0xC0);

PACKED(struct trace_t {
  hybrid_vector normal;
  float fraction;
  int32_t sflags;
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
  int32_t hitPartition;
  uint8_t _padding4C[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"normal\": {}, \"fraction\": {}, \"sflags\": 0x{:08X}, "
        "\"cflags\": "
        "{:08X}, "
        "\"brushType\": {}, \"brushId\": {}, \"hitType\": {}, \"hitId\": {}, "
        "\"modelIndex\": {}, \"subModelBoneIndex\": {}, \"partName\": "
        "0x{:08X}, "
        "\"boneIndex\": {}, \"partGroup\": {}, \"allsolid\": {}, "
        "\"startsolid\": {}, "
        "\"walkable\": {}, \"staticModel\": {:p}, \"hitPartition\": {} }}",
        normal.serialize(), fraction, sflags, cflags,
        game::serialize(brushType), brushId, game::serialize(hitType), hitId,
        modelIndex, subModelBoneIndex, partName, boneIndex, partGroup,
        ::serialize(allsolid), ::serialize(startsolid), ::serialize(walkable),
        reinterpret_cast<const void *>(staticModel), hitPartition);
  }
});
ASSERT_SIZE(trace_t, 0x50);

struct BulletFireParams {
  int32_t weaponEntIndex;
  int32_t ignoreEntIndex;
  float damageMultiplier;
  int32_t methodOfDeath;
  vec3_t origStart;
  vec3_t start;
  vec3_t end;
  vec3_t dir;
  vec3_t origDir;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"weaponEntIndex\": {}, \"ignoreEntIndex\": {}, "
        "\"damageMultiplier\": {}, "
        "\"methodOfDeath\": {}, \"origStart\": {}, \"start\": {}, \"end\": {}, "
        "\"dir\": {}, \"origDir\": {} }}",
        weaponEntIndex, ignoreEntIndex, damageMultiplier, methodOfDeath,
        origStart.serialize(), start.serialize(), end.serialize(),
        dir.serialize(), origDir.serialize());
  }
};

PACKED(struct BulletTraceResults {
  trace_t trace;
  level::gentity_t *hitEnt;
  vec3_t hitPos;
  bool ignoreHitEnt;
  uint8_t _padding65[3];
  int32_t depthSurfaceType;
  uint8_t _padding6C[4];

  std::string serialize() const noexcept;
});

PACKED(struct BulletTrace_Cmd {
  LocalClientNum_t localClientNum;
  BulletFireParams bp;
  const weapon::WeaponDef *weapDef;
  level::cl::centity_t *attacker;
  BulletTraceResults br;
  int32_t lastSurfaceType;
  bool traceHit;
  uint8_t _paddingD5[3];
  weapon::Weapon weapon;
  vec3_t tracerStart;
  bool drawTracer;
  bool isPlayer;
  uint8_t _paddingED[2];
  int32_t command_index;
  uint8_t _paddingF4[12];

  std::string serialize() const noexcept;
});

PACKED(struct GroundTrace {
  trace_t trace;
  vec3_t location;
  int32_t onGround;
  int32_t hasGround;
  int32_t validGroundNormal;
  uint8_t _padding68[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"trace\": {}, \"location\": {}, \"onGround\": {}, "
                       "\"hasGround\": {}, "
                       "\"validGroundNormal\": {} }}",
                       trace.serialize(), location.serialize(), onGround,
                       hasGround, validGroundNormal);
  }
});
ASSERT_SIZE(GroundTrace, 0x70);

} // namespace cm
} // namespace game
