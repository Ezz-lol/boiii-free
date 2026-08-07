#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// e.g. BG_ASM_
namespace bg {
WEAK symbol<bool(const vec3_t *pos, const vec3_t *mapCenter)> BG_ValidateOrigin{
    0x1426824E0, 0x1406A9BB0};
WEAK symbol<void(const level::trajectory_t *tr, int32_t atTime, vec3_t *result)>
    BG_EvaluateTrajectory{0x14267AF10, 0x1406A3DE0};
WEAK symbol<void(const level::trajectory_t *tr, int32_t atTime, vec3_t *result)>
    BG_EvaluateTrajectoryDelta{0x14267ABE0, 0x1406A3AB0};

WEAK symbol<qboolean(int32_t wheel0, int32_t wheel1, int32_t wheel2)>
    BG_VehicleWheelsCollinear{0x0, 0x1406D09C0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleStrafeMove{0x0, 0x1406D0080};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleStepSlideMove{0x0, 0x1406CFBA0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleSteerMove{0x0, 0x1406CF430};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleSlideMove{0x0, 0x1406CE2E0};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehiclePlaneMove{0x0, 0x14069E710};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)> BG_VehicleMove{
    0x0, 0x1406CE190};
WEAK symbol<void(move::pmove_t *pm, const move::pml_t *pml)>
    BG_VehicleHelicopterMove{0x0, 0x14069DC30};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 cm::GroundTrace *groundTrace, float dtt)>
    BG_VehicleGroundTrace{0x0, 0x1406CDFE0};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleGroundPlant{0x0, 0x1406CD680};
WEAK symbol<void(move::pmove_t *pm, vehicle::VehicleState *vehicleState,
                 const cm::GroundTrace *groundTrace, bool gravity, float dt)>
    BG_VehicleAirMove{0x0, 0x1406CD500};

} // namespace bg
} // namespace game