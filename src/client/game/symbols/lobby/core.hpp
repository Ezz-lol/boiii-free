#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
inline constexpr clientplatform_t LOBBY_PLATFORM = clientplatform_t::PC;

WEAK symbol<int32_t(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ECC0B0, 0x14048A360};
WEAK symbol<bool(LobbyType lobbyType)> LobbyHost_IsHost{0x141ECC700, 0x14048AA10};
WEAK symbol<void()> LobbyHostLaunch_LaunchGame_f{0x0, 0x14048F6E0};
WEAK symbol<session::LobbySession *(LobbyType lobbyType)>
    LobbyHostData_GetSession{0x141ED03E0, 0x14048E700};
WEAK symbol<bool(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                 XUID fromXUID, LobbyType lobbyType, uint8_t migrationBits)>
    IsLobbyStateValid{0x141EC6E50, 0x140485630};

} // namespace lobby
} // namespace game
