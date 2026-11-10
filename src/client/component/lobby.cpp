#include <std_include.hpp>

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include "network.hpp"
#include "party.hpp"

#include <utils/hook.hpp>

namespace lobby {
using namespace game;
using namespace game::lobby;
using namespace game::lobby::msg;
namespace {
#ifndef NDEBUG
template <const LobbyMsg_MsgHandleCallbackPtr &Handler, ConstString Table,
          ConstString Name>
void LobbyMsgHandler_TraceExec(ControllerIndex_t controllerIndex,
                               net::netadr_t *adr, XUID xuid, LobbyMsg *msg) {
  game::trace("[Lobby][Msg][{}][{}][{}] Received message from {:016X}@{}",
              Table.c_str(), Name.c_str(), serialize(controllerIndex), xuid,
              adr->serialize());
  return Handler(controllerIndex, adr, xuid, msg);
}

#ifndef TRACE_MSG_HANDLER
#define TRACE_MSG_HANDLER(table, name)                                         \
  static const LobbyMsg_MsgHandleCallbackPtr table##_##name##_orig =           \
      table->name.function;                                                    \
  static LobbyMsg_MsgHandleCallback &table##_##name##_new =                    \
      LobbyMsgHandler_TraceExec<table##_##name##_orig, #table, #name>;         \
  table->name.function = &table##_##name##_new;
#endif

inline void trace_host_client_msg_handlers() {
  TRACE_MSG_HANDLER(host_clientMessageHandlers, JoinRequest);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, JoinMemberInfo);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, JoinAgreementResponse);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, JoinResponse);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, ServerListInfo);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, PeerToPeerInfo);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, LobbyClientHeartbeat);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, LobbyClientDisconnect);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, LobbyClientReliableData);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, LobbyClientContent);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, LobbyModifiedStats);
  TRACE_MSG_HANDLER(host_clientMessageHandlers, VoicePacket);
}

inline void trace_client_host_msg_handlers() {
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyStatePrivate);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyStateGame);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyHostHeartbeat);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyHostDisconnect);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyHostDisconnectClient);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyHostLeaveWithParty);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, JoinAgreementRequest);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, JoinComplete);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyMigrateAnnounceHost);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, IngameMigrateTo);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, IngameMigrateNewHost);
  TRACE_MSG_HANDLER(client_hostMessageHandlers, LobbyClientContent);
}

inline void trace_p2p_msg_handlers() {
  TRACE_MSG_HANDLER(p2p_messageHandlers, ConnectivityTest);
  TRACE_MSG_HANDLER(p2p_messageHandlers, MigrateBandwidthTest);
  TRACE_MSG_HANDLER(p2p_messageHandlers, MigrateStart);
  TRACE_MSG_HANDLER(p2p_messageHandlers, VoicePacket);
  TRACE_MSG_HANDLER(p2p_messageHandlers, DemoState);
}
inline void trace_msg_handlers() {
  trace_host_client_msg_handlers();
  trace_client_host_msg_handlers();
  trace_p2p_msg_handlers();
}
#endif

template <const auto Val> decltype(Val) return_const() { return Val; }

utils::hook::detour build_route_list_hook;
void build_route_list_stub(ControllerIndex_t controllerIndex,
                           session::LobbySession *lobbySession,
                           session::SessionClient *outClient, XUID ourXuid,
                           int32_t *routeList) {
  if (party::cl_connected_to_dedi.get_bool()) {
    std::fill_n(routeList, MAX_PLAYERS, 0);
    return;
  }
  build_route_list_hook.invoke<void>(controllerIndex, lobbySession, outClient,
                                     ourXuid, routeList);
}

utils::hook::detour build_host_route_list_hook;
void build_host_route_list_stub(ControllerIndex_t controllerIndex,
                                session::LobbySession *lobbySession,
                                session::SessionClient *outClient, XUID ourXuid,
                                int32_t *routeList) {
  if (!party::cl_connected_to_dedi.get_bool() ||
      lobbySession->type != LobbyType::GAME) {
    build_host_route_list_hook.invoke<void>(controllerIndex, lobbySession,
                                            outClient, ourXuid, routeList);
    return;
  }
  for (int32_t i = 0; i < MAX_PLAYERS; ++i) {
    const session::SessionClient &client = lobbySession->client[i];
    if (client.activeClient && client.xuid != ourXuid) {
      *routeList |= 1 << i;
    }
  }
}

utils::hook::detour server_status_not_running_hook;
void server_status_not_running_stub(session::LobbyGameData *gameData) {
  if (!party::cl_connected_to_dedi.get_bool()) {
    server_status_not_running_hook.invoke<void>(gameData);
  }
}

utils::hook::detour connect_from_lobby_hook;
void connect_from_lobby_stub(ControllerIndex_t controllerIndex,
                             net::XSESSION_INFO *hostInfo,
                             const net::netadr_t *addr, int numPublicSlots,
                             int numPrivateSlots, const char *mapname,
                             const char *gametype, const char *usermap) {
  if (!party::is_direct_connect() && party::is_dedi_server(*addr)) {
    return;
  }
  connect_from_lobby_hook.invoke<void>(controllerIndex, hostInfo, addr,
                                       numPublicSlots, numPrivateSlots, mapname,
                                       gametype, usermap);
}
utils::hook::detour add_client_at_position_hook;
session::SessionClient *add_client_at_position_stub(
    ControllerIndex_t controllerIndex, session::LobbySession *lobbySession,
    XUID xuid, const char *gamertag, bool isGuest,
    dw::net::bdCommonAddrRef commonAdr, LobbyID lobbyID, XUID probedXuid,
    int32_t qport, net::netsrc_t localNetID, ClientNum_t atPosition,
    uint8_t latencyBand, ContentFlags dlcBits, uint32_t joinOrder,
    const char *reason) {
  if (atPosition == INVALID_CLIENT_INDEX &&
      lobbySession == LobbyHostData_GetSession(LobbyType::GAME)) {
    foreach_connected_client([&](sv::client_s &client, ClientNum_t num) {
      if (client.get_xuid() == xuid && num < MAX_PLAYERS &&
          !lobbySession->client[num].xuid) {
        atPosition = num;
      }
    });
  }
  return add_client_at_position_hook.invoke<session::SessionClient *>(
      controllerIndex, lobbySession, xuid, gamertag, isGuest, commonAdr,
      lobbyID, probedXuid, qport, localNetID, atPosition, latencyBand, dlcBits,
      joinOrder, reason);
}
} // namespace
class component final : public generic_component {
  DEFINE_COMPONENT_NAME("lobby");

public:
  void post_unpack() override {
#ifndef NDEBUG
    trace_msg_handlers();
#endif

    // Always return false for `Com_SessionMode_IsPublicOnlineGame` check in
    // `LobbyHost_IsPartyAllowed`, ensuring that players can always hot-join a
    // lobby and be allowed a slot during runtime
    utils::hook::call(host::party::LobbyHost_IsPartyAllowed.offset(0x9a),
                      return_const<false>);

    // Always return false for `Com_SessionMode_IsPublicOnlineGame` check in
    // `LobbyHost_CanFit`, ensuring that players can always hot-join a
    // lobby and be allowed a slot during runtime
    utils::hook::call(
        host::LobbyHost_CanFit.offset(game::select_value(0x6e, 0x69)),
        return_const<false>);

    if (game::is_server()) {
      add_client_at_position_hook.create(
          session::LobbySession_AddClientAtPosition.get(),
          add_client_at_position_stub);
    }

    if (game::is_client()) {
      for (const size_t offset : {0xa4, 0x26d}) {
        utils::hook::call(host::LobbyHost_HandleJoinRequest.offset(offset),
                          return_const<LobbyNetworkMode::LIVE>);
      }
      build_route_list_hook.create(
          game::lobby::voice::LobbyVoice_BuildRouteList.get(),
          build_route_list_stub);
      build_host_route_list_hook.create(
          game::lobby::voice::LobbyVoice_BuildHostRouteList.get(),
          build_host_route_list_stub);
      server_status_not_running_hook.create(
          client::ServerStatusNotRunning.get(), server_status_not_running_stub);
      connect_from_lobby_hook.create(cl::CL_ConnectFromLobby.get(),
                                     connect_from_lobby_stub);
      utils::hook::set<uint32_t>(
          game::lobby::voice::TransmitQueuedVoiceData.offset(0x58), 40);
      utils::hook::set<uint8_t>(
          game::lobby::voice::TransmitQueuedVoiceData.offset(0x64), 2);
      utils::hook::set<uint32_t>(
          game::lobby::voice::LobbyVoice_Update.offset(0x101), 40);
      for (const size_t offset : {0xbc, 0x10d}) {
        utils::hook::set<uint8_t>(
            game::lobby::voice::LobbyVoice_Update.offset(offset), 2);
      }
    }
  }
};
} // namespace lobby

REGISTER_COMPONENT(lobby::component)
