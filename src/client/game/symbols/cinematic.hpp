#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cinematic {
WEAK symbol<void(const char *name, const char *key, unsigned int playbackFlags,
                 float volume, void *callbackInfo, int id)>
    Cinematic_StartPlayback{0x1412BE3A0};
WEAK symbol<void(uint64_t id, bool cancelAll)> Cinematic_StopPlayback{
    0x1412BEA70};
} // namespace cinematic
} // namespace game

#endif