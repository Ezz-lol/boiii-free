#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> log_append{0x15686EC90, 0x1568EDC10,
                                               0x14948DB90};
WEAK symbol<EngineDependentDvarMut> logfile{0x15686E8A8, 0x1568ED828,
                                            0x14948EE48};
WEAK symbol<EngineDependentDvarMut> log_filename{0x15686EC88, 0x1568EDC08,
                                                 0x14948DB88};
} // namespace game
