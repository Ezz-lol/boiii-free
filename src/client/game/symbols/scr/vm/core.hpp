#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace vm {
WEAK symbol<var::ScrVarGlobPool> gScrVarGlob{0x1451A3500, 0x1426BDE80};
WEAK symbol<var::ScrVarPubPool> gScrVarPub{0x1451A3700, 0x1426BE080};

WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs_0,
                 volatile ScrVmContext_t *vmc, bool *terminate,
                 builtin::BuiltinFunction builtinFunction)>
    VM_CallBuiltinFunction{0x1412CE190, 0x14015C160};
WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs_0,
                 volatile ScrVmContext_t *vmc, bool *terminate,
                 builtin::BuiltinMethod builtinMethod)>
    VM_CallBuiltinMethod{0x1412CE200, 0x14015C1D0};
// Note: not an opcode handler. Just has the same signature as a
// `VM_OP_FUNC`. Used internally by several other opcode handlers.
WEAK symbol<op::VM_OP_FUNC> VM_PostCallBuiltin{0x1412CFC40, 0x14015D9E0};

} // namespace vm
} // namespace scr
} // namespace game