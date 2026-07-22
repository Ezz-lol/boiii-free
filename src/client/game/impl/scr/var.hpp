#pragma once
#include <game/game.hpp>
#include <game/utils.hpp>

namespace game {
namespace scr {
namespace var {

inline const char *Scr_TypeName(ScrVarType type) {
  return var_typename->pool[+type];
}
bool ScrVar_EvalBool_Impl(scriptInstance_t inst, ScrVarValue_t *value);

inline ScrVar_t *ScrVar_Dereference(scriptInstance_t inst,
                                    const ScrVar_t *ptr) {
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

  return const_cast<ScrVar_t *>(ptr);
}

inline ScrVarValue_t *ScrVar_Dereference(scriptInstance_t inst,
                                         const ScrVarValue_t *ptr) {
  return &ScrVar_Dereference(inst, ptr->var())->value;
}

inline bool ScrVar_ArrayLike(scriptInstance_t inst,
                             const ScrVarValue_t *array) {
  switch (array->type) {
  case ScrVarType::POINTER: {
    const ScrVarValue_t *deref = ScrVar_Dereference(inst, array);
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

inline bool ScrVar_ValidIndex(scriptInstance_t inst, ScrVarValue_t *array,
                              ScrVarValue_t *index) {
  switch (array->type) {
  case ScrVarType::POINTER: {
    ScrVarValue_t *deref = ScrVar_Dereference(inst, array);
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
                                      const ScrVarValue_t *array) {
  return ScrVar_Dereference(inst, array)->var()->o.size;
}
} // namespace var
} // namespace scr
} // namespace game