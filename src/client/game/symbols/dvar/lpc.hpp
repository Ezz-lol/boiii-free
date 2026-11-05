#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> lpc_forcedownloadfiles{
    0x156262C80, 0x1562E1BF0, 0x149114E30};
WEAK symbol<EngineDependentDvarMut> lpc_maxattempts{0x156262C78, 0x1562E1BE8,
                                                    0x149114E28};
} // namespace game
