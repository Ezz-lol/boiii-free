#include <std_include.hpp>

#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"

#include "auth.hpp"
#include "friends.hpp"
#include "getinfo.hpp"
#include "nat.hpp"
#include "network.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"
#include "toast.hpp"
#include "upnp.hpp"
#include "workshop.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <random>
#include <sstream>

namespace nat {
namespace {
constexpr std::chrono::seconds REGISTER_INTERVAL = 5s;
constexpr std::chrono::seconds JOIN_TIMEOUT = 12s;
constexpr size_t MAX_CANDIDATES = 16;
constexpr const char *FRIEND_REJOIN_ROUTE = "192.0.2.254:40000";

bool open_to_friends{};
bool registration_confirmed{};
bool identity_warning_shown{};
std::string hosting_token;
std::string reflected_endpoint;

struct join_attempt {
  bool active{};
  bool retried{};
  uint64_t friend_id{};
  std::string token;
  std::string fallback;
  std::vector<game::net::netadr_t> candidates;
  std::chrono::steady_clock::time_point expires;
  std::chrono::steady_clock::time_point next_rendezvous;
};

struct host_probe {
  std::string token;
  game::net::netadr_t candidate{};
  std::chrono::steady_clock::time_point expires;
};

join_attempt joining;
std::vector<host_probe> host_probes;

struct lobby_request {
  bool active{};
  std::string token;
  game::net::netadr_t host{};
  std::chrono::steady_clock::time_point expires;
};

lobby_request lobby_joining;
std::mutex peers_mutex;
typedef std::unordered_map<std::string, game::net::netadr_t> lobbyPeerMap_t;
lobbyPeerMap_t lobby_peers;
utils::hook::detour dw_common_addr_to_netadr_hook;
utils::hook::detour dw_get_connection_status_hook;

struct lookup_state {
  std::unordered_set<uint64_t> pending;
  std::vector<friend_presence> results;
  lookup_callback done;
};

std::mutex lookup_mutex;
typedef std::unordered_map<std::string, lookup_state> lookupMap_t;
static lookupMap_t lookup_requests;

void finish_lookup(const std::string &request) {
  lookup_callback done;
  std::vector<friend_presence> results;
  {
    std::lock_guard lock(lookup_mutex);
    const lookupMap_t::iterator found = lookup_requests.find(request);
    if (found == lookup_requests.end())
      return;
    results = std::move(found->second.results);
    done = std::move(found->second.done);
    lookup_requests.erase(found);
  }
  if (done)
    done(std::move(results));
}

std::string payload_string(const network::data_view &data) {
  return {reinterpret_cast<const char *>(data.data()), data.size()};
}

std::vector<std::string> fields(const std::string &value) {
  std::istringstream input(value);
  std::vector<std::string> result;
  for (std::string field; input >> field;)
    result.emplace_back(std::move(field));
  return result;
}

bool valid_token(const std::string &token) {
  if (token.size() < 8 || token.size() > 64)
    return false;

  return std::all_of(token.begin(), token.end(), [](const unsigned char c) {
    return std::isalnum(c) || c == '-' || c == '_';
  });
}

std::string new_token() {
  static constexpr char alphabet[] = "0123456789abcdef";
  std::random_device entropy;
  std::mt19937 generator(entropy());
  std::uniform_int_distribution<unsigned int> pick(0, 15);
  std::string result(16, '0');
  for (auto &character : result)
    character = alphabet[pick(generator)];
  return result;
}

uint16_t local_port() {
  const uint16_t port = party::get_local_port();
  return port >= 1024 ? port : 3074;
}

std::string local_endpoint() {
  static const std::string local_ip = [] {
    const SOCKET socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_handle == INVALID_SOCKET)
      return std::string{};

    sockaddr_in route{};
    route.sin_family = AF_INET;
    route.sin_port = htons(53);
    inet_pton(AF_INET, "1.1.1.1", &route.sin_addr);

    std::string result;
    if (connect(socket_handle, reinterpret_cast<sockaddr *>(&route),
                sizeof(route)) == 0) {
      sockaddr_in local{};
      int size = sizeof(local);
      if (getsockname(socket_handle, reinterpret_cast<sockaddr *>(&local),
                      &size) == 0) {
        char ip[INET_ADDRSTRLEN]{};
        if (inet_ntop(AF_INET, &local.sin_addr, ip, sizeof(ip)))
          result = ip;
      }
    }
    closesocket(socket_handle);
    return result;
  }();

  return local_ip.empty()
             ? std::string{}
             : utils::string::va("%s:%hu", local_ip.c_str(), local_port());
}

void send_rendezvous(const std::string &command, const std::string &token) {
  const std::vector<game::net::netadr_t> &masters =
      server_list::get_master_servers();
  if (masters.empty())
    return;

  std::string message = token;
  if (const std::string local = local_endpoint(); !local.empty())
    message.append(" ").append(local);
  for (const game::net::netadr_t &master : masters)
    network::send(master, command, message);
}

void send_friend_publish() {
  const game::XUID friend_code = auth::get_guid();
  if (!friend_code) {
    if (!identity_warning_shown) {
      identity_warning_shown = true;
      toast::error("Friends unavailable",
                   "BOIII Friend Code could not be generated.");
    }
    return;
  }
  if (hosting_token.empty())
    return;
  const char *payload =
      utils::string::va("1 %llu %s", friend_code, hosting_token.c_str());
  send_rendezvous("friendPublish", payload);
}

void add_join_candidate(const std::string &text) {
  if (joining.candidates.size() >= MAX_CANDIDATES)
    return;

  const game::net::netadr_t candidate = network::address_from_string(text);
  if (!network::is_connectable_address(candidate))
    return;

  const bool duplicate =
      std::any_of(joining.candidates.begin(), joining.candidates.end(),
                  [&candidate](const game::net::netadr_t &existing) {
                    return network::are_addresses_equal(existing, candidate);
                  });
  if (!duplicate)
    joining.candidates.push_back(candidate);
}

std::string to_hex(const void *data, const size_t size) {
  std::string result;
  for (size_t i = 0; i < size; ++i)
    result += std::format("{:02x}", static_cast<const uint8_t *>(data)[i]);
  return result;
}

bool from_hex(const std::string &text, void *out, const size_t size) {
  if (text.size() != size * 2)
    return false;
  uint8_t *bytes = static_cast<uint8_t *>(out);
  for (size_t i = 0; i < size; ++i) {
    if (std::from_chars(text.data() + i * 2, text.data() + i * 2 + 2, bytes[i],
                        16)
            .ec != std::errc{})
      return false;
  }
  return true;
}

std::string peer_key(const uint8_t *address) {
  return {reinterpret_cast<const char *>(address), sizeof(game::net::XNADDR)};
}

void add_peer(const uint8_t *address, const game::net::netadr_t &endpoint) {
  std::lock_guard lock(peers_mutex);
  lobby_peers.insert_or_assign(peer_key(address), endpoint);
}

std::optional<game::net::netadr_t> find_peer(const uint8_t *address) {
  constexpr size_t ENDPOINT_SIZE = 6;
  std::lock_guard lock(peers_mutex);
  if (const lobbyPeerMap_t::iterator peer = lobby_peers.find(peer_key(address));
      peer != lobby_peers.end())
    return peer->second;

  std::optional<game::net::netadr_t> match;
  for (const auto &[key, endpoint] : lobby_peers) {
    if (std::memcmp(key.data(), address, ENDPOINT_SIZE) == 0) {
      if (match)
        return std::nullopt;
      match = endpoint;
    }
  }
  return match;
}

bool dw_common_addr_to_netadr_stub(game::net::netadr_t *netadr,
                                   const uint8_t *common_addr,
                                   const game::net::bdSecurityID *sec_id) {
  if (const std::optional<game::net::netadr_t> peer = find_peer(common_addr)) {
    *netadr = *peer;
    return true;
  }
  return dw_common_addr_to_netadr_hook.invoke<bool>(netadr, common_addr,
                                                    sec_id);
}

int32_t dw_get_connection_status_stub(const game::net::netadr_t *netadr) {
  constexpr int32_t CONNECTED = 2;
  return is_lobby_peer(*netadr)
             ? CONNECTED
             : dw_get_connection_status_hook.invoke<int32_t>(netadr);
}

game::lobby::session::HostInfo *lobby_host_info(
    const game::lobby::LobbyType type = game::lobby::LobbyType::PRIVATE) {
  if (game::is_legacy_client() || !game::lobby::LobbyHost_IsHost(type))
    return nullptr;
  game::lobby::session::LobbySession *session =
      game::lobby::LobbyHostData_GetSession(type);
  return session ? &session->host.info : nullptr;
}

bool hosting_match() {
  return getinfo::is_host() && !game::com::Com_IsRunningUILevel();
}

bool can_host() {
  return hosting_match() || lobby_host_info() ||
         lobby_host_info(game::lobby::LobbyType::GAME);
}

void show_error(const std::string &message) {
  game::ui::UI_OpenErrorPopupWithMessage(game::LOCAL_CLIENT_0,
                                         game::errorCode::UI, message.c_str());
}

void connect_to(const game::net::netadr_t &endpoint) {
  const std::string address = network::address_to_string(endpoint);
  if (address.empty())
    return;
  party::connect(endpoint);
}

void request_lobby(const game::net::netadr_t &host, const std::string &token) {
  const game::lobby::session::HostInfo *own = lobby_host_info();
  lobby_joining = {true, token, host, std::chrono::steady_clock::now() + 4s};
  network::send(host, "lobbyJoin",
                token + " " +
                    (own ? to_hex(own->serializedAdr.addrBuff,
                                  sizeof(own->serializedAdr.addrBuff))
                         : "-"));
}

void complete_join(const game::net::netadr_t &endpoint) {
  if (!joining.active)
    return;
  joining.active = false;
  request_lobby(endpoint, joining.token);
}

void show_join_failure(const uint16_t port) {
  show_error(std::format(
      "Could not reach your friend.\n\nAsk your friend to restart their game, "
      "then try again. If it keeps failing, the host can enable UPnP on their "
      "router or forward UDP port {} to their PC, and both of you should allow "
      "BOIII through Windows Firewall. Swapping who hosts also works.",
      port ? port : local_port()));
}

void lobby_join_complete(int32_t, const game::lobby::JoinResult result) {
  using game::lobby::JoinResult;
  std::string message;
  switch (result) {
  case JoinResult::SUCCESS:
    return;
  case JoinResult::LOBBY_FULL:
  case JoinResult::COULD_NOT_RESERVE:
    message = "Your friend's lobby is full.";
    break;
  case JoinResult::NOT_JOINABLE_NOT_IDLE:
  case JoinResult::MIGRATE_IN_PROGRESS:
    message = "Your friend's lobby is busy starting or changing a match. Try "
              "again in a few seconds.";
    break;
  case JoinResult::JOIN_DISABLED:
  case JoinResult::NOT_JOINABLE_NOT_HOSTING:
  case JoinResult::NOT_JOINABLE_CLOSED:
  case JoinResult::NOT_JOINABLE_INVITE_ONLY:
  case JoinResult::NOT_JOINABLE_FRIENDS_ONLY:
    message = "Your friend's party is closed. Ask them to open it to friends.";
    break;
  case JoinResult::NETWORK_MODE_MISMATCH:
    message = "One of you is in LAN or offline mode. Both of you need to be "
              "in online play.";
    break;
  case JoinResult::MISMATCH_PROTOCOL_VERSION:
  case JoinResult::MISMATCH_NETFIELD_CHECKSUM:
  case JoinResult::MISMATCH_FFOTD_VERSION_TO_NEW:
  case JoinResult::MISMATCH_FFOTD_VERSION_TO_OLD:
  case JoinResult::MISMATCH_PLAYLISTID:
  case JoinResult::MISMATCH_PLAYLIST_VERSION_TO_NEW:
  case JoinResult::MISMATCH_PLAYLIST_VERSION_TO_OLD:
    message = "Your game doesn't match your friend's. Both of you should "
              "update BOIII to the same version.";
    break;
  case JoinResult::JOIN_ALREADY_IN_PROGRESS:
    message = "Already joining a lobby. Please wait.";
    break;
  default:
    message = std::format(
        "Your friend's lobby stopped responding while joining (code {}). Try "
        "again. If it keeps failing, a firewall is blocking UDP traffic: "
        "allow BOIII through Windows Firewall on both PCs.",
        static_cast<uint32_t>(result));
    break;
  }
  show_error(message);
}

void join_lobby(const game::net::netadr_t &host,
                const std::vector<std::string> &parts) {
  game::lobby::session::HostInfo info{};
  info.xuid = std::strtoull(parts[1].c_str(), nullptr, 16);
  info.serializedAdr.valid = 1;
  if (!info.xuid ||
      !from_hex(parts[2], info.serializedAdr.addrBuff,
                sizeof(info.serializedAdr.addrBuff)) ||
      !from_hex(parts[3], &info.secId, sizeof(info.secId)) ||
      !from_hex(parts[4], &info.secKey, sizeof(info.secKey)))
    return;

  const game::lobby::LobbyType target = parts[5] == "game"
                                            ? game::lobby::LobbyType::GAME
                                            : game::lobby::LobbyType::PRIVATE;
  std::vector<std::string> content{"", "", ""};
  size_t name_index = 6;
  if (parts[6].starts_with("c:")) {
    content = utils::string::split(parts[6].substr(2), ':');
    content.resize(3);
    name_index = 7;
  }
  std::string name;
  for (size_t i = name_index; i < parts.size(); ++i)
    name.append(name.empty() ? "" : " ").append(parts[i]);
  utils::string::copy(info.name, name.c_str());

  if (game::com::Com_IsInGame() || !lobby_host_info()) {
    show_error("Leave your current match to join your friend's lobby.");
    return;
  }

  const std::string &mod = content[0];
  const std::string &map = content[1];
  if (!workshop::check_valid_mod_id(mod, {}) ||
      (!map.empty() &&
       !workshop::check_valid_usermap_id(map, content[2], {}, {}))) {
    if (joining.friend_id) {
      friends::remember_browser_route(joining.friend_id, FRIEND_REJOIN_ROUTE);
      workshop::set_pending_download_reconnect(FRIEND_REJOIN_ROUTE);
    }
    return;
  }

  const std::string loaded_mod = game::ugc::UGC_ActiveMod_PublisherId();
  if (mod != (loaded_mod == "usermaps" ? std::string{} : loaded_mod)) {
    workshop::setup_same_mod_as_host(game::LOCAL_CLIENT_0, {}, mod);
    scheduler::once(
        [friend_id = joining.friend_id] {
          friends::connect_to_friend(friend_id);
        },
        scheduler::main, 3s);
    return;
  }

  add_peer(info.serializedAdr.addrBuff, host);
  using game::lobby::LobbyType;
  if (!game::lobby::LobbyJoin_Begin(
          0,
          game::com::Com_LocalClient_GetControllerIndex(game::LOCAL_CLIENT_0),
          LobbyType::PRIVATE, target, lobby_join_complete)) {
    show_error("Already joining a lobby. Please wait.");
    return;
  }
  game::lobby::LobbyJoin_Add(info.xuid, info.name, &info.secId, &info.secKey,
                             &info.serializedAdr, game::lobby::JoinType::FRIEND,
                             0);
  game::lobby::LobbyJoin_Finalize();
}

void update_punching() {
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();

  if (joining.active) {
    if (now >= joining.expires && !joining.retried) {
      printf("[Friends] Could not reach your friend yet, retrying\n");
      joining.retried = true;
      joining.candidates.clear();
      joining.expires = now + JOIN_TIMEOUT;
      joining.next_rendezvous = now;
    } else if (now >= joining.expires) {
      const game::net::netadr_t fallback =
          network::address_from_string(joining.fallback);
      joining.active = false;
      if (joining.candidates.empty())
        printf("[Friends] The master server did not send your friend's "
               "address\n");
      else
        printf("[Friends] Could not reach your friend directly (%zu "
               "addresses tried)\n",
               joining.candidates.size());
      if (network::is_connectable_address(fallback))
        request_lobby(fallback, joining.token);
      else
        show_join_failure(fallback.port);
    } else {
      if (joining.candidates.empty() && now >= joining.next_rendezvous) {
        send_rendezvous("privJoin", joining.token);
        joining.next_rendezvous = now + 1s;
      }
      for (const game::net::netadr_t &candidate : joining.candidates)
        network::send(candidate, "punch", joining.token);
    }
  }

  if (lobby_joining.active && now >= lobby_joining.expires) {
    lobby_joining.active = false;
    printf("[Friends] Your friend's game did not answer\n");
    show_join_failure(lobby_joining.host.port);
  }

  std::erase_if(host_probes, [now](const host_probe &probe) {
    return now >= probe.expires;
  });
  for (const host_probe &probe : host_probes)
    network::send(probe.candidate, "punch", probe.token);
}

void refresh_lobby_buttons() {
  if (const uint16_t global = game::ui::UI_Model_GetGlobalModel()) {
    if (const uint16_t model = game::ui::UI_Model_CreateModelFromPath(
            global, "lobbyRoot.lobbyButtonUpdate"))
      game::ui::UI_Model_ForceNotify(model);
  }
}

void set_open(const bool enabled) {
  open_to_friends = enabled;
  game::Dvar_SetFromStringByName("nat_open", enabled ? "1" : "0", true);
  game::Dvar_SetFromStringByName("friends_open", enabled ? "1" : "0", true);
  refresh_lobby_buttons();
  game::Dvar_SetFromStringByName("com_pauseSupported", enabled ? "0" : "1",
                                 true);
  if (enabled)
    upnp::open_port(local_port());
  else
    upnp::close_port();
  if (!enabled) {
    registration_confirmed = false;
    identity_warning_shown = false;
    hosting_token.clear();
    reflected_endpoint.clear();
    host_probes.clear();
  }
}

void update_hosting() {
  if (!open_to_friends) {
    if (const game::XUID friend_code = auth::get_guid())
      send_rendezvous("friendPublish",
                      utils::string::va("1 %llu -", friend_code));
    return;
  }
  if (!can_host())
    return;

  if (hosting_token.empty()) {
    hosting_token = new_token();
    registration_confirmed = false;
    identity_warning_shown = false;
  }
  send_rendezvous("privRegister", hosting_token);
  send_friend_publish();
}

void receive_lobby_join(const game::net::netadr_t &sender,
                        const network::data_view &data,
                        game::LocalClientNum_t) {
  const std::vector<std::string> parts = fields(payload_string(data));
  if (parts.size() != 2 || hosting_token.empty() || parts[0] != hosting_token ||
      !network::is_connectable_address(sender))
    return;

  if (hosting_match()) {
    network::send(sender, "lobbyGame", hosting_token);
    return;
  }

  const game::lobby::session::HostInfo *game_lobby =
      lobby_host_info(game::lobby::LobbyType::GAME);
  const game::lobby::session::HostInfo *host =
      game_lobby ? game_lobby : lobby_host_info();
  game::net::XNADDR peer{};
  if (!host || !from_hex(parts[1], peer.addrBuff, sizeof(peer.addrBuff)))
    return;

  add_peer(peer.addrBuff, sender);
  const std::string mod = workshop::get_mod_publisher_id();
  const std::string map(game::get_dvar_string("ui_mapname").value_or(""));
  network::send(sender, "lobbyHost",
                std::format("{} {:x} {} {} {} {} c:{}:{}:{} {}", hosting_token,
                            host->xuid,
                            to_hex(host->serializedAdr.addrBuff,
                                   sizeof(host->serializedAdr.addrBuff)),
                            to_hex(&host->secId, sizeof(host->secId)),
                            to_hex(&host->secKey, sizeof(host->secKey)),
                            game_lobby ? "game" : "party",
                            mod == "usermaps" ? std::string{} : mod, map,
                            workshop::get_usermap_publisher_id(map),
                            host->name));
}

bool from_lobby_request(const game::net::netadr_t &sender,
                        const std::string &token) {
  if (!lobby_joining.active || token != lobby_joining.token ||
      !network::is_connectable_address(sender))
    return false;
  lobby_joining.active = false;
  return true;
}

void receive_lobby_host(const game::net::netadr_t &sender,
                        const network::data_view &data,
                        game::LocalClientNum_t) {
  const std::vector<std::string> parts = fields(payload_string(data));
  if (parts.size() >= 7 && from_lobby_request(sender, parts[0]))
    scheduler::once([sender, parts] { join_lobby(sender, parts); },
                    scheduler::main);
}

void receive_lobby_game(const game::net::netadr_t &sender,
                        const network::data_view &data,
                        game::LocalClientNum_t) {
  if (from_lobby_request(sender, payload_string(data)))
    connect_to(sender);
}

void receive_register_ack(const game::net::netadr_t &sender,
                          const network::data_view &data,
                          game::LocalClientNum_t) {
  if (!is_rendezvous(sender) || hosting_token.empty())
    return;

  const game::net::netadr_t endpoint =
      network::address_from_string(payload_string(data));
  if (network::is_connectable_address(endpoint)) {
    reflected_endpoint = network::address_to_string(endpoint);
    registration_confirmed = true;
  }
}

void receive_peer(const game::net::netadr_t &sender,
                  const network::data_view &data, game::LocalClientNum_t) {
  if (!is_rendezvous(sender))
    return;

  const std::vector<std::string> parts = fields(payload_string(data));
  if (parts.size() < 2 || !valid_token(parts.front()))
    return;

  const std::basic_string<char> &token = parts.front();
  if (joining.active && token == joining.token) {
    for (size_t i = 1; i < parts.size(); ++i)
      add_join_candidate(parts[i]);
    return;
  }

  if (token != hosting_token)
    return;

  for (size_t i = 1; i < parts.size() && host_probes.size() < MAX_CANDIDATES;
       ++i) {
    const game::net::netadr_t candidate =
        network::address_from_string(parts[i]);
    if (!network::is_connectable_address(candidate))
      continue;
    const bool duplicate = std::any_of(host_probes.begin(), host_probes.end(),
                                       [&candidate](const host_probe &probe) {
                                         return network::are_addresses_equal(
                                             probe.candidate, candidate);
                                       });
    if (!duplicate)
      host_probes.push_back(
          {token, candidate, std::chrono::steady_clock::now() + 10s});
  }
}

void receive_rejection(const game::net::netadr_t &sender,
                       const network::data_view &, game::LocalClientNum_t) {
  if (!is_rendezvous(sender) || !joining.active)
    return;
  printf("[Friends] Your friend's party is no longer open on the master "
         "server\n");
  joining.expires = std::chrono::steady_clock::now();
}

void receive_punch(const game::net::netadr_t &sender,
                   const network::data_view &data, game::LocalClientNum_t) {
  const std::string token = payload_string(data);
  const bool valid_join = joining.active && token == joining.token;
  const bool valid_host = !hosting_token.empty() && token == hosting_token;
  if ((!valid_join && !valid_host) || !network::is_connectable_address(sender))
    return;

  network::send(sender, "punchAck", token);
  if (valid_join)
    complete_join(sender);
}

void receive_punch_ack(const game::net::netadr_t &sender,
                       const network::data_view &data, game::LocalClientNum_t) {
  const std::string token = payload_string(data);
  if (joining.active && token == joining.token &&
      network::is_connectable_address(sender))
    complete_join(sender);
}

void receive_friend_presence(const game::net::netadr_t &sender,
                             const network::data_view &data,
                             game::LocalClientNum_t) {
  if (!is_rendezvous(sender))
    return;
  const std::vector<std::string> parts = fields(payload_string(data));
  if (parts.size() != 5 || parts[0] != "1" || !valid_token(parts[3]))
    return;
  const game::XUID steam_id = std::strtoull(parts[2].c_str(), nullptr, 10);
  const game::net::netadr_t endpoint = network::address_from_string(parts[4]);
  bool complete{};
  {
    std::lock_guard lock(lookup_mutex);
    const lookupMap_t::iterator request = lookup_requests.find(parts[1]);
    if (request == lookup_requests.end() ||
        !request->second.pending.erase(steam_id)) {
      return;
    }
    request->second.results.push_back(
        {steam_id, parts[3],
         network::is_connectable_address(endpoint)
             ? network::address_to_string(endpoint)
             : std::string{}});
    complete = request->second.pending.empty();
  }
  if (complete) {
    scheduler::once([request = parts[1]] { finish_lookup(request); },
                    scheduler::main);
  }
}

void receive_friend_status(const game::net::netadr_t &sender,
                           const network::data_view &data, const bool online) {
  if (!is_rendezvous(sender))
    return;
  const std::vector<std::string> parts = fields(payload_string(data));
  if (parts.size() != 3 || parts[0] != "1")
    return;
  const game::XUID steam_id = std::strtoull(parts[2].c_str(), nullptr, 10);
  bool complete{};
  {
    std::lock_guard lock(lookup_mutex);
    const lookupMap_t::iterator request = lookup_requests.find(parts[1]);
    if (request == lookup_requests.end() ||
        !request->second.pending.erase(steam_id)) {
      return;
    }
    if (online)
      request->second.results.push_back({steam_id, {}, {}});
    complete = request->second.pending.empty();
  }
  if (complete) {
    scheduler::once([request = parts[1]] { finish_lookup(request); },
                    scheduler::main);
  }
}

void receive_friend_offline(const game::net::netadr_t &sender,
                            const network::data_view &data,
                            game::LocalClientNum_t) {
  receive_friend_status(sender, data, false);
}

void receive_friend_online(const game::net::netadr_t &sender,
                           const network::data_view &data,
                           game::LocalClientNum_t) {
  receive_friend_status(sender, data, true);
}
} // namespace

std::string get_host_token() {
  return registration_confirmed ? hosting_token : std::string{};
}

std::string get_host_endpoint() {
  if (const std::string mapped = upnp::external_endpoint(); !mapped.empty())
    return mapped;
  if (hosting_token.empty() || !registration_confirmed)
    return {};
  if (!reflected_endpoint.empty())
    return reflected_endpoint;
  return local_endpoint();
}

void begin_join(const std::string &token, const std::string &fallback_address,
                const uint64_t friend_id) {
  if (!valid_token(token)) {
    show_error(
        "Your friend's party is closed. Ask them to open it to friends.");
    return;
  }

  if (joining.active && joining.token == token)
    return;

  joining = {};
  joining.active = true;
  joining.friend_id = friend_id;
  joining.token = token;
  const game::net::netadr_t fallback =
      network::address_from_string(fallback_address);
  if (network::is_connectable_address(fallback))
    joining.fallback = network::address_to_string(fallback);
  joining.expires = std::chrono::steady_clock::now() + JOIN_TIMEOUT;
  joining.next_rendezvous = std::chrono::steady_clock::now() + 1s;
  send_rendezvous("privJoin", token);
  toast::show("Friends", "Joining your friend...", "t7_icon_connect_overlays");
}

bool is_lobby_peer(const game::net::netadr_t &address) {
  std::lock_guard lock(peers_mutex);
  return std::ranges::any_of(
      lobby_peers, [&address](lobbyPeerMap_t::value_type &peer) {
        return network::are_addresses_equal(peer.second, address);
      });
}

bool is_rendezvous(const game::net::netadr_t &sender) {
  const std::vector<game::net::netadr_t> &masters =
      server_list::get_master_servers();
  return std::ranges::any_of(
      masters, [&sender](const game::net::netadr_t &master) {
        return sender.addr == master.addr && sender.port == master.port;
      });
}

void send_invite(const uint64_t steam_id, const std::string &sender_name) {
  const game::XUID friend_code = auth::get_guid();
  if (!friend_code || !steam_id)
    return;
  const std::string payload =
      std::format("1 {} {} {}", steam_id, friend_code, sender_name);
  for (const game::net::netadr_t &master : server_list::get_master_servers())
    network::send(master, "friendInvite", payload);
}

bool set_open_to_friends(const bool enabled) {
  if (enabled && !can_host())
    return false;
  set_open(enabled);
  update_hosting();
  return true;
}

void refresh_friends(const std::vector<game::XUID> &steam_ids,
                     lookup_callback callback) {
  std::unordered_set<game::XUID> pending;
  for (const game::XUID steam_id : steam_ids) {
    if (steam_id)
      pending.insert(steam_id);
  }
  if (pending.empty()) {
    scheduler::once(
        [done = std::move(callback)]() mutable {
          if (done)
            done(std::vector<friend_presence>{});
        },
        scheduler::main);
    return;
  }

  std::string request;
  {
    std::lock_guard lock(lookup_mutex);
    do {
      request = new_token();
    } while (lookup_requests.contains(request));
    lookup_requests.emplace(request,
                            lookup_state{pending, {}, std::move(callback)});
  }
  for (const game::XUID steam_id : pending)
    send_rendezvous("friendLookup",
                    utils::string::va("1 %s %llu", request.c_str(), steam_id));

  scheduler::once([request] { finish_lookup(request); }, scheduler::main, 2s);
}

class component final : public client_component {
  DEFINE_COMPONENT_NAME("nat");

public:
  void post_unpack() override {
    scheduler::once(
        [] {
          game::register_dvar_bool("nat_open", false, game::DVAR_NONE,
                                   "Allow friends to join this match");
        },
        scheduler::main);

    network::on("privRegisterAck", receive_register_ack);
    network::on("privPeer", receive_peer);
    network::on("privReject", receive_rejection);
    network::on("punch", receive_punch);
    network::on("punchAck", receive_punch_ack);
    network::on("friendPresence", receive_friend_presence);
    network::on("friendOffline", receive_friend_offline);
    network::on("friendOnline", receive_friend_online);
    network::on("lobbyJoin", receive_lobby_join);
    network::on("lobbyHost", receive_lobby_host);
    network::on("lobbyGame", receive_lobby_game);

    if (!game::is_legacy_client()) {
      dw_common_addr_to_netadr_hook.create(game::dw::dwCommonAddrToNetadr.get(),
                                           dw_common_addr_to_netadr_stub);
      dw_get_connection_status_hook.create(
          game::dw::dwGetConnectionStatus.get(), dw_get_connection_status_stub);
    }

    scheduler::loop(update_punching, scheduler::main, 250ms);
    scheduler::loop(update_hosting, scheduler::main, REGISTER_INTERVAL);
  }

  void pre_destroy() override {
    std::lock_guard lock(lookup_mutex);
    lookup_requests.clear();
  }
};
} // namespace nat

REGISTER_COMPONENT(nat::component)
