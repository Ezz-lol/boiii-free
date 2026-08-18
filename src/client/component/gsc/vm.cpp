#include <std_include.hpp>
#include "vm.hpp"
#include <game/impl/scr/vm/op.hpp>
#include "gsc_funcs.hpp"
#include "loader/component_loader.hpp"
#include <utils/string.hpp>

namespace gsc {
namespace vm {

inline void flush_stack(scriptInstance_t inst,
                        volatile vm::function_stack_t *fs,
                        volatile vm::ScrVmContext_t *vmc,
                        volatile bool *terminate) {
  /*
    Checking if any of the pointers we flush here is a non-null value
    prior to a flush results in the value being cached following the function
    call. We unfortunately have to simply trust that the engine has not passed
    us any invalid pointers.

    However, these should generally be valid except in the case that the Scr VM
    has reached an extent of memory corruption where a crash is inevitable
    regardless - here, or in the opcode handler. As such, this seems to be
    acceptable.
  */
  _mm_clflush(const_cast<const vm::function_stack_t *>(fs));
  _mm_mfence();
  _mm_clflush(const_cast<const vm::ScrVmContext_t *>(vmc));
  _mm_mfence();
  _mm_clflush(const_cast<const bool *>(terminate));
  _mm_mfence();

  _mm_clflush(const_cast<const scrVmPub_t *>(
      &game::scr::vm::gScrVmPub->instance[inst]));
  _mm_mfence();

  _mm_clflush(const_cast<const scrVarGlob_t *>(
      &game::scr::vm::gScrVarGlob->instance[inst]));
  _mm_mfence();

  _mm_clflush(const_cast<const ScrVar_t *>(
      game::scr::vm::gScrVarGlob->instance[inst].scriptVariables));
  _mm_mfence();

  _mm_clflush(const_cast<const function_stack_t *>(
      &game::scr::vm::gFs->instance[inst]));
  _mm_mfence();
}

game::scr::vm::op::VM_OP_FUNC_PTR
    HOOKED_OPCODE_HANDLERS[+vm::op::Opcode::Count] = {nullptr};

game::scr::vm::op::VM_OP_FUNC_PTR
    HOOKED_OPCODE_ERR_HANDLERS[+vm::op::Opcode::Count] = {nullptr};

template <const vm::op::Opcode Op>
inline void __attribute__((optnone))
invoke_opcode(scriptInstance_t inst, volatile vm::function_stack_t *fs,
              volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {
  HOOKED_OPCODE_HANDLERS[+Op](inst, fs, vmc, terminate);
}

template <const vm::op::Opcode Op>
inline void __attribute__((optnone))
invoke_opcode_err(scriptInstance_t inst, volatile vm::function_stack_t *fs,
                  volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {
#ifndef NDEBUG
  game::trace("[Scr][VM] Entered VM_%s_ErrRecovery\n", vm::op::serialize(Op));
#endif
  HOOKED_OPCODE_ERR_HANDLERS[+Op](inst, fs, vmc, terminate);
}

template <const game::scr::vm::op::Opcode Op>
__attribute__((optnone)) void
flush_exec(scriptInstance_t inst, volatile vm::function_stack_t *fs,
           volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {

  flush_stack(inst, fs, vmc, terminate);
  invoke_opcode<Op>(inst, fs, vmc, terminate);
  flush_stack(inst, fs, vmc, terminate);
}

template <const game::scr::vm::op::Opcode Op>
__attribute__((optnone)) void
flush_exec_err(scriptInstance_t inst, volatile vm::function_stack_t *fs,
               volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {

  flush_stack(inst, fs, vmc, terminate);
  invoke_opcode_err<Op>(inst, fs, vmc, terminate);
  flush_stack(inst, fs, vmc, terminate);
}

inline bool try_redirect(scriptInstance_t inst,
                         volatile vm::function_stack_t *fs) {
  if (gsc::detours_enabled.load(std::memory_order_acquire) &&
      inst == SCRIPTINSTANCE_SERVER) {
    // Step behind the opcode in bytecode. Bytecode position increments past the
    // opcode just prior to execution in `VM_Execute`.
    uint8_t *redirected = fs->pos - sizeof(op::OP_TYPE);
    if (gsc::function_replacements.contains(redirected)) {
      fs->pos = gsc::function_replacements[redirected];
      return true;
    }
  }
  return false;
}

template <const vm::op::Opcode Op>
__attribute__((optnone)) void
redirect_or_default(scriptInstance_t inst, volatile vm::function_stack_t *fs,
                    volatile vm::ScrVmContext_t *vmc,
                    volatile bool *terminate) {
  if (!try_redirect(inst, fs)) {
    invoke_opcode<Op>(inst, fs, vmc, terminate);
  }
}

void hook_opcode(vm::op::Opcode opcode, const vm::op::VM_OP_FUNC_PTR hook,
                 vm::op::VM_OP_FUNC_PTR *out_orig) {
  if (vm::op::OPCODE_BYTECODE_MAP.contains(opcode)) [[likely]] {
    if (!*out_orig) {
      *out_orig =
          *vm::op::op_handler(vm::op::OPCODE_BYTECODE_MAP.at(opcode)[0]);
    }

    for (const vm::op::OP_TYPE bytecode :
         vm::op::OPCODE_BYTECODE_MAP.at(opcode)) {
      if (bytecode == 0x0000) {
        break;
      }
      vm::op::VM_OP_FUNC_PTR *handler = vm::op::op_handler(bytecode);
      if (*handler == *out_orig) [[likely]] {
        *handler = hook;
      } else [[unlikely]] {
        fprintf(
            stderr,
            "Warning: Opcode handler for opcode 0x%02X with jumptable index "
            "0x%04X at 0x%p did not match expected function pointer. Expected: "
            "0x%p, "
            "got: 0x%p. Skipping hook application.\n",
            +opcode, bytecode, game::derelocate(handler),
            game::derelocate(*out_orig), game::derelocate(*handler));
        fflush(stderr);
      }
    }
  } else [[unlikely]] {
    fprintf(stderr,
            "Warning: could not find valid bytecode value for opcode: "
            "0x%02X. Hook will not be applied.\n",
            static_cast<uint8_t>(opcode));
    fflush(stderr);
  }
}

void hook_opcode_err(vm::op::Opcode opcode, const vm::op::VM_OP_FUNC_PTR hook,
                     vm::op::VM_OP_FUNC_PTR *out_orig) {
  if (vm::op::OPCODE_BYTECODE_MAP.contains(opcode)) [[likely]] {

    if (!*out_orig) {
      *out_orig =
          *vm::op::op_err_handler(vm::op::OPCODE_BYTECODE_MAP.at(opcode)[0]);
    }

    for (const vm::op::OP_TYPE bytecode :
         vm::op::OPCODE_BYTECODE_MAP.at(opcode)) {
      if (bytecode == 0x0000) {
        break;
      }
      vm::op::VM_OP_FUNC_PTR *handler = vm::op::op_err_handler(bytecode);
      if (*handler == *out_orig) [[likely]] {
        *handler = hook;
      } else [[unlikely]] {
        fprintf(stderr,
                "Warning: Opcode error recovery handler for opcode 0x%02X with "
                "jumptable index "
                "0x%04X at 0x%p did not match expected function pointer. "
                "Expected: 0x%p, "
                "got: 0x%p. Skipping hook application.\n",
                +opcode, bytecode, game::derelocate(handler),
                game::derelocate(*out_orig), game::derelocate(*handler));
        fflush(stderr);
      }
    }
  } else [[unlikely]] {
    fprintf(stderr,
            "Warning: could not find valid bytecode value for opcode: "
            "0x%02X. Hook will not be applied.\n",
            static_cast<uint8_t>(opcode));
    fflush(stderr);
  }
}

#ifndef NSOP
#define NSOP(op_enumeration) op::Opcode::op_enumeration
#endif

#ifndef HOOK_OP
#define HOOK_OP(opcode_enum, hook)                                             \
  hook_opcode(NSOP(opcode_enum), hook<NSOP(opcode_enum)>,                      \
              &HOOKED_OPCODE_HANDLERS[+NSOP(opcode_enum)]);
#endif
#ifndef HOOK_FLUSH_EXEC
#define HOOK_FLUSH_EXEC(opcode_enum) HOOK_OP(opcode_enum, flush_exec)
#endif

#ifndef HOOK_OP_ERR
#define HOOK_OP_ERR(opcode_enum, hook)                                         \
  hook_opcode_err(NSOP(opcode_enum), hook<NSOP(opcode_enum)>,                  \
                  &HOOKED_OPCODE_ERR_HANDLERS[+NSOP(opcode_enum)]);
#endif
#ifndef HOOK_ERR_FLUSH_EXEC
#define HOOK_ERR_FLUSH_EXEC(opcode_enum)                                       \
  HOOK_OP_ERR(opcode_enum, flush_exec_err)
#endif

inline void flush_exec_all() {
  // Uses TAC encryption when accessing the `cgArray` in client. Hangs forever
  // if original implementation is called from outside the BO3 engine address
  // space.
  HOOKED_OPCODE_HANDLERS[+op::Opcode::GetTime] =
      &op::VM_OP_GetTime_Handler_Impl;

  // For `replacefunc` redirects
  HOOK_OP(SafeCreateLocalVariables, redirect_or_default);
  HOOK_OP(CheckClearParams, redirect_or_default);

#ifndef NDEBUG
  // Error handlers
  HOOK_ERR_FLUSH_EXEC(Bit_And);
  HOOK_ERR_FLUSH_EXEC(Bit_Or);
  HOOK_ERR_FLUSH_EXEC(Bit_Xor);
  HOOK_ERR_FLUSH_EXEC(BoolComplement);
  HOOK_ERR_FLUSH_EXEC(BoolNot);
  HOOK_ERR_FLUSH_EXEC(CallBuiltin);
  HOOK_ERR_FLUSH_EXEC(CallBuiltinMethod);
  HOOK_ERR_FLUSH_EXEC(CastBool);
  HOOK_ERR_FLUSH_EXEC(CastFieldObject);
  HOOK_ERR_FLUSH_EXEC(ClearArray);
  HOOK_ERR_FLUSH_EXEC(ClearFieldVariable);
  HOOK_ERR_FLUSH_EXEC(ClearParams);
  HOOK_ERR_FLUSH_EXEC(Dec);
  HOOK_ERR_FLUSH_EXEC(DecTop);
  HOOK_ERR_FLUSH_EXEC(DevblockBegin);
  HOOK_ERR_FLUSH_EXEC(Divide);
  HOOK_ERR_FLUSH_EXEC(End);
  HOOK_ERR_FLUSH_EXEC(EndOn);
  HOOK_ERR_FLUSH_EXEC(EndSwitch);
  HOOK_ERR_FLUSH_EXEC(Equal);
  HOOK_ERR_FLUSH_EXEC(EvalArray);
  HOOK_ERR_FLUSH_EXEC(EvalArrayRef);
  HOOK_ERR_FLUSH_EXEC(EvalFieldVariable);
  HOOK_ERR_FLUSH_EXEC(EvalFieldVariableRef);
  HOOK_ERR_FLUSH_EXEC(EvalLevelFieldVariable);
  HOOK_ERR_FLUSH_EXEC(EvalLevelFieldVariableRef);
  HOOK_ERR_FLUSH_EXEC(EvalLocalVariableCached);
  HOOK_ERR_FLUSH_EXEC(EvalLocalVariableCachedDebug);
  HOOK_ERR_FLUSH_EXEC(EvalLocalVariableRefCached);
  HOOK_ERR_FLUSH_EXEC(EvalLocalVariableRefCachedDebug);
  HOOK_ERR_FLUSH_EXEC(EvalSelfFieldVariable);
  HOOK_ERR_FLUSH_EXEC(EvalSelfFieldVariableRef);
  HOOK_ERR_FLUSH_EXEC(FirstArrayKey);
  HOOK_ERR_FLUSH_EXEC(GetAnim);
  HOOK_ERR_FLUSH_EXEC(GetAnimation);
  HOOK_ERR_FLUSH_EXEC(GetAnimObject);
  HOOK_ERR_FLUSH_EXEC(GetAPIFunction);
  HOOK_ERR_FLUSH_EXEC(GetByte);
  HOOK_ERR_FLUSH_EXEC(GetClasses);
  HOOK_ERR_FLUSH_EXEC(GetClassesObject);
  HOOK_ERR_FLUSH_EXEC(GetEmptyArray);
  HOOK_ERR_FLUSH_EXEC(GetFloat);
  HOOK_ERR_FLUSH_EXEC(GetFunction);
  HOOK_ERR_FLUSH_EXEC(GetGame);
  HOOK_ERR_FLUSH_EXEC(GetGameRef);
  HOOK_ERR_FLUSH_EXEC(GetHash);
  HOOK_ERR_FLUSH_EXEC(GetInteger);
  HOOK_ERR_FLUSH_EXEC(GetIString);
  HOOK_ERR_FLUSH_EXEC(GetLevel);
  HOOK_ERR_FLUSH_EXEC(GetLevelObject);
  HOOK_ERR_FLUSH_EXEC(GetNegByte);
  HOOK_ERR_FLUSH_EXEC(GetNegUnsignedShort);
  HOOK_ERR_FLUSH_EXEC(GetSelf);
  HOOK_ERR_FLUSH_EXEC(GetSelfObject);
  HOOK_ERR_FLUSH_EXEC(GetString);
  HOOK_ERR_FLUSH_EXEC(GetTime);
  HOOK_ERR_FLUSH_EXEC(GetUintptr);
  HOOK_ERR_FLUSH_EXEC(GetUndefined);
  HOOK_ERR_FLUSH_EXEC(GetUnsignedShort);
  HOOK_ERR_FLUSH_EXEC(GetVector);
  HOOK_ERR_FLUSH_EXEC(GetWorld);
  HOOK_ERR_FLUSH_EXEC(GetWorldObject);
  HOOK_ERR_FLUSH_EXEC(GetZero);
  HOOK_ERR_FLUSH_EXEC(GreaterThan);
  HOOK_ERR_FLUSH_EXEC(GreaterThanOrEqualTo);
  HOOK_ERR_FLUSH_EXEC(Inc);
  HOOK_ERR_FLUSH_EXEC(IsDefined);
  HOOK_ERR_FLUSH_EXEC(Jump);
  HOOK_ERR_FLUSH_EXEC(JumpOnFalse);
  HOOK_ERR_FLUSH_EXEC(JumpOnFalseExpr);
  HOOK_ERR_FLUSH_EXEC(JumpOnTrue);
  HOOK_ERR_FLUSH_EXEC(JumpOnTrueExpr);
  HOOK_ERR_FLUSH_EXEC(LessThan);
  HOOK_ERR_FLUSH_EXEC(LessThanOrEqualTo);
  HOOK_ERR_FLUSH_EXEC(Minus);
  HOOK_ERR_FLUSH_EXEC(Modulus);
  HOOK_ERR_FLUSH_EXEC(Multiply);
  HOOK_ERR_FLUSH_EXEC(New);
  HOOK_ERR_FLUSH_EXEC(NextArrayKey);
  HOOK_ERR_FLUSH_EXEC(NotEqual);
  HOOK_ERR_FLUSH_EXEC(Notify);
  HOOK_ERR_FLUSH_EXEC(Plus);
  HOOK_ERR_FLUSH_EXEC(PreScriptCall);
  HOOK_ERR_FLUSH_EXEC(ProfileStart);
  HOOK_ERR_FLUSH_EXEC(ProfileStop);
  HOOK_ERR_FLUSH_EXEC(Return);
  HOOK_ERR_FLUSH_EXEC(SafeDecTop);
  HOOK_ERR_FLUSH_EXEC(SafeSetVariableFieldCached);
  HOOK_ERR_FLUSH_EXEC(SafeSetWaittillVariableFieldCached);
  HOOK_ERR_FLUSH_EXEC(ScriptFunctionCall);
  HOOK_ERR_FLUSH_EXEC(ScriptFunctionCallClass);
  HOOK_ERR_FLUSH_EXEC(ScriptFunctionCallPointer);
  HOOK_ERR_FLUSH_EXEC(ScriptMethodCall);
  HOOK_ERR_FLUSH_EXEC(ScriptMethodCallPointer);
  HOOK_ERR_FLUSH_EXEC(ScriptMethodThreadCall);
  HOOK_ERR_FLUSH_EXEC(ScriptMethodThreadCallPointer);
  HOOK_ERR_FLUSH_EXEC(ScriptThreadCall);
  HOOK_ERR_FLUSH_EXEC(ScriptThreadCallClass);
  HOOK_ERR_FLUSH_EXEC(ScriptThreadCallPointer);
  HOOK_ERR_FLUSH_EXEC(SetVariableField);
  HOOK_ERR_FLUSH_EXEC(ShiftLeft);
  HOOK_ERR_FLUSH_EXEC(ShiftRight);
  HOOK_ERR_FLUSH_EXEC(SizeOf);
  HOOK_ERR_FLUSH_EXEC(SuperEqual);
  HOOK_ERR_FLUSH_EXEC(SuperNotEqual);
  HOOK_ERR_FLUSH_EXEC(Switch);
  HOOK_ERR_FLUSH_EXEC(Vector);
  HOOK_ERR_FLUSH_EXEC(VectorConstant);
  HOOK_ERR_FLUSH_EXEC(VectorScale);
  HOOK_ERR_FLUSH_EXEC(Wait);
  HOOK_ERR_FLUSH_EXEC(WaitRealTime);
  HOOK_ERR_FLUSH_EXEC(WaitTill);
  HOOK_ERR_FLUSH_EXEC(WaitTillFrameEnd);
  HOOK_ERR_FLUSH_EXEC(WaitTillMatch);
  HOOK_ERR_FLUSH_EXEC(SafeCreateLocalVariables);
  HOOK_ERR_FLUSH_EXEC(CheckClearParams);
#endif
}

struct component final : generic_component {
  void post_unpack() override { flush_exec_all(); }
};

} // namespace vm
} // namespace gsc
REGISTER_COMPONENT(gsc::vm::component);