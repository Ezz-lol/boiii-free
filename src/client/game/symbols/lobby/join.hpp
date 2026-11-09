#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace join {

WEAK symbol<lobby::join::Join> s_join{0x1556CB6D0, 0x15574A640, 0x14857E170};
WEAK symbol<LocalClientPool<JoinClient>> s_joinClient{0x1556CB4B0, 0x0,
                                                      0x14857E000};
WEAK symbol<int32_t> s_successTimeout{0x1556CB6CC, 0x0, 0x14857E16C};
WEAK symbol<JoinSourceStatePool<const char *>> s_joinNames{0x143390390, 0x0,
                                                           0x141122260};

WEAK symbol<bool(int32_t actionId, ControllerIndex_t controllerIndex,
                 LobbyType sourceLobbyType, LobbyType targetLobbyType,
                 joinCompleteCallback joinComplete)>
    LobbyJoin_Begin{0x141ED7B60, 0x0, 0x0};
WEAK symbol<bool(net::HostInfo *host)> LobbyJoin_ConnectToHost{0x141ED7B70, 0x0,
                                                               0x140495B30};
WEAK symbol<bool(XUID xuid, const char *name, net::bdSecurityID *secId,
                 net::bdSecurityKey *secKey, net::SerializedAdr *serializedAdr,
                 JoinType joinType, uint64_t reservationKey)>
    LobbyJoin_Add{0x141ED7B50, 0x0, 0x0};
WEAK symbol<void()> LobbyJoin_Finalize{0x141ED7C50, 0x0, 0x0};
WEAK symbol<void(Agreement *agreement, XUID xuid, bool agrees)>
    Agreement_Update{0x141ED8CF0, 0x0, 0x140496B40};
WEAK symbol<bool(Agreement *agreement, LobbyType sourceLobbyType)>
    Agreement_Add{0x141ED8C10, 0x0, 0x140496A60};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 LobbyType srcLobbyType, bool shouldCommit)>
    LobbyHostMsg_SendJoinComplete{0x141ED4DA0, 0x0, 0x140492FB0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 net::HostInfo *hostInfo, uint64_t reservationKey, int nonce,
                 LobbyType srcLobbyType, LobbyType targetLobbyType,
                 LobbyParams params)>
    LobbyHostMsg_SendAgreementRequest{0x141ED4BC0, 0x0, 0x140492DD0};

WEAK symbol<void(join::Join *join)> LobbyTaskUpdate{0x141ED9F50, 0x0,
                                                    0x140497DA0};
/*
   Named `ChangeState` in engine.

   No clue how the two modules' functions are differentiated in the engine's
   source code - maybe they just exist in different source code files?
   They have the same signature, so these are not overrides.
*/
WEAK symbol<void(LobbyType lobbyType)> Host_ChangeState{0x141ED0800, 0x0,
                                                        0x14048EB20};
// Named `ChangeState` in engine.
WEAK symbol<void(LobbyType lobbyType)> Client_ChangeState{0x141EC1240, 0x0,
                                                          0x140481E20};

alwaysinline void ChangeState(LobbyModule module, LobbyType lobbyType) {
  switch (module) {
  case LobbyModule::CLIENT:
    Client_ChangeState(lobbyType);
    break;
  case LobbyModule::HOST:
    Host_ChangeState(lobbyType);
    break;
  default:
    break;
  }
}

WEAK symbol<void(ControllerIndex_t controllerIndex, XUID fromXuid,
                 msg::Msg_JoinParty *joinReq,
                 msg::Msg_JoinResponse *joinResponse)>
    JoinRequestLuaCheck{0x141ED4340, 0x0, 0x140492550};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 JoinPartyMember *members, int memberCount,
                 uint64_t clientJoinNonce, XUID probedXuid, uint64_t *outKey)>
    LobbyJoin_Reserve{0x141ED7F90, 0x0, 0x140495EA0};
WEAK symbol<void(net::HostInfo *host)> LobbyJoin_DisconnectFromHost{
    0x141ED7BF0, 0x0, 0x140495BB0};
WEAK symbol<bool(JoinClient *joinClient)> SendDisagree{0x141ED8A90, 0x0,
                                                       0x1404968E0};
} // namespace join
} // namespace lobby
} // namespace game
