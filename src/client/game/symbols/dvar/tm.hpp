#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> tm_failsThreshold{0x157A22B18, 0x157AA1A18,
                                                      0x0};
WEAK symbol<EngineDependentDvarMut> tm_logTimeLimit{0x157A22B38, 0x157AA1A38,
                                                    0x0};
WEAK symbol<EngineDependentDvarMut> tm_pendingeThreshold{0x157A22B28,
                                                         0x157AA1A28, 0x0};
WEAK symbol<EngineDependentDvarMut> tm_startedThreshold{0x157A22B10,
                                                        0x157AA1A10, 0x0};
WEAK symbol<EngineDependentDvarMut> tm_succeedsThreshold{0x157A22B20,
                                                         0x157AA1A20, 0x0};
WEAK symbol<EngineDependentDvarMut> tm_taskManagerDebug{0x157A22B08,
                                                        0x157AA1A08, 0x0};
WEAK symbol<EngineDependentDvarMut> tm_taskTrackerOn{0x157A22B00, 0x157AA1A00,
                                                     0x0};
WEAK symbol<EngineDependentDvarMut> tm_typeToDisplay{0x157A22B30, 0x157AA1A30,
                                                     0x0};
} // namespace game
