#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace host {
WEAK symbol<join::JoinResult(
    ControllerIndex_t *controllerIndex, const LobbyType lobbyType,
    join::JoinPartyMember *members, int32_t memberCount, XUID lobbyHostXuid,
    XUID probedXuid)>
    LobbyHost_CanFit{0x141ECAE10, 0x0141ED7820, 0x140489000};
WEAK symbol<void(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                 XUID fromXuid, net::msg::msg_t *msg)>
    LobbyHost_HandleJoinRequest{0x141ED2600, 0x0, 0x0};
} // namespace host
} // namespace lobby
} // namespace game
