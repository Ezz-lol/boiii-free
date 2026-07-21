#pragma once

#include "quake/vec.hpp"
#include "quake/core.hpp"
#include "db/xasset/core.hpp"

namespace game {
namespace rope {

enum class rope_constraint_e : uint32_t {
  ROPE_PAIR_CONSTRAINT = 0x0,
  ROPE_WORLD_CONSTRAINT = 0x1,
  ROPE_DENTITY_CONSTRAINT = 0x2,
  ROPE_CENTITY_CONSTRAINT = 0x3,
};

struct par_t {
  vec3_t p;
  vec3_t p0;
  vec3_t p_prev;
  int32_t flags;
};

#pragma pack(push, 1)
class constraint_t {
public:
  vec3_t p;
  rope_constraint_e type;
  int32_t entity_index;
  scr::ScrString_t bone_name_hash;
  uint8_t pi1;
  uint8_t pi2;
  uint8_t _padding1A[2];
};
#pragma pack(pop)

struct rope_frame_verts_t {
  int num_verts;
  vec3_t v[60];
};

struct rope_client_verts_t {
  rope_frame_verts_t frame_verts[2];
  uint32_t frame_index;
};

#pragma pack(push, 1)
struct rope_t {
  par_t m_particles[30];
  constraint_t m_constraints[35];
  int32_t m_entity_anchors[3];
  int32_t m_num_particles;
  int32_t m_num_constraints;
  int32_t m_num_entity_anchors;
  int32_t m_num_draw_verts;
  rope_client_verts_t m_client_verts;
  vec3_t m_min;
  vec3_t m_max;
  vec3_t m_start;
  vec3_t m_end;
  qboolean m_in_use;
  qboolean m_visible;
  qboolean m_dist_constraint;
  int32_t m_flags;
  uint8_t _paddingE8C[4];
  db::xasset::MaterialHandle m_material;
  float m_seglen;
  float m_length;
  float m_width;
  float m_scale;
  vec3_t m_force_scale;
  int32_t m_health;
  int32_t m_frame;
  int32_t m_stable_count;
  int32_t m_static_rope;
  int32_t m_model;
  int32_t m_slice_count;
  uint8_t _paddingECC[4];
};
#pragma pack(pop)

} // namespace rope
} // namespace game