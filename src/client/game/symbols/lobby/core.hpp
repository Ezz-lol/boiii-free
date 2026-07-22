#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
WEAK symbol<int32_t(LobbyType lobbyType, LobbyClientType clientType)>
    LobbyHost_GetClientCount{0x141ED8AC0, 0x14048A360};
} // namespace lobby
} // namespace game