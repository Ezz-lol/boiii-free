#include "../std_include.hpp"
#include "loader/component_loader.hpp"
#include "../game/game.hpp"
#include "../game/utils.hpp"

#include "party.hpp"
#include "auth.hpp"
#include "network.hpp"
#include "network_password.hpp"
#include "scheduler.hpp"
#include "workshop.hpp"
#include "profile_infos.hpp"
#include "friends.hpp"
#include "steam_proxy.hpp"
#include "toast.hpp"

#include <game/utils.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>
#include <utils/concurrency.hpp>

#include "game/impl/cl/cl.hpp"

namespace party {
namespace {
std::atomic_bool is_connecting_to_dedi{false};
game::net::netadr_t connect_host{{}, {}, game::net::NA_BAD, {}};

std::mutex hostname_mutex;
std::string cached_server_hostname;
int cached_server_max_clients = 0;

void update_dedi_dvar(bool on_dedi) {
  game::Dvar_SetFromStringByName("cl_connected_to_dedi", on_dedi ? "1" : "0",
                                 true);
}

struct server_query {
  bool sent{false};
  game::net::netadr_t host{};
  std::string challenge{};
  query_callback callback{};
  std::chrono::high_resolution_clock::time_point query_time{};
};

utils::concurrency::container<std::vector<server_query>> &get_server_queries() {
  static utils::concurrency::container<std::vector<server_query>>
      server_queries;
  return server_queries;
}

void connect_to_lobby(const game::ControllerIndex_t controllerIndex,
                      const game::net::netadr_t &addr,
                      const std::string &mapname, const std::string &gamemode,
                      const std::string &usermap_id,
                      const std::string &mod_id) {

  auth::clear_stored_guids();
  auth::clear_stored_challenge();
  workshop::setup_same_mod_as_host(
      game::com::Com_ControllerIndex_GetLocalClientNum(controllerIndex),
      usermap_id, mod_id, true);

  game::net::XSESSION_INFO info{};

  int32_t publicSlots = 0;
  for (game::LocalClientNum_t localClientIdx = game::LOCAL_CLIENT_0;
       localClientIdx < game::LOCAL_CLIENT_COUNT; localClientIdx++) {

    if (game::com::Com_LocalClient_IsBeingUsed(localClientIdx)) {

      publicSlots++;
    }
  }

  // publicSlots is entirely unused within CL_ConnectFromLobby, but just
  // for the sake of completeness, we will pass the correct value.
  game::cl::CL_ConnectFromLobby(controllerIndex, &info, &addr, publicSlots, 0,
                                mapname.data(), gamemode.data(),
                                usermap_id.data());
}

void launch_mode(const game::eModes mode) {
  scheduler::once(
      [=] {
        const game::LocalClientNum_t local_client = game::INVALID_LOCAL_CLIENT;
        const game::eModes current_mode = game::com::Com_SessionMode_GetMode();
        game::com::Com_SwitchMode(local_client, current_mode, mode, 6);
      },
      scheduler::main);
}

void connect_to_lobby_with_mode_internal(const game::net::netadr_t &addr,
                                         const game::eModes mode,
                                         const std::string &mapname,
                                         const std::string &gametype,
                                         const std::string &usermap_id,
                                         const std::string &mod_id,
                                         const bool was_retried = false) {

  if (game::com::Com_SessionMode_IsMode(mode)) {

    connect_to_lobby(
        game::com::Com_LocalClient_GetControllerIndex(game::LOCAL_CLIENT_0),
        addr, mapname, gametype, usermap_id, mod_id);

    return;
  }

  if (!was_retried) {
    scheduler::once(
        [=] {
          connect_to_lobby_with_mode_internal(addr, mode, mapname, gametype,
                                              usermap_id, mod_id, true);
        },
        scheduler::main, 5s);

    launch_mode(mode);
  }
}

game::lobby::LobbyMainMode convert_mode(const game::eModes mode) {
  switch (mode) {
  case game::eModes::CAMPAIGN:
    return game::lobby::LobbyMainMode::CP;
  case game::eModes::MULTIPLAYER:
    return game::lobby::LobbyMainMode::MP;
  case game::eModes::ZOMBIES:
    return game::lobby::LobbyMainMode::ZM;
  default:
    return game::lobby::LobbyMainMode::INVALID;
  }
}

void connect_to_session(const game::net::netadr_t &addr,
                        const std::string &hostname, const uint64_t xuid,
                        const game::eModes mode) {
  const auto LobbyJoin_Begin = reinterpret_cast<bool (*)(
      int actionId, game::ControllerIndex_t controllerIndex,
      game::lobby::LobbyType sourceLobbyType,
      game::lobby::LobbyType targetLobbyType)>(0x141ED94D0_g);

  if (!LobbyJoin_Begin(0, game::CONTROLLER_INDEX_FIRST,
                       game::lobby::LobbyType::PRIVATE,
                       game::lobby::LobbyType::PRIVATE)) {
    return;
  }

  game::lobby::Join &join = *game::lobby::s_join;

  game::lobby::JoinHost &host = join.hostList[0];
  memset(&host, 0, sizeof(host));

  host.info.netAdr = addr;
  host.info.xuid = xuid;
  utils::string::copy(host.info.name, hostname.data());

  host.lobbyType = game::lobby::LobbyType::PRIVATE;
  host.lobbyParams.networkMode = game::lobby::LobbyNetworkMode::LIVE;
  host.lobbyParams.mainMode = convert_mode(mode);

  host.retryCount = 0;
  host.retryTime = game::sys::Sys_Milliseconds();

  join.potentialHost = host;
  join.hostCount = 1;
  join.processedCount = 1;
  join.state = game::lobby::JoinSourceState::ASSOCIATING;
  join.startTime = game::sys::Sys_Milliseconds();

  /*join.targetLobbyType = game::lobby::LobbyType::PRIVATE;
  join.sourceLobbyType = game::lobby::LobbyType::PRIVATE;
  join.controllerIndex = game::CONTROLLER_INDEX_FIRST;
  join.joinType = game::lobby::JOIN_TYPE_NORMAL;
  join.joinResult = game::lobby::JOIN_RESULT_INVALID;
  join.isFinalized = false;*/

  // LobbyJoinSource_Finalize
  join.isFinalized = true;
}

void handle_connect_query_response(const bool success,
                                   const game::net::netadr_t &target,
                                   const utils::info_string &info,
                                   uint32_t ping) {
  if (!success) {
    const std::string msg = utils::string::va(
        "No response from server %u.%u.%u.%u:%hu", target.ipv4.a, target.ipv4.b,
        target.ipv4.c, target.ipv4.d, target.port);
    printf("Connect failed: %s\n", msg.c_str());
    toast::show("Connect failed", "No response from server",
                "t7_icon_connect_overlays");
    return;
  }

  is_connecting_to_dedi = info.get("dedicated") == "1";
  update_dedi_dvar(is_connecting_to_dedi.load());

  {
    std::lock_guard lock(hostname_mutex);
    cached_server_hostname = info.get("hostname");
    const std::string max_clients_str = info.get("sv_maxclients");
    cached_server_max_clients =
        max_clients_str.empty() ? 0 : atoi(max_clients_str.data());
  }

  if (atoi(info.get("protocol").data()) != PROTOCOL) {
    const char *msg = "Invalid protocol.";
    printf("Connect failed: %s\n", msg);
    toast::show("Connect failed", msg, "t7_icon_connect_overlays");
    return;
  }

  const int32_t sub_protocol = atoi(info.get("sub_protocol").data());
  if (sub_protocol != SUB_PROTOCOL && sub_protocol != (SUB_PROTOCOL - 1)) {
    const char *msg = "Invalid sub-protocol.";
    printf("Connect failed: %s\n", msg);
    toast::show("Connect failed", msg, "t7_icon_connect_overlays");
    return;
  }

  const std::string gamename = info.get("gamename");
  if (gamename != "T7"s) {
    const char *msg = "Invalid gamename.";
    printf("Connect failed: %s\n", msg);
    toast::show("Connect failed", msg, "t7_icon_connect_overlays");
    return;
  }

  // Verify network password
  const std::string server_net_hash = info.get("net_password_hash");
  if (!server_net_hash.empty() && server_net_hash != "0") {
    if (!network_password::is_password_set()) {
      const char *msg = "Server requires a network password.";
      printf("Connect failed: %s\n", msg);
      toast::show("Connect failed", msg, "t7_icon_connect_overlays");
      return;
    }

    const std::string client_hash =
        network_password::get_password_hash_string();
    if (client_hash != server_net_hash) {
      const char *msg = "Network password mismatch.";
      printf("Connect failed: %s\n", msg);
      toast::show("Connect failed", msg, "t7_icon_connect_overlays");
      return;
    }
  } else if (network_password::is_password_set()) {
    printf("Client has network password set but server does not. Allowing "
           "connection.\n");
  }

  const std::string mapname = info.get("mapname");
  if (mapname.empty()) {
    const char *msg = "Invalid map.";
    printf("Connect failed: %s\n", msg);
    toast::show("Connect failed", msg, "t7_icon_connect_overlays");
    return;
  }

  const std::string gametype = info.get("gametype");
  if (gametype.empty()) {
    const char *msg = "Invalid gametype.";
    printf("Connect failed: %s\n", msg);
    toast::show("Connect failed", msg, "t7_icon_connect_overlays");
    return;
  }

  const std::string mod_id = info.get("modId");

  const std::string workshop_id = info.get("workshop_id").empty()
                                      ? info.get("usermapId")
                                      : info.get("workshop_id");
  const std::string base_url = info.get("sv_wwwBaseURL").empty()
                                   ? info.get("sv_wwwBaseUrl")
                                   : info.get("sv_wwwBaseURL");

  // const std::string hostname = info.get("sv_hostname");
  const std::string playmode = info.get("playmode");
  const game::eModes mode =
      static_cast<game::eModes>(std::atoi(playmode.data()));
  // const game::XUID xuid = strtoull(info.get("xuid").data(), nullptr, 16);

  scheduler::once(
      [=] {
        const char *addr_str =
            utils::string::va("%i.%i.%i.%i:%hu", target.ipv4.a, target.ipv4.b,
                              target.ipv4.c, target.ipv4.d, target.port);

        // Always save latest address for mod reconnect (mod unload/reload)
        workshop::set_pending_mod_reconnect(addr_str);

        const std::string usermap_id =
            workshop::get_usermap_publisher_id(mapname);

        if (workshop::check_valid_usermap_id(mapname, usermap_id, workshop_id,
                                             base_url) &&
            workshop::check_valid_mod_id(mod_id, workshop_id)) {
          game::com::Com_SessionMode_SetGameMode(
              game::eGameModes::MATCHMAKING_PLAYLIST);

          // connect_to_session(target, hostname, xuid, mode);
          connect_to_lobby_with_mode_internal(target, mode, mapname, gametype,
                                              usermap_id, mod_id);
        } else {
          const char *msg = utils::string::va(
              "Missing or invalid workshop/map dependencies for server %s.",
              addr_str);
          printf("Connect failed: %s\n", msg);
          toast::show("Connect failed", "Missing workshop/map dependencies",
                      "t7_icon_connect_overlays");
          // Save download reconnect
          workshop::set_pending_download_reconnect(addr_str);
        }
      },
      scheduler::main);
}

void connect_stub(const char *address) {
  if (address) {
    const game::net::netadr_t target = network::address_from_string(address);
    if (target.type == game::net::NA_BAD) {
      printf("Connect failed: invalid address \"%s\"\n", address);
      toast::show("Connect failed", "Invalid address",
                  "t7_icon_connect_overlays");
      return;
    }

    connect_host = target;
    toast::show("Connecting", address, "t7_icon_connect_overlays");
  }

  profile_infos::clear_profile_infos();

  if (address) {
    std::string game_info = friends::get_friend_game_info_by_address(address);
    if (!game_info.empty()) {
      std::vector<std::string> parts = utils::string::split(game_info, '|');
      if (parts.size() >= 4) {
        std::string mapname = parts[1];
        std::string gametype = parts[2];
        game::eModes mode =
            static_cast<game::eModes>(std::atoi(parts[3].c_str()));
        std::string mod_id = parts.size() >= 5 ? parts[4] : "";

        if (!mapname.empty() && !gametype.empty()) {

          scheduler::once(
              [=] {
                std::string usermap_id =
                    workshop::get_usermap_publisher_id(mapname);
                game::com::Com_SessionMode_SetGameMode(
                    game::eGameModes::MATCHMAKING_PLAYLIST);
                connect_to_lobby_with_mode_internal(
                    connect_host, mode, mapname, gametype, usermap_id, mod_id);
              },
              scheduler::main);
          return;
        }
      }
    }
  }

  query_server(connect_host, handle_connect_query_response);
}

void send_server_query(server_query &query) {
  query.sent = true;
  query.query_time = std::chrono::high_resolution_clock::now();
  query.challenge = utils::cryptography::random::get_challenge();

  network::send(query.host, "getInfo", query.challenge);
}

void handle_info_response(const game::net::netadr_t &target,
                          const network::data_view &data,
                          game::LocalClientNum_t clientNum) {

  bool found_query = false;
  server_query query{};

  const utils::info_string info{data};

  get_server_queries().access([&](std::vector<server_query> &server_queries) {
    for (std::vector<server_query>::iterator i = server_queries.begin();
         i != server_queries.end(); ++i) {
      if (i->host == target && i->challenge == info.get("challenge")) {
        found_query = true;
        query = std::move(*i);
        i = server_queries.erase(i);
        break;
      }
    }
  });

  if (found_query) {
    const std::chrono::nanoseconds ping =
        std::chrono::high_resolution_clock::now() - query.query_time;
    const std::chrono::milliseconds::rep ping_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(ping).count();

    query.callback(true, query.host, info, static_cast<uint32_t>(ping_ms));
  }
}

void cleanup_queried_servers() {
  std::vector<server_query> removed_queries{};

  get_server_queries().access([&](std::vector<server_query> &server_queries) {
    size_t sent_queries = 0;

    const std::chrono::high_resolution_clock::time_point now =
        std::chrono::high_resolution_clock::now();
    for (std::vector<server_query>::iterator i = server_queries.begin();
         i != server_queries.end();) {
      if (!i->sent) {
        if (++sent_queries < 40) {
          send_server_query(*i);
        }

        ++i;
        continue;
      }

      if ((now - i->query_time) < 1s) {
        ++i;
        continue;
      }

      removed_queries.emplace_back(std::move(*i));
      i = server_queries.erase(i);
    }
  });

  const utils::info_string empty{};
  for (const server_query &query : removed_queries) {
    query.callback(false, query.host, empty, 0);
  }
}
} // namespace

void query_server(const game::net::netadr_t &host, query_callback callback) {
  server_query query{};
  query.sent = false;
  query.host = host;
  query.callback = std::move(callback);

  get_server_queries().access([&](std::vector<server_query> &server_queries) {
    server_queries.emplace_back(std::move(query));
  });
}

void connect_to_lobby_with_mode(const game::net::netadr_t &addr,
                                const game::eModes mode,
                                const std::string &mapname,
                                const std::string &gametype,
                                const std::string &usermap_id,
                                const std::string &mod_id) {
  connect_to_lobby_with_mode_internal(addr, mode, mapname, gametype, usermap_id,
                                      mod_id, false);
}

game::net::netadr_t
get_connected_server(game::LocalClientNum_t localClientNum) {
  return game::cl::CL_GetLocalClientConnection(localClientNum)->serverAddress;
}

game::net::netadr_t get_connect_host() { return connect_host; }

bool is_host(const game::net::netadr_t &addr) {
  return get_connected_server() == addr || connect_host == addr;
}

void join_session(const game::net::netadr_t &addr, const std::string &hostname,
                  const uint64_t xuid, const game::eModes mode) {
  connect_to_session(addr, hostname, xuid, mode);
}

uint16_t get_local_port() { return game::port(); }

std::string get_server_hostname() {
  std::lock_guard lock(hostname_mutex);
  return cached_server_hostname;
}

int get_server_max_clients() {
  std::lock_guard lock(hostname_mutex);
  return cached_server_max_clients;
}

void clear_server_info() {
  std::lock_guard lock(hostname_mutex);
  cached_server_hostname.clear();
  cached_server_max_clients = 0;
}

struct component final : client_component {
  void post_unpack() override {
    (void)game::register_dvar_bool("cl_connected_to_dedi", false,
                                   game::DVAR_NONE,
                                   "True when connected to a dedicated server");

    utils::hook::jump(0x141EE5FE0_g, &connect_stub);

    network::on("infoResponse", handle_info_response);

    scheduler::loop(cleanup_queried_servers, scheduler::async, 100ms);
  }

  void pre_destroy() override {
    get_server_queries().access([](std::vector<server_query> &s) { s = {}; });
  }
};
} // namespace party

REGISTER_COMPONENT(party::component)