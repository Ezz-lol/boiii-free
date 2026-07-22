#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace builtin {
WEAK symbol<BuiltinFunction(ScrVarCanonicalName_t canonId, int32_t *type,
                            int32_t *min_args, int32_t *max_args)>
    Scr_GetFunction{0x141AF7820, 0x1402E9850};
WEAK symbol<ScrVarCanonicalName_t(BuiltinFunction *func)>
    Scr_GetFunctionReverseLookup{0x141AF78D0, 0x1402E9900};
WEAK symbol<BuiltinMethod(ScrVarCanonicalName_t canonId, int32_t *type,
                          int32_t *min_args, int32_t *max_args)>
    Scr_GetMethod{0x141AF79B0, 0x1402E99E0};
WEAK symbol<ScrVarCanonicalName_t(BuiltinMethod *method)>
    Scr_GetMethodReverseLookup{0x141AF7AF0, 0x1402E9B20};
} // namespace builtin
} // namespace scr
} // namespace game