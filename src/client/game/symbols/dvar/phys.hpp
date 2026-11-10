#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> phys_buoyancy{0x157F09440, 0x14A751FF8};
WEAK symbol<EngineDependentDvarMut> phys_buoyancydistancecutoff{0x157F09428,
                                                                0x14A751FE0};
WEAK symbol<EngineDependentDvarMut> phys_buoyancyfloatheightoffset{0x157F09438,
                                                                   0x14A751FF0};
WEAK symbol<EngineDependentDvarMut> phys_disableentsanddynentscollision{
    0x157F09448, 0x14A752000};
WEAK symbol<EngineDependentDvarMut> phys_gravity{0x157EC93F8, 0x14A751FB0};
WEAK symbol<EngineDependentDvarMut> phys_gravity_dir{0x157F09400, 0x14A751FB8};
WEAK symbol<EngineDependentDvarMut> phys_impact_distance_cutoff{0x157EC8FA0,
                                                                0x14A711B60};
WEAK symbol<EngineDependentDvarMut> phys_impact_fx{0x0, 0x14A711B50};
WEAK symbol<EngineDependentDvarMut> phys_impact_intensity_limit{0x157EC8F98,
                                                                0x14A711B58};
WEAK symbol<EngineDependentDvarMut> phys_maxfloattime{0x157F09420, 0x14A751FD8};
WEAK symbol<EngineDependentDvarMut> phys_piecesspawndistancecutoff{0x157F09430,
                                                                   0x14A751FE8};
WEAK symbol<EngineDependentDvarMut> phys_ragdoll_buoyancy{0x157F09458,
                                                          0x14A752010};
WEAK symbol<EngineDependentDvarMut> phys_usephysicsforlocalclientifpossible{
    0x157F09450, 0x14A752008};
WEAK symbol<EngineDependentDvarMut> phys_vehicledamagefrocescale{0x157F09410,
                                                                 0x14A751FC8};
WEAK symbol<EngineDependentDvarMut> phys_vehiclegravitymultiplier{0x157F09408,
                                                                  0x14A751FC0};
WEAK symbol<EngineDependentDvarMut> phys_vehiclewheelentitycollision{
    0x157F09418, 0x14A751FD0};
WEAK symbol<EngineDependentDvarMut> phys_wind_distance_cutoff{0x157EC8FA8,
                                                              0x14A711B68};
WEAK symbol<EngineDependentDvarMut> phys_wind_enabled{0x157F094E0, 0x14A7520A0};
WEAK symbol<EngineDependentDvarMut> phys_wind_force_direction{0x157F094F0,
                                                              0x14A7520B0};
WEAK symbol<EngineDependentDvarMut> Phys_Wind_Force_Speed{0x157F094E8,
                                                          0x14A7520A8};
} // namespace game
