#pragma once

#include "scr/scr.hpp"
#include "math.hpp"
#include "snd.hpp"
#include "core.hpp"
#include "asm.hpp"

namespace game {
namespace phys {
struct gentity_s;
#pragma pack(push, 1)

struct hitinfo_t {
  int32_t hittime;
  vec3_t hitp;
  vec3_t hitn;
  int32_t stype;
  int32_t entnum;
  float intensity;
};

enum PhysicsOwnerType : int32_t {
  PHYS_OWNER_DYNENT = 0x0,
  PHYS_OWNER_ENTITY = 0x1,
  PHYS_OWNER_GLASS = 0x2,
  PHYS_OWNER_PARTICLE = 0x3,
  PHYS_OWNER_RAGDOLL = 0x4,
  PHYS_OWNER_XDOLL = 0x5,
  PHYS_OWNER_COUNT = 0x6,
};

// sizeof=0x60
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
};
static_assert(sizeof(broad_phase_base) == 0x60,
              "broad_phase_base size must be 96 bytes");

template <typename Node> class phys_link_list_base {
public:
  Node *m_next_link;
};

struct phys_mat33 {
  math::Float4 x;
  math::Float4 y;
  math::Float4 z;
};

class rigid_body;

// sizeof=0x70
class pulse_sum_node : phys_link_list_base<pulse_sum_node> {
public:
  phys_mat33 m_world_inv_inertia;
  uint8_t _padding38[8];
  math::Dir3 t_vel;
  math::Dir3 a_vel;
  float m_inv_mass;
  uint8_t _padding64[4];
  rigid_body *m_rb;
};
static_assert(sizeof(pulse_sum_node) == 0x70,
              "pulse_sum_node size must be 112 bytes");

typedef db::xasset::FxImpactTable *FxImpactTablePtr;

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

struct rigid_body_constraint_contact;
struct rigid_body_constraint_custom_path;
struct rigid_body_constraint_custom_orientation;
struct rigid_body_constraint_upright;
struct rigid_body_constraint_angular_actuator;
struct rigid_body_constraint_ragdoll;
struct rigid_body_constraint_distance;
struct rigid_body_constraint_hinge;

class rigid_body_constraint;

// sizeof=0x18
class rigid_body_constraint {
public:
  rigid_body *b1;
  rigid_body *b2;
  rigid_body_constraint *m_next;
};
static_assert(sizeof(rigid_body_constraint) == 0x18,
              "rigid_body_constraint size must be 24 bytes");

// sizeof=0x4
class pulse_sum_cache {
public:
  float m_pulse_sum;
};
static_assert(sizeof(pulse_sum_cache) == 0x4,
              "pulse_sum_cache size must be 4 bytes");

// sizeof=0x60
class rigid_body_constraint_point : rigid_body_constraint {
public:
  uint8_t _padding00[8];
  math::Dir3 m_b1_r_loc;
  math::Dir3 m_b2_r_loc;
  pulse_sum_cache m_ps_cache_list[3];
  float m_stress;
  float m_spring_k;
  float m_damp_k;
  bool m_spring_enabled;
  uint8_t _padding59[7];
};
static_assert(sizeof(rigid_body_constraint_point) == 0x60,
              "rigid_body_constraint_point size must be 96 bytes");

class pulse_sum_node;
class rigid_body;
class rigid_body_constraint_wheel;

// sizeof=0x70
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
};
static_assert(sizeof(rb_inplace_partition_node) == 0x70,
              "rb_inplace_partition_node size must be 112 bytes");

// sizeof=0x1B0
class rigid_body {
public:
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
};
static_assert(sizeof(rigid_body) == 0x1B0, "rigid_body size must be 432 bytes");

// sizeof=0x10
struct rigid_body_pair_key {
  rigid_body *m_b1;
  rigid_body *m_b2;
};
static_assert(sizeof(rigid_body_pair_key) == 0x10,
              "rigid_body_pair_key size must be 16 bytes");

class NitrousVehicle;

// sizeof=0x8
class phys_gjk_geom {
public:
  // int32_t (**_vptr$phys_gjk_geom)(void);
  void *vtablePtr;
};
static_assert(sizeof(phys_gjk_geom) == 0x8,
              "phys_gjk_geom size must be 8 bytes");

typedef contents_t gjk_contents_t;

typedef uint32_t phys_gjk_geom_id;
namespace gjk_unique_id_database_t {
typedef phys_gjk_geom_id unique_id;

class gjk_unique_id_database_t {
public:
  unique_id m_counter;
};

} // namespace gjk_unique_id_database_t

// sizeof=0x60
class gjk_base_t : phys_gjk_geom {
public:
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
};
static_assert(sizeof(gjk_base_t) == 0x60, "gjk_base_t size must be 96 bytes");

// sizeof=0x10
class gjk_geom_list_t {
public:
  gjk_base_t *m_first_geom;
  int32_t m_geom_count;
  uint8_t _padding0C[4];
};
static_assert(sizeof(gjk_geom_list_t) == 0x10,
              "gjk_geom_list_t size must be 16 bytes");
// sizeof=0x160
class PhysObjUserData {
public:
  const PhysPreset *physPreset;
  rigid_body *body;
  NitrousVehicle *vehicle;
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
};
static_assert(sizeof(PhysObjUserData) == 0x160,
              "PhysObjUserData size must be 352 bytes");

struct DestructibleDef; // TODO
typedef DestructibleDef *DestructibleDefPtr;

struct ZBarrierBoard {
  db::xasset::XModel *pBoardModel;
  db::xasset::XModel *pAlternateBoardModel;
  db::xasset::XModel *pUpgradedBoardModel;
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
};

// sizeof=0x300
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
  db::xasset::XModel *pCollisionModel;
  ZBarrierBoard boards[6];
};
static_assert(sizeof(ZBarrierDef) == 0x300,
              "ZBarrierDef size must be 768 bytes");

// sizeof=0x8
struct ZBarrierPieceAnims {
  anim::XAnimTree *pAnimTree;
};

// sizeof=0xB8
struct ZBarrierType {
  uint8_t serverIndex;
  uint8_t clientIndex;
  uint8_t _padding02[6];
  ZBarrierDef *zbarrierDef;
  ZBarrierPieceAnims clientAnims[6];
  ZBarrierPieceAnims serverAnims[6];
  int32_t boardModelIndex[6];
  int32_t upgradedBoardModelIndex[6];
  int32_t alternateBoardModelIndex[6];
};
ASSERT_SIZE(ZBarrierType, 0xB8);

// sizeof=0x3C
struct cached_tag_mat_t {
  int32_t time;
  int32_t entnum;
  scr::ScrString_t name;
  vec3_t tagMat[4];
};
ASSERT_SIZE(cached_tag_mat_t, 0x3C);

// sizeof=0xC
struct trigger_info_t {
  uint16_t entnum;
  uint16_t otherEntnum;
  int32_t useCount;
  int32_t otherUseCount;
};
ASSERT_SIZE(trigger_info_t, 0xC);

// sizeof=0x10
struct sv_FxVisBlock_t {
  const gentity_s *fxEnt;
  float radius;
  uint8_t _padding0C[4];
};
ASSERT_SIZE(sv_FxVisBlock_t, 0x10);

// sizeof=0x18
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
};
ASSERT_SIZE(actor_prone_info_t, 0x18);

// sizeof=0x684
struct actorAntilagFrame_t {
  vec3_t position[64];
  vec3_t angles[64];
  uint8_t useCount[64];
  uint8_t inUse[64];
  int32_t time;
};
ASSERT_SIZE(actorAntilagFrame_t, 0x684);

enum class objcamState : uint32_t {
  OCS_UNUSED = 0x0,
  OCS_DISABLED = 0x1,
  OCS_ENABLED = 0x2,
};

#pragma pack(pop)

typedef int32_t FxUniqueHandle;

struct DestructibleBurnData {
  int32_t burnTime;
  FxUniqueHandle fx;
  snd::SndPlaybackId sndId;
};

struct DESTRUCTIBLE_PIECE_INFO {
  int16_t health;
  uint8_t _padding02[2];
  int32_t xdollHandle;
  FxUniqueHandle fx;
  DestructibleBurnData burnData;
};

struct LerpEntityStateDestructibleHit {
  uint32_t modelState[6];
};

struct DestructibleState {
  LerpEntityStateDestructibleHit state;
  int32_t time;
};

enum class AttachPointType : uint32_t {
  ATTACH_POINT_WORLD = 0x0,
  ATTACH_POINT_DYNENT = 0x1,
  ATTACH_POINT_ENT = 0x2,
  ATTACH_POINT_BONE = 0x3,
};

enum class ConstraintType : uint32_t {
  CONSTRAINT_NONE = 0x0,
  CONSTRAINT_POINT = 0x1,
  CONSTRAINT_DISTANCE = 0x2,
  CONSTRAINT_HINGE = 0x3,
  CONSTRAINT_ACTUATOR = 0x4,
  CONSTRAINT_FAKE_SHAKE = 0x5,
  CONSTRAINT_LAUNCH = 0x6,
  CONSTRAINT_ANTENNA = 0x7,
  CONSTRAINT_ROPE = 0x8,
  CONSTRAINT_LIGHT = 0x9,
  NUM_CONSTRAINT_TYPES = 0xA,
};

#pragma pack(push, 1)
struct PhysConstraint {
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
};
ASSERT_SIZE(PhysConstraint, 0xD0);
#pragma pack(pop)

#pragma pack(push, 1)
struct PhysConstraints {
  const char *name;
  uint32_t count;
  uint8_t _padding0C[4];
  PhysConstraint data[8];
};
ASSERT_SIZE(PhysConstraints, 0x690);
typedef PhysConstraints *PhysConstraintsPtr;
#pragma pack(pop)

typedef int32_t XPartBits[12];

#pragma pack(push, 1)
struct DestructibleStage {
  scr::ScrString_t showBone;
  scr::ScrString_t cosmeticShowBones[4];
  float breakHealth;
  float maxTime;
  uint32_t flags;
  db::xasset::FxEffectDefHandle baseEffect;
  scr::ScrString_t baseEffectTag;
  scr::ScrString_t breakAnim;
  db::xasset::FxEffectDefHandle breakEffect;
  scr::ScrString_t breakEffectTag;
  uint8_t _padding3C[4];
  const char *breakSound;
  const char *breakNotify;
  const char *loopSound;
  db::xasset::XModelPtr spawnModel[3];
  PhysPreset *physPreset;
};
ASSERT_SIZE(DestructibleStage, 0x78);
#pragma pack(pop)

#pragma pack(push, 1)
struct DestructiblePiece {
  DestructibleStage stages[7];
  uint8_t parentPiece;
  uint8_t _padding151[3];
  float parentDamagePercent;
  float bulletDamageScale;
  float explosiveDamageScale;
  float meleeDamageScale;
  float impactDamageScale;
  float entityDamageTransfer;
  uint8_t _padding16C[4];
  PhysConstraintsPtr physConstraints;
  int32_t health;
  uint8_t _padding17C[4];
  const char *damageSound;
  db::xasset::FxEffectDefHandle burnEffect;
  const char *burnSound;
  scr::ScrString_t enableLabel;
  XPartBits hideBones;
  uint8_t _padding1CC[4];
};
ASSERT_SIZE(DestructiblePiece, 0x3C8);
#pragma pack(pop)

#pragma pack(push, 1)
struct DestructibleDef {
  const char *name;
  db::xasset::XModelPtr model;
  db::xasset::XModelPtr pristineModel;
  int32_t numPieces;
  uint8_t _padding1C[4];
  DestructiblePiece *pieces;
  qboolean clientOnly;
  qboolean syncBaseHealthWithEntity;
};
ASSERT_SIZE(DestructibleDef, 0x30);
#pragma pack(pop)

#pragma pack(push, 1)
struct Destructible {
  int32_t entNum;
  uint8_t _padding04[4];
  DESTRUCTIBLE_PIECE_INFO *pieceArray;
  int32_t oldestBurnTime;
  bool entDestroyed;
  bool entDestroyedRespondedTo;
  uint8_t _padding16[2];
  int32_t pieceCount;
  uint8_t _padding1C[4];
  DestructibleDef *ddef;
  uint32_t flags;
  DestructibleState states[5];
  bool bHasBeenHit;
  uint8_t _paddingB9[7];
};
ASSERT_SIZE(Destructible, 0xC0);
#pragma pack(pop)

typedef intptr_t PhysObjId;

#pragma pack(push, 1)
// sizeof=0x4
union CollisionAabbTreeIndex {
  int32_t firstChildIndex;
  int32_t partitionIndex;
};
static_assert(sizeof(CollisionAabbTreeIndex) == 0x4,
              "CollisionAabbTreeIndex size must be 4 bytes");

// sizeof=0x20
struct CollisionAabbTree {
  vec3_t origin;
  uint16_t materialIndex;
  uint16_t childCount;
  vec3_t halfSize;
  CollisionAabbTreeIndex u;
};
static_assert(sizeof(CollisionAabbTree) == 0x20,
              "CollisionAabbTree size must be 32 bytes");
struct cbrush_t; // TODO

// sizeof=0x10
struct col_prim_t {
  int32_t type;
  uint8_t _padding04[4];
  union {
    const CollisionAabbTree *tree;
    const cbrush_t *brush;
  };
};
static_assert(sizeof(col_prim_t) == 0x10, "col_prim_t size must be 16 bytes");

class float4 {
public:
  uint8_t gap0[16];
};

// sizeof=10
class hybrid_vector {
public:
  float4 vec;
};
static_assert(sizeof(hybrid_vector) == 0x10,
              "hybrid_vector size must be 16 bytes");

class visitor_base_t {
public:
  // int32_t (**_vptr$visitor_base_t)(void);
  void *_vptr$visitor_base_t;
};

struct TraceThreadInfo; // TODO

class colgeom_visitor_t : visitor_base_t {
public:
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
};

#pragma pack(pop)
#pragma pack(push, 16)
template <size_t T> class colgeom_visitor_inlined_t : colgeom_visitor_t {
  int32_t nprims;
  bool overflow;
  col_prim_t prims[T];
};
#pragma pack(pop)

enum class JointType : int32_t {
  NONE = 0x0,
  HINGE = 0x1,
  SWIVEL = 0x2,
  COUNT = 0x3,
};

struct __attribute__((aligned(4))) SelfCollisionPair {
  scr::ScrString_t boneNames[2];
  uint8_t bonePair[2];
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
};

struct RagdollDef {
  uint32_t numBoneDefs;
  uint32_t numSelfCollisionPairs;
  RagdollBoneDef boneDefs[14];
  SelfCollisionPair selfCollisionPairs[33];
  const char *name;
};

#pragma pack(push, 1)
struct objcamCameraPoint {
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
};
ASSERT_SIZE(objcamCameraPoint, 0x38);
#pragma pack(pop)

#pragma pack(push, 1)
struct objcamCameraTable {
  objcamCameraPoint cameraPoints[256];
};
ASSERT_SIZE(objcamCameraTable, 0x3800);
#pragma pack(pop)

struct objcamCameraState {
  int32_t ocsLastCameraBase;
  int32_t ocsLastCamera;
  vec3_t ocsViewAngles;
};

} // namespace phys
} // namespace game