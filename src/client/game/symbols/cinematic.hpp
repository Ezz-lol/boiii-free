#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cinematic {
WEAK symbol<void(const char *name, const char *key, unsigned int playbackFlags,
                 float volume, void *callbackInfo, int id)>
    Cinematic_StartPlayback{0x1412BE3C0, 0x1412BE3A0, 0x0};
WEAK symbol<void(uint64_t id, bool cancelAll)> Cinematic_StopPlayback{
    0x1412BEA90, 0x1412BEA70, 0x0};
} // namespace cinematic
} // namespace game
