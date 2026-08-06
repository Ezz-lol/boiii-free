#include <std_include.hpp>
#include "scr.hpp"

#include <utils/string.hpp>

namespace game {
namespace scr {
void Scr_PlaySoundAtPosition_Impl(scriptInstance_t inst) {
  vec3_t origin;
  Scr_GetVector(SCRIPTINSTANCE_SERVER, 1u, &origin);
  if (!game::bg::BG_ValidateOrigin(&origin, &sv::svs->mapCenter)) {
    Scr_NetworkOriginError("sound", &origin);
  }
  const char *alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
  snd::SndAliasId alias_id = snd::SND_FindAliasId(alias);
  G_PlaySoundAliasAtPoint(&origin, alias_id);
}

std::vector<volatile var::ScrVarValue_t *>
Scr_GetArray(scriptInstance_t inst, var::ScrVarIndex_t idx) {

  volatile var::ScrVarValue_t *array_val = Scr_GetValue(inst, idx);
  if (array_val) {
    volatile var::ScrVar_t *array = array_val->var();
    if (array->value.type == var::ScrVarType::POINTER) {
      volatile var::ScrVar_t *element = array->deref(inst);
      if (element->value.type == var::ScrVarType::ARRAY) {
        const uint32_t len = array->array_len();
        std::vector<volatile var::ScrVarValue_t *> result = {nullptr};
        result.resize(len);
        for (uint32_t i = 0; i < len && element &&
                             element->value.type == var::ScrVarType::ARRAY;
             ++i, element = element->next_sibling(inst)) {
          result.push_back(&element->value);
        }
        return result;
      }
    }
  }
  return {};
}
} // namespace scr
} // namespace game