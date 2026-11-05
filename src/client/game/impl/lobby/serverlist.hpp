#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace serverlist {
msg::Msg_InfoResponseLobbyServer *
CheckForDuplicate(msg::Msg_InfoResponseLobbyServer &newLobbyServerInfo,
                  LobbyServerList &serverList);
} // namespace serverlist
} // namespace lobby
} // namespace game
