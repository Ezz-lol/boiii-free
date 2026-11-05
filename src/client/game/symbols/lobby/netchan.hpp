#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace netchan {
WEAK symbol<net::NetChanMsgType_e(LobbyGlobalChannel lobbyGlobalChannel)>
    LobbyNetChan_GetGlobalChannel{0x141EEC770, 0x0, 0x1404A6A90};
}
} // namespace lobby
} // namespace game
