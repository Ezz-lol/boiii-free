#include <std_include.hpp>

#include "zombies_lobby.hpp"
#include "zombies_lobby_vm.hpp"
#include "game/utils.hpp"
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "zombies/coop_runtime.hpp"
#include "zombies/lobby_policy.hpp"
#include "zombies/stock_session.hpp"
#include <array>
#include <chrono>
#include <cstring>
#include <mutex>
#include <optional>
#include <set>
#include <utils/cryptography.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>

namespace zombies::coop {
namespace {
using clock = std::chrono::steady_clock;
using address = game::net::netadr_t;
using hosts = std::unordered_set<address>;
std::mutex lobby_mutex;
lobby_advertisement published;
bool published_lan_respond{};
lan_reply_limiter response_limit;
struct discovery_request {
  std::string nonce;
  std::int64_t started{};
  server_list::callback callback;
  hosts found;
};
std::optional<discovery_request> discovery;
clock::time_point next_heartbeat{};

std::int64_t monotonic_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             clock::now().time_since_epoch())
      .count();
}
std::string bounded_string(std::string_view value) {
  return value.size() <= 64 ? std::string(value) : std::string{};
}
struct lobby_sample {
  lobby_policy_input policy;
  std::string map, gametype;
};
lobby_sample sample_lobby() {
  lobby_sample sample{};
  auto &p = sample.policy;
  p.enabled = utils::flags::has_flag("experimental-zombies-coop");
  if (!p.enabled || !game::is_client())
    return sample;
  p.zombies = game::lobby::base::LobbyBase_GetMainMode() ==
              game::lobby::LobbyMainMode::ZM;
  p.local_game_host =
      game::lobby::LobbyHost_IsHost(game::lobby::LobbyType::GAME);
  if (!p.zombies || !p.local_game_host)
    return sample;
  const auto *session =
      game::lobby::LobbyHostData_GetSession(game::lobby::LobbyType::GAME);
  if (!session)
    return sample;
  p.session_ready =
      session->module == game::lobby::LobbyModule::HOST &&
      session->type == game::lobby::LobbyType::GAME &&
      session->status == game::lobby::session::SessionStatus::IDLE &&
      session->active == game::lobby::session::SessionActive::ACTIVE &&
      !session->inRecovery && client_lobby_hooks_ready();
  p.custom = session->mode == game::lobby::LobbyMode::CUSTOM;
  p.capacity = static_cast<int>(session->host.maxClients);
  p.clients = game::lobby::LobbyHost_GetClientCount(
      game::lobby::LobbyType::GAME, game::lobby::LobbyClientType::ALL);
  p.open = game::get_dvar_bool("nat_open").value_or(false);
  const auto mode = game::lobby::base::LobbyBase_GetNetworkMode();
  p.lan = mode == game::lobby::LobbyNetworkMode::LAN;
  p.live = mode == game::lobby::LobbyNetworkMode::LIVE;
  p.browse_screen = *game::lobby::ui::s_uiScreen == (p.lan ? 410 : 402);
  p.in_match = !game::com::Com_IsRunningUILevel();
  p.adapter_ready = game::server_running() && admission_ready();
  const bool mod = game::ugc::UGC_ActiveMod_Loaded();
  const char *publisher = mod ? game::ugc::UGC_ActiveMod_PublisherId() : "";
  p.stock_content = stock_session_allowed(
      game::get_fs_game().value_or(""), mod, publisher ? publisher : "",
      game::ugc::active_usermap->publisherId[0] != 0);
  sample.map = bounded_string(
      game::get_dvar_string(p.in_match ? "mapname" : "ui_mapname")
          .value_or(""));
  sample.gametype =
      bounded_string(game::get_dvar_string("g_gametype").value_or(""));
  if (sample.gametype.empty())
    sample.gametype = "zclassic"; // proven ZM above
  p.map = sample.map;
  p.advertise_opt_in = game::get_dvar_bool("zm_coopAdvertise").value_or(false);
  p.lan_discoverable =
      game::get_dvar_bool("zm_coopLanDiscoverable").value_or(false);
  return sample;
}

void refresh_lobby_buttons() {
  if (const auto global = game::ui::UI_Model_GetGlobalModel()) {
    if (const auto model = game::ui::UI_Model_GetModelFromPath(global, "lobbyRoot.lobbyButtonUpdate"))
      (void)game::ui::UI_Model_ForceNotify(model);
  }
}

void publish_lobby() {
  auto sample = sample_lobby();
  sample.policy.map = sample.map; // rebind after a possible SSO move on return
  const auto result = lobby_listing_policy(sample.policy);
  lobby_advertisement snapshot{};
  snapshot.enabled = sample.policy.enabled;
  snapshot.active = result.active;
  snapshot.can_advertise = result.can_advertise;
  snapshot.advertise = result.advertise;
  snapshot.lan = sample.policy.lan;
  snapshot.in_match = sample.policy.in_match;
  snapshot.capacity = result.active ? sample.policy.capacity : 0;
  snapshot.clients = result.active ? sample.policy.clients : 0;
  snapshot.map = result.active ? std::move(sample.map) : std::string{};
  snapshot.gametype =
      result.active ? std::move(sample.gametype) : std::string{};
  bool listing_changed{};
  {
    const std::lock_guard lock(lobby_mutex);
    listing_changed = published.can_advertise != snapshot.can_advertise ||
        published.advertise != snapshot.advertise;
    published = snapshot;
    published_lan_respond = result.lan_respond;
  }
  if (listing_changed) refresh_lobby_buttons();
  // Revalidate eligibility every second. Never open a lobby, create a NAT
  // mapping, or list anything until its owner explicitly opts in.
  if (!snapshot.advertise) {
    next_heartbeat = {};
    return;
  }
  const auto now = clock::now();
  if (now < next_heartbeat)
    return;
  next_heartbeat = now + std::chrono::seconds(60);
  for (const auto &master : server_list::get_master_servers())
    network::send(master, "heartbeat", "T7");
}

bool local_source(const address &peer) {
  return (peer.type == game::net::netadrtype_t::RAWIP) && peer.port != 0 &&
         private_lan_ipv4(ntohl(peer.addr));
}
std::string_view nonce_view(const network::data_view &data) {
  if (data.size() > 128)
    return {};
  return {reinterpret_cast<const char *>(data.data()), data.size()};
}
void receive_query(const address &peer, const network::data_view &data,
                   game::LocalClientNum_t) {
  const auto nonce = nonce_view(data);
  if (!local_source(peer) || !valid_lan_nonce(nonce))
    return;
  {
    const std::lock_guard lock(lobby_mutex);
    if (!published_lan_respond ||
        !response_limit.allow(ntohl(peer.addr), monotonic_ms()))
      return;
  }
  // Reply is no larger than the validated query; public sources never reflect.
  network::send(peer, "zmLobbyReply", std::string(nonce));
}
void receive_reply(const address &peer, const network::data_view &data,
                   game::LocalClientNum_t) {
  const auto nonce = nonce_view(data);
  if (!local_source(peer) || !valid_lan_nonce(nonce))
    return;
  const std::lock_guard lock(lobby_mutex);
  if (!discovery ||
      !accept_lan_reply(discovery->nonce, nonce, discovery->started,
                        monotonic_ms(), discovery->found.size()))
    return;
  discovery->found.insert(peer);
}
void finish_discovery() {
  std::optional<discovery_request> complete;
  {
    const std::lock_guard lock(lobby_mutex);
    if (!discovery ||
        monotonic_ms() - discovery->started < lan_query_lifetime_ms)
      return;
    complete = std::move(discovery);
    discovery.reset();
  }
  // No discovery mutex is held while Steam/browser code invokes callbacks.
  complete->callback(true, complete->found);
}
void begin_discovery(server_list::callback callback) {
  if (!callback)
    return;
  server_list::callback cancelled;
  std::string bytes(16, '\0');
  utils::cryptography::random::get_data(bytes.data(), bytes.size());
  const auto nonce = utils::string::dump_hex(bytes, "");
  {
    const std::lock_guard lock(lobby_mutex);
    if (discovery)
      cancelled = std::move(discovery->callback);
    discovery =
        discovery_request{nonce, monotonic_ms(), std::move(callback), {}};
  }
  // Cancelled callback observes a new generation; no old reply can fill it.
  if (cancelled)
    cancelled(false, hosts{});
  std::set<std::uint16_t> ports;
  for (std::uint16_t port = 27017; port <= 27027; ++port)
    ports.insert(port);
  if (const auto bound = network::get_bound_port())
    ports.insert(bound);
  for (const auto port : ports) {
    network::send(network::address_from_ip(INADDR_BROADCAST, port),
                  "zmLobbyQuery", nonce);
    // Limited broadcast may not loop back to other clients on this PC.
    network::send(network::address_from_ip(htonl(INADDR_LOOPBACK), port),
                  "zmLobbyQuery", nonce);
  }
}
struct lobby_component final : client_component {
  DEFINE_COMPONENT_NAME("zombies/lobby");
  void post_unpack() override {
    // Dvars are registered from a main frame after engine initialization.
    scheduler::once(
        [] {
          (void)game::register_dvar_bool(
              "zm_coopAdvertise", false, game::dvarFlags_e::ARCHIVE,
              "Explicitly list an open expanded custom Zombies lobby in the "
              "public browser.");
          (void)game::register_dvar_bool(
              "zm_coopLanDiscoverable", true, game::dvarFlags_e::ARCHIVE,
              "Answer private LAN discovery only while hosting an open "
              "expanded LAN Zombies lobby.");
        },
        scheduler::pipeline::main);
    network::on("zmLobbyQuery", receive_query);
    network::on("zmLobbyReply", receive_reply);
    scheduler::loop(publish_lobby, scheduler::pipeline::main,
                    std::chrono::seconds(1));
    scheduler::loop(finish_discovery, scheduler::pipeline::main,
                    std::chrono::milliseconds(100));
  }
};
} // namespace

lobby_advertisement get_lobby_advertisement() {
  const std::lock_guard lock(lobby_mutex);
  return published;
}
void request_lan_lobbies(server_list::callback callback) {
  if (!game::is_client()) {
    scheduler::once(
        [callback = std::move(callback)] {
          if (callback)
            callback(false, hosts{});
        },
        scheduler::pipeline::main);
    return;
  }
  scheduler::once(
      [callback = std::move(callback)]() mutable {
        begin_discovery(std::move(callback));
      },
      scheduler::pipeline::main);
}
} // namespace zombies::coop
REGISTER_COMPONENT(zombies::coop::lobby_component)
