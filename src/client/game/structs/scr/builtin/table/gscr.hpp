#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {
namespace gscr {
struct GScrBuiltinFunctionTable {
  const BuiltinFunctionDef functions[336];
};

struct GScrSentientFunctionTable {
  const BuiltinFunctionDef functions[8];
};

struct GScrBuiltinMethodTable {
  const BuiltinMethodDef methods[345];
};

} // namespace gscr
} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game