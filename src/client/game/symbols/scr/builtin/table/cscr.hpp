#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace cscr {
WEAK symbol<BuiltinFunctionTable> builtin_functions{0x1432E16B0, 0x141077690};
WEAK symbol<BuiltinMethodTable> builtin_methods{0x1432E3610, 0x1410795F0};
WEAK symbol<GfxFunctionTable> gfx_functions{0x1432DC710, 0x141072710};
WEAK symbol<GfxMethodTable> gfx_methods{0x1432DD450, 0x141073450};
WEAK symbol<MathFunctionTable> math_functions{0x1432DD9A0, 0x1410739A0};
WEAK symbol<SoundFunctionTable> sound_functions{0x1432DDAD0, 0x141073AD0};
WEAK symbol<SoundMethodTable> sound_methods{0x1432DDEB0, 0x141073EB0};
WEAK symbol<UIFunctionTable> ui_functions{0x1432DDFC0, 0x141073FC0};
WEAK symbol<UtilFunctionTable> util_functions{0x1432DE250, 0x141074250};
WEAK symbol<UtilMethodTable> util_methods{0x1432DE4B0, 0x1410744B0};
} // namespace cscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game