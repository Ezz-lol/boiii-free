#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace bb {

WEAK symbol<void()> BB_PrintUsercmdBurstHistogram{0x1422505D0};
WEAK symbol<void(ControllerIndex_t controllerIndex)> BB_CheckSend{0x14229DDD0};
WEAK symbol<bool(const char *name)> BB_CheckType{0x14229DE20};
WEAK symbol<void()> BB_Disable_f{0x14229DEC0};
WEAK symbol<void()> BB_Enable_f{0x14229DF70};
WEAK symbol<void()> BB_Init{0x14229E070};
WEAK symbol<void(bool daily, const char *recipient, const char *msg,
                 bool immediate)>
    BB_Notify{0x14229E430};
WEAK symbol<void(ControllerIndex_t controllerIndex, const char *name,
                 const char *fmt, ...)>
    BB_Print{0x14229E480};
WEAK symbol<void()> BB_ProfileLog_EndThreadFrame{0x14229E7A0};
WEAK symbol<void()> BB_ProfileLog_SetTimeAndInit{0x14229E7C0};
WEAK symbol<int32_t(const char *name)> BB_RegisterHighWaterMark{0x14229E7E0};
WEAK symbol<void(const char *name)> BB_ResetHighWaterMark{0x14229E950};
WEAK symbol<void(ControllerIndex_t controllerIndex, bool immediate)> BB_Send{
    0x14229E9C0};
WEAK symbol<void()> BB_Send_f{0x14229EB70};
WEAK symbol<void(int32_t idx, size_t value)> BB_SetHighWaterMark{0x14229EB90};
WEAK symbol<void(const char *name, bool allowed)> BB_SetTypeAllowed{
    0x14229EBC0};
WEAK symbol<bool()> BB_ShouldSampleSession{0x14229ED70};
WEAK symbol<void()> BB_InitDefinitions{0x1422A0DF0};
WEAK symbol<int32_t(const char *name, const char *fmt, int *parameterCount,
                    const uint8_t **desc)>
    BB_ParseAndCacheFormatString{0x1422A0E30};
} // namespace bb
} // namespace game

#endif