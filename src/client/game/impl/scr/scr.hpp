#pragma once
#include <game/game.hpp>
#include <game/utils.hpp>

namespace game {
namespace scr {

inline level::gentity_t *GetEntity_Impl(const scr_entref_t *entref) {
  if (entref->classnum == 0) {
    return game::entity(entref->u.entnum);
  }
  Scr_ObjectError(SCRIPTINSTANCE_SERVER, "not an entity");
  return nullptr;
}

inline level::gentity_t *Scr_GetEntity_Impl(uint32_t index) {
  game::scr::scr_entref_t entref;
  game::scr::Scr_GetEntityRef(&entref, SCRIPTINSTANCE_SERVER, index);
  return GetEntity_Impl(&entref);
}

inline const char *Scr_TypeName(ScrVarType type) {
  return var_typename->pool[+type];
}
bool Scr_IsTrue_Impl(scriptInstance_t inst, ScrVarValue_t *value);
void Scr_PlaySoundAtPosition_Impl(scriptInstance_t inst);

} // namespace scr
} // namespace game