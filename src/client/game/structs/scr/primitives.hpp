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
} // namespace scr
} // namespace game