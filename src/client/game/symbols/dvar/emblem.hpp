#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> emblems_enabled{0x150BC5B88, 0x150C44B08,
                                                    0x148293838};
WEAK symbol<EngineDependentDvarMut> emblems_max_count{0x150BC5B80, 0x150C44B00,
                                                      0x148293830};
WEAK symbol<EngineDependentDvarMut> emblemVersion{0x150BC5B90, 0x150C44B10,
                                                  0x148293840};
WEAK symbol<EngineDependentDvarMut> default_emblems_max_count{
    0x150BC5B98, 0x150C44B18, 0x148293848};
WEAK symbol<EngineDependentDvarMut> allEmblemsPurchased{0x159B80478,
                                                        0x159BFF3F8, 0x0};
WEAK symbol<EngineDependentDvarMut> allEmblemsUnlocked{0x159B80470, 0x159BFF3F0,
                                                       0x0};

} // namespace game
