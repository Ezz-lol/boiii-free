#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> beamDrawCurveVerts{0x14421F588,
                                                       0x141E7ED08};
WEAK symbol<EngineDependentDvarMut> beamDrawNodes{0x14421F580, 0x141E7ED00};
WEAK symbol<EngineDependentDvarMut> beamDrawSlackInfo{0x14421F598, 0x141E7ED18};
WEAK symbol<EngineDependentDvarMut> beamDrawWaveVerts{0x14421F590, 0x141E7ED10};
} // namespace game
