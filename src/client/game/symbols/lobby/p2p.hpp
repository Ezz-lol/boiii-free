#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace p2p {
WEAK symbol<bool(session::LobbySession *clientSession, ClientNum_t peer)>
    LobbyPeerToPeer_CanITalkToPeer{0x141EECE50, 0x0, 0x1404A7170};

}
} // namespace lobby
} // namespace game
