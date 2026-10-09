#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace voice {
// Named `LobbyVoice_UnMuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_UnMuteClient_ByClientNum{0x0141EE9040, 0x141EF59B0, 0x1404A37A0};
// Named `LobbyVoice_MuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 ClientNum_t clientNum)>
    LobbyVoice_MuteClient_ByClientNum{0x141EE5A40, 0x141EF23B0, 0x1404A32E0};

// Named `LobbyVoice_UnMuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    LobbyVoice_UnMuteClient_ByXUID{0x141EE90E0, 0x141EF5A50, 0x1404A3860};
// Named `LobbyVoice_MuteClient` in engine - override
WEAK symbol<void(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    LobbyVoice_MuteClient_ByXUID{0x141EE5B40, 0x141EF24B0, 0x1404A33F0};

inline void LobbyVoice_UnMuteClient(ControllerIndex_t controllerIndex,
                                    LobbyType lobbyType,
                                    ClientNum_t clientNum) {
  LobbyVoice_UnMuteClient_ByClientNum(controllerIndex, lobbyType, clientNum);
}

inline void LobbyVoice_MuteClient(ControllerIndex_t controllerIndex,
                                  LobbyType lobbyType, ClientNum_t clientNum) {
  LobbyVoice_MuteClient_ByClientNum(controllerIndex, lobbyType, clientNum);
}

inline void LobbyVoice_UnMuteClient(ControllerIndex_t controllerIndex,
                                    LobbyType lobbyType, XUID xuid) {
  LobbyVoice_UnMuteClient_ByXUID(controllerIndex, lobbyType, xuid);
}

inline void LobbyVoice_MuteClient(ControllerIndex_t controllerIndex,
                                  LobbyType lobbyType, XUID xuid) {
  LobbyVoice_MuteClient_ByXUID(controllerIndex, lobbyType, xuid);
}

typedef fastcallPtr_t<void(ControllerIndex_t controllerIndex,
                           LobbyType lobbyType, ClientNum_t clientNum)>
    LobbyVoice_ClientNum_Func;

template <const LobbyVoice_ClientNum_Func Func>
inline void LobbyVoice_ForEachLobbyClientNum() {
  for (LobbyType lobbyType = LobbyType::FIRST; lobbyType < LobbyType::LAST;
       ++lobbyType) {
    for (ControllerIndex_t controllerIndex = CONTROLLER_INDEX_FIRST;
         controllerIndex < CONTROLLER_INDEX_COUNT; ++controllerIndex) {
      for (ClientNum_t clientNum = CLIENT_INDEX_FIRST;
           clientNum < CLIENT_INDEX_COUNT; ++clientNum) {
        Func(controllerIndex, lobbyType, clientNum);
      }
    }
  }
}

inline void UnMuteAllClients() {
  return LobbyVoice_ForEachLobbyClientNum<LobbyVoice_UnMuteClient>();
}

inline void MuteAllClients() {
  return LobbyVoice_ForEachLobbyClientNum<LobbyVoice_MuteClient>();
}
} // namespace voice
} // namespace lobby
} // namespace game
