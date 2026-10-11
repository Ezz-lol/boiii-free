#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace host {
WEAK
    symbol<JoinResult(ControllerIndex_t *controllerIndex,
                      const LobbyType lobbyType, join::JoinPartyMember *members,
                      int32_t memberCount, XUID lobbyHostXuid, XUID probedXuid)>
        LobbyHost_CanFit{0x141ECAE10, 0x0141ED7820, 0x140489000};
} // namespace host
} // namespace lobby
} // namespace game
