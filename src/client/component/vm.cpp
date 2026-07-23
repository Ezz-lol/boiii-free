#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

namespace vm {
inline bool vm_op_call_state_valid(game::scr::scriptInstance_t inst) {
  const auto *frame = game::scr::vm::gScrVmPub->instance[inst].function_frame;
  return game::valid_module_ptr(frame) &&
        game::valid_module_ptr(frame->fs.startTop) &&
        game::valid_module_ptr(frame->fs.top);
}

void log_invalid_vm_state(const char *handler_name) {
  fprintf(stderr, "[vm] skipped %s: invalid function_frame state\n",
         handler_name);
  game::com::Com_Printf(
      0, game::consoleLabel_e::DEFAULT,
      "[vm] skipped %s: invalid function_frame state\n", handler_name);
}

utils::hook::detour VM_OP_ScriptFunctionCall_Handler_hook;
void VM_OP_ScriptFunctionCall_Handler_Safe(
    game::scr::scriptInstance_t inst, game::scr::vm::function_stack_t *fs,
    volatile game::scr::vm::ScrVmContext_t *vmc, bool *terminate) {
  if (vm_op_call_state_valid(inst)) {
    VM_OP_ScriptFunctionCall_Handler_hook.invoke(inst, fs, vmc, terminate);
  } else {
    log_invalid_vm_state("VM_OP_ScriptFunctionCall_Handler");
  }
}

utils::hook::detour VM_OP_ScriptMethodCall_Handler_hook;
void VM_OP_ScriptMethodCall_Handler_Safe(
    game::scr::scriptInstance_t inst, game::scr::vm::function_stack_t *fs,
    volatile game::scr::vm::ScrVmContext_t *vmc, bool *terminate) {
  if (vm_op_call_state_valid(inst)) {
    VM_OP_ScriptMethodCall_Handler_hook.invoke(inst, fs, vmc, terminate);
  } else {
    log_invalid_vm_state("VM_OP_ScriptMethodCall_Handler");
  }
}

utils::hook::detour VM_OP_ScriptMethodThreadCall_Handler_hook;
void VM_OP_ScriptMethodThreadCall_Handler_Safe(
    game::scr::scriptInstance_t inst, game::scr::vm::function_stack_t *fs,
    volatile game::scr::vm::ScrVmContext_t *vmc, bool *terminate) {
  if (vm_op_call_state_valid(inst)) {
    VM_OP_ScriptMethodThreadCall_Handler_hook.invoke(inst, fs, vmc, terminate);
  } else {
    log_invalid_vm_state("VM_OP_ScriptMethodThreadCall_Handler");
  }
}

utils::hook::detour VM_OP_ScriptThreadCall_Handler_hook;
void VM_OP_ScriptThreadCall_Handler_Safe(
    game::scr::scriptInstance_t inst, game::scr::vm::function_stack_t *fs,
    volatile game::scr::vm::ScrVmContext_t *vmc, bool *terminate) {
  if (vm_op_call_state_valid(inst)) {
    VM_OP_ScriptThreadCall_Handler_hook.invoke(inst, fs, vmc, terminate);
  } else {
    log_invalid_vm_state("VM_OP_ScriptThreadCall_Handler");
  }
}

struct component final : generic_component {
  void post_unpack() override {
    /*
       Fix memory access exception in the VM_OP_Script*Call_Handler opcode
       handlers, observed crashing during zombies round transitions (e.g.
       Der Eisendrache). Root cause is difficult to narrow down due to a
       callstack obfuscated by arxan: the embedded-call-count guard already
       present in these handlers is intact and unmodified, but the
       function_frame pointer they index into is sometimes invalid despite
       the guard passing. We circumvent this by verifying function_frame and
       its startTop/top fields are valid before letting the original handler
       run, logging and skipping the call instead of invoking it when they
       are not.
    */
    VM_OP_ScriptFunctionCall_Handler_hook.create(
        game::scr::vm::op::VM_OP_ScriptFunctionCall_Handler.get(),
        VM_OP_ScriptFunctionCall_Handler_Safe);
    VM_OP_ScriptMethodCall_Handler_hook.create(
        game::scr::vm::op::VM_OP_ScriptMethodCall_Handler.get(),
        VM_OP_ScriptMethodCall_Handler_Safe);
    VM_OP_ScriptMethodThreadCall_Handler_hook.create(
        game::scr::vm::op::VM_OP_ScriptMethodThreadCall_Handler.get(),
        VM_OP_ScriptMethodThreadCall_Handler_Safe);
    VM_OP_ScriptThreadCall_Handler_hook.create(
        game::scr::vm::op::VM_OP_ScriptThreadCall_Handler.get(),
        VM_OP_ScriptThreadCall_Handler_Safe);
  }
};
} // namespace vm

REGISTER_COMPONENT(vm::component)
