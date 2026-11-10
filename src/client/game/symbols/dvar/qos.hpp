#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> qos_echo_chance{0x15138F238, 0x14836DFD0};
WEAK symbol<EngineDependentDvarMut> qos_firstupdatems{0x15138F1E0, 0x14836DF78};
WEAK symbol<EngineDependentDvarMut> qos_lastupdatems{0x15138F1E8, 0x14836DF80};
WEAK symbol<EngineDependentDvarMut> qoslocklessqueue{0x1556CE758, 0x1485811F8};
WEAK symbol<EngineDependentDvarMut> qosmaxallowedping{0x1556CE738, 0x1485811D8};
WEAK symbol<EngineDependentDvarMut> qos_maxprobewait{0x15138F1F0, 0x14836DF88};
WEAK symbol<EngineDependentDvarMut> qos_minechoservers{0x15138F200,
                                                       0x14836DF98};
WEAK symbol<EngineDependentDvarMut> qos_minpercent{0x15138F1D8, 0x14836DF70};
WEAK symbol<EngineDependentDvarMut> qos_minprobes{0x15138F1D0, 0x14836DF68};
WEAK symbol<EngineDependentDvarMut> qos_packetlosspercent{0x15138F228,
                                                          0x14836DFC0};
WEAK symbol<EngineDependentDvarMut> qospreferredping{0x1556CE730, 0x1485811D0};
WEAK symbol<EngineDependentDvarMut> qos_simulatelongqos{0x15138F1F8,
                                                        0x14836DF90};
} // namespace game
