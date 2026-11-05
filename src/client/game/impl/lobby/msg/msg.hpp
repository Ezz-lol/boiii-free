#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace msg {

template <Packet P>
bool LobbyMsg_SendToAdr(const ControllerIndex_t controllerIndex,
                        net::NetChanMsgType_e channel, LobbyModule destModule,
                        net::netadr_t &targetAdr, XUID targetXUID, P &msgData) {
  constexpr auto MESSAGE_BUF_LEN = 0x20000;

  LobbyMsg lobbyMsg{};
  uint8_t buf[MESSAGE_BUF_LEN] = {0};
  return lobbyMsg.write(msgData, buf) &&
         LobbyMsgTransport_SendToAdr(controllerIndex, channel, destModule,
                                     &targetAdr, targetXUID, &lobbyMsg.msg,
                                     lobbyMsg.msgType);
}
} // namespace msg
} // namespace lobby
} // namespace game
