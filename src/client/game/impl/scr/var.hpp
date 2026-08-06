#pragma once
#include <game/game.hpp>
#include <game/utils.hpp>

namespace game {
namespace scr {
namespace var {

inline const char *Scr_TypeName(ScrVarType type) {
  return var_typename->pool[+type];
}
qboolean Scr_IsTrue_Impl(scriptInstance_t inst, volatile ScrVarValue_t *value);

inline bool ScrVar_ArrayLike(scriptInstance_t inst,
                             volatile ScrVarValue_t *array) {
  return array->array_like(inst);
}

inline bool ScrVar_ValidIndex(scriptInstance_t inst,
                              volatile ScrVarValue_t *array,
                              volatile ScrVarValue_t *index) {
  switch (array->type) {
  case ScrVarType::POINTER: {
    volatile ScrVarValue_t *deref = array->deref(inst);
    // Anything can be an index here - the VM uses hashing to resolve the
    // numeric index where required
    return deref && deref->type == ScrVarType::ARRAY;
  }
  case ScrVarType::STRING:
  case ScrVarType::VECTOR: {
    return index->type == ScrVarType::INT;
  }
  default: {
    return false;
  }
  }
}

inline uint32_t ScrVar_ArrayLike_Size(scriptInstance_t inst,
                                      volatile ScrVarValue_t *array) {
  return array->deref(inst)->var()->o.size;
}

inline constexpr bool ScrVar_StringLike(ScrVarType_t type) {
  switch (type) {
  case ScrVarType::LOCALIZED_STRING:
  case ScrVarType::STRING:
    return true;
  default:
    return false;
  }
}

inline constexpr bool ScrVar_StringLike(volatile ScrVarValue_t *val) {
  return ScrVar_StringLike(val->type);
}

inline constexpr float ScrVar_CastFloat(volatile ScrVarValue_t *val) {
  if (val) {
    switch (val->type) {
    case ScrVarType::POINTER:
    case ScrVarType::UINT64:
    case ScrVarType::UINTPTR_T:
    case ScrVarType::CODEPOS:
    case ScrVarType::PRECODEPOS:
    case ScrVarType::API_FUNCTION:
    case ScrVarType::FUNCTION:
    case ScrVarType::ANIMATION:
    case ScrVarType::CLASS:
    case ScrVarType::STRUCT:
    case ScrVarType::ENTITY:
    case ScrVarType::ARRAY: {
      /*
        Blindly re-interpret the value as an integer, then cast to float.
        This will only lead to erroneous behaviour if the user unknowingly
        passed a non-floating point value. In this case, we can only assume
        they intended to have the passed value converted to a float, rather
        than throwing an error.
      */
      return static_cast<float>(val->u.uint64Value);
    }
    case ScrVarType::INT:
    case ScrVarType::ENTITY_OFFSET:
    case ScrVarType::HASH: {
      return static_cast<float>(val->u.uintValue);
    }
    case ScrVarType::FLOAT: {
      return val->u.floatValue;
    }

    case ScrVarType::VECTOR: {
      if (val->u.vectorValue) {
        return *val->u.vectorValue;
      }
      break;
    }
    case ScrVarType::STRING:
    case ScrVarType::LOCALIZED_STRING: {
      const char *str = sl::SL_ConvertToString(val->u.stringValue);
      if (str && str[0]) {
        const size_t len = strlen(str);
        float f = 0.0f;
        auto [ptr, ec] = std::from_chars(str, str + len, f);
        if (ec == std::errc()) {
          return f;
        }
      }
      break;
    }
    default:
      break;
    }
  }

  return std::numeric_limits<float>::quiet_NaN();
}
} // namespace var
} // namespace scr
} // namespace game