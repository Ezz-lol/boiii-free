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

#ifndef HOOK_MSG_HANDLER
#define HOOK_MSG_HANDLER(table, name)                                          \
  static const LobbyMsg_MsgHandleCallbackPtr table##_##name##_orig =           \
      table->name.function;                                                    \
  static LobbyMsg_MsgHandleCallback &table##_##name##_new =                    \
      LobbyMsgHandler_TraceExec<table##_##name##_orig, #table, #name>;         \
  table->name.function = &table##_##name##_new;
#endif

inline void trace_host_client_msg_handlers() {
  HOOK_MSG_HANDLER(host_clientMessageHandlers, JoinRequest);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, JoinMemberInfo);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, JoinAgreementResponse);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, JoinResponse);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, ServerListInfo);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, PeerToPeerInfo);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, LobbyClientHeartbeat);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, LobbyClientDisconnect);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, LobbyClientReliableData);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, LobbyClientContent);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, LobbyModifiedStats);
  HOOK_MSG_HANDLER(host_clientMessageHandlers, VoicePacket);
}

inline void trace_client_host_msg_handlers() {
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyStatePrivate);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyStateGame);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyHostHeartbeat);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyHostDisconnect);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyHostDisconnectClient);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyHostLeaveWithParty);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, JoinAgreementRequest);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, JoinComplete);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyMigrateAnnounceHost);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, IngameMigrateTo);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, IngameMigrateNewHost);
  HOOK_MSG_HANDLER(client_hostMessageHandlers, LobbyClientContent);
}

inline void trace_p2p_msg_handlers() {
  HOOK_MSG_HANDLER(p2p_messageHandlers, ConnectivityTest);
  HOOK_MSG_HANDLER(p2p_messageHandlers, MigrateBandwidthTest);
  HOOK_MSG_HANDLER(p2p_messageHandlers, MigrateStart);
  HOOK_MSG_HANDLER(p2p_messageHandlers, VoicePacket);
  HOOK_MSG_HANDLER(p2p_messageHandlers, DemoState);
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
