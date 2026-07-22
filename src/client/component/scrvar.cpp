#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <game/impl/scr/var.hpp>
#include <utils/hook.hpp>

#include <mmeapi.h>

namespace scrvar {

using namespace game;
using namespace game::scr;
using namespace game::scr::var;

utils::hook::detour ScrVar_AddRefValue_hook;
void ScrVar_AddRefValue_Safe(scriptInstance_t inst, ScrVarValue_t *value) {
  if (valid_scrvarvalue_ptr(inst, value)) {
    return ScrVar_AddRefValue_hook.invoke<void>(inst, value);
  }
}

utils::hook::detour ScrVar_ReleaseVariable_hook;
uint32_t ScrVar_ReleaseVariable_Safe(scriptInstance_t inst, ScrVarIndex_t id) {
  if (valid_scrvar_index(inst, id)) {
    return ScrVar_ReleaseVariable_hook.invoke<uint32_t>(inst, id);
  }

  // Return 0 refcount for non-existent ScrVar
  return 0;
}

inline constexpr bool valid_scrvarvalue(ScrVarValue_t *value) {
  switch (value->type) {
  case ScrVarType::VECTOR: {
    return value->u.vectorValue != nullptr;
  }
  case ScrVarType::LOCALIZED_STRING:
  case ScrVarType::STRING:
    return sl::valid_refstring_index(value->u.string);
  default:
    return true;
  }
}

utils::hook::detour ScrVar_ReleaseValue_hook;
void ScrVar_ReleaseValue_Safe(scriptInstance_t inst, ScrVarValue_t *value) {
  if (valid_scrvarvalue_ptr(inst, value) && valid_scrvarvalue(value)) {
    ScrVar_ReleaseValue_hook.invoke(inst, value);
  }
}

utils::hook::detour ScrVar_EvalVariable_hook;
ScrVarValue_t *ScrVar_EvalVariable_Safe(ScrVarValue_t *retstr,
                                        scriptInstance_t inst,
                                        ScrVarIndex_t id) {
  if (id == 0 /* entity field */ ||
      valid_scrvar_index(inst,
                         id) /* index of variable to return as reference */) {
    return ScrVar_EvalVariable_hook.invoke<ScrVarValue_t *>(retstr, inst, id);
  }

  return retstr;
}

utils::hook::detour ScrVar_EvalArray_hook;
void ScrVar_EvalArray_DefaultEmpty(scriptInstance_t inst, ScrVarValue_t *value,
                                   ScrVarValue_t *index) {
  if (valid_scrvarvalue_ptr(inst, value) &&
      valid_scrvarvalue_ptr(inst, index)) {
    if (ScrVar_ArrayLike(inst, value) &&
        ScrVar_ValidIndex(inst, value, index)) {

      return ScrVar_EvalArray_hook.invoke<void>(inst, value, index);
    } else {
      index->u.pointerValue = 0;
      index->type = ScrVarType::INT;

      const ScrVarIndex_t allocated = ScrVar_AllocArray(inst);
      value->type = ScrVarType::POINTER;
      value->u.pointerValue = allocated;
    }
  }
}
void stub_func() { return; }

utils::hook::detour ScrVar_EvalFloatBool_hook;
utils::hook::detour ScrVar_EvalBool_hook;

inline void handle_invalid_scrvars() {
  ScrVar_ReleaseVariable_hook.create(ScrVar_ReleaseVariable.get(),
                                     ScrVar_ReleaseVariable_Safe);
  ScrVar_ReleaseValue_hook.create(ScrVar_ReleaseValue.get(),
                                  ScrVar_ReleaseValue_Safe);
  ScrVar_AddRefValue_hook.create(ScrVar_AddRefValue.get(),
                                 ScrVar_AddRefValue_Safe);
  ScrVar_EvalVariable_hook.create(ScrVar_EvalVariable.get(),
                                  ScrVar_EvalVariable_Safe);
  ScrVar_EvalArray_hook.create(ScrVar_EvalArray.get(),
                               ScrVar_EvalArray_DefaultEmpty);
  // Fix common "cannot cast undefined to bool" error in flagsys.gsc on
  // launching usermap in private match
  ScrVar_EvalFloatBool_hook.create(game::scr::var::ScrVar_EvalFloatBool.get(),
                                   game::scr::var::ScrVar_EvalBool_Impl);
  ScrVar_EvalBool_hook.create(game::scr::var::ScrVar_EvalBool.get(),
                              game::scr::var::ScrVar_EvalBool_Impl);
}

class component final : public generic_component {
public:
  void post_unpack() override { handle_invalid_scrvars(); }
};
} // namespace scrvar
REGISTER_COMPONENT(scrvar::component);