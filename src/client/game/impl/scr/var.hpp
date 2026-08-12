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
  return array && array->array_like(inst);
}

inline bool ScrVar_ValidIndex(scriptInstance_t inst,
                              volatile ScrVarValue_t *array,
                              volatile ScrVarValue_t *index) {
  return array && index && array->valid_index(inst, index);
}

inline uint32_t ScrVar_ArrayLike_Size(scriptInstance_t inst,
                                      volatile ScrVarValue_t *array) {
  return array && array->size(inst);
}

inline constexpr bool ScrVar_StringLike(ScrVarType_t type) {
  return string_like(type);
}

inline constexpr bool ScrVar_StringLike(volatile ScrVarValue_t *val) {
  return val && val->string_like();
}

inline constexpr float ScrVar_CastFloat(volatile ScrVarValue_t *val) {
  if (val) {
    const std::optional<float> result = val->cast_float();
    if (result.has_value()) {
      return result.value();
    }
  }

  return std::numeric_limits<float>::quiet_NaN();
}
} // namespace var
} // namespace scr
} // namespace game