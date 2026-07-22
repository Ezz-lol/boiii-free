#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace cscr {
WEAK symbol<CScrBuiltinFunctionTable> builtin_functions{0x1432E16B0,
                                                        0x141077690};
WEAK symbol<CScrBuiltinMethodTable> builtin_methods{0x1432E3610, 0x1410795F0};
WEAK symbol<CScrGfxFunctionTable> gfx_functions{0x1432DC710, 0x141072710};
WEAK symbol<CScrGfxMethodTable> gfx_methods{0x1432DD450, 0x141073450};
WEAK symbol<CScrMathFunctionTable> math_functions{0x1432DD9A0, 0x1410739A0};
WEAK symbol<CScrSoundFunctionTable> sound_functions{0x1432DDAD0, 0x141073AD0};
WEAK symbol<CScrSoundMethodTable> sound_methods{0x1432DDEB0, 0x141073EB0};
WEAK symbol<CScrUIFunctionTable> ui_functions{0x1432DDFC0, 0x141073FC0};
WEAK symbol<CScrUtilFunctionTable> util_functions{0x1432DE250, 0x141074250};
WEAK symbol<CScrUtilMethodTable> util_methods{0x1432DE4B0, 0x1410744B0};
} // namespace cscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game