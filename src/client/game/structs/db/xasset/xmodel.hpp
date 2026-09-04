#pragma once

#include <cstdint>
#include <game/structs/macros.hpp>
#include "game/structs/db/xasset/core.hpp"
#include "game/structs/db/xasset/xbone.hpp"

namespace game {

namespace fp16 {
struct half4_pack;
}

namespace anim {
struct DObjAnimMat;
}

namespace db {
namespace xasset {

namespace phys {
struct PhysPreset;
}

namespace xmodel {

// Verified
struct __declspec(align(4)) XModelMesh {
  const char *name;
  XSurface *surfs;
  XSurfaceShared *shared;
  XPakEntryInfo xpakEntry;
  XPartBits partBits;
  float avgRenderTriArea;
  float avgCollisionTriArea;
  uint32_t nameHash;
  uint8_t numSurfs;
  uint8_t lodEstimate;
};
ASSERT_SIZE(XModelMesh, 0x78);

// TODO
struct XModel;
typedef XModel *XModelPtr;

struct GfxLodModelParams {
  uint32_t numLods;
  float areaScale;
  float cullOutRadius;
  float areaScaleBias[3];
  float cullOutRadiusScale[3];
  union {
    float averageTriAreas[8];
    float manualDistances[8];
  };
};

enum class XModelLod : int8_t {
  XMODEL_LOD_NONE = -1,
  XMODEL_LOD_0 = 0x0,
  XMODEL_LOD_1 = 0x1,
  XMODEL_LOD_2 = 0x2,
  XMODEL_LOD_3 = 0x3,
  XMODEL_LOD_4 = 0x4,
  XMODEL_LOD_5 = 0x5,
  XMODEL_LOD_6 = 0x6,
  XMODEL_LOD_7 = 0x7,
  XMODEL_LOD_COUNT = 0x8,
  XMODEL_LOD_LOW = 0x0,
  XMODEL_LOD_HIGH = 0x7,
};

PACKED(struct XModelMeshMaterials {
  uint16_t numMaterials;
  uint8_t _padding02[6];
  Material **materials;
  vec_t *himipInvSqRadii;
});

struct XModelCollTri_s {
  vec4_t plane;
  vec4_t svec;
  vec4_t tvec;
};

typedef XModelCollTri_s XModelCollTri;

struct XModelCollSurf_s {
  XModelCollTri *collTris;
  int numCollTris;
  vec3_t mins;
  vec3_t maxs;
  BoneIndex boneIdx;
  uint8_t _padding26[2];
  contents_t contents;
  int surfFlags;
};

typedef XModelCollSurf_s XModelCollSurf;

struct XModel;
PACKED(struct XSubmodel {
  XModel *model;
  scr::ScrString_t boneName;
  vec3_t trans;
  vec3_t rot;
  uint8_t _padding24[4];
});

typedef uint64_t XModelUsage;
PACKED(struct XModel {
  const char *name;
  uint8_t numBones;
  uint8_t numRootBones;
  uint16_t numCosmeticBones;
  uint8_t _padding0C[4];
  const scr::ScrString_t *boneNames;
  const byte *parentList;
  const fp16::half4_pack *quats;
  const float *trans;
  const byte *partClassification;
  anim::DObjAnimMat *baseMat;
  GfxLodModelParams lodParams;
  XModelLod highestShadowLod;
  XModelLod highestRenderLod;
  uint8_t _padding86[2];
  XModelMeshPtr meshes[8];
  XModelMeshMaterials *meshMaterials;
  XModelUsage usage;
  XModelCollSurf *collSurfs;
  int32_t numCollSurfs;
  contents_t contents;
  const xbone::XBoneInfo *boneInfo;
  float radius;
  vec3_t mins;
  vec3_t maxs;
  uint8_t _padding10C[4];
  XModelMeshPtr collLod;
  byte numCollmaps;
  byte useMe;
  uint8_t _padding11A[2];
  int32_t flags;
  void *_unknown;
  db::xasset::phys::PhysPreset *physPreset;
  Collmap *collmaps;
  phys::PhysConstraints *physConstraints;
  int32_t numSubmodels;
  uint8_t _padding144[4];
  XSubmodel *submodels;
  uint32_t nameHash;
  XPartBits combinedPartBits;
  uint8_t _padding184[4];
});
// Verified
ASSERT_SIZE(XModel, 0x188);
typedef XModel *XModelPtr;

struct XModelAlias {
  const char *name;
  XModelPtr model[50];
};
// Verified
ASSERT_SIZE(XModelAlias, 0x198);

typedef XModelAlias *XModelAliasPtr;

} // namespace xmodel
} // namespace xasset
} // namespace db
} // namespace game