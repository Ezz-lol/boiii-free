#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace debug {
WEAK symbol<DebugHistoryPool<15>> s_history{0x1556CEBA0, 0x15574DB10,
                                            0x148581640};

WEAK symbol<void(DebugSystem system, const char *str)> AddRow{
    0x141EDF470, 0x141EEBDE0, 0x14049D020};
WEAK symbol<void(const char *stateTo, const char *reason)>
    LobbyDebug_JoinStateChanged{0x141EE20F0, 0x0, 0x14049FC50};
WEAK symbol<void(int32_t count)> LobbyDebug_AgreementCount{0x141EE1520, 0x0,
                                                           0x14049F0A0};
WEAK symbol<void(int32_t count)> LobbyDebug_ReservationCount{0x141EE2290, 0x0,
                                                             0x14049FDF0};
WEAK symbol<void(XUID xuid, uint64_t key)> LobbyDebug_ReservationProcessed{
    0x141EE22C0, 0x0, 0x14049FE20};
} // namespace debug
} // namespace lobby
} // namespace game
