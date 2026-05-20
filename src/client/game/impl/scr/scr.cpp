#include "scr.hpp"
#include <string>
#include <utils/string.hpp>
#include <cstdint>

namespace game {
namespace scr {

const char *Scr_TypeName(ScrVarType type) {
  return (*var_typename)[static_cast<uint32_t>(type)];
}

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
      return 0;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return 1;
  }

  case ScrVarType::INT: {
    if (value->u.uintValue == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return 0;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return 1;
  }
  case ScrVarType::UINT64: {
    if (value->u.uint64Value == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return 0;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return 1;
  }
  case ScrVarType::UINTPTR_T: {
    if (value->u.uintptrValue == 0) {
      value->type = ScrVarType::INT;
      value->u.uintValue = qfalse;
      return 0;
    }
    value->type = ScrVarType::INT;
    value->u.uintValue = qtrue;
    return 1;
  }
  case ScrVarType::UNDEFINED: {
    value->type = ScrVarType::INT;
    value->u.uintValue = qfalse;
    return 0;
  }
  default: {
    ScrVar_ReleaseValue(inst, value);
    std::string error_string =
        utils::string::va("cannot cast %s to bool", Scr_TypeName(type));
    Scr_Error(inst, error_string.c_str(), 0);
    return 0;
  }
  }
}
} // namespace scr
} // namespace game