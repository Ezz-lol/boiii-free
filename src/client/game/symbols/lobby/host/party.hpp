#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace host {
namespace party {
// Return value seems to represent an enumeration, but currently unsure what
// specifical enum this is in the engine:
// - 0x22: clientCount + reservationCount >= maxClients
// - 0x20: memberCount + ClientCount + reservationCount > MaxClients
// - 0x1: lobbyType != LOBBY_TYPE_GAME || !Com_SessionMode_IsPublicOnlineGame()
// || (!is_client() && MatchmakingHopper_IsParked())
WEAK symbol<uint32_t(const ControllerIndex_t controllerIndex,
                     const LobbyType lobbyType, int32_t memberCount)>
    LobbyHost_IsPartyAllowed{0x141ECC7F0, 0x141ED9200, 0x14048AAE0};

} // namespace party
} // namespace host
} // namespace lobby
} // namespace game
