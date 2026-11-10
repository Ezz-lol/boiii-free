#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace host {
WEAK symbol<join::JoinResult(
    ControllerIndex_t controllerIndex, const LobbyType lobbyType,
    join::JoinPartyMember *members, int32_t memberCount, XUID lobbyHostXuid,
    XUID probedXuid)>
    LobbyHost_CanFit{0x141ECAE10, 0x140489000};
WEAK symbol<void(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                 XUID fromXuid, net::msg::msg_t *msg)>
    LobbyHost_HandleJoinRequest{0x141ED2600, 0x0};
WEAK symbol<void(const ControllerIndex_t controllerIndex,
                 net::netadr_t *fromAdr, uint64_t joinNonce,
                 msg::Msg_JoinMemberInfo *msgData, XUID probedXuid)>
    LobbyHost_AddJoiningClient{0x141ECA840, 0x140488A40};
WEAK symbol<bool(const LobbyType lobbyType, bool isLocalJoining,
                 join::JoinResult *errorResult)>
    LobbyHost_IsJoinable{0x141ECC740, 0x14048AA30};
WEAK symbol<bool(ControllerIndex_t controllerIndex, const LobbyType lobbyType,
                 const join::JoinType joinType, XUID fromXUID,
                 msg::Msg_JoinResponse *joinResponse)>
    LobbyHost_PartyPrivacyCheck{0x141ECCB00, 0x14048ADF0};
WEAK symbol<session::SessionClient *(LobbyType lobbyType)>
    LobbyHostData_GetSessionLobbyList{0x141ED0400, 0x14048E720};
WEAK symbol<bool(const LobbyType lobbyType, XUID xuid, const char *reason)>
    LobbyHost_RemoveClient{0x141ECD250, 0x14048B540};
WEAK symbol<bool(LobbyType lobbyType)> LobbyHost_IsHost{0x141ECC700, 0x14048AA10};
WEAK symbol<bool(const ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID disconnectClientXuid,
                 LobbyDisconnectClient disconnectClient)>
    LobbyHostMsg_SendDisconnectClient{0x141ED4CE0, 0x140492EF0};

} // namespace host
} // namespace lobby
} // namespace game
