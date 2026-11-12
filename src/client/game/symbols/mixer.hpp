#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace mixer {
WEAK symbol<int32_t(uint32_t mixerID, uint32_t waveInLineID)>
    Mixer_GetMuteState{0x1422BA2B0, 0x140597EA0};
WEAK symbol<int32_t(uint32_t mixerID, uint32_t waveInLineID)>
    Mixer_GetRecordLevel{0x1422BA400, 0x140597FF0};
WEAK symbol<MMRESULT()> Mixer_InitWaveInLineInfo{0x1422BA550, 0x140598140};
WEAK symbol<void()> Mixer_RestoreWaveInMuteStates{0x1422BA6F0, 0x1405982E0};
WEAK symbol<void()> Mixer_RestoreWaveInRecordLevels{0x1422BA880, 0x140598470};
WEAK symbol<void(int32_t muteState)> Mixer_SetWaveInMuteStates{0x1422BAA10,
                                                               0x140598600};
WEAK symbol<void(int32_t newLevel)> Mixer_SetWaveInRecordLevels{0x1422BABA0,
                                                                0x140598790};

} // namespace mixer
} // namespace game
