#include <std_include.hpp>
#include "var.hpp"

#include <utils/string.hpp>

namespace game {
namespace scr {
namespace var {
// Scr_IsTrue impl with added support for undefined, uintptr_t, uint64
// types to allow graceful handling without error
qboolean Scr_IsTrue_Impl([[maybe_unused]] scriptInstance_t inst,
                         volatile ScrVarValue_t *value) {
  const std::optional<qboolean> result = value->cast_bool();
  if (result.has_value()) {
    value->type = ScrVarType::INT;
    value->u.intValue = result.value();
    return result.value();
  }

  var::ScrVar_ReleaseValue(inst, value);
  Scr_Error(
      inst,
      utils::string::va("cannot cast %s to bool", Scr_TypeName(value->type)),
      false);
  return qfalse;
}
} // namespace var

} // namespace scr
} // namespace game