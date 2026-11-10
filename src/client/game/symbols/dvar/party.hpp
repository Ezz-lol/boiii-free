#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> party_autoteams{0x1556CE440, 0x148580EE0};
WEAK symbol<EngineDependentDvarMut> partychatdisallowed{0x15118D9C0,
                                                        0x1482DEED0};
WEAK symbol<EngineDependentDvarMut> party_maxplayers{0x1556CE438, 0x148580ED8};
WEAK symbol<EngineDependentDvarMut> partymigrate_maxbwpackets{0x1556CE7C0,
                                                              0x148581260};
WEAK symbol<EngineDependentDvarMut> partyMigrate_NomineeRecalcInterval{
    0x1556CE7E0, 0x148581280};
WEAK symbol<EngineDependentDvarMut> partymigrate_testinterval{0x1556CE7F0,
                                                              0x148581290};
WEAK symbol<EngineDependentDvarMut> party_minplayers{0x1556CE430, 0x148580ED0};
WEAK symbol<EngineDependentDvarMut> partyprivacycheckinterval{0x1556CE470,
                                                              0x148580F10};
WEAK symbol<EngineDependentDvarMut> partyprivacyenabled{0x1556CE460,
                                                        0x148580F00};
WEAK symbol<EngineDependentDvarMut> partyprivacypumpenabled{0x1556CE468,
                                                            0x148580F08};
WEAK symbol<EngineDependentDvarMut> leavewithpartyenabled{0x1556CE488,
                                                          0x148580F28};

} // namespace game
