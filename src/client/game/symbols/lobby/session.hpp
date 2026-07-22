#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace session {
WEAK symbol<int(LobbySession *lobbySession, LobbyClientType clientType)>
    LobbySession_GetClientCount{0x141ED8B30, 0x0};

WEAK symbol<lobby::Join> s_join{0x15574A640};
WEAK symbol<SessionClient *(
    ControllerIndex_t controllerIndex, LobbySession *lobbySession, XUID xuid,
    const char *gamertag, dw::net::bdCommonAddrRef commonAdr, LobbyID lobbyID,
    XUID probedXuid, int32_t qport, net::netsrc_t localNetID,
    ClientNum_t atPosition, uint8_t latencyBand, ContentFlags dlcBits,
    uint32_t joinOrder, LobbyClientType clientType, const char *reason)>
    LobbySession_AddClientAtPosition{0x141EFFAB0};
} // namespace session
} // namespace lobby
} // namespace game