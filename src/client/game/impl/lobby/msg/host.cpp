#include <std_include.hpp>

#include "host.hpp"
#include "msg.hpp"

namespace game {
namespace lobby {
namespace msg {
namespace host {

bool LobbyHostMsg_SendAgreementRequest_Impl(
    const ControllerIndex_t controllerIndex, net::HostInfo *hostInfo,
    uint64_t reservationKey, int nonce, LobbyType srcLobbyType,
    LobbyType targetLobbyType, LobbyParams params) {

  Msg_JoinAgreementRequest msgData{};
  msgData.hostXuid = hostInfo->xuid;
  strscpy(msgData.hostName, hostInfo->name);
  msgData.secKey = hostInfo->secKey;
  msgData.secId = hostInfo->secId;
  msgData.serializedAdr.addrBuff = hostInfo->serializedAdr.addrBuff;
  msgData.reservationKey = reservationKey;
  msgData.agreementNonce = nonce;
  msgData.sourceLobbyType = srcLobbyType;
  msgData.destinationLobbyType = targetLobbyType;
  msgData.destinationLobbyParams = params;
  msgData.serverLoc = static_cast<int32_t>(hostInfo->serverLocation);
  const net::NetChanMsgType_e msgType =
      lobby::netchan::LobbyNetChan_GetGlobalChannel(LobbyGlobalChannel::JOIN);
  session::LobbySession *const session = LobbyHostData_GetSession(srcLobbyType);
  return LobbyMsg_SendToClient<Msg_JoinAgreementRequest>(
      controllerIndex, session, msgData, msgType);
}
} // namespace host
} // namespace msg
} // namespace lobby
} // namespace game
