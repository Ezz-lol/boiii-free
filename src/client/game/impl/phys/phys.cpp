#include <std_include.hpp>
#include "phys.hpp"

namespace game {
namespace phys {
void path_constraint_update_sv(rigid_body_constraint_custom_path *vpc,
                               const vehicle::NitrousVehicle *nitrousVehicle) {
  if (vpc) {
    level::gentity_t *ent = nitrousVehicle->m_owner;
    if (ent && ent->vehicle) {
      vpc->m_path_mat =
          math::RotTranMat43::from_euler(ent->vehicle->pathPos.angles);
      vpc->m_path_mat.setPosition(ent->vehicle->pathPos.origin);

      // Reset location vector
      vpc->b1_r_loc.x = 0.0f;
      vpc->b1_r_loc.y = 0.0f;
      vpc->b1_r_loc.w = 0.0f;

      if (nitrousVehicle->m_num_colliding_wheels >= 3 ||
          nitrousVehicle->m_vehicle_def->type == vehicle::VehicleType::BOAT) {
        vpc->b1_r_loc.z = -6.0f;
      } else {
        vpc->b1_r_loc.z = 0.0f;
      }

    } else {
      const int32_t physTime = Phys_GetCurrentTime();
      // Inter-update logic / rendering
      if (vpc->m_timestamp > physTime) {
        vpc->m_timestamp = physTime;
      }
    }
  }
}
} // namespace phys
} // namespace game