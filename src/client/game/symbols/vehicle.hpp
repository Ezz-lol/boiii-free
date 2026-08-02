#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

namespace vehicle {
WEAK symbol<void(NitrousVehicle *nveh_in)> collide_vehicle_wheels{0x0,
                                                                  0x1405D8390};
} // namespace vehicle
} // namespace game