#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace client {
WEAK symbol<void(session::LobbyGameData *gameData)> ServerStatusNotRunning{
    0x141EC1DB0, 0x0, 0x0};
} // namespace client
} // namespace lobby
} // namespace game
