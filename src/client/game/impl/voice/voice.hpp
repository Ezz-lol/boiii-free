#pragma once

#include "game/game.hpp"

namespace game {
namespace voice {
int32_t LiveSteam_Voice_StopRecording_Impl(dsound_sample_t *pRecSample);
int32_t Record_Stop_Impl(dsound_sample_t *sample);
int32_t DSOUNDRecord_DestroySample_Impl(dsound_sample_t *pRecSample);
int32_t Record_DestroySample_Impl(dsound_sample_t *sample);
int32_t Sound_DestroySample_Impl(dsound_sample_t *sample);
void DSOUNDRecord_Shutdown_Impl();
void Record_Shutdown_Impl();
ULONG DSound_Shutdown_Impl();
ULONG Sound_Shutdown_Impl();
void Voice_Shutdown_Impl();
} // namespace voice
} // namespace game
