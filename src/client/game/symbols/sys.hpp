#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace sys {
WEAK symbol<int()> Sys_Milliseconds{0x142332870, 0x1405972F0};
WEAK symbol<void()> Sys_ShowConsole{0x1423333C0, 0x140597E40};
WEAK symbol<TLSData *()> Sys_GetTLS{0x1421837B0, 0x140525EB0};
WEAK symbol<TLSData *()> Sys_IsDatabaseReady{0x142183A60};
WEAK symbol<void(CriticalSection)> Sys_EnterCriticalSection{0x140119190,
                                                            0x140055230};
WEAK symbol<void(CriticalSection)> Sys_LeaveCriticalSection{0x1401191E0,
                                                            0x140055280};
} // namespace sys
} // namespace game

#endif