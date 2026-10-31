#include <std_include.hpp>

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include "auth.hpp"
#include "command.hpp"
#include "friends.hpp"
#include "game/impl/ugc/ugc.hpp"
#include "nat.hpp"
#include "network.hpp"
#include "network_password.hpp"
#include "party.hpp"
#include "profile_infos.hpp"
#include "scheduler.hpp"
#include "toast.hpp"
#include "workshop/workshop.hpp"

#include <game/utils.hpp>
#include <utils/concurrency.hpp>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>
#include <utils/info_string.hpp>
#include <utils/string.hpp>

#include <game/impl/cl/cl.hpp>
#include <game/ui_scripting/execution.hpp>

#include <frozen/string.h>
#include <frozen/unordered_map.h>

namespace party {
game::EngineDependentDvarMut cl_connected_to_dedi;
namespace {
constexpr uint16_t default_server_port = 27017;

std::atomic_bool is_connecting_to_dedi{false};
game::net::netadr_t connect_host{{}, {}, game::net::NA_BAD, {}};

std::mutex hostname_mutex;
std::string cached_server_hostname;
int cached_server_max_clients = 0;

std::string normalize_connect_address(std::string address) {
  utils::string::trim(address);

  const size_t separator = address.find(':');
  const std::string host = address.substr(0, separator);
  if (utils::string::to_lower(host) == "localhost") {
    address.replace(0, host.size(), "127.0.0.1");
  }

  if (separator == std::string::npos) {
    address.append(":" + std::to_string(default_server_port));
  }

  return address;
}

void update_dedi_dvar(bool on_dedi) { cl_connected_to_dedi.set(on_dedi); }

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
                      const std::string_view &mapname,
                      const std::string_view &gamemode,
                      const std::string_view &usermap_id,
                      const std::string_view &mod_id) {

  auth::clear_stored_guids();
  auth::clear_stored_challenge();
  workshop::setup_same_mod_as_host(
      game::com::Com_ControllerIndex_GetLocalClientNum(controllerIndex),
      usermap_id, mod_id, true);

  game::net::XSESSION_INFO info{};

  int32_t publicSlots = 0;
  for (game::LocalClientNum_t localClientIdx = game::LOCAL_CLIENT_0;
       localClientIdx < game::LOCAL_CLIENT_COUNT; ++localClientIdx) {
    if (game::com::Com_LocalClient_IsBeingUsed(localClientIdx)) {
      ++publicSlots;
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
        game::com::Com_SwitchMode(game::LOCAL_CLIENT_0,
                                  game::com::Com_SessionMode_GetMode(), mode,
                                  6);
      },
      scheduler::main);
}

struct GameModeConfiguration {
  game::eModes mode = game::eModes::INVALID;
  game::name_t name = {NULL};
  game::name_t gametype = {NULL};

  inline constexpr GameModeConfiguration() noexcept {}

  inline constexpr GameModeConfiguration(const game::eModes mode) noexcept
      : mode(mode) {}

  inline constexpr GameModeConfiguration(
      const game::eModes mode, const std::string_view &name_val) noexcept
      : mode(mode) {
    strscpy(name, name_val);
  }

  inline constexpr GameModeConfiguration(
      const game::eModes mode, const std::string_view &name_val,
      const std::string_view &gametype_val) noexcept
      : mode(mode) {
    strscpy(name, name_val);
    strscpy(gametype, gametype_val);
  }
};

static constexpr GameModeConfiguration
    DEFAULT_GAMEMODE_CONFIGURATION_ARRAY[+game::eModes::COUNT] = {
        // Zombies
        GameModeConfiguration(game::eModes::ZOMBIES, "zm", "zclassic"),
        // Multiplayer
        GameModeConfiguration(game::eModes::MULTIPLAYER, "mp", "tdm"),
        // Campaign
        GameModeConfiguration(game::eModes::CAMPAIGN, "cp", "coop")};

static constexpr frozen::unordered_map<frozen::string, GameModeConfiguration,
                                       +game::eModes::COUNT>
    DEFAULT_GAMEMODE_CONFIGURATIONS = frozen::make_unordered_map(map_array(
        DEFAULT_GAMEMODE_CONFIGURATION_ARRAY,
        [](const GameModeConfiguration &config) constexpr {
          return std::pair<frozen::string, GameModeConfiguration>(
              frozen::string(config.name,
                             std::char_traits<char>::length(config.name)),
              config);
        }));

inline constexpr std::optional<GameModeConfiguration>
get_default_gamemode_config_for_map(const std::string_view &map) {
  const size_t prefix_idx = map.find("_");
  if (prefix_idx != std::string_view::npos) {
    const std::string_view prefix = map.substr(0, prefix_idx);
    if (DEFAULT_GAMEMODE_CONFIGURATIONS.contains(prefix)) {
      return DEFAULT_GAMEMODE_CONFIGURATIONS.at(prefix);
    }
  }

  return std::nullopt;
}

bool is_frontend_ready() {
  return game::com::Com_IsRunningUILevel() && !workshop::are_xzones_loading() &&
         game::live::Live_IsDemonwareFetchingDone(game::CONTROLLER_INDEX_FIRST);
}

bool is_mode_ready(const GameModeConfiguration &mode) {
  return game::com::Com_SessionMode_IsMode(mode.mode) &&
         workshop::is_xzone_loaded(utils::string::va("%s_common", mode.name));
}

const char *lobby_main_mode(const game::eModes mode) {
  switch (mode) {
  case game::eModes::ZOMBIES:
    return "LOBBY_MAINMODE_ZM";
  case game::eModes::MULTIPLAYER:
    return "LOBBY_MAINMODE_MP";
  default:
    return "LOBBY_MAINMODE_CP";
  }
}

ui_scripting::table lua_globals() {
  return ui_scripting::table{(*game::lua::hks::primary_luaVM)->globals.v.table};
}

void launch_from_frontend(const GameModeConfiguration mode,
                          const std::string mapname) {
  enum class stage { mod, lobby, launch };
  stage current = stage::mod;
  int32_t ready_ticks = 0;

  scheduler::schedule(
      [=]() mutable {
        if (!is_frontend_ready()) {
          return scheduler::cond_continue;
        }

        try {
          const ui_scripting::table globals = lua_globals();
          const ui_scripting::table engine = globals["Engine"];
          const int32_t game_lobby =
              globals["Enum"]["LobbyType"]["LOBBY_TYPE_GAME"].as<int32_t>();

          if (current == stage::mod) {
            if (globals["CoD"]["mapsTable"][mapname]
                    .is<ui_scripting::table>() ||
                globals["LuaUtils"]["Mods_IsUsingMods"]()[0].as<bool>()) {
              current = stage::lobby;
            } else {
              globals["LuaUtils"]["Mods_LoadMod"]("usermaps");
              current = stage::lobby;
              return scheduler::cond_continue;
            }
          }

          if (current == stage::lobby) {
            const std::string_view gametype =
                game::com::Com_SessionMode_IsMode(mode.mode)
                    ? game::get_dvar_string("ui_gametype").value_or("")
                    : "";
            game::Dvar_SetFromStringByName("ui_mapname", mapname.c_str(), true);
            if (gametype.empty() || gametype == "frontend") {
              game::Dvar_SetFromStringByName("ui_gametype", mode.gametype,
                                             true);
            }

            ui_scripting::table params{};
            params.set("controller", 0);
            params.set(
                "mainMode",
                globals["Enum"]["LobbyMainMode"][lobby_main_mode(mode.mode)]
                    .as<int32_t>());
            engine["LobbyVM_CallFunc"]("Devmap", params);
            current = stage::launch;
            return scheduler::cond_continue;
          }

          if (!is_mode_ready(mode) ||
              !engine["IsLobbyActive"](game_lobby)[0].as<bool>() ||
              !engine["IsLobbyHost"](game_lobby)[0].as<bool>()) {
            ready_ticks = 0;
            return scheduler::cond_continue;
          }
          if (++ready_ticks < 2) {
            return scheduler::cond_continue;
          }

          game::cbuf::Cbuf_AddText(
              game::LOCAL_CLIENT_0,
              utils::string::va(
                  "lobbyLaunchGame %d %s %s\n", game_lobby, mapname.c_str(),
                  std::string(game::get_dvar_string("ui_gametype").value_or(""))
                      .c_str()));
        } catch (const std::exception &e) {
          printf("[Map] Could not start %s: %s\n", mapname.c_str(), e.what());
        }
        return scheduler::cond_end;
      },
      scheduler::main, 250ms);
}

utils::hook::detour sv_map_hook;
utils::hook::detour sv_devmap_hook;

bool needs_usermaps_mod(const std::string_view &mapname) {
  return std::string_view(game::ugc::UGC_ActiveMod_PublisherId()).empty() &&
         game::ugc::UGC_GetUsermapByInternalName(mapname).has_value();
}

template <utils::hook::detour *Hook> void sv_map_in_mode_stub() {
  std::optional<GameModeConfiguration> mode;
  std::string mapname;
  bool leave_match = false;
  {
    const command::params_sv params{};
    if (params.size() > 1 && game::is_new_client()) {
      leave_match = game::com::Com_IsInGame() && needs_usermaps_mod(params[1]);
      if (!game::com::Com_IsInGame() || leave_match) {
        mode = get_default_gamemode_config_for_map(params[1]);
        mapname = params[1];
      }
    }
  }

  if (!mode) {
    Hook->invoke<void>();
    return;
  }

  if (leave_match) {
    game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "disconnect\n");
  }
  launch_from_frontend(*mode, mapname);
}

bool ensure_networkmode_online() {
  using namespace game;
  using namespace game::lobby;
  switch (com::Com_SessionMode_GetNetworkMode()) {
  case eNetworkModes::ONLINE:
  case eNetworkModes::SYSTEMLINK:
    switch (base::LobbyBase_GetNetworkMode()) {
    case LobbyNetworkMode::LAN:
    case LobbyNetworkMode::LIVE:
      return true;
    default:
      break;
    }
  default:
    break;
  }

  base::LobbyBase_SetNetworkMode(LobbyNetworkMode::LIVE);
  return false;
}

void call_lobby_vm(const char *function) {
  try {
    lua_globals()["Engine"]["LobbyVM_CallFunc"](function,
                                                ui_scripting::table{});
  } catch (const std::exception &e) {
    printf("LobbyVM %s failed: %s\n", function, e.what());
  }
}

void with_party_lobby(const std::function<void()> &callback) {
  if (nat::has_party_lobby()) {
    callback();
    return;
  }
  const auto deadline = std::chrono::steady_clock::now() + 10s;
  scheduler::schedule(
      [=] {
        if (!nat::has_party_lobby() &&
            std::chrono::steady_clock::now() < deadline) {
          call_lobby_vm("StartPartyLobby");
          return scheduler::cond_continue;
        }
        callback();
        return scheduler::cond_end;
      },
      scheduler::main, 100ms);
}

void update_server_lobby() {
  static bool requested = false;
  const game::connstate_t state =
      game::cg::clientUIActives->actives[0].connectionState;
  if (state == game::connstate_t::DISCONNECTED) {
    requested = false;
  } else if (state == game::connstate_t::ACTIVE && !requested &&
             cl_connected_to_dedi.get_bool() && !game::server_running() &&
             nat::has_party_lobby()) {
    requested = true;
    nat::join_server_lobby(get_connected_server(game::LOCAL_CLIENT_0));
  }
}

void connect_in_mode(const game::net::netadr_t &addr, const game::eModes mode,
                     const std::string_view &mapname,
                     const std::string_view &gametype,
                     const std::string_view &usermap_id,
                     const std::string_view &mod_id,
                     const std::string_view &lobby_view) {
  // The lambdas below run later, so they need their own copies
  const std::string map{mapname}, type{gametype}, usermap{usermap_id},
      mod{mod_id}, lobby{lobby_view};
  const auto connect = [=] {
    game::com::Com_SessionMode_SetGameMode(
        game::eGameModes::MATCHMAKING_PLAYLIST);
    const auto connect_direct = [=] {
      connect_to_lobby(
          game::com::Com_LocalClient_GetControllerIndex(game::LOCAL_CLIENT_0),
          addr, map, type, usermap, mod);
    };
    if (lobby.empty()) {
      with_party_lobby(connect_direct);
      return;
    }
    nat::join_match_lobby(addr, lobby == "party" ? [] {
      printf("Connect failed: Could not join their party.\n");
      toast::show("Connect failed", "Could not join their party",
                  "t7_icon_connect_overlays");
    } : std::function<void()>(connect_direct));
  };

  if (ensure_networkmode_online() && game::com::Com_SessionMode_IsMode(mode)) {
    connect();
    return;
  }

  launch_mode(mode);
  const auto deadline = std::chrono::steady_clock::now() + 15s;
  scheduler::schedule(
      [=] {
        if (ensure_networkmode_online() &&
            game::com::Com_SessionMode_IsMode(mode)) {
          connect();
          return scheduler::cond_end;
        }
        if (std::chrono::steady_clock::now() < deadline) {
          return scheduler::cond_continue;
        }
        printf("Connect failed: Could not switch to the server's game mode.\n");
        toast::show("Connect failed", "Could not switch to the server's mode",
                    "t7_icon_connect_overlays");
        return scheduler::cond_end;
      },
      scheduler::main, 250ms);
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
    toast::show("Connect failed",
                utils::string::va("No response. The match may have ended, or "
                                  "a firewall is blocking UDP port %hu.",
                                  target.port),
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
  const std::string lobby =
      is_connecting_to_dedi && info.get("sv_running") == "1"
          ? std::string{}
          : info.get("lobby");
  if (mapname == "core_frontend" && lobby.empty()) {
    printf("Connect failed: The host is in the menus, not in a match.\n");
    toast::show("Friend in menus",
                "They are not in a match. Ask them to open their party, then "
                "join from your friends list.",
                "t7_icon_connect_overlays");
    return;
  }
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
  const std::string base_uri = info.get("sv_wwwBaseURL").empty()
                                   ? info.get("sv_wwwBaseUrl")
                                   : info.get("sv_wwwBaseURL");

  // const std::string hostname = info.get("sv_hostname");
  const std::string playmode = info.get("playmode");
  const game::eModes mode =
      static_cast<game::eModes>(std::atoi(playmode.data()));
  // const game::XUID xuid = strtoull(info.get("xuid").data(), nullptr, 16);

  // Stock maps have no usermap entry, so the id stays empty
  const game::ugc::OptionalUGCItemRef usermap_item =
      game::ugc::UGC_GetUsermapByInternalName(mapname);
  const std::string usermap_id =
      usermap_item.has_value() ? usermap_item->get().publisherId : "";
  const std::string addr_str = target.toString();
  scheduler::once(
      [=] {
        // Always save latest address for mod reconnect (mod unload/reload)
        workshop::set_pending_mod_reconnect(addr_str);

        if (workshop::check_valid_usermap_id(mapname, usermap_id, workshop_id,
                                             base_uri) &&
            workshop::check_valid_mod_id(mod_id, workshop_id, base_uri)) {
          connect_in_mode(target, mode, mapname, gametype, usermap_id, mod_id,
                          lobby);
        } else {
          const char *msg = utils::string::va(
              "Missing or invalid workshop/map dependencies for server %s.",
              addr_str.c_str());
          printf("Connect failed: %s\n", msg);
          toast::show("Connect failed", "Missing workshop/map dependencies",
                      "t7_icon_connect_overlays");
          // Save download reconnect
          workshop::set_pending_download_reconnect(addr_str);
        }
      },
      scheduler::main);
}

void connect_finish(const game::net::netadr_t &target) {
  connect_host = target;

  profile_infos::clear_profile_infos();

  query_server(connect_host, handle_connect_query_response);
}

void connect_stub(const char *address) {
  if (address) {
    const std::string address_copy = normalize_connect_address(address);

    const auto friend_unavailable = [](const char *reason) {
      printf("Connect failed: %s\n", reason);
      toast::show("Friend unavailable", reason, "t7_icon_connect_overlays");
    };

    const bool is_loopback = address_copy.starts_with("127.");
    if (const auto friend_id =
            is_loopback ? 0 : friends::find_browser_route(address_copy)) {
      if (!friends::connect_to_friend(friend_id))
        friend_unavailable("No joinable match was found");
      return;
    }

    if (address_copy == "0.0.0.0" || address_copy.starts_with("0.0.0.0:")) {
      friend_unavailable("Friend is offline or their party is closed");
      return;
    }

    toast::show("Connecting", address_copy, "t7_icon_connect_overlays");

    network::resolvedAddrCallback_t resolveCb =
        [address_copy](game::net::netadr_t target) -> void {
      scheduler::once(
          [address_copy, target] {
            if (target.type == game::net::NA_BAD) {
              printf("Connect failed: invalid address \"%s\"\n",
                     address_copy.c_str());
              toast::show("Connect failed", "Invalid address",
                          "t7_icon_connect_overlays");
              return;
            }

            if (network::is_ip_address(target) &&
                (target.addr == 0 || target.port == 0)) {
              printf("Connect failed: Friend is offline or their party is "
                     "closed\n");
              toast::show("Friend unavailable",
                          "Friend is offline or their party is closed",
                          "t7_icon_connect_overlays");
              return;
            }

            connect_finish(target);
          },
          scheduler::main);
    };
    // Resolve the address on a background thread.
    network::address_from_string_async(address_copy, resolveCb);
  } else {
    connect_finish(connect_host);
  }
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

  if (game::com::Com_IsRunningUILevel()) {
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
}
} // namespace

void connect(const game::net::netadr_t &target) { connect_finish(target); }

void query_server(const game::net::netadr_t &host, query_callback callback) {
  server_query query{};
  query.sent = false;
  query.host = host;
  query.callback = std::move(callback);

  get_server_queries().access([&](std::vector<server_query> &server_queries) {
    server_queries.emplace_back(std::move(query));
  });
}

game::net::netadr_t
get_connected_server(game::LocalClientNum_t localClientNum) {
  return game::cl::CL_GetLocalClientConnection(localClientNum)->serverAddress;
}

game::net::netadr_t get_connect_host() { return connect_host; }

bool is_host(const game::net::netadr_t &addr) {
  return get_connected_server() == addr || connect_host == addr;
}

uint16_t get_local_port() {
  const uint16_t port = network::get_bound_port();
  return port ? port : game::port();
}

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
  DEFINE_COMPONENT_NAME("party");

  void post_unpack() override {
    cl_connected_to_dedi =
        game::register_dvar_bool("cl_connected_to_dedi", false, game::DVAR_NONE,
                                 "True when connected to a dedicated server");

    utils::hook::jump(game::select(0x141ED9670, 0x141EE5FE0, 0x0),
                      &connect_stub);

    network::on("infoResponse", handle_info_response);

    scheduler::loop(cleanup_queried_servers, scheduler::async, 100ms);
    scheduler::loop(update_server_lobby, scheduler::main, 500ms);

    sv_map_hook.create(game::sv::SV_Map_f.get(),
                       sv_map_in_mode_stub<&sv_map_hook>);
    sv_devmap_hook.create(game::sv::SV_DevMap_f.get(),
                          sv_map_in_mode_stub<&sv_devmap_hook>);
  }

  void pre_destroy() override {
    get_server_queries().access([](std::vector<server_query> &s) { s = {}; });
  }
};
} // namespace party

REGISTER_COMPONENT(party::component)
