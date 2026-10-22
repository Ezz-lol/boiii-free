#include <std_include.hpp>



namespace game {
namespace phys {
std::string pulse_sum_node::serialize() const noexcept {
  return std::format(
      "{{ \"base\": {}, \"m_world_inv_inertia\": {}, \"t_vel\": {}, "
      "\"a_vel\": {}, \"m_inv_mass\": {}, \"m_rb\": {} }}",
      base.serialize(), m_world_inv_inertia.serialize(), t_vel.serialize(),
      a_vel.serialize(), m_inv_mass, ::serialize(m_rb));
}

std::string rigid_body_constraint::serialize() const noexcept {
  return std::format("{{ \"b1\": {}, \"b2\": {}, \"m_next\": {} }}",
                     ::serialize(b1), ::serialize(b2), ::serialize(m_next));
}

std::string rb_inplace_partition_node::serialize() const noexcept {
  return std::format(
      "{{ \"m_rbc_point_first\": {}, \"m_rbc_hinge_first\": {}, "
      "\"m_rbc_dist_first\": {}, "
      "\"m_rbc_ragdoll_first\": {}, \"m_rbc_wheel_first\": {}, "
      "\"m_rbc_angular_actuator_first\": {}, "
      "\"m_rbc_upright_first\": {}, \"m_rbc_custom_orientation_first\": {}, "
      "\"m_rbc_custom_path_first\": {}, "
      "\"m_rbc_contact_first\": {}, \"m_partition_head\": {}, "
      "\"m_partition_tail\": {}, "
      "\"m_next_node\": {}, \"m_partition_size\": {} }}",
      ::serialize(m_rbc_point_first), ::serialize(m_rbc_hinge_first),
      ::serialize(m_rbc_dist_first), ::serialize(m_rbc_ragdoll_first),
      ::serialize(m_rbc_wheel_first), ::serialize(m_rbc_angular_actuator_first),
      ::serialize(m_rbc_upright_first),
      ::serialize(m_rbc_custom_orientation_first),
      ::serialize(m_rbc_custom_path_first), ::serialize(m_rbc_contact_first),
      ::serialize(m_partition_head), ::serialize(m_partition_tail),
      ::serialize(m_next_node), m_partition_size);
}

std::string PhysObjUserData::serialize() const noexcept {
  return std::format(
      "{{ \"physPreset\": {}, \"body\": {}, \"vehicle\": {}, "
      "\"m_gjk_geom_list\": {}, \"m_next_link\": {}, \"cg2rb\": {}, "
      "\"m2w\": {}, \"m_bpb\": {}, \"tvel\": {}, \"avel\": {}, "
      "\"friction\": {}, \"bounce\": {}, \"damping_linear\": {}, "
      "\"damping_angular\": {}, \"id\": {}, \"underwater\": {}, "
      "\"buoyancy\": {}, \"timeBuoyant\": {}, \"timeRipple\": {}, "
      "\"trDuration\": {}, \"refcount\": {}, \"m_time_since_last_event\": "
      "{}, "
      "\"m_time_since_last_reeval\": {}, \"m_flags\": 0x{:08X}, \"mass\": "
      "{}, "
      "\"buoyancyBoxMin\": {}, \"buoyancyBoxMax\": {}, "
      "\"centerOfMassOffset\": {}, "
      "\"owner\": {}, \"owner_type\": {}, \"hitinfo\": {} }}",
      ::serialize(physPreset), ::serialize(body), ::serialize(vehicle),
      m_gjk_geom_list.serialize(), ::serialize(m_next_link), cg2rb.serialize(),
      m2w.serialize(), ::serialize(m_bpb), tvel.serialize(), avel.serialize(),
      friction, bounce, damping_linear, damping_angular, id, underwater,
      buoyancy, timeBuoyant, timeRipple, trDuration, refcount,
      m_time_since_last_event, m_time_since_last_reeval, m_flags, mass,
      buoyancyBoxMin.serialize(), buoyancyBoxMax.serialize(),
      centerOfMassOffset.serialize(), owner, game::phys::serialize(owner_type),
      hitinfo.serialize());
}

std::string sv_FxVisBlock_t::serialize() const noexcept {
  return std::format("{{ \"fxEnt\": {}, \"radius\": {} }}", ::serialize(fxEnt),
                     radius);
}

std::string contact_point_info::serialize() const noexcept {
  return std::format(
      "{{ \"m_normal\": {}, \"m_fric_coef\": {}, \"m_bounce_coef\": {}, "
      "\"m_max_restitution_vel\": {}, \"m_flags\": 0x{:08X}, "
      "\"m_point_pair_count\": {}, "
      "\"m_list_b1_r_loc\": {}, \"m_list_b2_r_loc\": {}, "
      "\"m_list_pulse_sum_cache_info\": {}, "
      "\"m_next_link\": {}, \"m_rb2_entity\": {}, \"m_translation_lambda\": "
      "{}, "
      "\"m_pcp\": {}, \"m_rbc_contact\": {} }}",
      m_normal.serialize(), m_fric_coef, m_bounce_coef, m_max_restitution_vel,
      m_flags, m_point_pair_count, ::serialize(m_list_b1_r_loc),
      ::serialize(m_list_b2_r_loc), ::serialize(m_list_pulse_sum_cache_info),
      ::serialize(m_next_link), m_rb2_entity, m_translation_lambda,
      ::serialize(m_pcp), ::serialize(m_rbc_contact));
}

} // namespace phys
} // namespace game
//
