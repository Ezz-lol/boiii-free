#pragma once

#include <cstdint>

#include <game/structs/db/xasset/core.hpp>

namespace game {
namespace db {
namespace xasset {
namespace xsurface {

struct XSurfaceCollisionLeaf {
  uint16_t triangleBeginIndex;
};

struct XSurfaceCollisionAabb {

  typedef vec3<uint16_t> Aabb;

  Aabb mins[3];
  Aabb maxs[3];
};

struct XSurfaceCollisionNode {
  XSurfaceCollisionAabb aabb;
  uint16_t childBeginIndex;
  uint16_t childCount;
};

struct XSurfaceCollisionTree {
  vec3_t trans;
  vec3_t scale;
  uint32_t nodeCount;
  XSurfaceCollisionNode *nodes;
  uint32_t leafCount;
  XSurfaceCollisionLeaf *leafs;
};

struct XRigidVertList {
  uint16_t boneOffset;
  uint16_t vertCount;
  uint16_t triOffset;
  uint16_t triCount;
  XSurfaceCollisionTree *collisionTree;
};

typedef uint16_t XSurfaceFlags;

PACKED(struct XSurfaceShared {
  uint32_t flags;
  uint32_t vertCount;
  uint32_t skinVertCount;
  uint32_t indexCount;
  uint32_t blendTargetVertCount;
  uint32_t blendShapeVertCount;
  uint8_t *data;
  uint32_t dataSize;
  uint32_t posOffset;
  uint32_t vertsOffset;
  uint32_t indicesOffset;
  uint32_t skinWeightsOffset;
  uint32_t blendShapeVertsOffset;
  gfx::GfxVertexBuffer vbPos;
  gfx::GfxVertexBuffer vbVerts;
  gfx::GfxVertexBuffer vbSkinWeights;
  gfx::GfxIndexBuffer ibIndices;
  gfx::GfxBuffer bBlendTargets;
  uint32_t blendTargetCount;
  uint8_t _padding7C[4];
  const scr::ScrString_t *blendTargetNames;
  gfx::BlendTargetRemap *blendTargetRemap;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"flags\": 0x{:08X}, \"vertCount\": {}, \"skinVertCount\": {}, "
        "\"indexCount\": {}, "
        "\"blendTargetVertCount\": {}, \"blendShapeVertCount\": {}, \"data\": "
        "{:p}, \"dataSize\": {}, "
        "\"posOffset\": {}, \"vertsOffset\": {}, \"indicesOffset\": {}, "
        "\"skinWeightsOffset\": {}, "
        "\"blendShapeVertsOffset\": {}, \"blendTargetCount\": {}, "
        "\"blendTargetNames\": {}, "
        "\"blendTargetRemap\": {} }}",
        flags, vertCount, skinVertCount, indexCount, blendTargetVertCount,
        blendShapeVertCount, reinterpret_cast<const void *>(data), dataSize,
        posOffset, vertsOffset, indicesOffset, skinWeightsOffset,
        blendShapeVertsOffset, blendTargetCount,
        ::serialize(blendTargetNames, blendTargetCount),
        ::serialize(blendTargetRemap, blendTargetCount));
  }
});

PACKED(struct XSurface {
  uint8_t tileMode;
  uint8_t vertListCount;
  XSurfaceFlags flags;
  uint16_t vertCount;
  uint16_t triCount;
  uint32_t baseVertOffset;
  uint32_t baseIndexOffset;
  XSurfaceShared *shared;
  XRigidVertList *vertList;
  XPartBits partBits;
  const char *name;
  uint8_t _padding58[8];
});

} // namespace xsurface
} // namespace xasset
} // namespace db
} // namespace game
