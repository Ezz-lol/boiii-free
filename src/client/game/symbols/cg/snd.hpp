#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cg {
namespace snd {
WEAK symbol<void(game::snd::cmd::SndCommandBuffer *buffer)> SND_CommandCG{
    0x142270780, 0x140545B60};
WEAK symbol<void(game::snd::SndEntHandle ent, scr::ScrString_t xanimName)>
    CG_PlayFacialAnimation{0x14102ED90, 0x14011E680};
WEAK symbol<void(int32_t msec, int32_t lengthNotifyData)>
    CG_ScriptSndLengthNotify{0x141039030, 0x14011F2D0};
WEAK symbol<void(game::snd::SndEntHandle handle)> SND_SetEntState{0x142273800,
                                                                  0x140548DC0};
WEAK symbol<void(int32_t msec, const char *lengthNotifyData)>
    CG_SubtitleSndLengthNotify{0x14104F8E0, 0x140120200};
WEAK symbol<void(LocalClientNum_t localClientNum, int32_t entityNum,
                 const vec3_t *origin, const vec3_t *direction, int32_t fadeMs,
                 bool doNotify, float attenuation, game::snd::SndAliasId id)>
    CG_PlaySound{0x1410324D0};

} // namespace snd
} // namespace cg
} // namespace game
