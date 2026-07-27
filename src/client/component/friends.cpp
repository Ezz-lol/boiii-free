#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include "friends.hpp"
#include "auth.hpp"

#include <game/utils.hpp>
#include "command.hpp"
#include "getinfo.hpp"
#include "network.hpp"
#include "nat.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "steam_proxy.hpp"
#include "toast.hpp"
#include "workshop.hpp"
#include "name.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include <utils/http.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace friends {
namespace {
constexpr const char *FRIENDS_FILE = "boiii_players/user/friends.json";
constexpr int MAX_FRIENDS = 200;

std::mutex public_ip_mutex;
std::string cached_public_ip;
std::atomic_bool public_ip_fetched{false};

// Helper to find VPN/Virtual LAN IPs (Radmin, Hamachi)
std::string get_preferred_local_ip() {
  ULONG outBufLen = 15000;
  std::vector<unsigned char> buffer(outBufLen);
  PIP_ADAPTER_INFO pAdapterInfo =
      reinterpret_cast<IP_ADAPTER_INFO *>(buffer.data());

  // Retry with larger buffer if needed
  if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
    buffer.resize(outBufLen);
    pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO *>(buffer.data());
  }

  if (GetAdaptersInfo(pAdapterInfo, &outBufLen) != NO_ERROR) {
    return "";
  }

  std::string radmin_ip;
  std::string hamachi_ip;
  std::string other_vpn_ip; // e.g. ZeroTier often uses managed ranges, but we
                            // can detect 10.x if needed

  PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
  while (pAdapter) {
    const std::string ip = pAdapter->IpAddressList.IpAddress.String;
    if (ip != "0.0.0.0" && !ip.empty()) {
      // Radmin VPN usually uses 26.x.x.x
      if (ip.starts_with("26.")) {
        radmin_ip = ip;
      }
      // Hamachi usually uses 25.x.x.x
      else if (ip.starts_with("25.")) {
        hamachi_ip = ip;
      }
    }
    pAdapter = pAdapter->Next;
  }

  // Priority: Radmin > Hamachi
  if (!radmin_ip.empty())
    return radmin_ip;
  if (!hamachi_ip.empty())
    return hamachi_ip;

  return "";
}

void fetch_public_ip() {
  try {
    std::optional<std::string> resp =
        utils::http::get_data("https://api.ipify.org", {}, {}, 1);
    if (resp.has_value() && !resp->empty()) {
      std::lock_guard lock(public_ip_mutex);
      cached_public_ip = *resp;
      public_ip_fetched.store(true);
    }
  } catch (...) {
  }
}

struct friend_state {
  std::vector<friend_entry> list;
};

utils::concurrency::container<friend_state> friends_data;

std::mutex browser_routes_mutex;
std::unordered_map<std::string, game::XUID> browser_routes;

void save_friends() {
  friends_data.access([](const friend_state &state) {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> w(buf);
    w.StartArray();
    for (const friend_entry &f : state.list) {
      w.StartObject();
      w.Key("steam_id");
      w.Uint64(f.steam_id);
      w.Key("name");
      w.String(f.name.c_str());
      w.EndObject();
    }
    w.EndArray();
    utils::io::write_file(FRIENDS_FILE,
                          std::string(buf.GetString(), buf.GetSize()));
  });
}

void load_friends() {
  if (!utils::io::file_exists(FRIENDS_FILE))
    return;

  std::string data;
  if (!utils::io::read_file(FRIENDS_FILE, &data) || data.empty())
    return;

  rapidjson::Document doc;
  if (doc.Parse(data.c_str()).HasParseError() || !doc.IsArray())
    return;

  friends_data.access([&doc](friend_state &state) {
    state.list.clear();

    for (auto &item : doc.GetArray()) {
      if (!item.IsObject())
        continue;

      friend_entry entry{};

      auto si = item.FindMember("steam_id");
      if (si != item.MemberEnd()) {
        if (si->value.IsUint64())
          entry.steam_id = si->value.GetUint64();
        else if (si->value.IsString())
          entry.steam_id = std::strtoull(si->value.GetString(), nullptr, 10);
      }

      // backwards compat with old "xuid" field
      if (entry.steam_id == 0) {
        auto xi = item.FindMember("xuid");
        if (xi != item.MemberEnd()) {
          if (xi->value.IsUint64())
            entry.steam_id = xi->value.GetUint64();
          else if (xi->value.IsString())
            entry.steam_id = std::strtoull(xi->value.GetString(), nullptr, 10);
        }
      }

      if (entry.steam_id == 0)
        continue;

      auto ni = item.FindMember("name");
      if (ni != item.MemberEnd() && ni->value.IsString())
        entry.name = ni->value.GetString();
      else
        entry.name = "Unknown";

      entry.state = status::offline;

      bool exists = false;
      for (const friend_entry &e : state.list) {
        if (e.steam_id == entry.steam_id) {
          exists = true;
          break;
        }
      }
      if (!exists)
        state.list.push_back(std::move(entry));
    }
  });
}

// Resolves the address other players can use to connect to us
std::string get_own_connect_address() {
  uint16_t local_port = party::get_local_port();
  const std::string vpn_ip = get_preferred_local_ip();

  // If we found a Radmin/Hamachi IP, prioritize it immediately
  if (!vpn_ip.empty()) {
    return utils::string::va("%s:%u", vpn_ip.c_str(),
                             static_cast<unsigned>(local_port));
  }

  if (game::com::Com_IsInGame()) {
    game::net::netadr_t connected = party::get_connected_server();

    if (connected.type == game::net::NA_LOOPBACK) {
      if (public_ip_fetched.load()) {
        std::lock_guard lock(public_ip_mutex);
        if (!cached_public_ip.empty()) {
          return utils::string::va("%s:%u", cached_public_ip.c_str(),
                                   static_cast<unsigned>(local_port));
        }
      }
    } else if ((connected.type == game::net::NA_IP ||
                connected.type == game::net::NA_RAWIP) &&
               connected.port >= 1024 && connected.ipv4.a != 127 &&
               connected.addr != 0) {
      bool is_private = (connected.ipv4.a == 10) ||
                        (connected.ipv4.a == 172 && connected.ipv4.b >= 16 &&
                         connected.ipv4.b <= 31) ||
                        (connected.ipv4.a == 192 && connected.ipv4.b == 168);

      if (is_private && public_ip_fetched.load()) {
        std::lock_guard lock(public_ip_mutex);
        if (!cached_public_ip.empty()) {
          return utils::string::va("%s:%u", cached_public_ip.c_str(),
                                   connected.port);
        }
      }

      return utils::string::va("%u.%u.%u.%u:%u", connected.ipv4.a,
                               connected.ipv4.b, connected.ipv4.c,
                               connected.ipv4.d, connected.port);
    }
  }

  if (public_ip_fetched.load()) {
    std::lock_guard lock(public_ip_mutex);
    if (!cached_public_ip.empty()) {
      return utils::string::va("%s:%u", cached_public_ip.c_str(),
                               static_cast<unsigned>(local_port));
    }
  }

  std::string local_ip;
  SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock != INVALID_SOCKET) {
    sockaddr_in target{};
    target.sin_family = AF_INET;
    target.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &target.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr *>(&target), sizeof(target)) ==
        0) {
      sockaddr_in local{};
      int len = sizeof(local);
      getsockname(sock, reinterpret_cast<sockaddr *>(&local), &len);
      char buf[INET_ADDRSTRLEN]{};
      inet_ntop(AF_INET, &local.sin_addr, buf, sizeof(buf));
      local_ip = buf;
    }
    closesocket(sock);
  }

  if (local_ip.empty())
    return "";
  return utils::string::va("%s:%u", local_ip.c_str(),
                           static_cast<unsigned>(local_port));
}

} // namespace

void add_friend(game::XUID steam_id, const std::string &fname) {
  if (steam_id == 0)
    return;
  if (const auto own_id = auth::get_guid(); own_id == steam_id)
    return;

  friends_data.access([&](friend_state &state) {
    for (auto &e : state.list) {
      if (e.steam_id == steam_id) {
        if (!fname.empty() && fname != "Unknown")
          e.name = fname;
        return;
      }
    }
    if (static_cast<int>(state.list.size()) >= MAX_FRIENDS)
      return;

    friend_entry entry{};
    entry.steam_id = steam_id;
    entry.name = fname.empty() ? "Unknown" : fname;
    entry.state = status::offline;
    state.list.push_back(std::move(entry));
  });
  save_friends();
}

void remove_friend(game::XUID steam_id) {
  friends_data.access([&](friend_state &state) {
    std::erase_if(state.list, [steam_id](const friend_entry &e) {
      return e.steam_id == steam_id;
    });
  });
  save_friends();
}

bool is_friend(game::XUID steam_id) {
  bool found = false;
  friends_data.access([&](const friend_state &state) {
    for (const friend_entry &e : state.list)
      if (e.steam_id == steam_id) {
        found = true;
        break;
      }
  });
  return found;
}

int get_friend_count() {
  return static_cast<int>(get_friends().size());
}

friend_entry get_friend_by_index(int index) {
  const auto entries = get_friends();
  return index >= 0 && index < static_cast<int>(entries.size())
             ? entries[index]
             : friend_entry{};
}

std::vector<friend_entry> get_friends() {
  std::vector<friend_entry> result;
  friends_data.access([&](const friend_state &state) { result = state.list; });
  const auto own_id = auth::get_guid();
  if (own_id)
    std::erase_if(result, [own_id](const friend_entry &entry) {
      return entry.steam_id == own_id;
    });
  return result;
}

bool invite_to_game(game::XUID steam_id) {
  const std::string connect_str = get_own_connect_address();
  if (connect_str.empty())
    return false;

  const std::string_view mapname = game::get_mapname().value_or("");
  const std::string_view gametype = game::get_g_gametype().value_or("");
  game::eModes playmode = game::com::Com_SessionMode_GetMode();
  const std::string mod_id = workshop::get_mod_publisher_id();
  game::XUID own_friend_code = auth::get_guid();
  std::string own_name = name::get_player_name();
  if (own_name.empty())
    own_name = "Player";

  // enriched format: addr|map|gametype|mode|mod|sender_id|sender_name
  const char *enriched = utils::string::va(
      "%s|%s|%s|%d|%s|%llu|%s", connect_str.c_str(), mapname.data(),
      gametype.data(), static_cast<int32_t>(playmode), mod_id.c_str(),
      own_friend_code, own_name.c_str());

  if (!is_friend(steam_id)) {
    std::string target_name = steam_proxy::get_steam_friend_name(steam_id);
    if (target_name.empty())
      target_name = "Friend";
    add_friend(steam_id, target_name);
  } else {
    // Update name if we have a better one from Steam
    const std::string target_name =
        steam_proxy::get_steam_friend_name(steam_id);
    if (!target_name.empty())
      add_friend(steam_id, target_name);
  }

  try {
    steam_proxy::invite_friend(steam_id, enriched);
    return true;
  } catch (...) {
  }
  return false;
}

std::string get_presence_server(game::XUID steam_id) {
  std::string result;
  friends_data.access([&](const friend_state &state) {
    for (const friend_entry &e : state.list) {
      if (e.steam_id == steam_id) {
        result = e.server_address;
        break;
      }
    }
  });
  return result;
}

std::vector<friend_server_info> get_friend_server_addresses() {
  std::vector<friend_server_info> result;
  std::unordered_set<game::XUID> seen_ids;

  std::vector<friend_entry> all_friends;
  friends_data.access(
      [&](const friend_state &state) { all_friends = state.list; });

  for (const friend_entry &entry : all_friends) {
    if (entry.steam_id == 0 || seen_ids.count(entry.steam_id))
      continue;
    seen_ids.insert(entry.steam_id);

    const std::string addr = entry.server_address;

    // green online and red offline
    const std::string color_prefix = addr.empty() ? "^1" : "^2";
    result.push_back({entry.steam_id, addr, color_prefix + entry.name});
  }

  return result;
}

std::string get_friend_game_info_by_address(const game::net::netadr_t target) {
  std::vector<friend_entry> all_friends;
  friends_data.access(
      [&](const friend_state &state) { all_friends = state.list; });

  for (const friend_entry &entry : all_friends) {
    if (entry.steam_id == 0)
      continue;

    steam_proxy::request_friend_rich_presence(entry.steam_id);
    const std::string game_info = steam_proxy::get_friend_rich_presence(
        entry.steam_id, "boiii_game_info");
    if (game_info.empty())
      continue;

    const std::vector<std::string> parts = utils::string::split(game_info, '|');
    if (parts.empty())
      continue;

    // Check if the address in the RP data matches the requested address
    if (parts[0] == std::string_view(target.toString()))
      return game_info;

    // Also try matching resolved addresses
    if (target.type != game::net::NA_BAD) {
      game::net::netadr_t friend_addr = network::address_from_string(parts[0]);
      if (friend_addr.type != game::net::NA_BAD &&
          network::are_addresses_equal(friend_addr, target))
        return game_info;
    }
  }

  return "";
}

std::string get_friend_game_info_by_address(const std::string &address) {
  if (address.empty())
    return "";

  game::net::netadr_t target = network::address_from_string(address);
  return get_friend_game_info_by_address(target);
}

bool connect_to_friend(game::XUID steam_id) {
  if (steam_id == 0) {
    return false;
  }

  // Check if friend is in our list and has a server address
  std::string addr_str;
  std::string join_token;
  friends_data.access([&](const friend_state &state) {
    for (const friend_entry &e : state.list) {
      if (e.steam_id == steam_id && !e.server_address.empty()) {
        addr_str = e.server_address;
        join_token = e.join_token;
        break;
      }
    }
  });

  if (addr_str.empty()) {
    // Friend is not in-game / not reachable
    scheduler::once(
        [] {
          game::ui::UI_OpenErrorPopupWithMessage(
              0, game::errorCode::UI,
              "Friend is not online or not in a joinable game.");
        },
        scheduler::main);
    return false;
  }

  if (!join_token.empty()) {
    nat::begin_join(join_token, addr_str);
    return true;
  }

  // Try enriched game info for proper mode/map connection
  const std::string game_info =
      steam_proxy::get_friend_rich_presence(steam_id, "boiii_game_info");
  if (!game_info.empty()) {
    const std::vector<std::string> parts = utils::string::split(game_info, '|');
    if (parts.size() >= 4) {
      const std::string connect_addr = parts[0];
      const std::string mapname = parts[1];
      const std::string gametype = parts[2];
      game::eModes mode =
          static_cast<game::eModes>(std::atoi(parts[3].c_str()));
      const std::string mod_id = parts.size() >= 5 ? parts[4] : "";

      game::net::netadr_t target = network::address_from_string(connect_addr);
      if (target.type != game::net::NA_BAD && !mapname.empty() &&
          !gametype.empty()) {
        game::com::Com_SessionMode_SetGameMode(
            game::eGameModes::MATCHMAKING_PLAYLIST);
        const std::string usermap_id =
            workshop::get_usermap_publisher_id(mapname);
        party::connect_to_lobby_with_mode(target, mode, mapname, gametype,
                                          usermap_id, mod_id);
        return true;
      }
    }
  }

  // Fallback: raw connect
  const game::net::netadr_t fallback_addr =
      network::address_from_string(addr_str);
  if (network::is_connectable_address(fallback_addr)) {
    const char *sanitized = utils::string::va(
        "%i.%i.%i.%i:%hu", fallback_addr.ipv4.a, fallback_addr.ipv4.b,
        fallback_addr.ipv4.c, fallback_addr.ipv4.d, fallback_addr.port);
    game::cbuf::Cbuf_AddText(0, utils::string::va("connect %s\n", sanitized));
    return true;
  }

  return false;
}

void reset_master_presence() {
  friends_data.access([](friend_state &state) {
    for (auto &entry : state.list) {
      entry.state = status::offline;
      entry.server_address.clear();
      entry.join_token.clear();
    }
  });
}

void clear_master_presence(const game::XUID steam_id) {
  friends_data.access([steam_id](friend_state &state) {
    const auto found = std::ranges::find(state.list, steam_id,
                                         &friend_entry::steam_id);
    if (found == state.list.end())
      return;
    found->state = status::offline;
    found->server_address.clear();
    found->join_token.clear();
  });
}

void set_master_presence(const game::XUID steam_id,
                         const std::string &address,
                         const std::string &join_token) {
  const auto parsed = network::address_from_string(address);
  if (!steam_id || !network::is_connectable_address(parsed)) {
    return;
  }

  friends_data.access([&](friend_state &state) {
    const auto found = std::ranges::find(state.list, steam_id,
                                         &friend_entry::steam_id);
    if (found == state.list.end())
      return;
    found->state = status::in_game;
    found->server_address = network::address_to_string(parsed);
    found->join_token = join_token;
  });
}

void clear_browser_routes() {
  std::lock_guard lock(browser_routes_mutex);
  browser_routes.clear();
}

void forget_browser_routes(const game::XUID steam_id) {
  std::lock_guard lock(browser_routes_mutex);
  std::erase_if(browser_routes, [steam_id](const auto &route) {
    return route.second == steam_id;
  });
}

void remember_browser_route(const game::XUID steam_id,
                            const std::string &address) {
  if (!steam_id || address.empty())
    return;
  const auto parsed = network::address_from_string(address);
  if (!network::is_ip_address(parsed))
    return;
  std::lock_guard lock(browser_routes_mutex);
  browser_routes[network::address_to_string(parsed)] = steam_id;
}

game::XUID take_browser_route(const std::string &address) {
  const auto parsed = network::address_from_string(address);
  if (!network::is_ip_address(parsed))
    return 0;
  const auto normalized = network::address_to_string(parsed);
  std::lock_guard lock(browser_routes_mutex);
  const auto found = browser_routes.find(normalized);
  if (found == browser_routes.end())
    return 0;
  const auto steam_id = found->second;
  browser_routes.erase(found);
  return steam_id;
}

struct component final : client_component {
  void post_unpack() override {
    load_friends();

    game::register_dvar_bool("friends_open", false, game::DVAR_NONE,
                             "Advertise this private match to saved friends");
    command::add("friends_open", [](const command::params &) {
      const bool currently_open =
          game::get_dvar_bool("friends_open").value_or(false);
      if (!currently_open && !getinfo::is_host()) {
        game::ui::UI_OpenErrorPopupWithMessage(
            0, game::errorCode::UI,
            "Start a private match before allowing friends to join.");
        return;
      }

      const bool enabled = !currently_open;
      game::Dvar_SetFromStringByName("friends_open", enabled ? "1" : "0",
                                     true);
      if (!nat::set_open_to_friends(enabled)) {
        game::Dvar_SetFromStringByName("friends_open", "0", true);
        toast::warn("Friends", "Start a private match before opening the party.");
      } else if (enabled) {
        toast::success("FRIENDS", "Friends can now join.");
      } else {
        toast::warn("FRIENDS", "Friends can no longer join.");
      }
    });
    scheduler::once([] { fetch_public_ip(); }, scheduler::async, 2000ms);

  }

};
} // namespace friends

REGISTER_COMPONENT(friends::component)
