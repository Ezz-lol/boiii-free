#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace hk {
class hkVisualDebugger;

WEAK symbol<void(hkVisualDebugger *visualDebugger, int32_t listenPort)>
    hkVisualDebugger_serve{0x1429E2980, 0x0, 0x1408DE370};
} // namespace hk
} // namespace game
