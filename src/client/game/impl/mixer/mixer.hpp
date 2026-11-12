#pragma once

#include "game/game.hpp" // IWYU pragma: export

namespace game {
namespace mixer {
void Mixer_SetWaveInMuteStates_Impl(int32_t newMuteState);
void Mixer_RestoreWaveInRecordLevels_Impl();
int32_t Mixer_GetRecordLevel_Impl(UINT mixerID, DWORD waveInLineID);
int32_t Mixer_GetMuteState_Impl(UINT mixerID, UINT waveInLineID);
MMRESULT Mixer_InitWaveInLineInfo_Impl();
void Mixer_RestoreWaveInMuteStates_Impl();
void Mixer_SetWaveInRecordLevels_Impl(int32_t newLevel);
} // namespace mixer
} // namespace game
