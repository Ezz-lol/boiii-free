#pragma once

#include "core.hpp"

namespace game {
namespace scr {
namespace vm {
namespace op {
typedef fastcall_t<void(scriptInstance_t inst, function_stack_t *fs,
                        volatile ScrVmContext_t *vmc, bool *terminate)>
    VM_OP_FUNC;
typedef VM_OP_FUNC *VM_OP_FUNC_PTR;
typedef uint16_t OP_TYPE;

constexpr OP_TYPE VM_OP_JUMP_TABLE_LEN = 0x2000;
struct VmOpJumpTable {
  array<VM_OP_FUNC_PTR, VM_OP_JUMP_TABLE_LEN> ops;
};

constexpr OP_TYPE VM_OP_TABLE_1_IDX_MASK =
    static_cast<OP_TYPE>(-(static_cast<int16_t>(VM_OP_JUMP_TABLE_LEN)));
} // namespace op
} // namespace vm
} // namespace scr
} // namespace game