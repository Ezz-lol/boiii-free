#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace vm {
WEAK symbol<void(scriptInstance_t inst, function_stack_t *fs,
                 ScrVmContext_t *vmc, bool *terminate,
                 BuiltinFunction builtinfunction_address)>
    VM_CallBuiltinMethod{0x0, 0x14015C1D0};
}
} // namespace scr
} // namespace game