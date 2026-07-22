#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace vm {
namespace op {

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

WEAK symbol<VM_OP_FUNC> VM_OP_CallBuiltin_Handler{0x1412CE460, 0x14015C3C0};
WEAK symbol<VM_OP_FUNC> VM_OP_CallBuiltinMethod_Handler{0x1412CE3A0,
                                                        0x14015C300};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptFunctionCallClass_Handler{0x1412CEB20,
                                                              0x14015CA30};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptFunctionCall_Handler{0x1412CEE80,
                                                         0x14015CCE0};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptFunctionCallPointer_Handler{0x1412CECB0,
                                                                0x14015CB70};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptMethodCall_Handler{0x1412CF1D0,
                                                       0x14015D030};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptMethodCallPointer_Handler{0x1412CEFA0,
                                                              0x14015CE00};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptMethodThreadCall_Handler{0x1412CF570,
                                                             0x14015D370};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptMethodThreadCallPointer_Handler{
    0x1412CF350, 0x14015D1B0};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptThreadCallClass_Handler{0x1412CF740,
                                                            0x14015D4E0};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptThreadCall_Handler{0x1412CFB10,
                                                       0x14015D8B0};
WEAK symbol<VM_OP_FUNC> VM_OP_ScriptThreadCallPointer_Handler{0x1412CF930,
                                                              0x14015D6D0};
} // namespace op
} // namespace vm
} // namespace scr
} // namespace game