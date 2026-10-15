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

struct PhysPreset : NamedXAsset {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"flags\": 0x{:08X}, \"mass\": {}, \"bounce\": {}, "
        "\"friction\": {}, \"damping_linear\": {}, \"damping_angular\": {}, "
        "\"bulletForceScale\": {}, "
        "\"explosiveForceScale\": {}, \"sndAliasPrefix\": {}, \"canFloat\": "
        "{}, \"gravityScale\": {}, "
        "\"centerOfMassOffset\": {}, \"buoyancyBoxMin\": {}, "
        "\"buoyancyBoxMax\": {}, \"trailFX\": {:p}, "
        "\"impactFX\": {:p}, \"impactSounds\": {:p} }}",
        ::serialize(name), flags, mass, bounce, friction, damping_linear,
        damping_angular, bulletForceScale, explosiveForceScale,
        ::serialize(sndAliasPrefix), canFloat, gravityScale,
        centerOfMassOffset.serialize(), buoyancyBoxMin.serialize(),
        buoyancyBoxMax.serialize(), reinterpret_cast<const void *>(trailFX),
        reinterpret_cast<const void *>(impactFX),
        reinterpret_cast<const void *>(impactSounds));
  }
};
// Verified
ASSERT_SIZE(PhysPreset, 0x78);

struct cplane_t {
  vec3_t normal;
  float dist;
  uint8_t type;
  uint8_t signbits;
  uint8_t pad[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"normal\": {}, \"dist\": {}, \"type\": {}, "
                       "\"signbits\": {}, \"pad\": {} }}",
                       normal.serialize(), dist, type, signbits,
                       ::serialize(pad));
  }
};

struct cbrushside_t {
  cplane_t *plane;
  contents_t cflags;
  int32_t sflags;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"plane\": {}, \"cflags\": 0x{:08X}, \"sflags\": 0x{:08X} }}",
        ::serialize(plane), cflags, sflags);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"mins\": {}, \"contents\": 0x{:08X}, \"maxs\": {}, "
        "\"numsides\": {}, "
        "\"sides\": {}, \"axial_cflags\": {}, \"axial_sflags\": "
        "{}, \"numverts\": {}, "
        "\"verts\": {} }}",
        mins.serialize(), contents, maxs.serialize(), numsides,
        ::serialize(sides),
        /* Adding matrix serialization overloads for all the array serialization
           util functions will be a pain. Leave as TODO for now. */
        "TODO", "TODO", numverts, ::serialize(verts));
  }
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"mins\": {}, \"contents\": 0x{:08X}, \"maxs\": {}, "
                       "\"numsides\": {}, "
                       "\"sides\": {}, \"axial_cflags\": {}, \"axial_sflags\": "
                       "{}, \"numverts\": {}, "
                       "\"verts\": {}, \"planes\": {} }}",
                       mins.serialize(), contents, maxs.serialize(), numsides,
                       ::serialize(sides),
                       /* Adding matrix serialization overloads for all the
                          array serialization util functions will be a pain.
                          Leave as TODO for now. */
                       "TODO", "TODO", numverts, ::serialize(verts),
                       ::serialize(planes));
  }
};

struct PhysGeomInfo {
  BrushWrapper *brush;
  int32_t type;
  vec3_t orientation[3];
  vec3_t offset;
  vec3_t halfLengths;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"brush\": {}, \"type\": {}, \"orientation\": {}, "
                       "\"offset\": {}, \"halfLengths\": {} }}",
                       ::serialize(brush), type, ::serialize(orientation),
                       offset.serialize(), halfLengths.serialize());
  }
};

struct __attribute__((aligned(8))) PhysGeomList {
  uint32_t count;
  PhysGeomInfo *geoms;
  contents_t contents;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"count\": {}, \"geoms\": {}, \"contents\": 0x{:08X} }}", count,
        ::serialize(geoms), contents);
  }
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

inline constexpr const char *serialize(const ConstraintType type) {
  switch (type) {
  case ConstraintType::NONE:
    return "CONSTRAINT_NONE";
  case ConstraintType::POINT:
    return "CONSTRAINT_POINT";
  case ConstraintType::DISTANCE:
    return "CONSTRAINT_DISTANCE";
  case ConstraintType::HINGE:
    return "CONSTRAINT_HINGE";
  case ConstraintType::ACTUATOR:
    return "CONSTRAINT_ACTUATOR";
  case ConstraintType::FAKE_SHAKE:
    return "CONSTRAINT_FAKE_SHAKE";
  case ConstraintType::LAUNCH:
    return "CONSTRAINT_LAUNCH";
  case ConstraintType::ANTENNA:
    return "CONSTRAINT_ANTENNA";
  case ConstraintType::ROPE:
    return "CONSTRAINT_ROPE";
  case ConstraintType::LIGHT:
    return "CONSTRAINT_LIGHT";
  default:
    return "CONSTRAINT_INVALID";
  }
}

enum AttachPointType : uint32_t {
  WORLD = 0x0,
  DYNENT = 0x1,
  ENT = 0x2,
  BONE = 0x3,
};

inline constexpr const char *serialize(const AttachPointType type) {
  switch (type) {
  case AttachPointType::WORLD:
    return "ATTACH_WORLD";
  case AttachPointType::DYNENT:
    return "ATTACH_DYNENT";
  case AttachPointType::ENT:
    return "ATTACH_ENT";
  case AttachPointType::BONE:
    return "ATTACH_BONE";
  default:
    return "ATTACH_INVALID";
  }
}

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"targetname\": 0x{:08X}, \"type\": {}, \"attach_point_type1\": "
        "{}, "
        "\"target_index1\": {}, \"target_ent1\": 0x{:08X}, \"target_bone1\": "
        "{:08X}, "
        "\"attach_point_type2\": {}, \"target_index2\": {}, \"target_ent2\": "
        "{:08X}, "
        "\"target_bone2\": 0x{:08X}, \"offset\": {}, \"pos\": {}, \"pos2\": "
        "{}, "
        "\"dir\": {}, \"flags\": 0x{:08X}, \"timeout\": {}, \"min_health\": "
        "{}, "
        "\"max_health\": {}, \"distance\": {}, \"damp\": {}, \"power\": {}, "
        "\"force_scale\": {}, \"misc_scale\": {}, \"minAngle\": {}, "
        "\"maxAngle\": {}, "
        "\"minAngleYaw\": {}, \"maxAngleYaw\": {}, \"material\": {:p}, "
        "\"model\": {}, "
        "\"slice_count\": {}, \"gravity\": {}, \"useAntennaXAxis\": {}, "
        "\"constraintHandle\": {:p}, \"rope_index\": {}, \"centity_num\": {} "
        "}}",
        targetname, game::db::xasset::phys::serialize(type),
        game::db::xasset::phys::serialize(attach_point_type1), target_index1,
        target_ent1, target_bone1,
        game::db::xasset::phys::serialize(attach_point_type2), target_index2,
        target_ent2, target_bone2, offset.serialize(), pos.serialize(),
        pos2.serialize(), dir.serialize(), flags, timeout, min_health,
        max_health, distance, damp, power, force_scale.serialize(), misc_scale,
        minAngle, maxAngle, minAngleYaw, maxAngleYaw,
        reinterpret_cast<const void *>(material), model, slice_count, gravity,
        useAntennaXAxis.serialize(),
        reinterpret_cast<const void *>(constraintHandle), rope_index,
        ::serialize(centity_num));
  }
});
ASSERT_SIZE(PhysConstraint, 0xD0);

PACKED(struct PhysConstraints : NamedXAsset {
  uint32_t count;
  uint8_t _padding0C[4];
  PhysConstraint data[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"count\": {}, \"data\": {} }}",
                       ::serialize(name), count, ::serialize(data, count));
  }
});
// Verified
ASSERT_SIZE(PhysConstraints, 0x690);
typedef PhysConstraints *PhysConstraintsPtr;
} // namespace phys
} // namespace xasset
} // namespace db
} // namespace game
