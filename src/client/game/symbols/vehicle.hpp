#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace vehicle {
WEAK symbol<phys::rigid_body_constraint_custom_path *(phys::rigid_body *rb)>
    path_constraint_create{0x1423177A0, 0x0, 0x1405C8850};
WEAK symbol<void(phys::rigid_body_constraint_custom_path *vpc,
                 const NitrousVehicle *nitrousVehicle)>
    path_constraint_update{0x142317870, 0x1423909E0, 0x1405C8920};

WEAK symbol<void(phys::rigid_body *rb)> fixup_wheel_constraints{0x0, 0x0,
                                                                0x1405E8010};

} // namespace vehicle
} // namespace game
