#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace debug {
WEAK symbol<DebugHistoryPool_cl> s_history_cl{0x1556CEBA0, 0x1556CEBA0, 0x0};
WEAK symbol<DebugHistoryPool_sv> s_history_sv{0x0, 0x0, 0x148581640};

inline EngineDependentDebugHistoryPool s_history() {
  EngineDependentDebugHistoryPool result{};
  if (is_server()) {
    result.sv = s_history_sv.get();
  } else {
    result.cl = s_history_cl.get();
  }

  return result;
}
WEAK symbol<void(DebugSystem system, const char *str)> AddRow{
    0x141EDF470, 0x141EEBDE0, 0x14049D020};
} // namespace debug
} // namespace lobby
} // namespace game
