#pragma once

#include <cstdint>

#include "game/structs/macros.hpp"
namespace game {
namespace scr {
typedef uint32_t ScrString_t;
typedef uint32_t ScrVarCanonicalName_t;
typedef ScrString_t ScriptString;

enum class scriptInstance_t : uint32_t {
  SERVER = 0x0,
  CLIENT = 0x1,
  COUNT = 0x2,
};
IMPL_ENUM_OPERATORS(scriptInstance_t);

alwaysinline constexpr bool validate(const scriptInstance_t inst) {
  return inst >= scriptInstance_t::SERVER && inst < scriptInstance_t::COUNT;
}

inline constexpr const char *serialize(scriptInstance_t inst) {
  switch (inst) {
  case scriptInstance_t::SERVER:
    return "scriptInstance_t::SERVER";
  case scriptInstance_t::CLIENT:
    return "scriptInstance_t::CLIENT";
  default:
    return "scriptInstance_t::INVALID";
  }
}

typedef intptr_t scr_funcptr_t;
} // namespace scr
} // namespace game
