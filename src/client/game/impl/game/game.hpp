#pragma once
#include "../../../game/game.hpp"
#include <cstdint>

namespace game {

void G_RegisterSoundWait_Impl(level::gentity_s *ent, snd::SndAliasId id,
                              scr::ScrString_t notifyString);
level::gentity_t *G_PlaySoundAliasWithNotify_Impl(level::gentity_t *ent,
                                                  snd::SndAliasId alias,
                                                  scr::ScrString_t notifyString,
                                                  uint32_t tag);
level::gentity_t *G_PlaySoundAlias_Impl(level::gentity_t *ent,
                                        snd::SndAliasId alias,
                                        scr::ScrString_t notifyString,
                                        BoneIndex bone);
level::gentity_t *G_PlaySoundAliasAtPoint_Impl(const vec3_t *origin,
                                               snd::SndAliasId alias);
} // namespace game