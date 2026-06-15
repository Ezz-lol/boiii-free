#pragma once
#include "../../../../game/game.hpp"

namespace game {
namespace scr {
namespace cmd {

void ScrCmd_PlaySound_Impl(scriptInstance_t inst, scr_entref_t *entref);
void ScrCmd_PlaySoundOnTag_Impl(scriptInstance_t inst, scr_entref_t *entref);
void ScrCmd_PlaySoundToAllButPlayer_Impl(scriptInstance_t inst,
                                         scr_entref_t *entref);
void ScrCmd_PlaySoundToPlayer_Impl(scriptInstance_t inst, scr_entref_t *entref);
void ScrCmd_PlaySoundToTeam_Impl(scriptInstance_t inst, scr_entref_t *entref);
void ScrCmd_PlaySoundWithNotify_Impl(scriptInstance_t inst,
                                     scr_entref_t *entref);
} // namespace cmd
} // namespace scr
} // namespace game