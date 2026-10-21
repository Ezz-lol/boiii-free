#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace join {

WEAK symbol<lobby::join::Join> s_join{0x1556CB6D0, 0x15574A640, 0x14857E170};

WEAK symbol<bool(int32_t actionId, ControllerIndex_t controllerIndex,
                 LobbyType sourceLobbyType, LobbyType targetLobbyType,
                 joinCompleteCallback joinComplete)>
    LobbyJoin_Begin{0x141ED7B60, 0x0, 0x0};
WEAK symbol<bool(XUID xuid, const char *name, net::bdSecurityID *secId,
                 net::bdSecurityKey *secKey, net::SerializedAdr *serializedAdr,
                 JoinType joinType, uint64_t reservationKey)>
    LobbyJoin_Add{0x141ED7B50, 0x0, 0x0};
WEAK symbol<void()> LobbyJoin_Finalize{0x141ED7C50, 0x0, 0x0};
} // namespace join

} // namespace lobby
} // namespace game
