#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace msg {
// We, the client, received a packet from host
WEAK symbol<LobbyClientHostMsgHandlersTable> client_hostMessageHandlers{
    0x142F9EDA0, 0x14301DE80, 0x140E59160};
// We, a peer, received a packet from another peer
WEAK symbol<LobbyP2PMsgHandlersTable> p2p_MessageHandlers{
    0x142FA2BA0, 0x143021C90, 0x140E5D360};
// We, the host, received a packet from client
WEAK symbol<LobbyHostClientMsgHandlersTable> host_clientMessageHandlers{
    0x142FA0100, 0x14301F1F0, 0x140E5A5C0};

inline LobbyMsgHandler *handler(LobbyModule module, MsgType msgType) {
  switch (msgType) {
  case MsgType::JOIN_LOBBY:
    return &host_clientMessageHandlers->JoinRequest;
  case MsgType::JOIN_MEMBER_INFO:
    return &host_clientMessageHandlers->JoinMemberInfo;
  case MsgType::JOIN_AGREEMENT_RESPONSE:
    return &host_clientMessageHandlers->JoinAgreementResponse;
  case MsgType::JOIN_RESPONSE:
    return &host_clientMessageHandlers->JoinResponse;
  case MsgType::SERVERLIST_INFO:
    return &host_clientMessageHandlers->ServerListInfo;
  case MsgType::PEER_TO_PEER_INFO:
    return &host_clientMessageHandlers->PeerToPeerInfo;
  case MsgType::LOBBY_CLIENT_HEARTBEAT:
    return &host_clientMessageHandlers->LobbyClientHeartbeat;
  case MsgType::LOBBY_CLIENT_DISCONNECT:
    return &host_clientMessageHandlers->LobbyClientDisconnect;
  case MsgType::LOBBY_CLIENT_RELIABLE_DATA:
    return &host_clientMessageHandlers->LobbyClientReliableData;
  case MsgType::LOBBY_CLIENT_CONTENT: {
    switch (module) {
    case LobbyModule::CLIENT:
      return &client_hostMessageHandlers->LobbyClientContent;
    case LobbyModule::HOST:
      return &host_clientMessageHandlers->LobbyClientContent;
    default:
      goto invalid;
    }
  }
  case MsgType::LOBBY_MODIFIED_STATS:
    return &host_clientMessageHandlers->LobbyModifiedStats;
  case MsgType::VOICE_PACKET: {
    switch (module) {
    case LobbyModule::HOST:
      return &host_clientMessageHandlers->VoicePacket;

    case LobbyModule::PEER_TO_PEER:
      return &p2p_MessageHandlers->VoicePacket;
    default:
      goto invalid;
    }
  }
  case MsgType::LOBBY_STATE_PRIVATE:
    return &client_hostMessageHandlers->LobbyStatePrivate;
  case MsgType::LOBBY_STATE_GAME:
    return &client_hostMessageHandlers->LobbyStateGame;
  case MsgType::LOBBY_HOST_HEARTBEAT:
    return &client_hostMessageHandlers->LobbyHostHeartbeat;
  case MsgType::LOBBY_HOST_DISCONNECT:
    return &client_hostMessageHandlers->LobbyHostDisconnect;
  case MsgType::LOBBY_HOST_DISCONNECT_CLIENT:
    return &client_hostMessageHandlers->LobbyHostDisconnectClient;
  case MsgType::LOBBY_HOST_LEAVE_WITH_PARTY:
    return &client_hostMessageHandlers->LobbyHostLeaveWithParty;
  case MsgType::JOIN_AGREEMENT_REQUEST:
    return &client_hostMessageHandlers->JoinAgreementRequest;
  case MsgType::JOIN_COMPLETE:
    return &client_hostMessageHandlers->JoinComplete;
  case MsgType::MIGRATE_ANNOUNCE_HOST:
    return &client_hostMessageHandlers->LobbyMigrateAnnounceHost;
  case MsgType::INGAME_MIGRATE_TO:
    return &client_hostMessageHandlers->IngameMigrateTo;
  case MsgType::MIGRATE_NEW_HOST:
    return &client_hostMessageHandlers->IngameMigrateNewHost;
  case MsgType::PEER_TO_PEER_CONNECTIVITY_TEST:
    return &p2p_MessageHandlers->ConnectivityTest;
  case MsgType::LOBBY_MIGRATE_TEST:
    return &p2p_MessageHandlers->MigrateBandwidthTest;
  case MsgType::MIGRATE_START:
    return &p2p_MessageHandlers->MigrateStart;
  case MsgType::DEMO_STATE:
    return &p2p_MessageHandlers->DemoState;

  invalid:
  default:
    return nullptr;
  }
}
} // namespace msg
} // namespace lobby
} // namespace game
