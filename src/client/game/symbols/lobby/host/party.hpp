#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace host {
namespace party {
/*
  Returns:
    - JoinResult::LOBBY_FULL if: clientCount + reservationCount >= maxClients
    - JoinResult::OVER_MAX_PARTY_LIMIT if: memberCount + ClientCount +
      reservationCount > MaxClients
    - JoinResult::SUCCESS if: `lobbyType != LOBBY_TYPE_GAME ||
      !Com_SessionMode_IsPublicOnlineGame()
      || (!is_client() && MatchmakingHopper_IsParked())`
*/
WEAK symbol<join::JoinResult(const ControllerIndex_t controllerIndex,
                             const LobbyType lobbyType, int32_t memberCount)>
    LobbyHost_IsPartyAllowed{0x141ECC7F0, 0x141ED9200, 0x14048AAE0};

} // namespace party
} // namespace host
} // namespace lobby
} // namespace game
