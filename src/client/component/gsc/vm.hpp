#pragma once

#include <std_include.hpp>
#include <game/game.hpp>

namespace gsc {
namespace vm {
using namespace game;
using namespace game::scr;
using namespace game::scr::vm;

extern game::scr::vm::op::VM_OP_FUNC_PTR
    HOOKED_OPCODE_HANDLERS[+vm::op::Opcode::Count];

void hook_opcode(vm::op::Opcode opcode, const vm::op::VM_OP_FUNC_PTR hook,
                 vm::op::VM_OP_FUNC_PTR *out_orig);
} // namespace vm
} // namespace gsc