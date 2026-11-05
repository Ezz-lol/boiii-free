#include <std_include.hpp>

#include "serverlist.hpp"

namespace game {
namespace lobby {
namespace serverlist {
msg::Msg_InfoResponseLobbyServer *
CheckForDuplicate(msg::Msg_InfoResponseLobbyServer &newLobbyServerInfo,
                  LobbyServerList &serverList) {
  for (msg::Msg_InfoResponseLobbyServer &serverInfo :
       serverList.getServerInfo()) {
    if ((serverInfo.lobby.host.hostXuid != 0 &&
         newLobbyServerInfo.lobby.host.hostXuid != 0 &&
         serverInfo.lobby.host.hostXuid ==
             newLobbyServerInfo.lobby.host.hostXuid) ||
        (serverInfo.lobby.client.hostXuid != 0 &&
         newLobbyServerInfo.lobby.client.hostXuid != 0 &&
         serverInfo.lobby.client.hostXuid ==
             newLobbyServerInfo.lobby.client.hostXuid)) {
      return &serverInfo;
    }
  }
  return nullptr;
}

} // namespace serverlist
} // namespace lobby
} // namespace game
