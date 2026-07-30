#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace he {

WEAK symbol<void(int32_t firstParmIndex, int32_t lastParmIndex,
                 const char *errorContext, char *string, uint32_t stringLimit)>
    Scr_ConstructMessageString{0x141AF3D50, 0x1402E91F0};
}
} // namespace scr
} // namespace game