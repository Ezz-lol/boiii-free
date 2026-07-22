#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace bg {
WEAK symbol<BGScrCommonFunctionTable> common_functions{0x1432D6AB0,
                                                       0x14106CAB0};
WEAK symbol<BGScrMathFunctionTable> math_functions{0x1432D6F00, 0x14106CF00};
WEAK symbol<BGScrUtilFunctionTable> util_functions{0x1432D7530, 0x14106D530};
WEAK symbol<BGScrWeaponFunctionTable> weapon_functions{0x1432D7DE0,
                                                       0x14106DDE0};
} // namespace bg
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game