#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> profileDirtyInterval{
    0x1511F0838, 0x15126F7B8, 0x148336158};
WEAK symbol<EngineDependentDvarMut> profileGetInterval{0x1511F0840, 0x15126F7C0,
                                                       0x148336160};
} // namespace game
