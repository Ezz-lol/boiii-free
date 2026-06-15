#pragma once
#include "../../../game/game.hpp"

namespace game {
namespace scr {

level::gentity_t *GetEntity_Impl(scr_entref_t entref);
level::gentity_t *Scr_GetEntity_Impl(uint32_t index);
const char *Scr_TypeName(ScrVarType type);
bool Scr_IsTrue_Impl(scriptInstance_t inst, ScrVarValue_t *value);
void Scr_PlaySoundAtPosition_Impl(scriptInstance_t inst);

} // namespace scr
} // namespace game