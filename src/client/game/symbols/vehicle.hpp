#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

namespace vehicle {
WEAK symbol<void(NitrousVehicle *nveh_in)> collide_vehicle_wheels{0x0,
                                                                  0x1405D8390};
WEAK symbol<phys::rigid_body_constraint_custom_path *(phys::rigid_body *rb)>
    path_constraint_create{0x0, 0x1405C8850};
WEAK symbol<void(phys::rigid_body_constraint_custom_path *vpc,
                 const NitrousVehicle *nitrousVehicle)>
    path_constraint_update{0x0, 0x1405C8920};

WEAK symbol<void(phys::rigid_body *rb)> fixup_wheel_constraints{0x0,
                                                                0x1405E8010};

WEAK symbol<void()> G_ClearVehicleInputs{0x0, 0x1405C1200};

} // namespace vehicle
} // namespace game