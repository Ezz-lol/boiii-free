#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
WEAK symbol<int32_t(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ECC0B0, 0x141ED8AC0, 0x14048A360};
WEAK symbol<bool(LobbyType lobbyType)> LobbyHost_IsHost{0x141ECC700, 0x0, 0x0};
WEAK symbol<session::LobbySession *(LobbyType lobbyType)>
    LobbyHostData_GetSession{0x141ED03E0, 0x0, 0x0};
WEAK symbol<bool(int32_t actionId, ControllerIndex_t controllerIndex,
                 LobbyType sourceLobbyType, LobbyType targetLobbyType,
                 joinCompleteCallback joinComplete)>
    LobbyJoin_Begin{0x141ED7B60, 0x0, 0x0};
WEAK symbol<bool(XUID xuid, const char *name, net::bdSecurityID *secId,
                 net::bdSecurityKey *secKey, net::SerializedAdr *serializedAdr,
                 JoinType joinType, uint64_t reservationKey)>
    LobbyJoin_Add{0x141ED7B50, 0x0, 0x0};
WEAK symbol<void()> LobbyJoin_Finalize{0x141ED7C50, 0x0, 0x0};
} // namespace lobby
} // namespace game
