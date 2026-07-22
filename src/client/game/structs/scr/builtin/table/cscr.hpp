#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace cscr {
struct CScrBuiltinFunctionTable {
  BuiltinFunctionDef functions[251];
};
struct CScrBuiltinMethodTable {
  const BuiltinMethodDef methods[246];
};
struct CScrGfxFunctionTable {
  const BuiltinFunctionDef functions[106];
};
struct CScrGfxMethodTable {
  const BuiltinMethodDef methods[42];
};
struct CScrMathFunctionTable {
  const BuiltinFunctionDef functions[9];
};
struct CScrSoundFunctionTable {
  const BuiltinFunctionDef functions[31];
};
struct CScrSoundMethodTable {
  const BuiltinMethodDef methods[8];
};
struct CScrUIFunctionTable {
  const BuiltinFunctionDef functions[20];
};
struct CScrUtilFunctionTable {
  const BuiltinFunctionDef functions[19];
};
struct CScrUtilMethodTable {
  const BuiltinMethodDef methods[2];
};

} // namespace cscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game