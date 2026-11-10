#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> gts_validation_enabled{0x15686ED08,
                                                           0x14948DC08};
WEAK symbol<EngineDependentDvarMut> geographicalMatchmakingOverride{
    0x151395A10, 0x148374610};

} // namespace game
