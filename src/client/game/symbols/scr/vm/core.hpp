#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace vm {
WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs,
                 volatile ScrVmContext_t *vmc, bool *terminate,
                 builtin::BuiltinFunction builtinFunction)>
    VM_CallBuiltinFunction{0x1412CE190, 0x14015C160};
WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs,
                 volatile ScrVmContext_t *vmc, bool *terminate,
                 builtin::BuiltinMethod builtinMethod)>
    VM_CallBuiltinMethod{0x1412CE200, 0x14015C1D0};
// Note: not an opcode handler. Just has the same signature as a
// `VM_OP_FUNC`. Used internally by several other opcode handlers.
WEAK symbol<op::VM_OP_FUNC> VM_PostCallBuiltin{0x1412CFC40, 0x14015D9E0};

// Script variable pools
WEAK symbol<var::ScrVarGlobPool> gScrVarGlob{0x1451A3500, 0x1426BDE80};
WEAK symbol<var::ScrVarPubPool> gScrVarPub{0x1451A3700, 0x1426BE080};

// VM runtime data pools
WEAK symbol<ScrVmGlobPool> gScrVmGlob{0x1451B4CC0, 0x1426CF610};
WEAK symbol<ScrVmPubPool> gScrVmPub{0x1451A3840, 0x1426BE190};

// Function stack
WEAK symbol<ScrPool<function_stack_t>> gFs{0x1451C4F30, 0x1426D7680};
WEAK symbol<op::OP_TYPE> g_endPos{0x1432E6340, 0x14107C140};

} // namespace vm
} // namespace scr
} // namespace game