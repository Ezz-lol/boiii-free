#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace gscr {
WEAK symbol<BuiltinFunctionTable> builtin_functions{0x143347C00, 0x1410D7E30};
WEAK symbol<BuiltinMethodTable> builtin_methods{0x14334A600, 0x1410DA830};
} // namespace gscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game