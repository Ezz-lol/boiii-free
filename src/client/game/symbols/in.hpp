#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace in {
WEAK symbol<bool()> IN_CanTalk{0x1422BB470, 0x0};
WEAK symbol<bool()> IN_IsTalkKeyHeld{0x14131B480, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex, int32_t device, bool force)>
    IN_SetActiveDevice{0x1420E3410, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex)> GamePad_Enabled{
    0x142285700, 0x0};
WEAK symbol<gamepadType_e(ControllerIndex_t controllerIndex)>
    GPad_GetGamepadType{0x142285470, 0x0};
WEAK symbol<bool(LocalClientNum_t localClientNum)> GPad_IsDurangoGamepad{
    0x1422863F0, 0x0};
} // namespace in
} // namespace game
