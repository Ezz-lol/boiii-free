#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace lobby {
WEAK symbol<lobby::Join> s_join{0x15574A640};
WEAK symbol<int(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ED8AC0, 0x14048A360};

WEAK symbol<int(int64_t lobbySession, LobbyClientType clientType)>
    LobbySession_GetClientCount{0x141ED8B30, 0x0};
} // namespace lobby
} // namespace game

#endif