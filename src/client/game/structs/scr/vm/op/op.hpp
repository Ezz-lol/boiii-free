#pragma once

#include "../core.hpp"
#include "code.hpp" // IWYU pragma: export

namespace game {
namespace scr {
namespace vm {
namespace op {

typedef fastcall_t<void(scriptInstance_t inst, function_stack_t *fs,
                        volatile ScrVmContext_t *vmc, bool *terminate)>
    VM_OP_FUNC;
typedef VM_OP_FUNC *VM_OP_FUNC_PTR;

constexpr OP_TYPE VM_OP_JUMP_TABLE_LEN = OPCODE_JUMPTABLE_ENTRY_MAP[0].size();
struct VmOpJumpTable {
  array<VM_OP_FUNC_PTR, VM_OP_JUMP_TABLE_LEN> ops;
};
} // namespace op
} // namespace vm
} // namespace scr
} // namespace game