#pragma once

#include "game/structs/db/xasset/core.hpp"
#include "game/structs/db/xasset/xbone.hpp"
#include <game/structs/fp16.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"surfs\": {:p}, \"shared\": {}, \"xpakEntry\": "
        "{}, "
        "\"partBits\": {}, \"avgRenderTriArea\": {}, \"avgCollisionTriArea\": "
        "{}, "
        "\"nameHash\": 0x{:08X}, \"numSurfs\": {}, \"lodEstimate\": {} }}",
        ::serialize(name), reinterpret_cast<const void *>(surfs),
        ::serialize(shared), xpakEntry.serialize(), ::serialize(partBits),
        avgRenderTriArea, avgCollisionTriArea, nameHash, numSurfs, lodEstimate);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"numLods\": {}, \"areaScale\": {}, \"cullOutRadius\": {}, "
        "\"areaScaleBias\": {}, \"cullOutRadiusScale\": {}, "
        "\"averageTriAreas\": {} }}",
        numLods, areaScale, cullOutRadius, ::serialize(areaScaleBias),
        ::serialize(cullOutRadiusScale), ::serialize(averageTriAreas));
  }
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

inline constexpr const char *serialize(const XModelLod lod) {
  switch (lod) {
  case XModelLod::XMODEL_LOD_NONE:
    return "XMODEL_LOD_NONE";
  case XModelLod::XMODEL_LOD_0:
    return "XMODEL_LOD_0";
  case XModelLod::XMODEL_LOD_1:
    return "XMODEL_LOD_1";
  case XModelLod::XMODEL_LOD_2:
    return "XMODEL_LOD_2";
  case XModelLod::XMODEL_LOD_3:
    return "XMODEL_LOD_3";
  case XModelLod::XMODEL_LOD_4:
    return "XMODEL_LOD_4";
  case XModelLod::XMODEL_LOD_5:
    return "XMODEL_LOD_5";
  case XModelLod::XMODEL_LOD_6:
    return "XMODEL_LOD_6";
  case XModelLod::XMODEL_LOD_7:
    return "XMODEL_LOD_7";
  default:
    return "XMODEL_LOD_INVALID";
  }
}

PACKED(struct XModelMeshMaterials {
  uint16_t numMaterials;
  uint8_t _padding02[6];
  Material **materials;
  vec_t *himipInvSqRadii;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"numMaterials\": {}, \"materials\": {:p}, "
                       "\"himipInvSqRadii\": {} }}",
                       numMaterials, reinterpret_cast<const void *>(materials),
                       ::serialize(himipInvSqRadii));
  }
});

struct XModelCollTri_s {
  vec4_t plane;
  vec4_t svec;
  vec4_t tvec;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"plane\": {}, \"svec\": {}, \"tvec\": {} }}",
                       plane.serialize(), svec.serialize(), tvec.serialize());
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"collTris\": {}, \"numCollTris\": {}, \"mins\": {}, "
        "\"maxs\": {}, \"boneIdx\": 0x{:04X}, \"contents\": 0x{:08X}, "
        "\"surfFlags\": "
        "{:08X} }}",
        ::serialize(collTris, numCollTris), numCollTris, mins.serialize(),
        maxs.serialize(), boneIdx, contents, surfFlags);
  }
};

typedef XModelCollSurf_s XModelCollSurf;

struct XModel;
PACKED(struct XSubmodel {
  XModel *model;
  scr::ScrString_t boneName;
  vec3_t trans;
  vec3_t rot;
  uint8_t _padding24[4];

  std::string serialize() const noexcept;
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

  std::string serialize() const noexcept;
});

inline std::string XSubmodel::serialize() const noexcept {
  return std::format(
      "{{ \"model\": {}, \"boneName\": 0x{:08X}, \"trans\": {}, \"rot\": {} }}",
      ::serialize(model), boneName, trans.serialize(), rot.serialize());
}

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
