#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace voice {
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_UnMuteClient{0x0141EE9040, 0x141EF59B0, 0x1404A37A0};

inline void UnMuteAllClients() {
  for (LobbyType lobbyType = LobbyType::FIRST; lobbyType < LobbyType::LAST;
       ++lobbyType) {
    for (ControllerIndex_t controllerIndex = CONTROLLER_INDEX_FIRST;
         controllerIndex < CONTROLLER_INDEX_COUNT; ++controllerIndex) {
      for (ClientNum_t clientNum = CLIENT_INDEX_FIRST;
           clientNum < CLIENT_INDEX_COUNT; ++clientNum) {
        LobbyVoice_UnMuteClient(controllerIndex, lobbyType, clientNum);
      }
    }
  }
}
} // namespace voice
} // namespace lobby
} // namespace game
