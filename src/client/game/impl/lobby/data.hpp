#pragma once

#include "game/game.hpp"

#include "game/utils.hpp"
namespace game {
namespace lobby {
namespace data {
alwaysinline std::optional<std::string_view> LobbyData_GetMap() {
  return get_mapname();
}
} // namespace data
} // namespace lobby
} // namespace game
