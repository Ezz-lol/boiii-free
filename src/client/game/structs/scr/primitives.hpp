#pragma once

#include <cstdint>
namespace game {
namespace scr {
typedef uint32_t ScrString_t;
typedef uint32_t ScrVarCanonicalName_t;
typedef ScrString_t ScriptString;

enum scriptInstance_t {
  SCRIPTINSTANCE_SERVER = 0x0,
  SCRIPTINSTANCE_CLIENT = 0x1,
  SCRIPTINSTANCE_MAX = 0x2,
};

inline constexpr const char *serialize(scriptInstance_t inst) {
  switch (inst) {
  case SCRIPTINSTANCE_SERVER:
    return "SCRIPTINSTANCE_SERVER";
  case SCRIPTINSTANCE_CLIENT:
    return "SCRIPTINSTANCE_CLIENT";
  default:
    return "SCRIPTINSTANCE_INVALID";
  }
}
} // namespace scr
} // namespace game