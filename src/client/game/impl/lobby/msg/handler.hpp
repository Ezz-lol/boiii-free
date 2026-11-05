#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace msg {
LobbyMsg_MsgHandleCallback HandleAgreementResponse;
LobbyMsg_MsgHandleCallback LobbyPeerToPeer_HandleConnectivityTest;
LobbyMsg_MsgHandleCallback HandleDemoState;
LobbyMsg_MsgHandleCallback HandleMemberInfo;
LobbyMsg_MsgHandleCallback HandleServerInfo;
LobbyMsg_MsgHandleCallback HandleJoinRequest;
// Named `HandleVoicePacket` in engine
LobbyMsg_MsgHandleCallback HandleP2PVoicePacket;
// Named `HandleVoicePacket` in engine
LobbyMsg_MsgHandleCallback HandleHostClientVoicePacket;
LobbyMsg_MsgHandleCallback HandleJoinResponse;
LobbyMsg_MsgHandleCallback HandleMigrateStart;
LobbyMsg_MsgHandleCallback HandleClientContent;
LobbyMsg_MsgHandleCallback HandleModifiedStats;
LobbyMsg_MsgHandleCallback HandleClientDisconnect;
LobbyMsg_MsgHandleCallback HandlePeerToPeerUpdate;
LobbyMsg_MsgHandleCallback HandleClientReliableData;
LobbyMsg_MsgHandleCallback HandleLobbyClientHeartbeat;
LobbyMsg_MsgHandleCallback HandleMigrateBandwidthTest;

} // namespace msg
} // namespace lobby
} // namespace game
