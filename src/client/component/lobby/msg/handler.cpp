#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "game/impl/lobby/msg/handler.hpp"

#include <utils/hook.hpp>
#include <utils/thread.hpp>

namespace lobby {
namespace msg {
using namespace game;
using namespace game::lobby;
using namespace game::lobby::msg;

inline void replace_host_clientMessageHandlers() {
  host_clientMessageHandlers->JoinRequest.function = &HandleJoinRequest;
  host_clientMessageHandlers->JoinMemberInfo.function = &HandleMemberInfo;
  host_clientMessageHandlers->JoinAgreementResponse.function =
      &HandleAgreementResponse;
  host_clientMessageHandlers->JoinResponse.function = &HandleJoinResponse;
  host_clientMessageHandlers->ServerListInfo.function = &HandleServerInfo;
  host_clientMessageHandlers->PeerToPeerInfo.function = &HandlePeerToPeerUpdate;
  host_clientMessageHandlers->LobbyClientHeartbeat.function =
      &HandleLobbyClientHeartbeat;
  host_clientMessageHandlers->LobbyClientDisconnect.function =
      &HandleClientDisconnect;
  host_clientMessageHandlers->LobbyClientReliableData.function =
      &HandleClientReliableData;
  host_clientMessageHandlers->LobbyClientContent.function =
      &Host_HandleClientContent;
  host_clientMessageHandlers->LobbyModifiedStats.function =
      &HandleModifiedStats;
  host_clientMessageHandlers->VoicePacket.function =
      &HandleHostClientVoicePacket;
}

inline void replace_p2p_messageHandlers() {
  p2p_messageHandlers->ConnectivityTest.function =
      &LobbyPeerToPeer_HandleConnectivityTest;
  p2p_messageHandlers->MigrateBandwidthTest.function =
      &HandleMigrateBandwidthTest;
  p2p_messageHandlers->MigrateStart.function = &HandleMigrateStart;
  p2p_messageHandlers->VoicePacket.function = &HandleP2PVoicePacket;
  p2p_messageHandlers->DemoState.function = &HandleDemoState;
}

inline void replace_client_hostMessageHandlers() {
  client_hostMessageHandlers->LobbyStatePrivate.function =
      &HandleLobbyStatePrivate;
  client_hostMessageHandlers->LobbyStateGame.function = &HandleLobbyStateGame;
  client_hostMessageHandlers->LobbyHostHeartbeat.function =
      &HandleLobbyHostHeartbeat;
  client_hostMessageHandlers->LobbyHostDisconnect.function =
      &HandleHostDisconnect;
  client_hostMessageHandlers->LobbyHostDisconnectClient.function =
      &HandleHostDisconnectClient;
  client_hostMessageHandlers->LobbyHostLeaveWithParty.function =
      &HandleHostLeaveWithParty;
  client_hostMessageHandlers->JoinAgreementRequest.function =
      &HandleAgreementRequest;
  client_hostMessageHandlers->JoinComplete.function = &HandleJoinComplete;
  client_hostMessageHandlers->LobbyMigrateAnnounceHost.function =
      &HandleMigrateAnnounceHost;
  client_hostMessageHandlers->IngameMigrateTo.function = &HandleIngameMigrateTo;
  client_hostMessageHandlers->IngameMigrateNewHost.function =
      &HandleIngameMigrateNewHost;
  client_hostMessageHandlers->LobbyClientContent.function =
      &Client_HandleClientContent;
}

inline void replace_message_handlers() {
  replace_host_clientMessageHandlers();
  replace_client_hostMessageHandlers();
  replace_p2p_messageHandlers();
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("lobby/msg/handler");

public:
  void post_unpack() override { replace_message_handlers(); }
};
} // namespace msg
} // namespace lobby

REGISTER_COMPONENT(lobby::msg::component)
