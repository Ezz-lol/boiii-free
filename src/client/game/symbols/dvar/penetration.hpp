#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> penetrationCount{0x0, 0x0, 0x14A1797A0};
WEAK symbol<EngineDependentDvarMut> penetrationCount_allies{0x0, 0x0,
                                                            0x14A1797C8};
WEAK symbol<EngineDependentDvarMut> penetrationCount_axis{0x0, 0x0,
                                                          0x14A1797B8};
} // namespace game
