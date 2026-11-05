#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> netchan_ackinterval{0x0, 0x0, 0x1498C1850};
WEAK symbol<EngineDependentDvarMut> netchan_debugSpew{0x0, 0x156E49AE0,
                                                      0x1498C1880};
WEAK symbol<EngineDependentDvarMut> netchan_debugSpewChan{0x0, 0x156E49AE8,
                                                          0x1498C1888};
WEAK symbol<EngineDependentDvarMut> netchan_doswap{0x0, 0x0, 0x1498C18A8};
WEAK symbol<EngineDependentDvarMut> netchan_drawgraph{0x0, 0x0, 0x1498C1890};
WEAK symbol<EngineDependentDvarMut> netchan_emergencyfreepercent{
    0x156DCAB58, 0x156E49AC8, 0x1498C1868};
WEAK symbol<EngineDependentDvarMut> netchan_fakeload{0x0, 0x0, 0x1498C18A0};
WEAK symbol<EngineDependentDvarMut> netchan_firstackwait{0x0, 0x156E49AD0,
                                                         0x1498C1870};
WEAK symbol<EngineDependentDvarMut> netchan_jqprofenabled{0x0, 0x0,
                                                          0x1498C18B8};
WEAK symbol<EngineDependentDvarMut> netchan_jqprofthreshold{0x0, 0x0,
                                                            0x1498C18C0};
WEAK symbol<EngineDependentDvarMut> netchan_logtofile{0x0, 0x0, 0x1498C1898};
WEAK symbol<EngineDependentDvarMut> netchan_mainthreadwaitms{0x0, 0x0,
                                                             0x1498C18B0};
WEAK symbol<EngineDependentDvarMut> netchan_msglifetime{
    0x156DCAB50, 0x156E49AC0, 0x1498C1860};
WEAK symbol<EngineDependentDvarMut> netchan_nackwaitms{0x0, 0x0, 0x1498C1878};
WEAK symbol<EngineDependentDvarMut> netchan_statsresetinterval{0x0, 0x0,
                                                               0x1498C1858};

} // namespace game
