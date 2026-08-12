#include <std_include.hpp>
#include "scr.hpp"

#include <utils/string.hpp>

namespace game {
namespace scr {
void Scr_PlaySoundAtPosition_Impl(scriptInstance_t inst) {
  vec3_t origin;
  Scr_GetVector(inst, 1u, &origin);
  if (!game::bg::BG_ValidateOrigin(&origin, &sv::svs->mapCenter)) {
    Scr_NetworkOriginError("sound", &origin);
  }
  const char *alias = Scr_GetString(inst, 0);
  snd::SndAliasId alias_id = snd::SND_FindAliasId(alias);
  G_PlaySoundAliasAtPoint(&origin, alias_id);
}

std::vector<volatile var::ScrVarValue_t *>
Scr_GetArray(scriptInstance_t inst, var::ScrVarIndex_t idx) {
  if (Scr_GetPointerType(inst, idx) == var::ScrVarType::ARRAY) {
    const var::ScrVarIndex_t arrayIdx = Scr_GetObject(inst, idx);
    volatile var::ScrVar_t *element =
        &vm::gScrVarGlob->instance[inst].scriptVariables[arrayIdx];

    // First array element also contains size of array
    const uint32_t len = element->size(inst);

    std::vector<volatile var::ScrVarValue_t *> result = {nullptr};
    result.resize(len);

    for (uint32_t i = 0; i < len && element;
         ++i, element = element->next_sibling(inst)) {
      result.push_back(&element->value);
    }
    return result;
  }
  return {};
}
} // namespace scr
} // namespace game