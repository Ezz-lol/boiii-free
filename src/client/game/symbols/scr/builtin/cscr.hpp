#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace builtin {
namespace cscr {
WEAK symbol<BuiltinFunction(ScrVarCanonicalName_t canonId, int32_t *type,
                            int32_t *min_args, int32_t *max_args)>
    CScr_GetFunction{0x140CBC9E0, 0x140103F80};
WEAK symbol<ScrVarCanonicalName_t(BuiltinFunction *func)>
    CScr_GetFunctionReverseLookup{0x140CBCB20, 0x1401040C0};
WEAK symbol<BuiltinMethod(ScrVarCanonicalName_t canonId, int32_t *type,
                          int32_t *min_args, int32_t *max_args)>
    CScr_GetMethod{0x140CEA300, 0x140105590};
WEAK symbol<ScrVarCanonicalName_t(BuiltinMethod *method)>
    CScr_GetMethodReverseLookup{0x140CEA440, 0x1401056D0};
} // namespace cscr
} // namespace builtin
} // namespace scr
} // namespace game