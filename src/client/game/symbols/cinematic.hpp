#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cinematic {
WEAK symbol<bool> s_loadingMovieSkippedState{0x144CCC6F0, 0x0};

WEAK symbol<void(const char *name, const char *key, unsigned int playbackFlags,
                 float volume, void *callbackInfo, int id)>
    Cinematic_StartPlayback{0x1412BE3C0, 0x0};
WEAK symbol<void(uint64_t id, bool cancelAll)> Cinematic_StopPlayback{
    0x1412BEA90, 0x0};
WEAK symbol<uint8_t> Cinematic_ResolvePaths_UsermapLoadedCall{0x1412BD47D, 0x0};
WEAK symbol<uint8_t> Cinematic_ResolvePaths_UsermapPathCall{0x1412BD486, 0x0};
WEAK symbol<uint8_t> Cinematic_ResolvePath_UsermapLoadedCall{0x1412BD638, 0x0};
WEAK symbol<uint8_t> Cinematic_ResolvePath_UsermapPathCall{0x1412BD641, 0x0};
WEAK symbol<void(bool skipState)> Cinematic_SetLoadingMovieSkippedState{
    0x1412BE390, 0x0};
} // namespace cinematic
} // namespace game
