#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace session {
WEAK symbol<int32_t(LobbySession *lobbySession, LobbyClientType clientType)>
    LobbySession_GetClientCount{0x141ED8B30, 0x0, 0x1404AD6F0};

WEAK symbol<SessionClient *(
    ControllerIndex_t controllerIndex, LobbySession *lobbySession, XUID xuid,
    const char *gamertag, bool isGuest, dw::net::bdCommonAddrRef commonAdr,
    LobbyID lobbyID, XUID probedXuid, int32_t qport, net::netsrc_t localNetID,
    ClientNum_t atPosition, uint8_t latencyBand, ContentFlags dlcBits,
    uint32_t joinOrder, const char *reason)>
    LobbySession_AddClientAtPosition{0x141EF3330, 0x141EFFAB0, 0x1404AD0D0};
WEAK symbol<net::HostInfo *(const LobbyType lobbyType)>
    LobbySession_GetHostInfo{0x141EBE820, 0x0, 0x14048A460};
WEAK symbol<ClientNum_t(LobbySession *lobbySession, XUID xuid)>
    LobbySession_FindClientIndexByXuid{0x141EF3820, 0x0, 0x1404AD5A0};
WEAK symbol<SessionClient *(LobbySession *lobbySession, XUID xuid)>
    LobbySession_GetClientByXuid{0x141EF3920, 0x0, 0x1404AD6A0};
WEAK symbol<void(LobbySession *lobbySession, int32_t dirtyBits)>
    LobbySession_DirtySet{0x141EF3660, 0x0, 0x1404AD400};
WEAK symbol<bool(LobbySession *lobbySession, XUID leader)>
    LobbySession_SetLeader{0x141EF53C0, 0x0, 0x1404AF0F0};
WEAK symbol<bool(LobbySession *lobbySession, XUID platformSessionID)>
    LobbySession_SetPlatformSessionID{0x141EF54B0, 0x0, 0x1404AF1E0};
WEAK symbol<bool(LobbySession *lobbySession, XUID xuid,
                 const MutableClientInfo *mutableClientInfo,
                 bool *mutableClientInfoChanged)>
    LobbySession_UpdateClient{0x141EF5590, 0x0, 0x1404AF2C0};
WEAK symbol<void(int32_t dirtyBits)> LobbySession_SetAllLobbySessionsDirty{
    0x141ECD350, 0x0, 0x1404AF2C0};
WEAK symbol<bool(LobbySession *lobbySession, XUID xuid, const char *reason)>
    LobbySession_RemoveClient{0x141EF5280, 0x0, 0x1404AEFC0};
} // namespace session
} // namespace lobby
} // namespace game
