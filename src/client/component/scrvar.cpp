#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <utils/hook.hpp>

#include <mmeapi.h>

namespace scrvar {

using namespace game;
using namespace game::scr;

utils::hook::detour ScrVar_AddRefValue_hook;
void ScrVar_AddRefValue_safe(scriptInstance_t inst, ScrVarValue_t *value) {
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

utils::hook::detour ScrVar_ReleaseValue_hook;
void ScrVar_ReleaseValue_Safe(scriptInstance_t inst, ScrVarValue_t *value) {
  if (valid_scrvarvalue_ptr(inst, value)) {
    ScrVar_ReleaseValue_hook.invoke(inst, value);
  }
}

utils::hook::detour ScrVar_EvalVariable_hook;
ScrVarValue_t *ScrVar_EvalVariable_safe(ScrVarValue_t *retstr,
                                        scriptInstance_t inst,
                                        ScrVarIndex_t id) {
  if (id == 0 /* entity field */ ||
      valid_scrvar_index(inst,
                         id) /* index of variable to return as reference */) {
    return ScrVar_EvalVariable_hook.invoke<ScrVarValue_t *>(retstr, inst, id);
  }

  return retstr;
}

void stub_func() { return; }

inline void handle_invalid_scrvars() {
  ScrVar_ReleaseVariable_hook.create(game::scr::ScrVar_ReleaseVariable.get(),
                                     ScrVar_ReleaseVariable_Safe);
  ScrVar_ReleaseValue_hook.create(game::scr::ScrVar_ReleaseValue.get(),
                                  ScrVar_ReleaseValue_Safe);
  ScrVar_AddRefValue_hook.create(game::scr::ScrVar_AddRefValue.get(),
                                 ScrVar_AddRefValue_safe);
  ScrVar_EvalVariable_hook.create(game::scr::ScrVar_EvalVariable.get(),
                                  ScrVar_EvalVariable_safe);
}

class component final : public generic_component {
public:
  void post_unpack() override { handle_invalid_scrvars(); }
};
} // namespace scrvar
REGISTER_COMPONENT(scrvar::component);