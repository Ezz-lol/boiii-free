#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace msg {

constexpr auto MESSAGE_BUF_LEN = 0x20000;
typedef array<uint8_t, MESSAGE_BUF_LEN> messageBuf_t;

template <Packet P>
bool LobbyMsg_SendToAdr(const ControllerIndex_t controllerIndex,
                        net::NetChanMsgType_e channel, LobbyModule destModule,
                        net::netadr_t &targetAdr, XUID targetXUID, P &msgData) {
  LobbyMsg lobbyMsg{};
  messageBuf_t buf = {0};
  return lobbyMsg.write(msgData, buf) &&
         LobbyMsgTransport_SendToAdr(controllerIndex, channel, destModule,
                                     &targetAdr, targetXUID, &lobbyMsg.msg,
                                     lobbyMsg.msgType);
}

template <Packet P>
bool LobbyMsg_SendToClient(const ControllerIndex_t controllerIndex,
                           session::LobbySession *const lobbySession,
                           P &msgData, net::NetChanMsgType_e channel) {

  LobbyMsg lobbyMsg{};
  messageBuf_t buf = {0};
  return lobbyMsg.write(msgData, buf) &&
         LobbyMsgTransport_SendTo(controllerIndex, lobbySession,
                                  LobbyModule::CLIENT, &lobbyMsg.msg,
                                  lobbyMsg.msgType, channel) != 0;
}
} // namespace msg
} // namespace lobby
} // namespace game
