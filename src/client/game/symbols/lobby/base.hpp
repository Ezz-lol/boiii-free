#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace base {
/*
  Sets:
    - Lobby UI network mode
    - LobbyVM `s_networkMode`
    - `Com_SessionMode` network mode, as set in `sessionModeState`
*/
WEAK symbol<void(LobbyNetworkMode networkMode)> LobbyBase_SetNetworkMode{
    0x141EE82F0};
WEAK symbol<LobbyNetworkMode()> LobbyBase_GetNetworkMode{0x141EE8160};
} // namespace base
} // namespace lobby
} // namespace game