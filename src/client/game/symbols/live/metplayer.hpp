#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace metplayer {
WEAK symbol<void(const ControllerIndex_t localControllerIndex,
                 const lobby::LobbyType lobbyType)>
    LiveMetPlayer_AddRecent{0x141E0D160, 0x141E19BF0, 0x0};

} // namespace metplayer

} // namespace live
} // namespace game