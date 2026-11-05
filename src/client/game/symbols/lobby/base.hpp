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
    0x141EDB980, 0x141EE82F0, 0x0};
WEAK symbol<LobbyNetworkMode()> LobbyBase_GetNetworkMode{
    0x141EDB7F0, 0x141EE8160, 0x1404994A0};
WEAK symbol<LobbyMainMode()> LobbyBase_GetMainMode{0x141EDB7E0, 0x0,
                                                   0x140499490};
} // namespace base
} // namespace lobby
} // namespace game
