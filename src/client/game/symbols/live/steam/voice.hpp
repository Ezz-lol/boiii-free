#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace steam {
namespace voice {
// Note: `sample` _is_ passed to this function as argument, but is unused; it
// can be called without the argument as well.
WEAK symbol<int32_t(game::voice::dsound_sample_t *sample)>
    LiveSteam_Voice_StopRecording{0x141D1BF60, 0x0, 0x1403D17F0};
WEAK symbol<int32_t(ControllerIndex_t controllerIndex, XUID xuid, byte *data,
                    int32_t size)>
    LiveSteam_Voice_IncomingVoiceData{0x1422BAEA0, 0x0, 0x140598920};
} // namespace voice
} // namespace steam
} // namespace live
} // namespace game
