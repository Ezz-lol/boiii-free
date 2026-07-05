#include "../../../std_include.hpp"
#include "scr.hpp"

#include "../../../../common/utils/string.hpp"

namespace game {
namespace scr {

// Scr_IsTrue impl with added support for undefined, uintptr_t, uint64 types
// to allow graceful handling without error
bool Scr_IsTrue_Impl([[maybe_unused]] scriptInstance_t inst,
                     ScrVarValue_t *value) {
  ScrVarType_t type = value->type;

  switch (type) {
  case ScrVarType::FLOAT: {
    if (value->u.floatValue == 0.0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return false;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return true;
  }

  case ScrVarType::INT: {
    if (value->u.uintValue == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return false;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return true;
  }
  case ScrVarType::UINT64: {
    if (value->u.uint64Value == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return false;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return true;
  }
  case ScrVarType::UINTPTR_T: {
    if (value->u.uintptrValue == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return false;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return true;
  }
  case ScrVarType::UNDEFINED: {
    value->type = ScrVarType::INT;
    value->u.uintValue = qfalse;
    return false;
  }
  default: {
    ScrVar_ReleaseValue(inst, value);
    Scr_Error(inst,
              utils::string::va("cannot cast %s to bool", Scr_TypeName(type)),
              0);
    return false;
  }
  }
}

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
} // namespace scr
} // namespace game