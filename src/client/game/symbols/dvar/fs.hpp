#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

WEAK symbol<EngineDependentDvarMut> fs_basegame{0x1579E63F0, 0x157A652F0,
                                                0x14A39C630};
WEAK symbol<EngineDependentDvarMut> fs_basepath{0x1579E63E8, 0x157A652E8,
                                                0x14A39C628};
WEAK symbol<EngineDependentDvarMut> fs_debug{0x1579E62D8, 0x157A651D8,
                                             0x14A39C518};
WEAK symbol<EngineDependentDvarMut> fs_game{0x1579E63F8, 0x157A652F8,
                                            0x14A39C638};
WEAK symbol<EngineDependentDvarMut> fs_homepath{0x1579E63E0, 0x157A652E0,
                                                0x14A39C620};
WEAK symbol<EngineDependentDvarMut> fs_ignoreLocalized{0x1579E62D0, 0x157A651D0,
                                                       0x14A39C510};
WEAK symbol<EngineDependentDvarMut> fs_restrict{0x1579E6400, 0x157A65300,
                                                0x14A39C640};
WEAK symbol<EngineDependentDvarMut> fs_usedevdir{0x0, 0x0, 0x14A39C508};
} // namespace game
