#include <std_include.hpp>

#include "voice.hpp"

#include "game/impl/mixer/mixer.hpp"

namespace game {
namespace voice {

using namespace game;
using namespace game::wave;

int32_t LiveSteam_Voice_StopRecording_Impl(dsound_sample_t *pRecSample) {

  if (*g_recording_initialized) {
    ::steam::SteamUser()->StopVoiceRecording();

    *g_current_sample = nullptr;
    *g_currently_recording = false;
    return 0;
  }

  return -1;
}
int32_t Record_Stop_Impl(dsound_sample_t *sample) {
  return LiveSteam_Voice_StopRecording_Impl(sample);
}

int32_t DSOUNDRecord_DestroySample_Impl(dsound_sample_t *pRecSample) {
  if (*g_recording_initialized) {
    if (pRecSample->DSB != nullptr) {
      pRecSample->DSB->Release();
      pRecSample->DSB = nullptr;
    }

    return 1;
  }

  return 0;
}

int32_t Record_DestroySample_Impl(dsound_sample_t *sample) {
  return DSOUNDRecord_DestroySample_Impl(sample);
}

int32_t Sound_DestroySample_Impl(dsound_sample_t *sample) {
  return Record_DestroySample_Impl(sample);
}

void DSOUNDRecord_Shutdown_Impl() { *g_recording_initialized = false; }

void Record_Shutdown_Impl() { DSOUNDRecord_Shutdown_Impl(); }

ULONG DSound_Shutdown_Impl() {
  if (*dsoundplay_initialized) {
    const ULONG result = get_lpds().Release();
    *dsoundplay_initialized = false;

    return result;
  }
  return 0;
}

ULONG Sound_Shutdown_Impl() { return DSound_Shutdown_Impl(); }

void Voice_Shutdown_Impl() {
  lobby::LobbyClientPool<dsound_sample_t *> &client_samples = *s_clientSamples;
  for (dsound_sample_t *sample : client_samples) {
    if (sample != nullptr) {
      Sound_DestroySample_Impl(sample);
    }
  }

  mixer::Mixer_RestoreWaveInRecordLevels_Impl();
  mixer::Mixer_RestoreWaveInMuteStates_Impl();

  s_waveInLineInfo = {};

  if (*recording) {
    Record_Stop_Impl(*currentRecordingSample);
    Record_DestroySample_Impl(*currentRecordingSample);
    *recording = false;
  }

  Record_Shutdown_Impl();
  Sound_Shutdown_Impl();

  *g_voice_initialised = qfalse;
  *g_voice_record_initialized = qfalse;
}

} // namespace voice
} // namespace game
