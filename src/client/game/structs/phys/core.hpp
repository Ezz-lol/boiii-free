#pragma once

#include <cstdint>
#include <game/structs/asm.hpp>
#include <game/structs/core.hpp>
#include <game/structs/db/xasset/destructible.hpp>
#include <game/structs/db/xasset/phys.hpp>
#include <game/structs/math.hpp>
#include <game/structs/snd/snd.hpp>
#include <game/symbol.hpp>

namespace game {

namespace vehicle {
struct NitrousVehicle;
}

namespace level {
struct gentity_s;
typedef gentity_s gentity_t;
} // namespace level

namespace phys {

#pragma pack(push, 1)

struct hitinfo_t {
  int32_t hittime;
  vec3_t hitp;
  vec3_t hitn;
  int32_t stype;
  int32_t entnum;
  float intensity;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"hittime\": {}, \"hitp\": {}, \"hitn\": {}, "
                       "\"stype\": {}, \"entnum\": {}, \"intensity\": {} }}",
                       hittime, hitp.serialize(), hitn.serialize(), stype,
                       entnum, intensity);
  }
};

enum class PhysicsOwnerType : int32_t {
  DYNENT = 0x0,
  ENTITY = 0x1,
  GLASS = 0x2,
  PARTICLE = 0x3,
  RAGDOLL = 0x4,
  XDOLL = 0x5,
  COUNT = 0x6,
};

inline constexpr const char *serialize(const PhysicsOwnerType type) {
  switch (type) {
  case PhysicsOwnerType::DYNENT:
    return "OWNER_DYNENT";
  case PhysicsOwnerType::ENTITY:
    return "OWNER_ENTITY";
  case PhysicsOwnerType::GLASS:
    return "OWNER_GLASS";
  case PhysicsOwnerType::PARTICLE:
    return "OWNER_PARTICLE";
  case PhysicsOwnerType::RAGDOLL:
    return "OWNER_RAGDOLL";
  case PhysicsOwnerType::XDOLL:
    return "OWNER_XDOLL";
  default:
    return "OWNER_INVALID";
  }
}

struct broad_phase_memory_info {
public:
  static constexpr symbol<thiscall_t<void(broad_phase_memory_info *)>>
      constructor{0x0, 0x0, 0x14000EEC0};

  inline broad_phase_memory_info() { constructor(this); }
  int32_t m_max_num_gjk_ci;
  int32_t m_max_num_sap_active_pairs;
  int32_t m_max_num_surface_types;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_max_num_gjk_ci\": {}, \"m_max_num_sap_active_pairs\": {}, "
        "\"m_max_num_surface_types\": {} }}",
        m_max_num_gjk_ci, m_max_num_sap_active_pairs, m_max_num_surface_types);
  }
};

struct broad_phase_base {
  math::Dir3 m_trace_aabb_min_whace;
  math::Dir3 m_trace_aabb_max_whace;
  math::Dir3 m_trace_translation;
  uint32_t m_flags;
  uint8_t _padding34[4];
  broad_phase_base *m_list_bpb_next;
  broad_phase_base *m_list_bpb_cluster_next;
  void *m_sap_node;
  void *m_user_data;
  uint32_t m_env_collision_flags;
  uint32_t m_my_collision_type_flags;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_trace_aabb_min_whace\": {}, \"m_trace_aabb_max_whace\": {}, "
        "\"m_trace_translation\": {}, "
        "\"m_flags\": 0x{:08X}, \"m_list_bpb_next\": {:p}, "
        "\"m_list_bpb_cluster_next\": {:p}, "
        "\"m_sap_node\": {:p}, \"m_user_data\": {:p}, "
        "\"m_env_collision_flags\": 0x{:08X}, "
        "\"m_my_collision_type_flags\": 0x{:08X} }}",
        m_trace_aabb_min_whace.serialize(), m_trace_aabb_max_whace.serialize(),
        m_trace_translation.serialize(), m_flags,
        reinterpret_cast<const void *>(m_list_bpb_next),
        reinterpret_cast<const void *>(m_list_bpb_cluster_next), m_sap_node,
        m_user_data, m_env_collision_flags, m_my_collision_type_flags);
  }
};
ASSERT_SIZE(broad_phase_base, 0x60);

template <typename Node> class phys_link_list_base {
public:
  Node *m_next_link;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_next_link\": {} }}", ::serialize(m_next_link));
  }
};

typedef vec3<math::Float4> phys_mat33;

struct rigid_body;

struct pulse_sum_node {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  phys_link_list_base<pulse_sum_node> base;
  phys_mat33 m_world_inv_inertia;
  uint8_t _padding38[8];
  math::Dir3 t_vel;
  math::Dir3 a_vel;
  float m_inv_mass;
  uint8_t _padding64[4];
  rigid_body *m_rb;

  std::string serialize() const noexcept;
};
ASSERT_SIZE(pulse_sum_node, 0x70);

typedef db::xasset::FxImpactTable *FxImpactTablePtr;

struct rigid_body_constraint_contact;
struct rigid_body_constraint_custom_path;
struct rigid_body_constraint_custom_orientation;
struct rigid_body_constraint_upright;
struct rigid_body_constraint_angular_actuator;
struct rigid_body_constraint_ragdoll;
struct rigid_body_constraint_distance;
struct rigid_body_constraint_hinge;

struct rigid_body_constraint;

struct rigid_body_constraint {
public:
  rigid_body *b1;
  rigid_body *b2;
  rigid_body_constraint *m_next;

  std::string serialize() const noexcept;
};
ASSERT_SIZE(rigid_body_constraint, 0x18);

struct pulse_sum_cache {
public:
  float m_pulse_sum;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_pulse_sum\": {} }}", m_pulse_sum);
  }
};
ASSERT_SIZE(pulse_sum_cache, 0x4);

struct rigid_body_constraint_point {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  math::Dir3 m_b1_r_loc;
  math::Dir3 m_b2_r_loc;
  pulse_sum_cache m_ps_cache_list[3];
  float m_stress;
  float m_spring_k;
  float m_damp_k;
  bool m_spring_enabled;
  uint8_t _padding59[7];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_b1_r_loc\": {}, \"m_b2_r_loc\": {}, "
        "\"m_ps_cache_list\": {}, \"m_stress\": {}, \"m_spring_k\": {}, "
        "\"m_damp_k\": {}, \"m_spring_enabled\": {} }}",
        base.serialize(), m_b1_r_loc.serialize(), m_b2_r_loc.serialize(),
        ::serialize(m_ps_cache_list), m_stress, m_spring_k, m_damp_k,
        ::serialize(m_spring_enabled));
  }
};
ASSERT_SIZE(rigid_body_constraint_point, 0x58);

struct pulse_sum_node;
struct rigid_body;
struct rigid_body_constraint_wheel;

struct rb_inplace_partition_node {
  rigid_body_constraint_point *m_rbc_point_first;
  rigid_body_constraint_hinge *m_rbc_hinge_first;
  rigid_body_constraint_distance *m_rbc_dist_first;
  rigid_body_constraint_ragdoll *m_rbc_ragdoll_first;
  rigid_body_constraint_wheel *m_rbc_wheel_first;
  rigid_body_constraint_angular_actuator *m_rbc_angular_actuator_first;
  rigid_body_constraint_upright *m_rbc_upright_first;
  rigid_body_constraint_custom_orientation *m_rbc_custom_orientation_first;
  rigid_body_constraint_custom_path *m_rbc_custom_path_first;
  rigid_body_constraint_contact *m_rbc_contact_first;
  rigid_body *m_partition_head;
  rigid_body *m_partition_tail;
  rigid_body *m_next_node;
  int32_t m_partition_size;
  uint8_t _padding6C[4];

  std::string serialize() const noexcept;
};
ASSERT_SIZE(rb_inplace_partition_node, 0x70);

struct rigid_body {
public:
  static constexpr symbol<thiscall_t<void(
      rigid_body *, const float mass, const math::Dir3 *inertia,
      const math::RotTranMat43 *mat, const math::Dir3 *t_vel,
      const math::Dir3 *a_vel, const int32_t stable_min_contact_count)>>
      set{0x1420C8840, 0x0, 0x14002C840};
  static constexpr symbol<
      thiscall_t<void(rigid_body *, const math::Dir3 *inertia)>>
      set_inertia{0x0, 0x0, 0x14002CDA0};

  math::Dir3 m_last_position;
  math::Dir3 m_moved_vec;
  float m_smallest_lambda;
  uint8_t _padding24[12];
  math::RotTranMat43 m_mat;
  math::Dir3 m_inv_inertia;
  math::Dir3 m_gravity_acc_vec;
  math::Dir3 m_t_vel;
  math::Dir3 m_a_vel;
  math::Dir3 m_last_t_vel;
  math::Dir3 m_last_a_vel;
  math::Dir3 m_force_sum;
  math::Dir3 m_torque_sum;
  float m_inv_mass;
  float m_max_avel;
  float m_max_delta_t;
  uint32_t m_flags;
  uint32_t m_tick;
  uint8_t _padding104[4];
  pulse_sum_node *m_node;
  int32_t m_constraint_count;
  int32_t m_contact_count;
  int32_t m_stable_min_contact_count;
  float m_stable_energy_time;
  float m_largest_vel_sq;
  float m_t_drag_coef;
  float m_a_drag_coef;
  uint8_t _padding12C[4];
  void *m_userdata;
  rb_inplace_partition_node m_partition_node;
  uint8_t _padding1A8[8];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_last_position\": {}, \"m_moved_vec\": {}, "
        "\"m_smallest_lambda\": {}, "
        "\"m_mat\": {}, \"m_inv_inertia\": {}, \"m_gravity_acc_vec\": {}, "
        "\"m_t_vel\": {}, "
        "\"m_a_vel\": {}, \"m_last_t_vel\": {}, \"m_last_a_vel\": {}, "
        "\"m_force_sum\": {}, "
        "\"m_torque_sum\": {}, \"m_inv_mass\": {}, \"m_max_avel\": {}, "
        "\"m_max_delta_t\": {}, "
        "\"m_flags\": 0x{:08X}, \"m_tick\": {}, \"m_node\": {:p}, "
        "\"m_constraint_count\": {}, "
        "\"m_contact_count\": {}, \"m_stable_min_contact_count\": {}, "
        "\"m_stable_energy_time\": {}, "
        "\"m_largest_vel_sq\": {}, \"m_t_drag_coef\": {}, \"m_a_drag_coef\": "
        "{}, "
        "\"m_userdata\": {:p}, \"m_partition_node\": {} }}",
        m_last_position.serialize(), m_moved_vec.serialize(), m_smallest_lambda,
        m_mat.serialize(), m_inv_inertia.serialize(),
        m_gravity_acc_vec.serialize(), m_t_vel.serialize(), m_a_vel.serialize(),
        m_last_t_vel.serialize(), m_last_a_vel.serialize(),
        m_force_sum.serialize(), m_torque_sum.serialize(), m_inv_mass,
        m_max_avel, m_max_delta_t, m_flags, m_tick,
        reinterpret_cast<const void *>(m_node), m_constraint_count,
        m_contact_count, m_stable_min_contact_count, m_stable_energy_time,
        m_largest_vel_sq, m_t_drag_coef, m_a_drag_coef, m_userdata,
        m_partition_node.serialize());
  }
};
ASSERT_SIZE(rigid_body, 0x1B0);

struct rigid_body_pair_key {
  rigid_body *m_b1;
  rigid_body *m_b2;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_b1\": {}, \"m_b2\": {} }}", ::serialize(m_b1),
                       ::serialize(m_b2));
  }
};
ASSERT_SIZE(rigid_body_pair_key, 0x10);

struct phys_gjk_geom {
public:
  // int32_t (**_vptr$phys_gjk_geom)(void);
  void *vtablePtr;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"vtablePtr\": {:p} }}", vtablePtr);
  }
};
ASSERT_SIZE(phys_gjk_geom, 0x8);

typedef contents_t gjk_contents_t;

typedef uint32_t phys_gjk_geom_id;
namespace gjk_unique_id_database_t {
typedef phys_gjk_geom_id unique_id;

struct gjk_unique_id_database_t {
public:
  unique_id m_counter;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_counter\": 0x{:08X} }}", m_counter);
  }
};

} // namespace gjk_unique_id_database_t

struct gjk_base_t {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  phys_gjk_geom base;
  uint8_t _padding00[8];
  math::Dir3 m_aabb_mn_loc;
  math::Dir3 m_aabb_mx_loc;
  gjk_unique_id_database_t::unique_id m_gjk_geom_id;
  uint8_t _padding34[4];
  const math::RotTranMat43 *m_xform_;
  gjk_base_t *m_next_geom;
  uint32_t m_flags;
  int32_t stype;
  gjk_contents_t m_contents;
  uint8_t _padding54[12];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_aabb_mn_loc\": {}, \"m_aabb_mx_loc\": {}, "
        "\"m_gjk_geom_id\": 0x{:08X}, \"m_xform_\": {}, \"m_next_geom\": {}, "
        "\"m_flags\": 0x{:08X}, \"stype\": {}, \"m_contents\": 0x{:08X} }}",
        base.serialize(), m_aabb_mn_loc.serialize(), m_aabb_mx_loc.serialize(),
        m_gjk_geom_id, ::serialize(m_xform_), ::serialize(m_next_geom), m_flags,
        stype, m_contents);
  }
};
ASSERT_SIZE(gjk_base_t, 0x60);

struct gjk_geom_list_t {
public:
  gjk_base_t *m_first_geom;
  int32_t m_geom_count;
  uint8_t _padding0C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_first_geom\": {}, \"m_geom_count\": {} }}",
                       ::serialize(m_first_geom), m_geom_count);
  }
};
ASSERT_SIZE(gjk_geom_list_t, 0x10);
struct PhysObjUserData {
public:
  const db::xasset::phys::PhysPreset *physPreset;
  rigid_body *body;
  vehicle::NitrousVehicle *vehicle;
  gjk_geom_list_t m_gjk_geom_list;
  PhysObjUserData *m_next_link;
  math::RotTranMat43 cg2rb;
  math::RotTranMat43 m2w;
  broad_phase_base *m_bpb;
  vec3_t tvel;
  vec3_t avel;
  float friction;
  float bounce;
  float damping_linear;
  float damping_angular;
  int32_t id;
  int32_t underwater;
  int32_t buoyancy;
  int32_t timeBuoyant;
  int32_t timeRipple;
  uint16_t trDuration;
  uint16_t refcount;
  float m_time_since_last_event;
  float m_time_since_last_reeval;
  int32_t m_flags;
  float mass;
  vec3_t buoyancyBoxMin;
  vec3_t buoyancyBoxMax;
  vec3_t centerOfMassOffset;
  int32_t owner;
  PhysicsOwnerType owner_type;
  hitinfo_t hitinfo;
  uint8_t _padding15C[4];

  std::string serialize() const noexcept;
};
ASSERT_SIZE(PhysObjUserData, 0x160);

struct ZBarrierBoard {
  db::xasset::xmodel::XModel *pBoardModel;
  db::xasset::xmodel::XModel *pAlternateBoardModel;
  db::xasset::xmodel::XModel *pUpgradedBoardModel;
  const char *pTearAnim;
  const char *pBoardAnim;
  db::xasset::FxEffectDefHandle repairEffect1;
  db::xasset::FxEffectDefHandle repairEffect2;
  vec3_t repairEffect1Offset;
  vec3_t repairEffect2Offset;
  uint32_t boardRepairSound;
  uint32_t boardRepairHoverSound;
  uint32_t pauseAndRepeatRepSound;
  float minPause;
  float maxPause;
  scr::ScrString_t zombieBoardTearStateName;
  scr::ScrString_t zombieBoardTearSubStateName;
  uint32_t numRepsToPullProBoard;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"pBoardModel\": {}, \"pAlternateBoardModel\": {}, "
        "\"pUpgradedBoardModel\": {}, "
        "\"pTearAnim\": {}, \"pBoardAnim\": {}, \"repairEffect1\": {}, "
        "\"repairEffect2\": {}, "
        "\"repairEffect1Offset\": {}, \"repairEffect2Offset\": {}, "
        "\"boardRepairSound\": 0x{:08X}, "
        "\"boardRepairHoverSound\": 0x{:08X}, \"pauseAndRepeatRepSound\": "
        "{:08X}, \"minPause\": {}, "
        "\"maxPause\": {}, \"zombieBoardTearStateName\": 0x{:08X}, "
        "\"zombieBoardTearSubStateName\": 0x{:08X}, "
        "\"numRepsToPullProBoard\": {} }}",
        ::serialize(pBoardModel), ::serialize(pAlternateBoardModel),
        ::serialize(pUpgradedBoardModel), ::serialize(pTearAnim),
        ::serialize(pBoardAnim), ::serialize(repairEffect1),
        ::serialize(repairEffect2), repairEffect1Offset.serialize(),
        repairEffect2Offset.serialize(), boardRepairSound,
        boardRepairHoverSound, pauseAndRepeatRepSound, minPause, maxPause,
        zombieBoardTearStateName, zombieBoardTearSubStateName,
        numRepsToPullProBoard);
  }
};

struct ZBarrierDef {
  const char *name;
  uint32_t generalRepairSound1;
  uint32_t generalRepairSound2;
  uint32_t upgradedGeneralRepairSound1;
  uint32_t upgradedGeneralRepairSound2;
  uint32_t delayBetweenRepSounds;
  float delayBetweenRepSoundsDuration;
  uint32_t earthquakeOnRepair;
  float earthquakeMinScale;
  float earthquakeMaxScale;
  float earthquakeMinDuration;
  float earthquakeMaxDuration;
  float earthquakeRadius;
  int32_t numBoardsInBarrier;
  uint32_t autoHideOpenPieces;
  uint32_t taunts;
  uint32_t reachThroughAttacks;
  scr::ScrString_t zombieTauntAnimState;
  scr::ScrString_t zombieReachThroughAnimState;
  int32_t numAttackSlots;
  float attackSpotHorzOffset;
  db::xasset::xmodel::XModel *pCollisionModel;
  ZBarrierBoard boards[6];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"generalRepairSound1\": 0x{:08X}, "
        "\"generalRepairSound2\": 0x{:08X}, "
        "\"upgradedGeneralRepairSound1\": 0x{:08X}, "
        "\"upgradedGeneralRepairSound2\": 0x{:08X}, "
        "\"delayBetweenRepSounds\": 0x{:08X}, "
        "\"delayBetweenRepSoundsDuration\": "
        "{}, \"earthquakeOnRepair\": 0x{:08X}, "
        "\"earthquakeMinScale\": {}, \"earthquakeMaxScale\": {}, "
        "\"earthquakeMinDuration\": {}, "
        "\"earthquakeMaxDuration\": {}, \"earthquakeRadius\": {}, "
        "\"numBoardsInBarrier\": {}, "
        "\"autoHideOpenPieces\": 0x{:08X}, \"taunts\": 0x{:08X}, "
        "\"reachThroughAttacks\": 0x{:08X}, "
        "\"zombieTauntAnimState\": 0x{:08X}, \"zombieReachThroughAnimState\": "
        "{:08X}, \"numAttackSlots\": {}, "
        "\"attackSpotHorzOffset\": {}, \"pCollisionModel\": {}, \"boards\": "
        "{} }}",
        ::serialize(name), generalRepairSound1, generalRepairSound2,
        upgradedGeneralRepairSound1, upgradedGeneralRepairSound2,
        delayBetweenRepSounds, delayBetweenRepSoundsDuration,
        earthquakeOnRepair, earthquakeMinScale, earthquakeMaxScale,
        earthquakeMinDuration, earthquakeMaxDuration, earthquakeRadius,
        numBoardsInBarrier, autoHideOpenPieces, taunts, reachThroughAttacks,
        zombieTauntAnimState, zombieReachThroughAnimState, numAttackSlots,
        attackSpotHorzOffset, ::serialize(pCollisionModel),
        ::serialize(boards));
  }
};
ASSERT_SIZE(ZBarrierDef, 0x300);

struct ZBarrierPieceAnims {
  anim::XAnimTree *pAnimTree;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"pAnimTree\": {} }}", ::serialize(pAnimTree));
  }
};

struct ZBarrierType {
  uint8_t serverIndex;
  ClientNum8_t clientIndex;
  uint8_t _padding02[6];
  ZBarrierDef *zbarrierDef;
  ZBarrierPieceAnims clientAnims[6];
  ZBarrierPieceAnims serverAnims[6];
  int32_t boardModelIndex[6];
  int32_t upgradedBoardModelIndex[6];
  int32_t alternateBoardModelIndex[6];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"serverIndex\": {}, \"clientIndex\": {}, \"zbarrierDef\": {}, "
        "\"clientAnims\": {}, \"serverAnims\": {}, \"boardModelIndex\": {}, "
        "\"upgradedBoardModelIndex\": {}, \"alternateBoardModelIndex\": {} }}",
        serverIndex, game::serialize(clientIndex), ::serialize(zbarrierDef),
        ::serialize(clientAnims), ::serialize(serverAnims),
        ::serialize(boardModelIndex), ::serialize(upgradedBoardModelIndex),
        ::serialize(alternateBoardModelIndex));
  }
};
ASSERT_SIZE(ZBarrierType, 0xB8);

struct cached_tag_mat_t {
  int32_t time;
  int32_t entnum;
  scr::ScrString_t name;
  vec3_t tagMat[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"time\": {}, \"entnum\": {}, \"name\": 0x{:08X}, "
                       "\"tagMat\": {} }}",
                       time, entnum, name, ::serialize(tagMat));
  }
};
ASSERT_SIZE(cached_tag_mat_t, 0x3C);

struct trigger_info_t {
  uint16_t entnum;
  uint16_t otherEntnum;
  int32_t useCount;
  int32_t otherUseCount;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"entnum\": {}, \"otherEntnum\": {}, \"useCount\": "
                       "{}, \"otherUseCount\": {} }}",
                       entnum, otherEntnum, useCount, otherUseCount);
  }
};
ASSERT_SIZE(trigger_info_t, 0xC);

struct sv_FxVisBlock_t {
  const level::gentity_t *fxEnt;
  float radius;
  uint8_t _padding0C[4];

  std::string serialize() const noexcept;
};
ASSERT_SIZE(sv_FxVisBlock_t, 0x10);

struct actor_prone_info_t {
  bool bCorpseOrientation;
  bool orientPitch;
  bool prone;
  uint8_t _padding03[1];
  int32_t iProneTime;
  int32_t iProneTrans;
  float fBodyHeight;
  union {
    float fTorsoPitch;
    float fBodyPitch;
  };
  union {
    float fWaistPitch;
    float fBodyRoll;
  };

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"bCorpseOrientation\": {}, \"orientPitch\": {}, \"prone\": {}, "
        "\"iProneTime\": {}, \"iProneTrans\": {}, \"fBodyHeight\": {}, "
        "\"fTorsoPitch\": {}, \"fWaistPitch\": {} }}",
        ::serialize(bCorpseOrientation), ::serialize(orientPitch),
        ::serialize(prone), iProneTime, iProneTrans, fBodyHeight, fTorsoPitch,
        fWaistPitch);
  }
};
ASSERT_SIZE(actor_prone_info_t, 0x18);

struct actorAntilagFrame_t {
  vec3_t position[64];
  vec3_t angles[64];
  uint8_t useCount[64];
  uint8_t inUse[64];
  int32_t time;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"position\": {}, \"angles\": {}, \"useCount\": {}, "
                       "\"inUse\": {}, \"time\": {} }}",
                       ::serialize(position), ::serialize(angles),
                       ::serialize(useCount), ::serialize(inUse), time);
  }
};
ASSERT_SIZE(actorAntilagFrame_t, 0x684);

enum class objcamState : uint32_t {
  OCS_UNUSED = 0x0,
  OCS_DISABLED = 0x1,
  OCS_ENABLED = 0x2,
};

inline constexpr const char *serialize(const objcamState state) {
  switch (state) {
  case objcamState::OCS_UNUSED:
    return "OCS_UNUSED";
  case objcamState::OCS_DISABLED:
    return "OCS_DISABLED";
  case objcamState::OCS_ENABLED:
    return "OCS_ENABLED";
  default:
    return "OCS_INVALID";
  }
}

#pragma pack(pop)

typedef int32_t FxUniqueHandle;

struct LocalClientFxUniqueHandle {
  LocalClientNum_t localClient;
  FxUniqueHandle fxHandle;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"localClient\": {}, \"fxHandle\": {} }}",
                       game::serialize(localClient), fxHandle);
  }
};

struct DestructibleBurnData {
  int32_t burnTime;
  FxUniqueHandle fx;
  snd::SndPlaybackId sndId;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"burnTime\": {}, \"fx\": {}, \"sndId\": 0x{:08X} }}", burnTime, fx,
        sndId);
  }
};

struct DESTRUCTIBLE_PIECE_INFO {
  int16_t health;
  uint8_t _padding02[2];
  int32_t xdollHandle;
  FxUniqueHandle fx;
  DestructibleBurnData burnData;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"health\": {}, \"xdollHandle\": {}, \"fx\": 0x{:08X}, "
        "\"burnData\": {} }}",
        health, xdollHandle, fx, burnData.serialize());
  }
};

struct LerpEntityStateDestructibleHit {
  uint32_t modelState[6];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"modelState\": {} }}", ::serialize(modelState));
  }
};

struct DestructibleState {
  LerpEntityStateDestructibleHit state;
  int32_t time;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"state\": {}, \"time\": {} }}", state.serialize(),
                       time);
  }
};

PACKED(struct Destructible {
  int32_t entNum;
  uint8_t _padding04[4];
  DESTRUCTIBLE_PIECE_INFO *pieceArray;
  int32_t oldestBurnTime;
  bool entDestroyed;
  bool entDestroyedRespondedTo;
  uint8_t _padding16[2];
  int32_t pieceCount;
  uint8_t _padding1C[4];
  db::xasset::destructible::DestructibleDef *ddef;
  uint32_t flags;
  DestructibleState states[5];
  bool bHasBeenHit;
  uint8_t _paddingB9[7];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"entNum\": {}, \"pieceArray\": {}, \"oldestBurnTime\": {}, "
        "\"entDestroyed\": {}, \"entDestroyedRespondedTo\": {}, "
        "\"pieceCount\": {}, "
        "\"ddef\": {}, \"flags\": 0x{:08X}, \"states\": {}, \"bHasBeenHit\": "
        "{} }}",
        entNum, ::serialize(pieceArray), oldestBurnTime,
        ::serialize(entDestroyed), ::serialize(entDestroyedRespondedTo),
        pieceCount, ::serialize(ddef), flags, ::serialize(states),
        ::serialize(bHasBeenHit));
  }
});
ASSERT_SIZE(Destructible, 0xC0);

typedef intptr_t PhysObjId;

PACKED(union CollisionAabbTreeIndex {
  int32_t firstChildIndex;
  int32_t partitionIndex;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"firstChildIndex\": {} }}", firstChildIndex);
  }
});
ASSERT_SIZE(CollisionAabbTreeIndex, 0x4);

PACKED(struct CollisionAabbTree {
  vec3_t origin;
  uint16_t materialIndex;
  uint16_t childCount;
  vec3_t halfSize;
  CollisionAabbTreeIndex u;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"origin\": {}, \"materialIndex\": {}, "
                       "\"childCount\": {}, \"halfSize\": {}, \"u\": {} }}",
                       origin.serialize(), materialIndex, childCount,
                       halfSize.serialize(), u.serialize());
  }
});
ASSERT_SIZE(CollisionAabbTree, 0x20);

PACKED(struct CollisionPartition {
  uint8_t triCount;
  uint8_t _padding01[3];
  int32_t firstTri;
  int32_t nuinds;
  int32_t fuind;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"triCount\": {}, \"firstTri\": {}, \"nuinds\": {}, "
                       "\"fuind\": {} }}",
                       triCount, firstTri, nuinds, fuind);
  }
});
ASSERT_SIZE(CollisionPartition, 0x10);

struct cbrush_t; // TODO

PACKED(struct col_prim_t {
  int32_t type;
  uint8_t _padding04[4];
  union {
    const CollisionAabbTree *tree;
    const cbrush_t *brush;
  };

  inline std::string serialize() const noexcept {
    return std::format("{{ \"type\": {}, \"tree\": {:p} }}", type,
                       reinterpret_cast<const void *>(tree));
  }
});
ASSERT_SIZE(col_prim_t, 0x10);

typedef vec4_t float4;

PACKED(struct hybrid_vector {
public:
  float4 vec;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"vec\": {} }}", vec.serialize());
  }
});
ASSERT_SIZE(hybrid_vector, 0x10);

PACKED(struct visitor_base_t {
public:
  int32_t (**_vptr$visitor_base_t)(void);

  inline std::string serialize() const noexcept {
    return std::format("{{ \"vtable\": {:p} }}",
                       reinterpret_cast<const void *>(_vptr$visitor_base_t));
  }
});

struct TraceThreadInfo; // TODO

PACKED(struct colgeom_visitor_t {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  visitor_base_t base;
  uint8_t _padding0[0xF];
  hybrid_vector m_mn;
  hybrid_vector m_mx;
  hybrid_vector m_p0;
  hybrid_vector m_p1;
  hybrid_vector m_delta;
  hybrid_vector m_rvec;
  float m_radius;
  contents_t m_mask;
  TraceThreadInfo *m_threadInfo;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_radius\": {}, \"m_mask\": 0x{:08X}, "
        "\"m_threadInfo\": {:p} }}",
        base.serialize(), m_radius, m_mask,
        reinterpret_cast<const void *>(m_threadInfo));
  }
});

#pragma pack(push, 16)
template <size_t T> class colgeom_visitor_inlined_t {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  colgeom_visitor_t base;
  int32_t nprims;
  bool overflow;
  col_prim_t prims[T];
};
#pragma pack(pop)

struct IgnoreEntParams {
  int baseEntity;
  int parentEntity;
  int ignoreEntType;
  bool ignoreSelf;
  bool ignoreParent;
  bool ignoreSiblings;
  bool ignoreChildren;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"baseEntity\": {}, \"parentEntity\": {}, \"ignoreEntType\": {}, "
        "\"ignoreSelf\": {}, \"ignoreParent\": {}, \"ignoreSiblings\": {}, "
        "\"ignoreChildren\": {} }}",
        baseEntity, parentEntity, ignoreEntType, ::serialize(ignoreSelf),
        ::serialize(ignoreParent), ::serialize(ignoreSiblings),
        ::serialize(ignoreChildren));
  }
};
typedef fastcallPtr_t<qboolean(int)> collide_entity_func_ptr;

struct col_context_t {
  contents_t mask;
  const col_prim_t *prims;
  int nprims;
  IgnoreEntParams *ignoreEntParams;
  int passEntityNum0;
  int passEntityNum1;
  qboolean staticmodels;
  qboolean locational;
  uint8_t *priorityMap;
  collide_entity_func_ptr collide_entity_func;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"mask\": 0x{:08X}, \"prims\": {}, \"nprims\": {}, "
        "\"ignoreEntParams\": {}, "
        "\"passEntityNum0\": {}, \"passEntityNum1\": {}, "
        "\"staticmodels\": {}, \"locational\": {}, "
        "\"priorityMap\": {:p} }}",
        mask, ::serialize(prims), nprims, ::serialize(ignoreEntParams),
        passEntityNum0, passEntityNum1, staticmodels.serialize(),
        locational.serialize(), reinterpret_cast<const void *>(priorityMap));
  }
};

enum class JointType : int32_t {
  NONE = 0x0,
  HINGE = 0x1,
  SWIVEL = 0x2,
  COUNT = 0x3,
};

inline constexpr const char *serialize(const JointType type) {
  switch (type) {
  case JointType::NONE:
    return "JOINT_NONE";
  case JointType::HINGE:
    return "JOINT_HINGE";
  case JointType::SWIVEL:
    return "JOINT_SWIVEL";
  default:
    return "JOINT_INVALID";
  }
}

struct __attribute__((aligned(4))) SelfCollisionPair {
  scr::ScrString_t boneNames[2];
  uint8_t bonePair[2];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"boneNames\": {}, \"bonePair\": {} }}",
                       ::serialize(boneNames), ::serialize(bonePair));
  }
};

struct __attribute__((aligned(4))) RagdollBoneDef {
  scr::ScrString_t ragdollBone;
  scr::ScrString_t childBone;
  float radius;
  float mass;
  float friction;
  float damping;
  float buoyancy;
  int32_t geomType;
  scr::ScrString_t parentBone;
  JointType jointType;
  float musclePowerOn;
  float musclePowerCollide;
  float musclePowerDown;
  vec3_t minAngles;
  vec3_t maxAngles;
  vec3_t axisFriction;
  bool mirror;
  uint8_t parentIndex;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"ragdollBone\": 0x{:08X}, \"childBone\": 0x{:08X}, \"radius\": "
        "{}, "
        "\"mass\": {}, \"friction\": {}, \"damping\": {}, \"buoyancy\": {}, "
        "\"geomType\": {}, \"parentBone\": 0x{:08X}, \"jointType\": {}, "
        "\"musclePowerOn\": {}, \"musclePowerCollide\": {}, "
        "\"musclePowerDown\": {}, "
        "\"minAngles\": {}, \"maxAngles\": {}, \"axisFriction\": {}, "
        "\"mirror\": {}, "
        "\"parentIndex\": {} }}",
        ragdollBone, childBone, radius, mass, friction, damping, buoyancy,
        geomType, parentBone, game::phys::serialize(jointType), musclePowerOn,
        musclePowerCollide, musclePowerDown, minAngles.serialize(),
        maxAngles.serialize(), axisFriction.serialize(), ::serialize(mirror),
        parentIndex);
  }
};

struct RagdollDef {
  uint32_t numBoneDefs;
  uint32_t numSelfCollisionPairs;
  RagdollBoneDef boneDefs[14];
  SelfCollisionPair selfCollisionPairs[33];
  const char *name;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"numBoneDefs\": {}, \"numSelfCollisionPairs\": {}, "
        "\"boneDefs\": {}, "
        "\"selfCollisionPairs\": {}, \"name\": {} }}",
        numBoneDefs, numSelfCollisionPairs,
        ::serialize(&boneDefs[0], numBoneDefs),
        ::serialize(&selfCollisionPairs[0], numSelfCollisionPairs),
        name ? name : "NULL");
  }
};

PACKED(struct objcamCameraPoint {
  phys::objcamState ocState;
  scr::ScrString_t ocName;
  int32_t ocIndex;
  int32_t ocEntNum;
  scr::ScrString_t ocEntTag;
  team_t ocTeam;
  vec3_t ocPos;
  vec3_t ocAngles;
  float ocMaxHorizAngle;
  float ocMaxVertAngle;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"ocState\": {}, \"ocName\": 0x{:08X}, \"ocIndex\": "
                       "{}, \"ocEntNum\": {}, "
                       "\"ocEntTag\": 0x{:08X}, \"ocTeam\": {}, \"ocPos\": {}, "
                       "\"ocAngles\": {}, "
                       "\"ocMaxHorizAngle\": {}, \"ocMaxVertAngle\": {} }}",
                       game::phys::serialize(ocState), ocName, ocIndex,
                       ocEntNum, ocEntTag, game::serialize(ocTeam),
                       ocPos.serialize(), ocAngles.serialize(), ocMaxHorizAngle,
                       ocMaxVertAngle);
  }
});
ASSERT_SIZE(objcamCameraPoint, 0x38);

PACKED(struct objcamCameraTable {
  objcamCameraPoint cameraPoints[256];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"cameraPoints\": {} }}", ::serialize(cameraPoints));
  }
});
ASSERT_SIZE(objcamCameraTable, 0x3800);

struct objcamCameraState {
  int32_t ocsLastCameraBase;
  int32_t ocsLastCamera;
  vec3_t ocsViewAngles;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"ocsLastCameraBase\": {}, \"ocsLastCamera\": {}, "
                       "\"ocsViewAngles\": {} }}",
                       ocsLastCameraBase, ocsLastCamera,
                       ocsViewAngles.serialize());
  }
};

template <typename T> class phys_simple_link_list {
public:
  T *m_first;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_first\": {:p} }}",
                       reinterpret_cast<const void *>(m_first));
  }
};

template <typename T> struct phys_inplace_avl_tree_node {
  T *m_left;
  T *m_right;
  int32_t m_balance;
  uint8_t _padding14[4];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_left\": {:p}, \"m_right\": {:p}, \"m_balance\": {} }}",
        reinterpret_cast<const void *>(m_left),
        reinterpret_cast<const void *>(m_right), m_balance);
  }
};

struct pulse_sum_node;

struct PhysObjUserData;
struct rigid_body;

struct WheelState {
  int32_t m_state;
  int32_t m_last_state;
  float m_rate[4];
};

typedef uint32_t phys_gjk_geom_id;

struct broad_phase_info {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead
  broad_phase_base base;
  rigid_body *m_rb;
  const math::RotTranMat43 *m_rb_to_world_xform;
  const math::RotTranMat43 *m_cg_to_world_xform;
  const math::RotTranMat43 *m_cg_to_rb_xform;
  const phys_gjk_geom *m_gjk_geom;
  phys_gjk_geom_id m_gjk_geom_id;
  int32_t m_surface_type;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_rb\": {}, \"m_rb_to_world_xform\": {}, "
        "\"m_cg_to_world_xform\": {}, \"m_cg_to_rb_xform\": {}, "
        "\"m_gjk_geom\": {}, "
        "\"m_gjk_geom_id\": 0x{:08X}, \"m_surface_type\": {} }}",
        base.serialize(), ::serialize(m_rb), ::serialize(m_rb_to_world_xform),
        ::serialize(m_cg_to_world_xform), ::serialize(m_cg_to_rb_xform),
        ::serialize(m_gjk_geom), m_gjk_geom_id, m_surface_type);
  }
};

struct cached_simplex_info {
  math::Dir3 m_indices[3];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_indices\": {} }}", ::serialize(m_indices));
  }
};

struct phys_gjk_geom_id_pair_key {
public:
  phys_gjk_geom_id m_id1;
  phys_gjk_geom_id m_id2;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"m_id1\": 0x{:08X}, \"m_id2\": 0x{:08X} }}", m_id1,
                       m_id2);
  }
};

struct phys_gjk_cache_info {
  math::Dir3 m_support_dir;
  cached_simplex_info m_support_a;
  cached_simplex_info m_support_b;
  int32_t m_support_count;
  phys_gjk_geom_id_pair_key m_key;
  uint32_t m_flags;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_support_dir\": {}, \"m_support_a\": {}, \"m_support_b\": {}, "
        "\"m_support_count\": {}, \"m_key\": {}, \"m_flags\": 0x{:08X} }}",
        m_support_dir.serialize(), m_support_a.serialize(),
        m_support_b.serialize(), m_support_count, m_key.serialize(), m_flags);
  }
};

struct phys_collision_pair {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead
  phys_link_list_base<phys_collision_pair> base;
  broad_phase_info *m_bpi1;
  broad_phase_info *m_bpi2;
  float m_hit_time;
  phys_gjk_cache_info *m_gjk_ci;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"base\": {}, \"m_bpi1\": {}, \"m_bpi2\": {}, "
                       "\"m_hit_time\": {}, \"m_gjk_ci\": {} }}",
                       base.serialize(), ::serialize(m_bpi1),
                       ::serialize(m_bpi2), m_hit_time, ::serialize(m_gjk_ci));
  }
};

struct __attribute__((aligned(16))) contact_point_info {
  struct pulse_sum_cache_info {
    pulse_sum_cache m_ps_cache_list[3];

    inline std::string serialize() const noexcept {
      return std::format("{{ \"m_ps_cache_list\": {} }}",
                         ::serialize(m_ps_cache_list));
    }
  };

  math::Dir3 m_normal;
  float m_fric_coef;
  float m_bounce_coef;
  float m_max_restitution_vel;
  int32_t m_flags;
  int32_t m_point_pair_count;
  math::Dir3 *m_list_b1_r_loc;
  math::Dir3 *m_list_b2_r_loc;
  contact_point_info::pulse_sum_cache_info *m_list_pulse_sum_cache_info;
  contact_point_info *m_next_link;
  const void *m_rb2_entity;
  float m_translation_lambda;
  phys_collision_pair *m_pcp;
  rigid_body_constraint_contact *m_rbc_contact;

  std::string serialize() const noexcept;
};

struct rigid_body_constraint_contact {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead
  rigid_body_constraint base;
  phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_1;
  phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_2;
  uint32_t m_solver_priority;
  phys_inplace_avl_tree_node<rigid_body_constraint_contact> m_avl_tree_node;
  rigid_body_pair_key m_avl_key;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_list_contact_point_info_buffer_1\": {}, "
        "\"m_list_contact_point_info_buffer_2\": {}, "
        "\"m_solver_priority\": 0x{:08X}, \"m_avl_tree_node\": {}, "
        "\"m_avl_key\": {} }}",
        base.serialize(), m_list_contact_point_info_buffer_1.serialize(),
        m_list_contact_point_info_buffer_2.serialize(), m_solver_priority,
        m_avl_tree_node.serialize(), m_avl_key.serialize());
  }
};

struct rigid_body_constraint_hinge {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  __attribute__((aligned(16))) math::Dir3 m_b1_r_loc;
  math::Dir3 m_b2_r_loc;
  math::Dir3 m_b1_axis_loc;
  math::Dir3 m_b2_axis_loc;
  math::Dir3 m_b1_a1_loc;
  math::Dir3 m_b1_a2_loc;
  math::Dir3 m_b1_ref_loc;
  math::Dir3 m_b2_ref_min_loc;
  math::Dir3 m_b2_ref_max_loc;
  float m_damp_k;
  uint32_t m_flags;
  pulse_sum_cache m_ps_cache[8];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"base\": {}, \"m_b1_r_loc\": {}, \"m_b2_r_loc\": "
                       "{}, \"m_b1_axis_loc\": {}, "
                       "\"m_b2_axis_loc\": {}, \"m_b1_a1_loc\": {}, "
                       "\"m_b1_a2_loc\": {}, \"m_b1_ref_loc\": {}, "
                       "\"m_b2_ref_min_loc\": {}, \"m_b2_ref_max_loc\": {}, "
                       "\"m_damp_k\": {}, \"m_flags\": 0x{:08X}, "
                       "\"m_ps_cache\": {} }}",
                       base.serialize(), m_b1_r_loc.serialize(),
                       m_b2_r_loc.serialize(), m_b1_axis_loc.serialize(),
                       m_b2_axis_loc.serialize(), m_b1_a1_loc.serialize(),
                       m_b1_a2_loc.serialize(), m_b1_ref_loc.serialize(),
                       m_b2_ref_min_loc.serialize(),
                       m_b2_ref_max_loc.serialize(), m_damp_k, m_flags,
                       ::serialize(m_ps_cache));
  }
};
struct rigid_body_constraint_distance {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  __attribute__((aligned(16))) math::Dir3 m_b1_r_loc;
  math::Dir3 m_b2_r_loc;
  float m_min_distance;
  float m_max_distance;
  float m_next_max_distance;
  float m_max_distance_vel;
  float m_damp_coef;
  uint32_t m_flags;
  pulse_sum_cache m_ps_cache_list[3];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_b1_r_loc\": {}, \"m_b2_r_loc\": {}, "
        "\"m_min_distance\": {}, "
        "\"m_max_distance\": {}, \"m_next_max_distance\": {}, "
        "\"m_max_distance_vel\": {}, "
        "\"m_damp_coef\": {}, \"m_flags\": 0x{:08X}, \"m_ps_cache_list\": {} "
        "}}",
        base.serialize(), m_b1_r_loc.serialize(), m_b2_r_loc.serialize(),
        m_min_distance, m_max_distance, m_next_max_distance, m_max_distance_vel,
        m_damp_coef, m_flags, ::serialize(m_ps_cache_list));
  }
};

struct __attribute__((aligned(8))) ragdoll_joint_limit_info {
  math::Dir3 m_b1_ud_loc;
  float m_b1_ud_limit_co_;
  float m_b1_ud_limit_si_;
  float m_b1_ud_active_limit_co_;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_b1_ud_loc\": {}, \"m_b1_ud_limit_co_\": {}, "
        "\"m_b1_ud_limit_si_\": {}, \"m_b1_ud_active_limit_co_\": {} }}",
        m_b1_ud_loc.serialize(), m_b1_ud_limit_co_, m_b1_ud_limit_si_,
        m_b1_ud_active_limit_co_);
  }
};

struct rigid_body_constraint_ragdoll {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  __attribute__((aligned(16))) math::Dir3 m_b1_r_loc;
  math::Dir3 m_b2_r_loc;
  uint32_t m_flags;
  pulse_sum_cache m_ps_cache_list[10];
  __attribute__((aligned(8))) math::Dir3 m_b1_axis_loc;
  math::Dir3 m_b2_axis_loc;
  math::Dir3 m_b1_a1_loc;
  math::Dir3 m_b1_a2_loc;
  math::Dir3 m_b1_ref_loc;
  math::Dir3 m_b2_ref_min_loc;
  math::Dir3 m_b2_ref_max_loc;
  ragdoll_joint_limit_info m_joint_limits[2];
  int32_t m_joint_limits_count;
  float m_damp_k;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_b1_r_loc\": {}, \"m_b2_r_loc\": {}, "
        "\"m_flags\": 0x{:08X}, \"m_ps_cache_list\": {}, \"m_b1_axis_loc\": "
        "{}, "
        "\"m_b2_axis_loc\": {}, \"m_b1_a1_loc\": {}, \"m_b1_a2_loc\": {}, "
        "\"m_b1_ref_loc\": {}, \"m_b2_ref_min_loc\": {}, \"m_b2_ref_max_loc\": "
        "{}, "
        "\"m_joint_limits\": {}, \"m_joint_limits_count\": {}, \"m_damp_k\": "
        "{} }}",
        base.serialize(), m_b1_r_loc.serialize(), m_b2_r_loc.serialize(),
        m_flags, ::serialize(m_ps_cache_list), m_b1_axis_loc.serialize(),
        m_b2_axis_loc.serialize(), m_b1_a1_loc.serialize(),
        m_b1_a2_loc.serialize(), m_b1_ref_loc.serialize(),
        m_b2_ref_min_loc.serialize(), m_b2_ref_max_loc.serialize(),
        ::serialize(m_joint_limits), m_joint_limits_count, m_damp_k);
  }
};
struct pulse_sum_normal {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  phys_link_list_base<pulse_sum_normal> base;
  __attribute__((aligned(16))) math::Dir3 m_ud;
  math::Dir3 m_b1_r;
  math::Dir3 m_b2_r;
  math::Dir3 m_b1_ap;
  math::Dir3 m_b2_ap;
  float m_pulse_sum_min;
  float m_pulse_sum_max;
  float m_pulse_sum;
  float m_right_side;
  float m_big_dirt;
  float m_cfm;
  float m_denom;
  float m_pulse_limit_ratio;
  uint32_t m_flags;
  pulse_sum_normal *m_pulse_parent;
  pulse_sum_node *m_b1;
  pulse_sum_node *m_b2;
  pulse_sum_cache *m_pulse_sum_cache;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_ud\": {}, \"m_b1_r\": {}, \"m_b2_r\": {}, "
        "\"m_b1_ap\": {}, \"m_b2_ap\": {}, \"m_pulse_sum_min\": {}, "
        "\"m_pulse_sum_max\": {}, "
        "\"m_pulse_sum\": {}, \"m_right_side\": {}, \"m_big_dirt\": {}, "
        "\"m_cfm\": {}, "
        "\"m_denom\": {}, \"m_pulse_limit_ratio\": {}, \"m_flags\": 0x{:08X}, "
        "\"m_pulse_parent\": {}, \"m_b1\": {}, \"m_b2\": {}, "
        "\"m_pulse_sum_cache\": {} }}",
        base.serialize(), m_ud.serialize(), m_b1_r.serialize(),
        m_b2_r.serialize(), m_b1_ap.serialize(), m_b2_ap.serialize(),
        m_pulse_sum_min, m_pulse_sum_max, m_pulse_sum, m_right_side, m_big_dirt,
        m_cfm, m_denom, m_pulse_limit_ratio, m_flags,
        ::serialize(m_pulse_parent), ::serialize(m_b1), ::serialize(m_b2),
        ::serialize(m_pulse_sum_cache));
  }
};
struct __attribute__((aligned(8))) rigid_body_constraint_wheel {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;

  struct syms {
    static constexpr symbol<thiscall_t<void(rigid_body_constraint_wheel *,
                                            float torque, double delta_t)>>
        add_wheel_torque{0x14002A9A0, 0x14002A9A0, 0x14002A710};
    static constexpr symbol<thiscall_t<void(rigid_body_constraint_wheel *)>>
        set_no_collision{0x14002B3A0, 0x14002B3A0, 0x14002B110};
    static constexpr symbol<thiscall_t<void(
        const rigid_body_constraint_wheel *, const math::RotTranMat43 *b1_mat,
        math::Dir3 *const p0, math::Dir3 *const p1)>>
        get_wheel_collide_segment{0x14002AE00, 0x14002AE00, 0x14002AB70};
    static constexpr symbol<thiscall_t<void(
        rigid_body_constraint_wheel *, const rigid_body *rb,
        const math::Dir3 *hitp_loc, const math::Dir3 *hitn_loc)>>
        set_collision{0x14002B340, 0x14002B340, 0x14002B0B0};
    static constexpr symbol<thiscall_t<void(rigid_body_constraint_wheel *)>>
        calc_penetration_depth{0x14002A9D0, 0x14002A9D0, 0x14002A740};
  };

  pulse_sum_cache m_ps_cache_list[4];
  pulse_sum_normal *m_ps_suspension;
  pulse_sum_normal *m_ps_side_fric;
  pulse_sum_normal *m_ps_fwd_fric;
  math::Dir3 m_b1_local_offset;
  math::RotTranMat43 m_b1_local_matrix;
  math::RotTranMat43 m_b1_world_matrix;
  math::Dir3 m_b2_hit_point_loc;
  math::Dir3 m_b2_hit_normal_loc;
  float m_wheel_radius;
  float m_wheel_steering_angle;
  uint32_t m_wheel_flags;
  int32_t m_wheel_state;
  float m_turning_radius_ratio_max_speed;
  float m_turning_radius_ratio_accel;
  float m_desired_speed_k;
  float m_acceleration_factor_k;
  float m_braking_factor_k;
  float m_wheel_spin_vel;
  float m_wheel_spin_angle;
  float m_peel_out_spin_vel;
  float m_wheel_fwd_move;
  float m_wheel_local_fwd_vel;
  float m_wheel_local_side_vel;
  float m_wheel_friction;
  float m_wheel_friction_hand_brake_fwd;
  float m_wheel_friction_hand_brake_side;
  float m_wheel_friction_surface_scale;
  float m_wheel_hand_brake;
  float m_roll_stability_factor;
  float m_pitch_stability_factor;
  float m_suspension_stiffness_k;
  float m_suspension_damp_k;
  float m_hard_limit_dist;
  float m_penetration_depth;
  float m_max_climb_angle;

  inline void add_wheel_torque(float torque, double delta_t) {
    return syms::add_wheel_torque(this, torque, delta_t);
  }

  inline void set_no_collision() { return syms::set_no_collision(this); }
  inline void get_wheel_collide_segment(const math::RotTranMat43 *b1_mat,
                                        math::Dir3 *const p0,
                                        math::Dir3 *const p1) const {
    return syms::get_wheel_collide_segment(this, b1_mat, p0, p1);
  }
  inline void set_collision(const rigid_body *rb, const math::Dir3 *hitp_loc,
                            const math::Dir3 *hitn_loc) {
    return syms::set_collision(this, rb, hitp_loc, hitn_loc);
  }
  inline void calc_penetration_depth() {
    return syms::calc_penetration_depth(this);
  }

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_ps_cache_list\": {}, \"m_ps_suspension\": {}, "
        "\"m_ps_side_fric\": {}, \"m_ps_fwd_fric\": {}, \"m_b1_local_offset\": "
        "{}, "
        "\"m_b1_local_matrix\": {}, \"m_b1_world_matrix\": {}, "
        "\"m_b2_hit_point_loc\": {}, "
        "\"m_b2_hit_normal_loc\": {}, \"m_wheel_radius\": {}, "
        "\"m_wheel_steering_angle\": {}, "
        "\"m_wheel_flags\": 0x{:08X}, \"m_wheel_state\": {}, "
        "\"m_turning_radius_ratio_max_speed\": {}, "
        "\"m_turning_radius_ratio_accel\": {}, \"m_desired_speed_k\": {}, "
        "\"m_acceleration_factor_k\": {}, "
        "\"m_braking_factor_k\": {}, \"m_wheel_spin_vel\": {}, "
        "\"m_wheel_spin_angle\": {}, "
        "\"m_peel_out_spin_vel\": {}, \"m_wheel_fwd_move\": {}, "
        "\"m_wheel_local_fwd_vel\": {}, "
        "\"m_wheel_local_side_vel\": {}, \"m_wheel_friction\": {}, "
        "\"m_wheel_friction_hand_brake_fwd\": {}, "
        "\"m_wheel_friction_hand_brake_side\": {}, "
        "\"m_wheel_friction_surface_scale\": {}, \"m_wheel_hand_brake\": {}, "
        "\"m_roll_stability_factor\": {}, \"m_pitch_stability_factor\": {}, "
        "\"m_suspension_stiffness_k\": {}, "
        "\"m_suspension_damp_k\": {}, \"m_hard_limit_dist\": {}, "
        "\"m_penetration_depth\": {}, "
        "\"m_max_climb_angle\": {} }}",
        base.serialize(), ::serialize(m_ps_cache_list),
        ::serialize(m_ps_suspension), ::serialize(m_ps_side_fric),
        ::serialize(m_ps_fwd_fric), m_b1_local_offset.serialize(),
        m_b1_local_matrix.serialize(), m_b1_world_matrix.serialize(),
        m_b2_hit_point_loc.serialize(), m_b2_hit_normal_loc.serialize(),
        m_wheel_radius, m_wheel_steering_angle, m_wheel_flags, m_wheel_state,
        m_turning_radius_ratio_max_speed, m_turning_radius_ratio_accel,
        m_desired_speed_k, m_acceleration_factor_k, m_braking_factor_k,
        m_wheel_spin_vel, m_wheel_spin_angle, m_peel_out_spin_vel,
        m_wheel_fwd_move, m_wheel_local_fwd_vel, m_wheel_local_side_vel,
        m_wheel_friction, m_wheel_friction_hand_brake_fwd,
        m_wheel_friction_hand_brake_side, m_wheel_friction_surface_scale,
        m_wheel_hand_brake, m_roll_stability_factor, m_pitch_stability_factor,
        m_suspension_stiffness_k, m_suspension_damp_k, m_hard_limit_dist,
        m_penetration_depth, m_max_climb_angle);
  }
};
struct rigid_body_constraint_angular_actuator {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  __attribute__((aligned(16))) math::RotTranMat43 m_target_mat;
  math::RotTranMat43 m_next_target_mat;
  math::Dir3 m_a_vel;
  float m_power;
  float m_power_scale;
  bool m_enabled;
  pulse_sum_cache m_ps_cache_list[3];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_target_mat\": {}, \"m_next_target_mat\": {}, "
        "\"m_a_vel\": {}, \"m_power\": {}, \"m_power_scale\": {}, "
        "\"m_enabled\": {}, "
        "\"m_ps_cache_list\": {} }}",
        base.serialize(), m_target_mat.serialize(),
        m_next_target_mat.serialize(), m_a_vel.serialize(), m_power,
        m_power_scale, ::serialize(m_enabled), ::serialize(m_ps_cache_list));
  }
};
struct rigid_body_constraint_upright {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  __attribute__((aligned(16))) math::Dir3 m_b1_forward_axis_loc;
  math::Dir3 m_b1_right_axis_loc;
  math::Dir3 m_b1_up_axis_loc;
  math::Dir3 m_b1_lean_axis_loc;
  math::Dir3 m_b2_up_axis_loc;
  math::Dir3 m_last_t_vel;
  math::Dir3 m_last_a_vel;
  float m_avg_side_force;
  float m_avg_normal_force;
  float m_lean_angle_calc_delta_t;
  float m_lean_angle;
  float m_lean_angle_multiplier;
  float m_max_lean_angle;
  float m_moving_average_total_time;
  bool m_enabled;
  pulse_sum_cache m_ps_cache_list[1];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_b1_forward_axis_loc\": {}, "
        "\"m_b1_right_axis_loc\": {}, "
        "\"m_b1_up_axis_loc\": {}, \"m_b1_lean_axis_loc\": {}, "
        "\"m_b2_up_axis_loc\": {}, "
        "\"m_last_t_vel\": {}, \"m_last_a_vel\": {}, \"m_avg_side_force\": {}, "
        "\"m_avg_normal_force\": {}, \"m_lean_angle_calc_delta_t\": {}, "
        "\"m_lean_angle\": {}, "
        "\"m_lean_angle_multiplier\": {}, \"m_max_lean_angle\": {}, "
        "\"m_moving_average_total_time\": {}, "
        "\"m_enabled\": {}, \"m_ps_cache_list\": {} }}",
        base.serialize(), m_b1_forward_axis_loc.serialize(),
        m_b1_right_axis_loc.serialize(), m_b1_up_axis_loc.serialize(),
        m_b1_lean_axis_loc.serialize(), m_b2_up_axis_loc.serialize(),
        m_last_t_vel.serialize(), m_last_a_vel.serialize(), m_avg_side_force,
        m_avg_normal_force, m_lean_angle_calc_delta_t, m_lean_angle,
        m_lean_angle_multiplier, m_max_lean_angle, m_moving_average_total_time,
        ::serialize(m_enabled), ::serialize(m_ps_cache_list));
  }
};
struct __attribute__((aligned(8))) rigid_body_constraint_custom_orientation {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  pulse_sum_cache m_ps_cache_list[5];
  bool m_active;
  bool m_no_orientation_correction;
  float m_torque_resistance_pitch_roll;
  float m_torque_resistance_yaw;
  float m_torque_resistance_yaw_max_collision_speed;
  float m_upright_strength;
  float m_desired_roll_angle;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_ps_cache_list\": {}, \"m_active\": {}, "
        "\"m_no_orientation_correction\": {}, "
        "\"m_torque_resistance_pitch_roll\": {}, "
        "\"m_torque_resistance_yaw\": {}, "
        "\"m_torque_resistance_yaw_max_collision_speed\": {}, "
        "\"m_upright_strength\": {}, \"m_desired_roll_angle\": {} }}",
        base.serialize(), ::serialize(m_ps_cache_list), ::serialize(m_active),
        ::serialize(m_no_orientation_correction),
        m_torque_resistance_pitch_roll, m_torque_resistance_yaw,
        m_torque_resistance_yaw_max_collision_speed, m_upright_strength,
        m_desired_roll_angle);
  }
};

struct user_rigid_body {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body base;
  const math::RotTranMat43 *m_dictator;
  math::RotTranMat43 m_dictator_mat;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"base\": {}, \"m_dictator\": {}, \"m_dictator_mat\": {} }}",
        base.serialize(), ::serialize(m_dictator), m_dictator_mat.serialize());
  }
};

PACKED(struct rigid_body_constraint_custom_path {
  // This is inherited. Inheritance breaks standard layout, so it's added as a
  // field here instead.
  rigid_body_constraint base;
  uint8_t _padding18[8];
  math::RotTranMat43 m_path_mat;
  math::Dir3 b1_r_loc;
  user_rigid_body *m_urb;
  int32_t m_timestamp;
  float m_spring_scale;
  pulse_sum_cache m_list_psc[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"base\": {}, \"m_path_mat\": {}, \"b1_r_loc\": {}, "
                       "\"m_urb\": {}, \"m_timestamp\": {}, "
                       "\"m_spring_scale\": {}, \"m_list_psc\": {} }}",
                       base.serialize(), m_path_mat.serialize(),
                       b1_r_loc.serialize(), ::serialize(m_urb), m_timestamp,
                       m_spring_scale, ::serialize(m_list_psc));
  }
});
ASSERT_OFFSET(rigid_body_constraint_custom_path, b1_r_loc, 0x60);
ASSERT_OFFSET(rigid_body_constraint_custom_path, m_path_mat, 0x20);

struct BodyState {
  vec3_t position;
  vec3_t rotation[3];
  vec3_t velocity;
  vec3_t angVelocity;
  vec3_t centerOfMassOffset;
  vec3_t buoyancyBoxMin;
  vec3_t buoyancyBoxMax;
  float mass;
  float friction;
  float bounce;
  int timeLastAsleep;
  int id;
  int buoyancy;
  int underwater;
  int owner;
  PhysicsOwnerType owner_type;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"position\": {}, \"rotation\": {}, \"velocity\": {}, "
        "\"angVelocity\": {}, \"centerOfMassOffset\": {}, \"buoyancyBoxMin\": "
        "{}, "
        "\"buoyancyBoxMax\": {}, \"mass\": {}, \"friction\": {}, \"bounce\": "
        "{}, "
        "\"timeLastAsleep\": {}, \"id\": {}, \"buoyancy\": {}, \"underwater\": "
        "{}, "
        "\"owner\": {}, \"owner_type\": {} }}",
        position.serialize(), ::serialize(rotation), velocity.serialize(),
        angVelocity.serialize(), centerOfMassOffset.serialize(),
        buoyancyBoxMin.serialize(), buoyancyBoxMax.serialize(), mass, friction,
        bounce, timeLastAsleep, id, buoyancy, underwater, owner,
        game::phys::serialize(owner_type));
  }
};

typedef rigid_body environment_rigid_body;

typedef fastcallPtr_t<void()> phys_collision_callback_t;

typedef fastcallPtr_t<bool(const broad_phase_base *b1,
                           const broad_phase_base *b2)>
    phys_should_collide_callback_t;

typedef fastcallPtr_t<void(void *)> phys_debug_callback_t;

struct phys_sys {
public:
  static constexpr symbol<rigid_body_constraint_custom_orientation *(
      rigid_body *const b1, rigid_body *const b2, const int no_error)>
      create_rbc_custom_orientation{0x14000B9A0, 0x0, 0x14000B810};
  static constexpr symbol<rigid_body *(const int no_error)> create_rigid_body{
      0x14000BE20, 0x0, 0x14000BC90};
  static inline void destroy(rigid_body *const rb) {
    static constexpr symbol<void(rigid_body *const rb)> sym{0x0, 0x0,
                                                            0x14000BD50};
    return sym(rb);
  }

  static inline void destroy(user_rigid_body *const rb) {
    static constexpr symbol<void(user_rigid_body *const rb)> sym{0x0, 0x0,
                                                                 0x14000BEE0};
    return sym(rb);
  }
  static constexpr symbol<void(rigid_body *const rb)> destroy_all_constraint{
      0x14000C0C0, 0x0, 0x14000BF30};
  static constexpr symbol<void(rigid_body *const rb)> fixup_wheel_constraints{
      0x0, 0x0, 0x14000C460};
  static constexpr symbol<environment_rigid_body *()>
      get_environment_rigid_body{0x0, 0x0, 0x14000C4D0};
  static constexpr symbol<void(phys_collision_callback_t collision_callback)>
      set_collision_callback{0x14000C690, 0x0, 0x14000C500};
  static constexpr symbol<void(const float max_delta_t)> set_max_delta_t{
      0x14000C6A0, 0x0, 0x14000C510};
  static constexpr symbol<void(const int max_v_iters)> set_v_tol{
      0x14000C6B0, 0x0, 0x14000C520};
};

enum class TraceBrushType : int32_t {
  NONE = 0x0,
  BRUSH = 0x1,
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

struct moveclip_t {
  vec3_t mins;
  vec3_t maxs;
  vec3_t outerSize;
  __declspec(align(16)) TraceExtents extents;
  int passEntityNum;
  int passEntityNum1;
  int passOwnerNum;
  contents_t contentmask;
  collide_entity_func_ptr collide_entity_func;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"mins\": {}, \"maxs\": {}, \"outerSize\": {}, \"extents\": {}, "
        "\"passEntityNum\": {}, \"passEntityNum1\": {}, \"passOwnerNum\": {}, "
        "\"contentmask\": 0x{:08X}, \"collide_entity_func\": {:p} }}",
        mins.serialize(), maxs.serialize(), outerSize.serialize(),
        extents.serialize(), passEntityNum, passEntityNum1, passOwnerNum,
        contentmask, reinterpret_cast<const void *>(collide_entity_func));
  }
};
} // namespace phys
} // namespace game
