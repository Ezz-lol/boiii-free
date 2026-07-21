#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
WEAK symbol<lobby::Join> s_join{0x15574A640};
WEAK symbol<int32_t(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ED8AC0, 0x14048A360};

WEAK symbol<int(int64_t lobbySession, LobbyClientType clientType)>
    LobbySession_GetClientCount{0x141ED8B30, 0x0};

/*
  Sets:
    - Lobby UI network mode
    - LobbyVM `s_networkMode`
    - `Com_SessionMode` network mode, as set in `sessionModeState`
*/
WEAK symbol<void(LobbyNetworkMode networkMode)> LobbyBase_SetNetworkMode{
    0x141EE82F0};
WEAK symbol<LobbyNetworkMode()> LobbyBase_GetNetworkMode{0x141EE8160};
} // namespace lobby
} // namespace game