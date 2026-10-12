#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
WEAK symbol<int32_t(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ECC0B0, 0x141ED8AC0, 0x14048A360};
WEAK symbol<bool(LobbyType lobbyType)> LobbyHost_IsHost{0x141ECC700, 0x0,
                                                        0x14048AA10};
WEAK symbol<void()> LobbyHostLaunch_LaunchGame_f{0x0, 0x0, 0x14048F6E0};
WEAK symbol<session::LobbySession *(LobbyType lobbyType)>
    LobbyHostData_GetSession{0x141ED03E0, 0x0, 0x14048E700};

} // namespace lobby
} // namespace game
