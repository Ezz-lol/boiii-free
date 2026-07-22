#pragma once

#include "game/structs/scr/builtin/core.hpp"

namespace game {
namespace scr {
namespace builtin {
namespace table {

struct ScrActorInterfaceMethodTable {
  const BuiltinMethodDef methods[8];
};
struct ScrActorMethodTable {
  const BuiltinMethodDef methods[141];
};
struct ScrBotMethodTable {
  const BuiltinMethodDef methods[69];
};
struct ScrPlayerMethodTable {
  const BuiltinMethodDef methods[402];
};

struct ScrCommonFunctionTable {
  const BuiltinFunctionDef functions[287];
};
struct ScrHelicopterMethodTable {
  const BuiltinMethodDef methods[7];
};
struct ScrHudElemMethodTable {
  const BuiltinMethodDef methods[28];
};
struct ScrMathFunctionTable {
  const BuiltinFunctionDef functions[4];
};
struct ScrScriptEntMethodTable {
  const BuiltinMethodDef methods[17];
};
struct ScrScriptVehicleMethodTable {
  const BuiltinMethodDef methods[127];
};
struct ScrSentientMethodTable {
  const BuiltinMethodDef methods[31];
};
struct ScrUtilFunctionTable {
  const BuiltinFunctionDef functions[3];
};

struct ScrCommonMethodsTable {
  const BuiltinMethodDef methods[218];
};

} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game