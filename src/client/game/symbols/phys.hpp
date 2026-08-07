#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

namespace phys {

namespace nuge {
WEAK
    symbol<void(const math::Dir3 *dim, math::Dir3 *unit_inertia, float *volume)>
        calc_box_inertia{0x0, 0x140024790};
}

WEAK symbol<void()> Phys_EffectsInit{0x0, 0x1405DAEB0};
WEAK symbol<void()> Phys_EffectsProcess{0x0, 0x1405DAF90};
WEAK symbol<void()> Phys_InitSoundAliases{0x0, 0x1405DAFA0};
WEAK symbol<void()> Phys_AddCacheImpulses{0x0, 0x1405DF370};
WEAK symbol<void()> Phys_CollisionCallback{0x0, 0x1405DF830};
WEAK symbol<PhysObjId(const BodyState *state, gjk_geom_list_t *gjk_geom_list,
                      const bool do_collision_test)>
    Phys_CreateBodyFromState{0x0, 0x1405DF990};
WEAK symbol<PhysObjUserData *()> Phys_CreateUserData{0x0, 0x1405E1480};
WEAK symbol<void(PhysObjUserData *userData)> Phys_DestroyUserData{0x0,
                                                                  0x1405E16E0};
WEAK symbol<int()> Phys_GetCurrentTime{0x0, 0x1405E2B70};
WEAK symbol<void()> Phys_Init{0x0, 0x1405E2F00};
WEAK symbol<void(PhysObjId physObjId, int collFlags)> Phys_ObjAddCollFlags{
    0x0, 0x1405E37B0};
WEAK symbol<void(PhysObjUserData *userData)> Phys_ObjDestroy{0x0, 0x1405E4240};
WEAK symbol<void(PhysObjId physObjId, int collFlags)> Phys_ObjRemoveCollFlags{
    0x0, 0x1405E4700};
WEAK symbol<void()> Phys_RagdollCollideWithWorld{0x0, 0x1405E4AC0};
WEAK symbol<void(int32_t timeNow)> Phys_RunToTime{0x0, 0x1405E5380};
WEAK symbol<bool(const broad_phase_base *bpi1, const broad_phase_base *bpi2)>
    Phys_ShouldCollideCallback{0x0, 0x1405E5680};
WEAK symbol<void(PhysObjUserData *userData, double deltaT)> Phys_UpdateUserData{
    0x0, 0x1405E5760};
WEAK symbol<void()> PhysicsSystem_Update{0x0, 0x1405E57F0};
WEAK symbol<void(float a1)> UpdateRigidBody{0x0, 0x1405E58B0};
WEAK symbol<void(rigid_body *body, int colflag_mask)> create_broad_phase_info{
    0x0, 0x1405E6B50};

WEAK symbol<void(rigid_body *body)> destroy_broad_phase_info{0x0, 0x1405E7D70};
WEAK symbol<rigid_body *(const cm::trace_t *traceResults)>
    GetTraceResultsRigidBody{0x0, 0x1405D7BC0};
WEAK symbol<void(const vec3_t *angles, const vec3_t *origin,
                 math::RotTranMat43 *outMat)>
    Phys_AnglesToMat44{0x14057F6C0, 0x1400AF410};

WEAK symbol<const dvar_t *> phys_wind_enabled{0x0, 0x14A7520A0};
WEAK symbol<const dvar_t *> Phys_Wind_Force_Speed{0x0, 0x14A7520A8};
WEAK symbol<const dvar_t *> phys_wind_force_direction{0x0, 0x14A7520B0};

WEAK symbol<void(const char *fmt)> PhysPrint{0x1423E5060, 0x1401FF9C0};
WEAK symbol<void(vehicle::NitrousVehicle *nveh_in)> collide_vehicle_wheels{
    0x1423B0070, 0x1405D8390};

} // namespace phys
} // namespace game