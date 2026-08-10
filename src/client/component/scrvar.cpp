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
using namespace scr::mt;

utils::hook::detour ScrVar_AddRefValue_hook;
void ScrVar_AddRefValue_Safe(scriptInstance_t inst,
                             volatile ScrVarValue_t *value) {
  if (valid_scrvarvalue_ptr(inst, value)) {
    ScrVar_AddRefValue_hook.invoke<void>(inst, value);
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

inline constexpr bool valid_scrvarvalue(volatile ScrVarValue_t *value) {
  switch (value->type) {
  case ScrVarType::VECTOR: {
    return valid_val_allocation_ptr(
        reinterpret_cast<uintptr_t>(value->u.vectorValue) - 1);
  }
  case ScrVarType::LOCALIZED_STRING:
  case ScrVarType::STRING:
    return sl::valid_refstring_index(value->u.string);
  default:
    return true;
  }
}

utils::hook::detour ScrVar_ReleaseValue_hook;
void ScrVar_ReleaseValue_Safe(scriptInstance_t inst,
                              volatile ScrVarValue_t *value) {
  if (valid_scrvarvalue_ptr(inst, value)) {
    if (valid_scrvarvalue(value) &&
        /* Do not attempt to free compile-time constant vectors
           embedded in script bytecode.

           Required in addition to the `MT_Free` hook below to ensure
           that there is no attempt to decrement the non-existent refcount for
           the vector in the memory pool.
        */
        (value->type != ScrVarType::VECTOR ||
         gScrMemTreeGlob->nodePool->contains(value->u.vectorValue))) {

      ScrVar_ReleaseValue_hook.invoke(inst, value);
    } else {
      value->type = ScrVarType::UNDEFINED;
      value->u.pointerValue = 0;
    }
  }
}

utils::hook::detour ScrVar_EvalVariable_hook;
volatile ScrVarValue_t *ScrVar_EvalVariable_Safe(volatile ScrVarValue_t *retstr,
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
void ScrVar_EvalArray_DefaultEmpty(scriptInstance_t inst,
                                   volatile ScrVarValue_t *value,
                                   volatile ScrVarValue_t *index) {
  if (valid_scrvarvalue_ptr(inst, value) &&
      valid_scrvarvalue_ptr(inst, index)) {
    if (value->array_like(inst) && value->valid_index(inst, index)) {

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

/*
  The engine attempts to free compile-time constant, bytecode-allocated
  vectors upon release, under the erroneous assumption that all script vectors
  were allocated by the script memory tree.

  This is a bug. Compile-time constant vectors can also be embedded directly
  into the script bytecode when using the `GetVector` opcode. Attempt to free a
  vector embedded into the script bytecode as though it were allocated in the
  script MemoryTree node pool results in a memory access exception.

  Treyarch circumvents this issue by never emitting the `GetVector` opcode
  in their GSC compiler. Upon attempting to use this opcode in our GSC compiler,
  this bug was encountered.

  We _could_ circumvent this bug the same way Treyarch does - by always
  using MemoryTree-allocated vectors via the `Vector` opcode, even where
  the vector is a compile-time constant - but this would be less performant at
  runtime.

  Thus, the below hook is used to ensure that a given allocation pointer is
  only used for an attempted free if it is a valid pointer to a node in
  gScrMemTreeGlob's node pool. Usage of an invalid node pointer - such as a
  pointer to a vector embedded in a script's bytecode - will result in an
  immediate return, without an attempted free.
*/
utils::hook::detour MT_Free_hook;
void MT_Free_IfValid(volatile void *ptr, int32_t numBytes) {
  if (gScrMemTreeGlob->nodePool && gScrMemTreeGlob->nodePool->contains(ptr)) {
    MT_Free_hook.invoke(ptr, numBytes);
  }
}

utils::hook::detour ScrVar_CastBool_NonInteger_hook;
utils::hook::detour Scr_IsTrue_hook;

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
  ScrVar_CastBool_NonInteger_hook.create(
      game::scr::var::ScrVar_CastBool_NonInteger.get(),
      game::scr::var::Scr_IsTrue_Impl);
  Scr_IsTrue_hook.create(game::scr::var::Scr_IsTrue.get(),
                         game::scr::var::Scr_IsTrue_Impl);
}

inline void handle_invalid_mt_allocs() {
  MT_Free_hook.create(MT_Free, MT_Free_IfValid);
}

class component final : public generic_component {
public:
  void post_unpack() override {
    handle_invalid_scrvars();
    handle_invalid_mt_allocs();
  }
};
} // namespace scrvar
REGISTER_COMPONENT(scrvar::component);