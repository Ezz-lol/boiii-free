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

/*
  Note: this does not work for pointer-typed `ScrVarValue_t` values stored
  in the VM runtime stack, as seen in `Scr_GetValue`'s
  return. Those pointers are an index from the _top_ of the VM runtime stack,
  rather than an absolute index to a value in the `scriptVariables` pool.

  The aforementioned pointer type functions similarly to x86's RIP-relative
  addressing, whereas those handled here function similarly to x86's absolute
  addressing.
*/
inline volatile ScrVar_t *ScrVar_Dereference(scriptInstance_t inst,
                                             volatile ScrVar_t *ptr) {
  switch (ptr->value.type) {
  case ScrVarType::POINTER:
    if (valid_scrvar_index(inst, ptr->value.u.pointerValue)) {
      return &vm::gScrVarGlob->instance[inst]
                  .scriptVariables[ptr->value.u.pointerValue];
    }
    break;
  default:
    break;
  }

  return ptr;
}

inline volatile ScrVarValue_t *ScrVar_Dereference(scriptInstance_t inst,
                                                  volatile ScrVarValue_t *ptr) {
  return &ScrVar_Dereference(inst, ptr->var())->value;
}

inline bool ScrVar_ArrayLike(scriptInstance_t inst,
                             volatile ScrVarValue_t *array) {
  switch (array->type) {
  case ScrVarType::POINTER: {
    volatile ScrVarValue_t *deref = ScrVar_Dereference(inst, array);
    return deref && deref->type == ScrVarType::ARRAY;
  }
  case ScrVarType::STRING:
  case ScrVarType::VECTOR: {
    return true;
  }
  default: {
    return false;
  }
  }
}

inline bool ScrVar_ValidIndex(scriptInstance_t inst,
                              volatile ScrVarValue_t *array,
                              volatile ScrVarValue_t *index) {
  switch (array->type) {
  case ScrVarType::POINTER: {
    volatile ScrVarValue_t *deref = ScrVar_Dereference(inst, array);
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
  return ScrVar_Dereference(inst, array)->var()->o.size;
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
} // namespace var
} // namespace scr
} // namespace game