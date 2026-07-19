#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace scr {
namespace bg {
WEAK symbol<BGScrUtilFunctionTable> util_functions{0x1432D7530, 0x14106D530};
WEAK symbol<void(scriptInstance_t inst)> BGScr_IsProfileBuild{0x1401660E0,
                                                              0x140099230};
} // namespace bg
} // namespace scr
} // namespace game