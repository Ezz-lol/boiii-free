#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace loot {
WEAK symbol<LootResult> s_lastResult{0x151272C40, 0x1512F1BB0, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t crateType,
                 int32_t currency)>
    Loot_BuyCrate{0x141E759F0, 0x141E82480, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> Loot_RewardIsProcessing{
    0x141E74120, 0x141E80BB0, 0x0};
WEAK symbol<uint32_t(ControllerIndex_t controllerIndex, int32_t currency)>
    Loot_GetCurrency{0x141DFC680, 0x141E09110, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, uint32_t count)>
    Loot_SpendVials{0x141E76DD0, 0x141E83860, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex, int32_t currency,
                 uint32_t amount)>
    Loot_IncCurrency{0x141E761C0, 0x141E82C50, 0x0};
} // namespace loot
} // namespace game
