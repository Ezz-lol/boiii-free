#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> redactionDisplayTime{
    0x1447854C8, 0x1448044C8, 0x14201F060};
WEAK symbol<EngineDependentDvarMut> redactionFadeDuration{
    0x1447854D0, 0x1448044D0, 0x14201F068};
WEAK symbol<EngineDependentDvarMut> redactionStartStrokeTime{
    0x1447854D8, 0x1448044D8, 0x14201F070};
WEAK symbol<EngineDependentDvarMut> redactionStrokeTime{
    0x1447854E0, 0x1448044E0, 0x14201F078};
} // namespace game
