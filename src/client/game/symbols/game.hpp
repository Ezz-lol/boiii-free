#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {

WEAK symbol<void(level::gentity_s *ent, level::gentity_s *target, int mode,
                 const char *chatText)>
    G_Say{0x0, 0x140299170};
WEAK symbol<void(const char *fmt, ...)> G_LogPrintf{0x0, 0x1402A7BB0};

WEAK symbol<void(int32_t levelTime, int32_t randomSeed, game::qboolean restart,
                 qboolean registerDvars,

                 qboolean savegame)>
    G_InitGame{0x1419CA420, 0x1402ABB80};

WEAK symbol<level::gentity_s *()> G_Spawn{0x141B81420, 0x140308570};
WEAK symbol<void(level::gentity_s *ed)> G_FreeEntity{0x141B77950, 0x1403067A0};
WEAK symbol<void()> G_PrintEntities{0x141B7BE40, 0x140307800};

WEAK symbol<void()> G_ClearVehicleInputs{0x1423812E0, 0x1405C1200};
WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, BoneIndex bone)>
    G_PlaySoundAlias{0x0, 0x140307480};
WEAK symbol<level::gentity_s *(const vec3_t *origin,
                               game::snd::SndAliasId alias)>
    G_PlaySoundAliasAtPoint{0x0, 0x1403075C0};
WEAK symbol<level::gentity_s *(level::gentity_s *ent,
                               game::snd::SndAliasId alias,
                               scr::ScrString_t notifyString, uint32_t tag)>
    G_PlaySoundAliasWithNotify{0x0, 0x1403076E0};

} // namespace game

#endif
