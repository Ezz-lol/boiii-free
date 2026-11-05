#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace client {
WEAK symbol<void(session::LobbyGameData *gameData)> ServerStatusNotRunning{
    0x141EC1DB0, 0x0, 0x0};
WEAK symbol<session::LobbySession *(const LobbyType lobbyType)>
    LobbyClient_GetSession{0x141EBF500, 0x0, 0x140480520};
WEAK symbol<int32_t(const LobbyType lobbyType)>
    LobbyClientSession_GetConnectivityBits{0x141EBE5C0, 0x0, 0x14047F680};
WEAK symbol<void(LobbyType lobbyType, int32_t connectivityBits)>
    LobbyClientSession_SetConnectivityBits{0x141EBFB00, 0x0, 0x140480A30};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, int32_t connectivityBits)>
    SendUpdatedConnectivityToHost{0x141EED370, 0x0, 0x1404A7690};
} // namespace client
} // namespace lobby
} // namespace game
