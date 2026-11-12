#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/impl/mixer/mixer.hpp>
#include <game/impl/voice/voice.hpp>

#include <utils/hook.hpp>

namespace voice {
using namespace game::mixer;
using namespace game::voice;

inline void expand_wave_info_pool() {
  utils::hook::jump(Mixer_GetMuteState, Mixer_GetMuteState_Impl);
  utils::hook::jump(Mixer_GetRecordLevel, Mixer_GetRecordLevel_Impl);
  utils::hook::jump(Mixer_InitWaveInLineInfo, Mixer_InitWaveInLineInfo_Impl);
  utils::hook::jump(Mixer_RestoreWaveInMuteStates,
                    Mixer_RestoreWaveInMuteStates_Impl);
  utils::hook::jump(Mixer_RestoreWaveInRecordLevels,
                    Mixer_RestoreWaveInRecordLevels_Impl);
  utils::hook::jump(Mixer_SetWaveInMuteStates, Mixer_SetWaveInMuteStates_Impl);
  utils::hook::jump(Mixer_SetWaveInRecordLevels,
                    Mixer_SetWaveInRecordLevels_Impl);
  utils::hook::jump(Voice_Shutdown, Voice_Shutdown_Impl);
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("voice");

public:
  void post_unpack() override { expand_wave_info_pool(); }
};
} // namespace voice
REGISTER_COMPONENT(voice::component);
