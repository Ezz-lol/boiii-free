#pragma once

#include "game/symbols/sym_include.hpp"
namespace game {
namespace wave {
WEAK symbol<WaveInLineInfoPool> builtin_s_waveInLineInfo{0x157DF73D0,
                                                         0x14A641470};

extern ExtendedWaveInLineInfoPool s_waveInLineInfo;
} // namespace wave
} // namespace game
