#include <std_include.hpp>
#include "../steam.hpp"

#include <game/game.hpp>

#include "component/party.hpp"
#include "component/network.hpp"
#include "component/nat.hpp"
#include "component/server_list.hpp"
#include "component/friends.hpp"
#include "component/steam_proxy.hpp"
#include "component/scheduler.hpp"

#include <utils/string.hpp>
#include <utils/concurrency.hpp>

namespace steam {
namespace {
struct server {
  bool handled{false};
  game::net::netadr_t address{};
  gameserveritem_t server_item{};
};

auto *const internet_request = reinterpret_cast<void *>(1);
auto *const favorites_request = reinterpret_cast<void *>(4);
auto *const history_request = reinterpret_cast<void *>(5);
auto *const friends_request = reinterpret_cast<void *>(3);

using servers = std::vector<server>;

::utils::concurrency::container<servers> internet_servers{};
::utils::concurrency::container<servers> favorites_servers{};
::utils::concurrency::container<servers> history_servers{};
::utils::concurrency::container<servers> friends_servers{};
std::atomic<matchmaking_server_list_response *> internet_response{};
std::atomic<matchmaking_server_list_response *> favorites_response{};
std::atomic<matchmaking_server_list_response *> history_response{};
std::atomic<matchmaking_server_list_response *> friends_response{};
std::atomic<bool> internet_refreshing{false};
std::atomic<bool> friends_refreshing{false};

struct friend_scan_result {
  game::net::netadr_t address{};
  gameserveritem_t item{};
};
std::mutex friend_scan_mutex;
uint64_t friend_scan_generation{};
std::unordered_map<uint64_t, friend_scan_result> friend_scan_results;

template <typename T> void copy_safe(T &dest, const char *in) {
  ::utils::string::copy(dest, in);
  ::utils::string::strip_material(dest, dest, std::extent_v<T>);
}

gameserveritem_t create_server_item(const game::net::netadr_t &address,
                                    const ::utils::info_string &info,
                                    const uint32_t ping, const bool success) {
  const auto sub_protocol = atoi(info.get("sub_protocol").data());

  gameserveritem_t server{};
  server.m_NetAdr.m_usConnectionPort = address.port;
  server.m_NetAdr.m_usQueryPort = address.port;
  server.m_NetAdr.m_unIP = ntohl(address.addr);
  server.m_nPing = static_cast<int>(ping);
  server.m_bHadSuccessfulResponse = success;
  server.m_bDoNotRefresh = false;

  copy_safe(server.m_szGameDir, "");
  copy_safe(server.m_szMap, info.get("mapname").data());
  copy_safe(server.m_szGameDescription, info.get("description").data());

  server.m_nAppID =
      (sub_protocol == SUB_PROTOCOL || sub_protocol == (SUB_PROTOCOL - 1))
          ? 311210
          : 0;
  server.m_nPlayers = atoi(info.get("clients").data());
  server.m_nMaxPlayers = atoi(info.get("sv_maxclients").data());
  server.m_nBotPlayers = atoi(info.get("bots").data());
  server.m_bPassword = info.get("isPrivate") == "1";
  server.m_bSecure = true;
  server.m_ulTimeLastPlayed = 0;
  server.m_nServerVersion = 1000;

  copy_safe(server.m_szServerName, info.get("hostname").data());

  const auto playmode = info.get("playmode");
  const auto mode = playmode.empty()
                        ? std::optional<game::eModes>{}
                        : static_cast<game::eModes>(std::atoi(playmode.data()));

  const auto *tags = ::utils::string::va(
      R"(\gametype\%s\dedicated\%s\ranked\false\hardcore\%s\zombies\%s\campaign\%s\playerCount\%d\bots\%d\rounds\%d\modName\%s\)",
      info.get("gametype").data(),
      info.get("dedicated") == "1" ? "true" : "false",
      info.get("hc") == "1" ? "true" : "false",
      mode.has_value() && *mode == game::eModes::ZOMBIES ? "true" : "false",
      mode.has_value() && *mode == game::eModes::CAMPAIGN ? "true" : "false",
      server.m_nPlayers, atoi(info.get("bots").data()),
      atoi(info.get("rounds_played").data()), info.get("modName").data());

  copy_safe(server.m_szGameTags, tags);

  server.m_steamID.bits = strtoull(info.get("xuid").data(), nullptr, 16);

  return server;
}

void handle_server_respone(
    const bool success, const game::net::netadr_t &host,
    const ::utils::info_string &info, const uint32_t ping,
    ::utils::concurrency::container<servers> &server_list,
    std::atomic<matchmaking_server_list_response *> &response, void *request) {
  bool all_handled = false;
  std::optional<int> index{};
  server_list.access([&](servers &srvs) {
    size_t i = 0;
    for (; i < srvs.size(); ++i) {
      if (srvs[i].address == host) {
        break;
      }
    }

    if (i >= srvs.size()) {
      return;
    }

    index = static_cast<int>(i);

    auto &srv = srvs[i];
    srv.handled = true;
    srv.server_item = create_server_item(host, info, ping, success);

    for (const auto &entry : srvs) {
      if (!entry.handled) {
        return;
      }
    }

    all_handled = true;
  });

  const auto res = response.load();
  if (!index || !res) {
    return;
  }

  if (success) {
    res->ServerResponded(request, *index);
  } else {
    res->ServerFailedToRespond(request, *index);
  }

  if (all_handled) {
    if (request == internet_request) {
      internet_refreshing = false;
    }
    res->RefreshComplete(request, eServerResponded);
  }
}

void handle_internet_server_response(const bool success,
                                     const game::net::netadr_t &host,
                                     const ::utils::info_string &info,
                                     const uint32_t ping) {
  handle_server_respone(success, host, info, ping, internet_servers,
                        internet_response, internet_request);
}

void handle_favorites_server_response(const bool success,
                                      const game::net::netadr_t &host,
                                      const ::utils::info_string &info,
                                      const uint32_t ping) {
  handle_server_respone(success, host, info, ping, favorites_servers,
                        favorites_response, favorites_request);
}

void handle_history_server_response(const bool success,
                                    const game::net::netadr_t &host,
                                    const ::utils::info_string &info,
                                    const uint32_t ping) {
  handle_server_respone(success, host, info, ping, history_servers,
                        history_response, history_request);
}

void ping_server(const game::net::netadr_t &server,
                 party::query_callback callback) {
  party::query_server(server, callback);
}

void finish_friend_scan(const uint64_t generation) {
  std::unordered_map<uint64_t, friend_scan_result> discovered;
  {
    std::lock_guard lock(friend_scan_mutex);
    if (generation != friend_scan_generation)
      return;
    discovered = friend_scan_results;
  }

  const auto entries = ::friends::get_friend_server_addresses();
  ::friends::clear_browser_routes();
  friends_servers.access([&](servers &output) {
    output.clear();
    output.reserve(entries.size());
    for (size_t i = 0; i < entries.size(); ++i) {
      const auto &entry = entries[i];
      server row{};
      const auto live = discovered.find(entry.steam_id);
      if (live != discovered.end()) {
        row.address = live->second.address;
        row.server_item = live->second.item;
        copy_safe(row.server_item.m_szServerName, entry.player_name.c_str());
        copy_safe(row.server_item.m_szGameDescription, "Friend match");
      } else {
        // RFC 5737 TEST-NET-1 is deliberately non-routable. party::connect_stub
        // maps it back to the SteamID before any connection is attempted.
        const auto placeholder = ::utils::string::va(
            "192.0.2.%u:%u", static_cast<unsigned>((i % 250) + 1),
            static_cast<unsigned>(40000 + (i % 20000)));
        row.address = network::address_from_string(placeholder);
        row.server_item = create_server_item(row.address, {}, 0, true);
        row.server_item.m_nAppID = 311210;
        row.server_item.m_steamID.bits = entry.steam_id;
        copy_safe(row.server_item.m_szServerName, entry.player_name.c_str());
        copy_safe(row.server_item.m_szGameDescription, "Offline");
      }
      row.handled = true;
      const auto route = network::address_to_string(row.address);
      ::friends::remember_browser_route(entry.steam_id, route);
      output.push_back(row);
    }
  });

  const auto response = friends_response.load();
  friends_refreshing = false;
  ::friends::notify_presence_changed();
  if (!response)
    return;
  for (int i = 0; i < static_cast<int>(entries.size()); ++i)
    response->ServerResponded(friends_request, i);
  response->RefreshComplete(friends_request,
                            entries.empty() ? eNoServersListedOnMasterServer
                                            : eServerResponded);
}

} // namespace

void *matchmaking_servers::RequestInternetServerList(
    unsigned int iApp, void **ppchFilters, unsigned int nFilters,
    matchmaking_server_list_response *pRequestServersResponse) {
  internet_response = pRequestServersResponse;
  internet_refreshing = true;

  server_list::request_servers(
      [](const bool success, const std::unordered_set<game::net::netadr_t> &s) {
        const auto res = internet_response.load();
        if (!res) {
          internet_refreshing = false;
          return;
        }

        if (!success) {
          internet_refreshing = false;
          res->RefreshComplete(internet_request, eServerFailedToRespond);
          return;
        }

        if (s.empty()) {
          internet_refreshing = false;
          res->RefreshComplete(internet_request,
                               eNoServersListedOnMasterServer);
          return;
        }

        internet_servers.access([&s](servers &srvs) {
          srvs = {};
          srvs.reserve(s.size());

          for (auto &address : s) {
            server new_server{};
            new_server.address = address;
            new_server.server_item = create_server_item(address, {}, 0, false);

            srvs.push_back(new_server);
          }
        });

        for (auto &srv : s) {
          ping_server(srv, handle_internet_server_response);
        }
      });

  return internet_request;
}

void *matchmaking_servers::RequestLANServerList(
    unsigned int iApp,
    matchmaking_server_list_response *pRequestServersResponse) {
  return reinterpret_cast<void *>(2);
}

void *matchmaking_servers::RequestFriendsServerList(
    unsigned int iApp, void **ppchFilters, unsigned int nFilters,
    matchmaking_server_list_response *pRequestServersResponse) {
  friends_response = pRequestServersResponse;
  friends_refreshing = true;

  ::friends::reset_master_presence();
  uint64_t generation{};
  {
    std::lock_guard lock(friend_scan_mutex);
    generation = ++friend_scan_generation;
    friend_scan_results.clear();
  }

  const auto saved = ::friends::get_friends();
  std::vector<uint64_t> ids;
  ids.reserve(saved.size());
  for (const auto &entry : saved)
    ids.push_back(entry.steam_id);

  nat::refresh_friends(ids, [generation](
                                std::vector<nat::friend_presence> live) {
    {
      std::lock_guard lock(friend_scan_mutex);
      if (generation != friend_scan_generation)
        return;
      for (const auto &presence : live) {
        const auto address = network::address_from_string(presence.endpoint);
        if (!network::is_connectable_address(address))
          continue;
        ::friends::set_master_presence(presence.steam_id, presence.endpoint,
                                       presence.token);
        auto item = create_server_item(address, {}, 0, true);
        item.m_nAppID = 311210;
        item.m_steamID.bits = presence.steam_id;
        friend_scan_results[presence.steam_id] = {address, item};
      }
    }
    finish_friend_scan(generation);
  });
  return friends_request;
}

void *matchmaking_servers::RequestFavoritesServerList(
    unsigned int iApp, void **ppchFilters, unsigned int nFilters,
    matchmaking_server_list_response *pRequestServersResponse) {
  favorites_response = pRequestServersResponse;

  auto &srvs = server_list::get_favorite_servers();
  srvs.access([&](std::unordered_set<game::net::netadr_t> s) {
    const auto res = favorites_response.load();
    if (!res) {
      return;
    }

    if (s.empty()) {
      res->RefreshComplete(favorites_request, eNoServersListedOnMasterServer);
      return;
    }

    favorites_servers.access([s](servers &srvs) {
      srvs = {};
      srvs.reserve(s.size());

      for (auto &address : s) {
        server new_server{};
        new_server.address = address;
        new_server.server_item = create_server_item(address, {}, 0, false);

        srvs.push_back(new_server);
      }
    });

    for (auto &srv : s) {
      ping_server(srv, handle_favorites_server_response);
    }
  });

  return favorites_request;
}

void *matchmaking_servers::RequestHistoryServerList(
    unsigned int iApp, void **ppchFilters, unsigned int nFilters,
    matchmaking_server_list_response *pRequestServersResponse) {
  history_response = pRequestServersResponse;

  auto &srvs = server_list::get_recent_servers();
  srvs.access([&](std::vector<game::net::netadr_t> s) {
    const auto res = history_response.load();
    if (!res) {
      return;
    }

    if (s.empty()) {
      res->RefreshComplete(history_request, eNoServersListedOnMasterServer);
      return;
    }

    history_servers.access([&s](servers &srvs) {
      srvs = {};
      srvs.reserve(s.size());

      for (auto &address : s) {
        server new_server{};
        new_server.address = address;
        new_server.server_item = create_server_item(address, {}, 0, false);

        srvs.push_back(new_server);
      }
    });

    for (auto &srv : s) {
      ping_server(srv, handle_history_server_response);
    }
  });

  return history_request;
}

void *matchmaking_servers::RequestSpectatorServerList(
    unsigned int iApp, void **ppchFilters, unsigned int nFilters,
    matchmaking_server_list_response *pRequestServersResponse) {
  return reinterpret_cast<void *>(6);
}

void matchmaking_servers::ReleaseRequest(void *hServerListRequest) {
  if (internet_request == hServerListRequest) {
    internet_response = nullptr;
  }
  if (favorites_request == hServerListRequest) {
    favorites_response = nullptr;
  }
  if (history_request == hServerListRequest) {
    history_response = nullptr;
  }
  if (friends_request == hServerListRequest) {
    friends_response = nullptr;
  }
}

gameserveritem_t *matchmaking_servers::GetServerDetails(void *hRequest,
                                                        int iServer) {
  if (internet_request != hRequest && favorites_request != hRequest &&
      history_request != hRequest && friends_request != hRequest) {
    return nullptr;
  }

  auto &servers_list = hRequest == favorites_request ? favorites_servers
                       : hRequest == history_request ? history_servers
                       : hRequest == friends_request ? friends_servers
                                                     : internet_servers;

  thread_local gameserveritem_t server_item{};
  return servers_list.access<gameserveritem_t *>(
      [iServer](const servers &s) -> gameserveritem_t * {
        if (iServer < 0 || static_cast<size_t>(iServer) >= s.size()) {
          return nullptr;
        }

        server_item = s[iServer].server_item;
        return &server_item;
      });
}

void matchmaking_servers::CancelQuery(void *hRequest) {}

void matchmaking_servers::RefreshQuery(void *hRequest) {
  if (hRequest == friends_request) {
    const auto response = friends_response.load();
    if (response)
      RequestFriendsServerList(311210, nullptr, 0, response);
  }
}

bool matchmaking_servers::IsRefreshing(void *hRequest) {
  if (hRequest == friends_request)
    return friends_refreshing;
  if (hRequest == internet_request)
    return internet_refreshing;
  return false;
}

int matchmaking_servers::GetServerCount(void *hRequest) {
  if (internet_request != hRequest && favorites_request != hRequest &&
      history_request != hRequest && friends_request != hRequest) {
    return 0;
  }

  auto &servers_list = hRequest == favorites_request ? favorites_servers
                       : hRequest == history_request ? history_servers
                       : hRequest == friends_request ? friends_servers
                                                     : internet_servers;
  return servers_list.access<int>(
      [](const servers &s) { return static_cast<int>(s.size()); });
}

void matchmaking_servers::RefreshServer(void *hRequest, const int iServer) {
  if (internet_request != hRequest && favorites_request != hRequest &&
      history_request != hRequest && friends_request != hRequest) {
    return;
  }

  if (hRequest == friends_request) {
    if (friends_refreshing)
      return;

    uint64_t steam_id{};
    friends_servers.access([&](const servers &items) {
      if (iServer >= 0 && static_cast<size_t>(iServer) < items.size())
        steam_id = items[iServer].server_item.m_steamID.bits;
    });
    if (!steam_id)
      return;

    uint64_t generation{};
    matchmaking_server_list_response *expected_response{};
    {
      std::lock_guard lock(friend_scan_mutex);
      generation = friend_scan_generation;
      expected_response = friends_response.load();
    }
    nat::refresh_friends(
        {steam_id}, [steam_id, generation, expected_response](
                        std::vector<nat::friend_presence> live) {
          {
            std::lock_guard lock(friend_scan_mutex);
            if (generation != friend_scan_generation ||
                expected_response != friends_response.load()) {
              return;
            }
          }
          const auto presence = std::ranges::find(
              live, steam_id, &nat::friend_presence::steam_id);
          const bool online = presence != live.end();
          if (online)
            ::friends::set_master_presence(steam_id, presence->endpoint,
                                           presence->token);
          else
            ::friends::clear_master_presence(steam_id);

          const auto saved = ::friends::get_friend_server_addresses();
          const auto entry = std::ranges::find(
              saved, steam_id, &::friends::friend_server_info::steam_id);
          if (entry == saved.end())
            return;

          std::optional<int> row_index;
          int item_count{};
          friends_servers.access([&](servers &items) {
            for (size_t i = 0; i < items.size(); ++i) {
              if (items[i].server_item.m_steamID.bits != steam_id)
                continue;

              server row{};
              if (online) {
                row.address = network::address_from_string(presence->endpoint);
                row.server_item = create_server_item(row.address, {}, 0, true);
                row.server_item.m_nAppID = 311210;
                row.server_item.m_steamID.bits = steam_id;
                copy_safe(row.server_item.m_szGameDescription, "Friend match");
              } else {
                const auto placeholder = ::utils::string::va(
                    "192.0.2.%u:%u", static_cast<unsigned>((i % 250) + 1),
                    static_cast<unsigned>(40000 + (i % 20000)));
                row.address = network::address_from_string(placeholder);
                row.server_item = create_server_item(row.address, {}, 0, true);
                row.server_item.m_nAppID = 311210;
                row.server_item.m_steamID.bits = steam_id;
                copy_safe(row.server_item.m_szGameDescription, "Offline");
              }
              copy_safe(row.server_item.m_szServerName,
                        entry->player_name.c_str());
              row.handled = true;
              items[i] = row;
              row_index = static_cast<int>(i);
              break;
            }
            std::stable_sort(
                items.begin(), items.end(),
                [](const server &left, const server &right) {
                  const bool left_online =
                      std::strcmp(left.server_item.m_szGameDescription,
                                  "Offline") != 0;
                  const bool right_online =
                      std::strcmp(right.server_item.m_szGameDescription,
                                  "Offline") != 0;
                  return left_online && !right_online;
                });
            for (size_t i = 0; i < items.size(); ++i) {
              if (items[i].server_item.m_steamID.bits == steam_id) {
                row_index = static_cast<int>(i);
                break;
              }
            }
            item_count = static_cast<int>(items.size());
          });

          if (!row_index)
            return;
          ::friends::clear_browser_routes();
          friends_servers.access([&](const servers &items) {
            for (const auto &item : items) {
              ::friends::remember_browser_route(
                  item.server_item.m_steamID.bits,
                  network::address_to_string(item.address));
            }
          });
          ::friends::notify_presence_changed();
          if (const auto response = friends_response.load()) {
            for (int i = 0; i < item_count; ++i)
              response->ServerResponded(friends_request, i);
          }
        });
    return;
  }

  std::optional<game::net::netadr_t> address{};
  auto &servers_list = hRequest == favorites_request ? favorites_servers
                       : hRequest == history_request ? history_servers
                                                     : internet_servers;
  servers_list.access([&](const servers &s) {
    if (iServer < 0 || static_cast<size_t>(iServer) >= s.size()) {
      return;
    }

    address = s[iServer].address;
  });

  if (address) {
    auto callback =
        hRequest == favorites_request ? handle_favorites_server_response
        : hRequest == history_request ? handle_history_server_response
                                      : handle_internet_server_response;
    ping_server(*address, callback);
  }
}

void *matchmaking_servers::PingServer(
    const unsigned int unIP, const unsigned short usPort,
    matchmaking_ping_response *pRequestServersResponse) {
  auto response = pRequestServersResponse;
  const auto addr = network::address_from_ip(htonl(unIP), usPort);

  party::query_server(
      addr, [response](const bool success, const game::net::netadr_t &host,
                       const ::utils::info_string &info, const uint32_t ping) {
        if (success) {
          auto server_item = create_server_item(host, info, ping, success);
          response->ServerResponded(server_item);
        } else {
          response->ServerFailedToRespond();
        }
      });

  return reinterpret_cast<void *>(static_cast<uint64_t>(7 + rand()));
}

int matchmaking_servers::PlayerDetails(unsigned int unIP, unsigned short usPort,
                                       void *pRequestServersResponse) {
  return 0;
}

int matchmaking_servers::ServerRules(unsigned int unIP, unsigned short usPort,
                                     void *pRequestServersResponse) {
  return 0;
}

void matchmaking_servers::CancelServerQuery(int hServerQuery) {}

bool is_server_list_refreshing() { return internet_refreshing; }

int get_raw_internet_server_count() {
  return internet_servers.access<int>(
      [](const servers &s) { return static_cast<int>(s.size()); });
}

gameserveritem_t *get_raw_internet_server_item(const int index) {
  thread_local gameserveritem_t item{};
  return internet_servers.access<gameserveritem_t *>(
      [index](const servers &s) -> gameserveritem_t * {
        if (index < 0 || static_cast<size_t>(index) >= s.size()) {
          return nullptr;
        }
        item = s[index].server_item;
        return &item;
      });
}
} // namespace steam
