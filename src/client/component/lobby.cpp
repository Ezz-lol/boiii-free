#include <std_include.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include "network.hpp"
#include "party.hpp"

#include <utils/byte_buffer.hpp>
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

template <const auto Val> decltype(Val) return_const() { return Val; }

constexpr const char *voice_packet_name = "voice";
utils::hook::detour build_and_send_voice_packet_hook;
utils::hook::detour is_client_muted_hook;
utils::hook::detour is_client_talking_hook;
utils::hook::detour is_client_connected_hook;
utils::hook::detour toggle_mute_hook;
std::array<int32_t, MAX_PLAYERS> talk_times{};
std::array<bool, MAX_PLAYERS> muted{};

bool in_game() { return com::Com_IsInGame(); }

bool is_client_muted_stub(ControllerIndex_t controllerIndex,
                          LobbyType lobbyType, ClientNum_t clientNum) {
  if (!in_game()) {
    return is_client_muted_hook.invoke<bool>(controllerIndex, lobbyType,
                                             clientNum);
  }
  return valid_client_num(clientNum) && muted[clientNum];
}

bool is_client_talking_stub(ControllerIndex_t controllerIndex,
                            LobbyType lobbyType, ClientNum_t clientNum) {
  if (!in_game()) {
    return is_client_talking_hook.invoke<bool>(controllerIndex, lobbyType,
                                               clientNum);
  }
  return valid_client_num(clientNum) &&
         sys::Sys_Milliseconds() - talk_times[clientNum] < 250;
}

bool is_client_connected_stub(ControllerIndex_t controllerIndex,
                              LobbyType lobbyType, ClientNum_t clientNum) {
  if (!in_game()) {
    return is_client_connected_hook.invoke<bool>(controllerIndex, lobbyType,
                                                 clientNum);
  }
  return valid_client_num(clientNum);
}

void toggle_mute_stub(ControllerIndex_t controllerIndex, LobbyType lobbyType,
                      ClientNum_t clientNum) {
  if (!in_game()) {
    toggle_mute_hook.invoke<void>(controllerIndex, lobbyType, clientNum);
    return;
  }
  if (valid_client_num(clientNum)) {
    muted[clientNum] = !muted[clientNum];
  }
}

void play_voice(utils::byte_buffer &buffer) {
  const auto talker = static_cast<ClientNum_t>(buffer.read<uint8_t>());
  if (!valid_client_num(talker)) {
    return;
  }

  talk_times[talker] = sys::Sys_Milliseconds();
  const auto count = buffer.read<uint8_t>();
  if (muted[talker]) {
    return;
  }

  std::array<uint8_t, sizeof(cl::voice::ClientVoicePacket_t::data)> chunk{};
  for (uint8_t i = 0; i < count; ++i) {
    const auto size = buffer.read<uint16_t>();
    if (!size || size > chunk.size()) {
      return;
    }
    buffer.read(chunk.data(), size);
    live::steam::voice::LiveSteam_Voice_IncomingVoiceData(
        CONTROLLER_INDEX_FIRST, talker, chunk.data() + 1, size - 1);
  }
}

std::optional<ClientNum_t>
find_client(const std::function<bool(const net::netadr_t &)> &match) {
  std::optional<ClientNum_t> result;
  foreach_connected_client([&](sv::client_s &client, const ClientNum_t num) {
    if (match(client.address)) {
      result = num;
    }
  });
  return result;
}

void relay_voice(const ClientNum_t talker, const std::string &voice) {
  const std::string payload = static_cast<char>(talker) + voice;
  const std::string echo{static_cast<char>(talker), '\0'};
  foreach_connected_client([&](sv::client_s &client, const ClientNum_t num) {
    const std::string &packet = num == talker ? echo : payload;
    if (client.address.type == net::NA_LOOPBACK) {
      utils::byte_buffer buffer(packet);
      play_voice(buffer);
    } else if (client.address.type != net::NA_BOT) {
      network::send(client.address, voice_packet_name, packet);
    }
  });
}

void receive_voice(const net::netadr_t &sender, const network::data_view &data,
                   LocalClientNum_t) {
  if (server_running()) {
    if (const auto talker = find_client(
            [&](const net::netadr_t &address) { return address == sender; })) {
      relay_voice(*talker, {data.begin(), data.end()});
    }
    return;
  }

  if (sender == party::get_connected_server()) {
    utils::byte_buffer buffer(data);
    play_voice(buffer);
  }
}

bool build_and_send_voice_packet_stub(ControllerIndex_t controllerIndex,
                                      session::LobbySession *lobbySession,
                                      XUID talkerXuid,
                                      cl::voice::voiceCommunication_t *vc) {
  if (!in_game()) {
    return build_and_send_voice_packet_hook.invoke<bool>(
        controllerIndex, lobbySession, talkerXuid, vc);
  }

  const int32_t count = std::min(vc->voicePacketCount, 10);
  utils::byte_buffer buffer{};
  buffer.write(static_cast<uint8_t>(count));
  for (int32_t i = 0; i < count; ++i) {
    const auto &packet = vc->voicePackets[i];
    const auto size = static_cast<uint16_t>(std::clamp(
        packet.dataSize, 0, static_cast<int32_t>(sizeof(packet.data))));
    buffer.write(size);
    buffer.write(packet.data, size);
  }

  if (!server_running()) {
    network::send(party::get_connected_server(), voice_packet_name,
                  buffer.get_buffer());
  } else if (const auto host = find_client([](const net::netadr_t &address) {
               return address.type == net::NA_LOOPBACK;
             })) {
    relay_voice(*host, buffer.get_buffer());
  }
  return true;
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
        host::LobbyHost_CanFit.offset(game::select_value(0x6e, 0x6e, 0x69)),
        return_const<false>);

    if (game::is_new_client()) {
      for (const size_t offset : {0xa4, 0x26d}) {
        utils::hook::call(host::LobbyHost_HandleJoinRequest.offset(offset),
                          return_const<LobbyNetworkMode::LIVE>);
      }
      build_and_send_voice_packet_hook.create(
          game::lobby::voice::BuildAndSendVoicePacket.get(),
          build_and_send_voice_packet_stub);
      is_client_muted_hook.create(
          game::lobby::voice::LobbyVoice_IsClientMuted.get(),
          is_client_muted_stub);
      is_client_talking_hook.create(
          game::lobby::voice::LobbyVoice_IsClientNumTalking.get(),
          is_client_talking_stub);
      is_client_connected_hook.create(
          game::lobby::voice::LobbyVoice_IsClientNumConnected.get(),
          is_client_connected_stub);
      toggle_mute_hook.create(game::lobby::voice::LobbyVoice_ToggleMute.get(),
                              toggle_mute_stub);

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

    if (game::is_new_client() || game::is_server()) {
      network::on(voice_packet_name, receive_voice);
    }
  }
};
} // namespace lobby

REGISTER_COMPONENT(lobby::component)
