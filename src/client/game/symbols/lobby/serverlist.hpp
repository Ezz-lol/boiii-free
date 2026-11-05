#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {

namespace serverlist {
WEAK symbol<LobbyServerList> s_lobbyServerList{0x1560ECF00, 0x0, 0x148F9F8B0};
WEAK symbol<LobbyMainMode> s_lobbyMainModeFilter{0x143390960, 0x0, 0x141122830};
} // namespace serverlist
} // namespace lobby
} // namespace game
