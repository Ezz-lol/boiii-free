#include <std_include.hpp>

#include "lobby.hpp"

#include <game/impl/ugc/ugc.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include "party.hpp"
#include "scheduler.hpp"
#include "workshop/workshop.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace lobby {
using namespace game;
using namespace game::lobby;
using namespace game::lobby::msg;
namespace {

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

struct map_selection {
  XUID host;
  std::string name;
  std::string id;
  std::shared_ptr<std::atomic<bool>> queued;
};

std::optional<map_selection> offered_usermap;

/**
 * @brief Find the private match session for the local guest in the frontend
 * @return The active game session, or nullptr if it is unavailable
 */
const session::LobbySession *private_match_guest_session() {
  // distinguish between a guest in a private match lobby vs a host running it
  if (!com::Com_IsRunningUILevel() || party::cl_connected_to_dedi.get_bool() ||
      !client::LobbyClient_IsActive(LobbyType::GAME) ||
      client::LobbyClient_IsHost(LobbyType::GAME)) {
    return nullptr;
  }

  const auto *session = client::LobbyClientData_GetSession(LobbyType::GAME);
  // game data may be unavailable while the session is recovering
  if (!session || session->mode != LobbyMode::CUSTOM || session->inRecovery ||
      session->lobbyData.type == nullptr ||
      *session->lobbyData.type != LobbyType::GAME) {
    return nullptr;
  }
  return session;
}

/**
 * @brief Check whether the guest needs usermaps for the selected map
 * @return True if the guest has the map installed and has no active mod
 */
bool guest_should_load_usermaps() {
  const auto *session = private_match_guest_session();
  if (!session || ugc::UGC_ActiveMod_PublisherId()[0]) {
    return false;
  }

  const auto &game_data = session->lobbyData.lobbyGame;
  // lobby can indentify the map by internal name or workshop id
  // only queue a mod load for an installed map
  return workshop::has_usermap(game_data.mapName) ||
         workshop::has_usermap(game_data.ugcInfo.publisherId);
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

bool offer_missing_guest_usermap(std::string_view id) {
  // lobby VM can identify a usermap as "usermaps/<Workshop ID>"
  constexpr std::string_view prefix = "usermaps/";
  if (id.starts_with(prefix)) {
    id.remove_prefix(prefix.size());
  }
  if (!utils::string::is_numeric(id)) {
    return false;
  }
  if (workshop::has_usermap(id)) {
    return true;
  }

  // lobby VM can request installation while the native session is
  // transitioning
  // workshop id is enough to make the offer
  const auto *session = private_match_guest_session();
  const std::string_view map =
      session ? session->lobbyData.lobbyGame.mapName : id;
  if (offered_usermap && offered_usermap->id == id) {
    // failed or canceled download can be retried
    if (!offered_usermap->queued->load()) {
      return true;
    }
    const auto entries = workshop::queue::list();
    const auto item =
        std::ranges::find(entries, id, &workshop::queue::entry::id);
    if (item != entries.end() &&
        item->state != workshop::queue::item_state::failed &&
        item->state != workshop::queue::item_state::cancelled) {
      return true;
    }
  }
  if (workshop::queue::position(id) < 0 &&
      !workshop::is_any_download_active()) {
    auto queued = std::make_shared<std::atomic<bool>>(false);
    offered_usermap = map_selection{session ? session->host.info.xuid : XUID{},
                                    std::string(map), std::string(id), queued};
    workshop::offer_download(ZoneType::USERMAP, map, id,
                             [queued] { queued->store(true); });
  }
  return true;
}

void refresh_downloaded_usermap_preview() {
  const auto *session = private_match_guest_session();
  if (!session || !offered_usermap ||
      offered_usermap->host != session->host.info.xuid ||
      offered_usermap->name != session->lobbyData.lobbyGame.mapName ||
      !ugc::UGC_GetUsermapByPublisherId(offered_usermap->id.c_str())) {
    return;
  }

  const auto global = game::ui::UI_Model_GetGlobalModel();
  if (global) {
    if (const auto model = game::ui::UI_Model_GetModelFromPath(
            global, "lobbyRoot.selectedMapId")) {
      // map selection has not changed, so refresh its image explicitly
      ugc::UGC_SetMapPreviewImageByPublisherId_Impl(
          offered_usermap->id.c_str());
      game::ui::UI_Model_ForceNotify(model);
    }
  }
}

void prepare_guest_usermap() {
  const auto *session = private_match_guest_session();
  if (!session) {
    offered_usermap.reset();
    return;
  }

  const auto &map = session->lobbyData.lobbyGame;
  if (offered_usermap && (offered_usermap->host != session->host.info.xuid ||
                          offered_usermap->name != map.mapName)) {
    offered_usermap.reset();
  }

  // offer the map as soon as the host selects it if the lobby has its ID
  const std::string_view id = map.ugcInfo.publisherId;
  if (!workshop::has_usermap(map.mapName)) {
    offer_missing_guest_usermap(id.empty() ? map.mapName : id);
  }

  // repeated lobby updates share one pending load
  static bool scheduled = false;
  if (scheduled || !guest_should_load_usermaps()) {
    return;
  }

  scheduled = true;
  scheduler::schedule(
      [] {
        // read current lobby again, something may have changed
        if (guest_should_load_usermaps()) {
          if (ugc::active_mod->loadState == ugc::ModLoadState::LOADING ||
              workshop::are_xzones_loading()) {
            return scheduler::COND_CONTINUE;
          }
          // load usermaps mod
          ugc::UGC_LoadModByPublisherId_Impl(LOCAL_CLIENT_0, "usermaps", true);
        }
        scheduled = false;
        return scheduler::COND_END;
      },
      scheduler::pipeline::main);
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("lobby");

public:
  void post_unpack() override {
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
