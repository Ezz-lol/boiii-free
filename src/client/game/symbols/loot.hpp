#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace loot {
WEAK symbol<LootResult> s_lastResult{0x151272C40, 0x1512F1BB0, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t crateType,
                 int32_t currency)>
    Loot_BuyCrate{0x141E759F0, 0x141E82480, 0x0};
} // namespace loot
} // namespace game
