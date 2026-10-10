#include <std_include.hpp>

#include <game/game.hpp>
#include <loader/component_loader.hpp>

#include <utils/concurrency.hpp>
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

} // namespace

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("lobby");

public:
  void post_unpack() override {
#ifndef NDEBUG
    trace_msg_handlers();
#endif
  }
};
} // namespace lobby

REGISTER_COMPONENT(lobby::component)
