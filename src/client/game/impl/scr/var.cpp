#include <std_include.hpp>
#include "var.hpp"

#include <utils/string.hpp>

namespace game {
namespace scr {
namespace var {
// ScrVar_EvalBool impl with added support for undefined, uintptr_t, uint64
// types to allow graceful handling without error
bool ScrVar_EvalBool_Impl([[maybe_unused]] scriptInstance_t inst,
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
    var::ScrVar_ReleaseValue(inst, value);
    Scr_Error(inst,
              utils::string::va("cannot cast %s to bool", Scr_TypeName(type)),
              0);
    return false;
  }
  }
}
} // namespace var

} // namespace scr
} // namespace game