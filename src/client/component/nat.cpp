#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"

#include "getinfo.hpp"
#include "auth.hpp"
#include "nat.hpp"
#include "network.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"
#include "toast.hpp"
#include "upnp.hpp"

#include <utils/string.hpp>

#include <random>
#include <sstream>

namespace nat {
namespace {
constexpr auto REGISTER_INTERVAL = 5s;
constexpr auto JOIN_TIMEOUT = 12s;
constexpr size_t MAX_CANDIDATES = 16;

bool open_to_friends{};
bool registration_confirmed{};
bool identity_warning_shown{};
std::string hosting_token;
std::string reflected_endpoint;

struct join_attempt {
  bool active{};
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

std::mutex lookup_mutex;
uint64_t lookup_generation{};
std::string lookup_request;
std::unordered_set<uint64_t> lookup_pending;
std::vector<friend_presence> lookup_results;
lookup_callback lookup_done;

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
  const auto port = party::get_local_port();
  return port >= 1024 ? port : 3074;
}

std::string local_endpoint() {
  static const std::string local_ip = [] {
    const auto socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

bool from_rendezvous(const game::net::netadr_t &sender) {
  const auto masters = server_list::get_master_servers();
  return std::ranges::any_of(masters, [&sender](const auto &master) {
    return sender.addr == master.addr && sender.port == master.port;
  });
}

void send_rendezvous(const std::string &command, const std::string &token) {
  const auto masters = server_list::get_master_servers();
  if (masters.empty())
    return;

  auto message = token;
  if (const auto local = local_endpoint(); !local.empty())
    message.append(" ").append(local);
  for (const auto &master : masters)
    network::send(master, command, message);
}

void send_friend_publish() {
  const auto friend_code = auth::get_guid();
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
  const auto payload =
      utils::string::va("1 %llu %s", friend_code, hosting_token.c_str());
  send_rendezvous("friendPublish", payload);
}

void add_join_candidate(const std::string &text) {
  if (joining.candidates.size() >= MAX_CANDIDATES)
    return;

  const auto candidate = network::address_from_string(text);
  if (!network::is_connectable_address(candidate))
    return;

  const auto duplicate =
      std::any_of(joining.candidates.begin(), joining.candidates.end(),
                  [&candidate](const game::net::netadr_t &existing) {
                    return network::are_addresses_equal(existing, candidate);
                  });
  if (!duplicate)
    joining.candidates.push_back(candidate);
}

void connect_to(const game::net::netadr_t &endpoint) {
  const auto address = network::address_to_string(endpoint);
  if (address.empty())
    return;
  party::connect(endpoint);
}

void complete_join(const game::net::netadr_t &endpoint) {
  if (!joining.active)
    return;
  joining.active = false;
  connect_to(endpoint);
}

void show_join_failure() {
  game::ui::UI_OpenErrorPopupWithMessage(
      0, game::errorCode::UI,
      "The friend could not be reached. Their match may have closed, or one "
      "of the networks may block UDP hole punching.");
}

void update_punching() {
  const auto now = std::chrono::steady_clock::now();

  if (joining.active) {
    if (now >= joining.expires) {
      const auto fallback = network::address_from_string(joining.fallback);
      joining.active = false;
      if (network::is_connectable_address(fallback))
        connect_to(fallback);
      else
        show_join_failure();
    } else {
      if (joining.candidates.empty() && now >= joining.next_rendezvous) {
        send_rendezvous("privJoin", joining.token);
        joining.next_rendezvous = now + 1s;
      }
      for (const auto &candidate : joining.candidates)
        network::send(candidate, "punch", joining.token);
    }
  }

  std::erase_if(host_probes, [now](const host_probe &probe) {
    return now >= probe.expires;
  });
  for (const auto &probe : host_probes)
    network::send(probe.candidate, "punch", probe.token);
}

void set_open(const bool enabled) {
  open_to_friends = enabled;
  game::Dvar_SetFromStringByName("nat_open", enabled ? "1" : "0", true);
  game::Dvar_SetFromStringByName("friends_open", enabled ? "1" : "0", true);
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
  if (!open_to_friends || !getinfo::is_host()) {
    if (open_to_friends || !hosting_token.empty())
      set_open(false);
    return;
  }
  if (hosting_token.empty()) {
    hosting_token = new_token();
    registration_confirmed = false;
    identity_warning_shown = false;
  }
  send_rendezvous("privRegister", hosting_token);
  send_friend_publish();
}

void receive_register_ack(const game::net::netadr_t &sender,
                          const network::data_view &data,
                          game::LocalClientNum_t) {
  if (!from_rendezvous(sender) || hosting_token.empty())
    return;

  const auto endpoint = network::address_from_string(payload_string(data));
  if (network::is_connectable_address(endpoint)) {
    reflected_endpoint = network::address_to_string(endpoint);
    registration_confirmed = true;
  }
}

void receive_peer(const game::net::netadr_t &sender,
                  const network::data_view &data, game::LocalClientNum_t) {
  if (!from_rendezvous(sender))
    return;

  const auto parts = fields(payload_string(data));
  if (parts.size() < 2 || !valid_token(parts.front()))
    return;

  const auto &token = parts.front();
  if (joining.active && token == joining.token) {
    for (size_t i = 1; i < parts.size(); ++i)
      add_join_candidate(parts[i]);
    return;
  }

  if (token != hosting_token)
    return;

  for (size_t i = 1; i < parts.size() && host_probes.size() < MAX_CANDIDATES;
       ++i) {
    const auto candidate = network::address_from_string(parts[i]);
    if (!network::is_connectable_address(candidate))
      continue;
    const auto duplicate = std::any_of(host_probes.begin(), host_probes.end(),
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
  if (!from_rendezvous(sender) || !joining.active)
    return;
  joining.expires = std::chrono::steady_clock::now();
}

void receive_punch(const game::net::netadr_t &sender,
                   const network::data_view &data, game::LocalClientNum_t) {
  const auto token = payload_string(data);
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
  const auto token = payload_string(data);
  if (joining.active && token == joining.token &&
      network::is_connectable_address(sender))
    complete_join(sender);
}

void receive_friend_presence(const game::net::netadr_t &sender,
                             const network::data_view &data,
                             game::LocalClientNum_t) {
  if (!from_rendezvous(sender))
    return;
  const auto parts = fields(payload_string(data));
  if (parts.size() != 5 || parts[0] != "1" || !valid_token(parts[3]))
    return;
  const auto steam_id = std::strtoull(parts[2].c_str(), nullptr, 10);
  const auto endpoint = network::address_from_string(parts[4]);
  std::lock_guard lock(lookup_mutex);
  if (parts[1] != lookup_request || !lookup_pending.contains(steam_id))
    return;
  lookup_pending.erase(steam_id);
  lookup_results.push_back({steam_id, parts[3],
                            network::is_connectable_address(endpoint)
                                ? network::address_to_string(endpoint)
                                : std::string{}});
}

void receive_friend_offline(const game::net::netadr_t &sender,
                            const network::data_view &data,
                            game::LocalClientNum_t) {
  if (!from_rendezvous(sender))
    return;
  const auto parts = fields(payload_string(data));
  if (parts.size() != 3 || parts[0] != "1")
    return;
  const auto steam_id = std::strtoull(parts[2].c_str(), nullptr, 10);
  std::lock_guard lock(lookup_mutex);
  if (parts[1] == lookup_request) {
    lookup_pending.erase(steam_id);
  }
}
} // namespace

std::string get_host_token() {
  return registration_confirmed ? hosting_token : std::string{};
}

std::string get_host_endpoint() {
  if (const auto mapped = upnp::external_endpoint(); !mapped.empty())
    return mapped;
  if (hosting_token.empty() || !registration_confirmed)
    return {};
  if (!reflected_endpoint.empty())
    return reflected_endpoint;
  return local_endpoint();
}

void begin_join(const std::string &token, const std::string &fallback_address) {
  if (!valid_token(token)) {
    show_join_failure();
    return;
  }

  if (joining.active && joining.token == token)
    return;

  joining = {};
  joining.active = true;
  joining.token = token;
  const auto fallback = network::address_from_string(fallback_address);
  if (network::is_connectable_address(fallback))
    joining.fallback = network::address_to_string(fallback);
  joining.expires = std::chrono::steady_clock::now() + JOIN_TIMEOUT;
  joining.next_rendezvous = std::chrono::steady_clock::now() + 1s;
  send_rendezvous("privJoin", token);
}

bool set_open_to_friends(const bool enabled) {
  if (enabled && !getinfo::is_host())
    return false;
  set_open(enabled);
  update_hosting();
  return true;
}

void refresh_friends(const std::vector<uint64_t> &steam_ids,
                     lookup_callback callback) {
  const auto generation = ++lookup_generation;
  const auto request = new_token();
  {
    std::lock_guard lock(lookup_mutex);
    lookup_request = request;
    lookup_pending = {steam_ids.begin(), steam_ids.end()};
    lookup_results.clear();
    lookup_done = std::move(callback);
  }
  for (const auto steam_id : steam_ids)
    send_rendezvous("friendLookup",
                    utils::string::va("1 %s %llu", request.c_str(), steam_id));

  scheduler::once(
      [generation] {
        lookup_callback done;
        std::vector<friend_presence> results;
        {
          std::lock_guard lock(lookup_mutex);
          if (generation != lookup_generation)
            return;
          results = lookup_results;
          done = std::move(lookup_done);
          lookup_pending.clear();
        }
        if (done)
          done(std::move(results));
      },
      scheduler::main, 2s);
}

class component final : public client_component {
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

    scheduler::loop(update_punching, scheduler::main, 250ms);
    scheduler::loop(update_hosting, scheduler::main, REGISTER_INTERVAL);
  }
};
} // namespace nat

REGISTER_COMPONENT(nat::component)
