#include <std_include.hpp>
#include "vm.hpp"
#include "gsc_funcs.hpp"
#include "loader/component_loader.hpp"

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

  _mm_clflush(&game::scr::vm::gScrVmPub->instance[inst]);
  _mm_mfence();

  _mm_clflush(&game::scr::vm::gScrVarGlob->instance[inst]);
  _mm_mfence();

  _mm_clflush(&game::scr::vm::gScrVarGlob->instance[inst].scriptVariables);
  _mm_mfence();
}

game::scr::vm::op::VM_OP_FUNC_PTR
    HOOKED_OPCODE_HANDLERS[+vm::op::Opcode::Count] = {nullptr};

template <const vm::op::Opcode Op>
inline void
invoke_opcode(scriptInstance_t inst, volatile vm::function_stack_t *fs,
              volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {
  HOOKED_OPCODE_HANDLERS[+Op](inst, fs, vmc, terminate);
}

template <const game::scr::vm::op::Opcode Op>
__attribute__((optnone)) void
flush_exec(scriptInstance_t inst, volatile vm::function_stack_t *fs,
           volatile vm::ScrVmContext_t *vmc, volatile bool *terminate) {

  flush_stack(inst, fs, vmc, terminate);
  invoke_opcode<Op>(inst, fs, vmc, terminate);
}

bool try_redirect(scriptInstance_t inst, volatile vm::function_stack_t *fs) {
  if (gsc::detours_enabled.load(std::memory_order_seq_cst) &&
      inst == SCRIPTINSTANCE_SERVER) {
    // Step back one operation in bytecode
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
  flush_stack(inst, fs, vmc, terminate);
  if (!try_redirect(inst, fs)) {
    invoke_opcode<Op>(inst, fs, vmc, terminate);
  }
}

void hook_opcode(vm::op::Opcode opcode, const vm::op::VM_OP_FUNC_PTR hook,
                 vm::op::VM_OP_FUNC_PTR *out_orig) {
  if (vm::op::OPCODE_BYTECODE_MAP.contains(opcode)) {
    vm::op::VM_OP_FUNC_PTR *handler =
        vm::op::op_handler(vm::op::OPCODE_BYTECODE_MAP.at(opcode)[0]);
    if (!*out_orig)
      *out_orig = *handler;
    if (*handler == *out_orig)
      *handler = hook;
  } else {
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

inline constexpr void flush_exec_all() {
  HOOK_FLUSH_EXEC(Bit_And);
  HOOK_FLUSH_EXEC(Bit_Or);
  HOOK_FLUSH_EXEC(Bit_Xor);
  HOOK_FLUSH_EXEC(BoolComplement);
  HOOK_FLUSH_EXEC(BoolNot);
  HOOK_FLUSH_EXEC(CallBuiltin);
  HOOK_FLUSH_EXEC(CallBuiltinMethod);
  HOOK_FLUSH_EXEC(CastBool);
  HOOK_FLUSH_EXEC(CastFieldObject);
  HOOK_FLUSH_EXEC(ClearArray);
  HOOK_FLUSH_EXEC(ClearFieldVariable);
  HOOK_FLUSH_EXEC(ClearParams);
  HOOK_FLUSH_EXEC(Dec);
  HOOK_FLUSH_EXEC(DecTop);
  HOOK_FLUSH_EXEC(Divide);
  HOOK_FLUSH_EXEC(End);
  HOOK_FLUSH_EXEC(EndOn);
  HOOK_FLUSH_EXEC(EndSwitch);
  HOOK_FLUSH_EXEC(Equal);
  HOOK_FLUSH_EXEC(EvalArray);
  HOOK_FLUSH_EXEC(EvalArrayRef);
  HOOK_FLUSH_EXEC(EvalFieldVariable);
  HOOK_FLUSH_EXEC(EvalFieldVariableRef);
  HOOK_FLUSH_EXEC(EvalLocalVariableCached);
  HOOK_FLUSH_EXEC(EvalLocalVariableCachedDebug);
  HOOK_FLUSH_EXEC(EvalLocalVariableRefCached);
  HOOK_FLUSH_EXEC(EvalLocalVariableRefCachedDebug);
  HOOK_FLUSH_EXEC(GetLevelObject);
  HOOK_FLUSH_EXEC(GetNegByte);
  HOOK_FLUSH_EXEC(GetNegUnsignedShort);
  HOOK_FLUSH_EXEC(GetSelf);
  HOOK_FLUSH_EXEC(GetSelfObject);
  HOOK_FLUSH_EXEC(GetString);
  HOOK_FLUSH_EXEC(GetTime);
  HOOK_FLUSH_EXEC(GetUintptr);
  HOOK_FLUSH_EXEC(GetUndefined);
  HOOK_FLUSH_EXEC(GetUnsignedShort);
  HOOK_FLUSH_EXEC(GetVector);
  HOOK_FLUSH_EXEC(GetZero);
  HOOK_FLUSH_EXEC(GreaterThan);
  HOOK_FLUSH_EXEC(GreaterThanOrEqualTo);
  HOOK_FLUSH_EXEC(Inc);
  HOOK_FLUSH_EXEC(IsDefined);
  HOOK_FLUSH_EXEC(Jump);
  HOOK_FLUSH_EXEC(JumpOnFalse);
  HOOK_FLUSH_EXEC(JumpOnFalseExpr);
  HOOK_FLUSH_EXEC(JumpOnTrue);
  HOOK_FLUSH_EXEC(JumpOnTrueExpr);
  HOOK_FLUSH_EXEC(LessThan);
  HOOK_FLUSH_EXEC(LessThanOrEqualTo);
  HOOK_FLUSH_EXEC(Minus);
  HOOK_FLUSH_EXEC(Modulus);
  HOOK_FLUSH_EXEC(Multiply);
  HOOK_FLUSH_EXEC(New);
  HOOK_FLUSH_EXEC(NextArrayKey);
  HOOK_FLUSH_EXEC(NotEqual);
  HOOK_FLUSH_EXEC(Notify);
  HOOK_FLUSH_EXEC(Plus);
  HOOK_FLUSH_EXEC(PreScriptCall);
  HOOK_FLUSH_EXEC(ProfileStart);
  HOOK_FLUSH_EXEC(ProfileStop);
  HOOK_FLUSH_EXEC(WaitRealTime);
  HOOK_FLUSH_EXEC(Return);
  HOOK_FLUSH_EXEC(SafeDecTop);
  HOOK_FLUSH_EXEC(SafeSetVariableFieldCached);
  HOOK_FLUSH_EXEC(SafeSetWaittillVariableFieldCached);
  HOOK_FLUSH_EXEC(ScriptFunctionCall);
  HOOK_FLUSH_EXEC(ScriptFunctionCallClass);
  HOOK_FLUSH_EXEC(ScriptFunctionCallPointer);
  HOOK_FLUSH_EXEC(ScriptMethodCall);
  HOOK_FLUSH_EXEC(ScriptMethodCallPointer);
  HOOK_FLUSH_EXEC(ScriptMethodThreadCall);
  HOOK_FLUSH_EXEC(ScriptMethodThreadCallPointer);
  HOOK_FLUSH_EXEC(ScriptThreadCall);
  HOOK_FLUSH_EXEC(ScriptThreadCallClass);
  HOOK_FLUSH_EXEC(ScriptThreadCallPointer);
  HOOK_FLUSH_EXEC(SetVariableField);
  HOOK_FLUSH_EXEC(ShiftLeft);
  HOOK_FLUSH_EXEC(ShiftRight);
  HOOK_FLUSH_EXEC(SizeOf);
  HOOK_FLUSH_EXEC(Switch);
  HOOK_FLUSH_EXEC(Vector);
  HOOK_FLUSH_EXEC(VectorConstant);
  HOOK_FLUSH_EXEC(VectorScale);
  HOOK_FLUSH_EXEC(Wait);
  HOOK_FLUSH_EXEC(WaitTill);
  HOOK_FLUSH_EXEC(WaitTillFrameEnd);
  HOOK_FLUSH_EXEC(WaitTillMatch);
  HOOK_FLUSH_EXEC(DevblockBegin);
  HOOK_FLUSH_EXEC(EvalLevelFieldVariable);
  HOOK_FLUSH_EXEC(EvalLevelFieldVariableRef);
  HOOK_FLUSH_EXEC(EvalSelfFieldVariable);
  HOOK_FLUSH_EXEC(EvalSelfFieldVariableRef);
  HOOK_FLUSH_EXEC(GetWorld);
  HOOK_FLUSH_EXEC(GetWorldObject);
  HOOK_FLUSH_EXEC(SuperEqual);
  HOOK_FLUSH_EXEC(SuperNotEqual);
  HOOK_FLUSH_EXEC(WaitRealTime);

  // For `replacefunc` redirects
  HOOK_OP(SafeCreateLocalVariables, redirect_or_default);
  HOOK_OP(CheckClearParams, redirect_or_default);
}

struct component final : generic_component {
  void post_unpack() override { flush_exec_all(); }
};

} // namespace vm
} // namespace gsc
REGISTER_COMPONENT(gsc::vm::component);