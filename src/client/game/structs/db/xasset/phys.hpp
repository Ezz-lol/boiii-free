#pragma once

#include "game/structs/db/xasset/core.hpp"
#include <cstdint>

namespace game {

namespace snd {
struct SoundsImpactTable;
typedef SoundsImpactTable *SoundsImpactTablePtr;
} // namespace snd

namespace db {
namespace xasset {
namespace phys {

struct PhysPreset {
  const char *name;
  int32_t flags;
  float mass;
  float bounce;
  float friction;
  float damping_linear;
  float damping_angular;
  float bulletForceScale;
  float explosiveForceScale;
  const char *sndAliasPrefix;
  int32_t canFloat;
  float gravityScale;
  vec3_t centerOfMassOffset;
  vec3_t buoyancyBoxMin;
  vec3_t buoyancyBoxMax;
  db::xasset::FxEffectDefHandle trailFX;
  FxImpactTablePtr impactFX;
  snd::SoundsImpactTablePtr impactSounds;
};
// Verified
ASSERT_SIZE(PhysPreset, 0x78);

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

enum class ConstraintType : uint32_t {
  NONE = 0x0,
  POINT = 0x1,
  DISTANCE = 0x2,
  HINGE = 0x3,
  ACTUATOR = 0x4,
  FAKE_SHAKE = 0x5,
  LAUNCH = 0x6,
  ANTENNA = 0x7,
  ROPE = 0x8,
  LIGHT = 0x9,
  COUNT = 0xA,
};

enum AttachPointType : uint32_t {
  WORLD = 0x0,
  DYNENT = 0x1,
  ENT = 0x2,
  BONE = 0x3,
};

PACKED(struct PhysConstraint {
  scr::ScrString_t targetname;
  ConstraintType type;
  AttachPointType attach_point_type1;
  int32_t target_index1;
  scr::ScrString_t target_ent1;
  scr::ScrString_t target_bone1;
  AttachPointType attach_point_type2;
  int32_t target_index2;
  scr::ScrString_t target_ent2;
  scr::ScrString_t target_bone2;
  vec3_t offset;
  vec3_t pos;
  vec3_t pos2;
  vec3_t dir;
  int32_t flags;
  int32_t timeout;
  int32_t min_health;
  int32_t max_health;
  float distance;
  float damp;
  float power;
  vec3_t force_scale;
  float misc_scale;
  float minAngle;
  float maxAngle;
  float minAngleYaw;
  float maxAngleYaw;
  uint8_t _padding94[4];
  db::xasset::MaterialHandle material;
  int32_t model;
  int32_t slice_count;
  float gravity;
  qboolean useAntennaXAxis;
  intptr_t constraintHandle;
  int32_t rope_index;
  int32_t centity_num[5];
});
ASSERT_SIZE(PhysConstraint, 0xD0);

PACKED(struct PhysConstraints {
  const char *name;
  uint32_t count;
  uint8_t _padding0C[4];
  PhysConstraint data[8];
});
// Verified
ASSERT_SIZE(PhysConstraints, 0x690);
typedef PhysConstraints *PhysConstraintsPtr;
} // namespace phys
} // namespace xasset
} // namespace db
} // namespace game