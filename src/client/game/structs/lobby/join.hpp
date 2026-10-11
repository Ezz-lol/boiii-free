#pragma once

#include "game/structs/lobby/core.hpp"
namespace game {
namespace lobby {
namespace join {
struct JoinPartyMember {
  XUID xuid;
  LobbyID lobbyID;
  float skillRating;
  float skillVariance;
};

} // namespace join
} // namespace lobby
} // namespace game
