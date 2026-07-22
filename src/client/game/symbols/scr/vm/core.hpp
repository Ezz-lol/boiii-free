#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace vm {

WEAK symbol<var::ScrVarGlobPool> gScrVarGlob{0x1451A3500, 0x1426BDE80};
WEAK symbol<var::ScrVarPubPool> gScrVarPub{0x1451A3700, 0x1426BE080};

WEAK symbol<VmOpJumpTable> gVmOpJumpTable1{0x143306350, 0x14109C150};
WEAK symbol<VmOpJumpTable> gVmOpJumpTable2{0x1432E6350, 0x14107C150};

inline VM_OP_FUNC_PTR *op_handler(OP_TYPE op) {
  if ((op & VM_OP_JUMP_TABLE_LEN) != 0) {
    return &gVmOpJumpTable1->ops[op & VM_OP_TABLE_1_IDX_MASK];
  }

  return &gVmOpJumpTable2->ops[op];
}

WEAK symbol<VmOpJumpTable> gVmErrRecoveryJumpTable1{0x143316350, 0x1410AC150};
WEAK symbol<VmOpJumpTable> gVmErrRecoveryJumpTable2{0x1432F6350, 0x14108C150};

inline VM_OP_FUNC_PTR *op_err_handler(OP_TYPE op) {
  if ((op & VM_OP_JUMP_TABLE_LEN) != 0) {
    return &gVmErrRecoveryJumpTable1->ops[op & VM_OP_TABLE_1_IDX_MASK];
  }

  return &gVmErrRecoveryJumpTable2->ops[op];
}

WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs,
                 ScrVmContext_t *vmc, bool *terminate,
                 builtin::BuiltinFunction builtinfunction_address)>
    VM_CallBuiltinMethod{0x0, 0x14015C1D0};
} // namespace vm
} // namespace scr
} // namespace game