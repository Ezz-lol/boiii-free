#pragma once

#include <game/game.hpp>

namespace game {
namespace phys {
void path_constraint_update_sv(rigid_body_constraint_custom_path *vpc,
                               const vehicle::NitrousVehicle *nitrousVehicle);
}
} // namespace game