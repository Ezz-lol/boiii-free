#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace bb {

WEAK symbol<void()> BB_PrintUsercmdBurstHistogram{0x1421F3AA0, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex)> BB_CheckSend{0x1422412B0,
                                                                  0x0};
WEAK symbol<bool(const char *name)> BB_CheckType{0x142241300, 0x0};
WEAK symbol<void()> BB_Disable_f{0x1422413A0, 0x0};
WEAK symbol<void()> BB_Enable_f{0x142241450, 0x0};
WEAK symbol<void()> BB_Init{0x142241550, 0x0};
WEAK symbol<void(bool daily, const char *recipient, const char *msg,
                 bool immediate)>
    BB_Notify{0x142241910, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex, const char *name,
                 const char *fmt, ...)>
    BB_Print{0x142241960, 0x0};
WEAK symbol<void()> BB_ProfileLog_EndThreadFrame{0x142241C80, 0x0};
WEAK symbol<void()> BB_ProfileLog_SetTimeAndInit{0x142241CA0, 0x0};
WEAK symbol<int32_t(const char *name)> BB_RegisterHighWaterMark{0x142241CC0,
                                                                0x0};
WEAK symbol<void(const char *name)> BB_ResetHighWaterMark{0x142241E30, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex, bool immediate)> BB_Send{
    0x142241EA0, 0x0};
WEAK symbol<void()> BB_Send_f{0x142242050, 0x0};
WEAK symbol<void(int32_t idx, size_t value)> BB_SetHighWaterMark{0x142242070,
                                                                 0x0};
WEAK symbol<void(const char *name, bool allowed)> BB_SetTypeAllowed{0x1422420A0,
                                                                    0x0};
WEAK symbol<bool()> BB_ShouldSampleSession{0x142242250, 0x0};
WEAK symbol<void()> BB_InitDefinitions{0x1422442D0, 0x0};
WEAK symbol<int32_t(const char *name, const char *fmt, int *parameterCount,
                    const uint8_t **desc)>
    BB_ParseAndCacheFormatString{0x142244310, 0x0};
} // namespace bb
} // namespace game
