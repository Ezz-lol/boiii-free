#include <std_include.hpp>

#include "auth.hpp"
#include "friends.hpp"
#include <loader/component_loader.hpp>

#include "command.hpp"
#include "name.hpp"
#include "nat.hpp"
#include "network.hpp"
#include "scheduler.hpp"
#include "toast.hpp"
#include <game/utils.hpp>

#include <utils/concurrency.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace friends {
namespace {
constexpr const char *FRIENDS_FILE = "boiii_players/user/friends.json";
constexpr const char *RECENT_FILE = "boiii_players/user/recent_players.json";
constexpr int MAX_FRIENDS = 200;
constexpr size_t MAX_RECENT = 100;
constexpr game::XUID MIN_PLAYER_XUID = 0x100000000;
constexpr game::XUID MIN_BOT_XUID = 0xFFFFFFFFFFFF0000;
constexpr auto INVITE_LIFETIME = 5min;
constexpr int64_t RECENT_SAVE_INTERVAL = 300;

struct friend_state {
  std::vector<friend_entry> list;
};

utils::concurrency::container<friend_state> friends_data;

utils::concurrency::container<std::vector<recent_player>> recent_players;
std::mutex invites_mutex;
std::unordered_map<game::XUID, std::chrono::steady_clock::time_point>
    pending_invites;

std::mutex browser_routes_mutex;
std::unordered_map<std::string, game::XUID> browser_routes;
std::atomic<bool> presence_refreshing{};
std::atomic<bool> social_update_pending{};
std::atomic<int32_t> social_update_revision{};

void sort_by_presence(std::vector<friend_entry> &entries) {
  std::stable_sort(entries.begin(), entries.end(),
                   [](const friend_entry &left, const friend_entry &right) {
                     return static_cast<int>(left.state) >
                            static_cast<int>(right.state);
                   });
}

void queue_social_update() {
  if (social_update_pending.exchange(true))
    return;
  scheduler::once(
      [] {
        social_update_pending = false;
        const auto global = game::ui::UI_Model_GetGlobalModel();
        if (!global)
          return;
        const auto update = game::ui::UI_Model_GetModelFromPath(
            global, "socialRoot.friends.update");
        if (!update)
          return;
        game::ui::UI_Model_SetInt(update, ++social_update_revision);
      },
      scheduler::main);
}

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

bool load_friends() {
  if (!utils::io::file_exists(FRIENDS_FILE))
    return false;

  std::string data;
  if (!utils::io::read_file(FRIENDS_FILE, &data) || data.empty())
    return false;

  rapidjson::Document doc;
  if (doc.Parse(data.c_str()).HasParseError() || !doc.IsArray())
    return false;

  std::vector<friend_entry> loaded;
  loaded.reserve(std::min<size_t>(doc.Size(), MAX_FRIENDS));
  std::unordered_set<game::XUID> seen;
  seen.reserve(loaded.capacity());

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

    if (entry.steam_id == 0 || seen.contains(entry.steam_id))
      continue;

    auto ni = item.FindMember("name");
    if (ni != item.MemberEnd() && ni->value.IsString())
      entry.name = ni->value.GetString();
    else
      entry.name = "Unknown";

    seen.insert(entry.steam_id);
    loaded.push_back(std::move(entry));
    if (loaded.size() == MAX_FRIENDS)
      break;
  }

  bool changed{};
  friends_data.access([&](friend_state &state) {
    changed = state.list.size() != loaded.size();
    if (!changed) {
      for (size_t i = 0; i < loaded.size(); ++i) {
        if (state.list[i].steam_id != loaded[i].steam_id ||
            state.list[i].name != loaded[i].name) {
          changed = true;
          break;
        }
      }
    }

    if (!changed)
      return;

    for (auto &entry : loaded) {
      const auto existing = std::ranges::find(state.list, entry.steam_id,
                                              &friend_entry::steam_id);
      if (existing == state.list.end())
        continue;
      entry.state = existing->state;
      entry.server_address = existing->server_address;
      entry.join_token = existing->join_token;
    }
    state.list = std::move(loaded);
  });

  return changed;
}

void save_recent_players(const std::vector<recent_player> &players) {
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> w(buf);
  w.StartArray();
  for (const recent_player &player : players) {
    w.StartObject();
    w.Key("steam_id");
    w.Uint64(player.steam_id);
    w.Key("name");
    w.String(player.name.c_str());
    w.Key("last_seen");
    w.Int64(player.last_seen);
    w.EndObject();
  }
  w.EndArray();
  utils::io::write_file(RECENT_FILE,
                        std::string(buf.GetString(), buf.GetSize()));
}

void load_recent_players() {
  std::string data;
  rapidjson::Document doc;
  if (!utils::io::read_file(RECENT_FILE, &data) ||
      doc.Parse(data.c_str()).HasParseError() || !doc.IsArray())
    return;

  std::vector<recent_player> loaded;
  for (const auto &item : doc.GetArray()) {
    if (!item.IsObject() || !item.HasMember("steam_id") ||
        !item["steam_id"].IsUint64())
      continue;
    recent_player player{};
    player.steam_id = item["steam_id"].GetUint64();
    if (item.HasMember("name") && item["name"].IsString())
      player.name = item["name"].GetString();
    if (item.HasMember("last_seen") && item["last_seen"].IsInt64())
      player.last_seen = item["last_seen"].GetInt64();
    loaded.push_back(std::move(player));
    if (loaded.size() == MAX_RECENT)
      break;
  }
  recent_players.access([&](std::vector<recent_player> &players) {
    players = std::move(loaded);
  });
}

__declspec(noinline) bool read_client_name(const int index, char *buffer,
                                           const int size) {
  __try {
    buffer[0] = '\0';
    return game::cl::CL_GetClientName(game::LOCAL_CLIENT_0, index, buffer, size,
                                      false) &&
           buffer[0] != '\0';
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

void record_recent_players() {
  if (!game::com::Com_IsInGame() || game::com::Com_IsRunningUILevel())
    return;

  const game::XUID own_id = auth::get_guid();
  const int64_t now = std::time(nullptr);
  std::vector<recent_player> seen;
  for (game::ClientNum_t index = game::CLIENT_INDEX_FIRST;
       index < game::get_max_client_count(); ++index) {
    const game::XUID xuid = auth::get_guid(index);
    char name[64];
    if (xuid < MIN_PLAYER_XUID || xuid >= MIN_BOT_XUID || xuid == own_id ||
        !read_client_name(static_cast<int>(index), name, sizeof(name)))
      continue;
    seen.push_back({xuid, name, now});
  }
  if (seen.empty())
    return;

  static int64_t last_save{};
  recent_players.access([&](std::vector<recent_player> &players) {
    bool changed{};
    for (recent_player &player : seen) {
      const auto existing =
          std::ranges::find(players, player.steam_id, &recent_player::steam_id);
      if (existing == players.end()) {
        players.insert(players.begin(), std::move(player));
        changed = true;
        continue;
      }
      changed |= existing->name != player.name;
      *existing = std::move(player);
      std::rotate(players.begin(), existing, existing + 1);
    }
    if (players.size() > MAX_RECENT)
      players.resize(MAX_RECENT);
    if (changed || now - last_save >= RECENT_SAVE_INTERVAL) {
      last_save = now;
      save_recent_players(players);
    }
  });
}

void receive_invite(const game::net::netadr_t &sender,
                    const network::data_view &data, game::LocalClientNum_t) {
  if (!nat::is_rendezvous(sender))
    return;
  const std::string payload(reinterpret_cast<const char *>(data.data()),
                            data.size());
  const size_t id_start = payload.find(' ');
  const size_t name_start = id_start == std::string::npos
                                ? id_start
                                : payload.find(' ', id_start + 1);
  if (!payload.starts_with("1 ") || name_start == std::string::npos)
    return;
  const game::XUID inviter =
      std::strtoull(payload.c_str() + id_start + 1, nullptr, 10);
  const std::string inviter_name = payload.substr(name_start + 1);
  if (!inviter || inviter == auth::get_guid() || inviter_name.empty())
    return;

  {
    std::lock_guard lock(invites_mutex);
    pending_invites[inviter] = std::chrono::steady_clock::now();
  }
  scheduler::once(
      [inviter, inviter_name] {
        add_friend(inviter, inviter_name);
        refresh_presence();
        toast::show("Invite",
                    inviter_name +
                        " invited you to play. Join from Social > Friends.",
                    "uie_t7_icon_menu_invite_sent");
      },
      scheduler::main);
}

void receive_invite_failed(const game::net::netadr_t &sender,
                           const network::data_view &data,
                           game::LocalClientNum_t) {
  if (!nat::is_rendezvous(sender))
    return;
  const game::XUID target = std::strtoull(
      std::string(reinterpret_cast<const char *>(data.data()), data.size())
          .c_str(),
      nullptr, 10);
  scheduler::once(
      [target] {
        toast::warn("Invite", get_known_name(target) +
                                  " is offline and can't be invited right "
                                  "now.");
      },
      scheduler::main);
}
} // namespace

std::vector<recent_player> get_recent_players() {
  std::vector<recent_player> result;
  recent_players.access(
      [&](const std::vector<recent_player> &players) { result = players; });
  return result;
}

size_t get_recent_count() {
  size_t count{};
  recent_players.access([&](const std::vector<recent_player> &players) {
    count = players.size();
  });
  return count;
}

std::string get_known_name(const game::XUID steam_id) {
  const std::optional<friend_entry> entry = find_friend(steam_id);
  std::string name = entry ? entry->name : std::string();
  if (name.empty() || name == "Unknown") {
    recent_players.access([&](const std::vector<recent_player> &players) {
      const auto found =
          std::ranges::find(players, steam_id, &recent_player::steam_id);
      if (found != players.end())
        name = found->name;
    });
  }
  return name.empty() ? "Unknown" : name;
}

bool has_invited_us(const game::XUID steam_id) {
  std::lock_guard lock(invites_mutex);
  const auto found = pending_invites.find(steam_id);
  return found != pending_invites.end() &&
         std::chrono::steady_clock::now() - found->second < INVITE_LIFETIME;
}

void reload_from_disk() {
  if (load_friends()) {
    notify_presence_changed();
  }
}

void add_friend(game::XUID steam_id, const std::string &fname) {
  if (steam_id != 0 && auth::get_guid() != steam_id) {
    friends_data.access([&](friend_state &state) {
      for (friend_entry &e : state.list) {
        if (e.steam_id == steam_id) {
          if (!fname.empty() && fname != "Unknown") {
            e.name = fname;
          }
          return;
        }
      }
      if (static_cast<int>(state.list.size()) >= MAX_FRIENDS) {
        return;
      }

      friend_entry entry{};
      entry.steam_id = steam_id;
      entry.name = fname.empty() ? "Unknown" : fname;
      entry.state = status::offline;
      state.list.push_back(std::move(entry));
    });
    save_friends();
    notify_presence_changed();
  }
}

void remove_friend(game::XUID steam_id) {
  friends_data.access([&](friend_state &state) {
    std::erase_if(state.list, [steam_id](const friend_entry &e) {
      return e.steam_id == steam_id;
    });
  });
  save_friends();
  notify_presence_changed();
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

int32_t get_friend_count() {
  return static_cast<int32_t>(get_friends().size());
}

friend_entry get_friend_by_index(int32_t index) {
  const std::vector<friend_entry> entries = get_friends();
  return index >= 0 && index < static_cast<int32_t>(entries.size())
             ? entries[index]
             : friend_entry{};
}

std::optional<friend_entry> find_friend(const game::XUID steam_id) {
  std::optional<friend_entry> result;
  friends_data.access([&](const friend_state &state) {
    const auto found =
        std::ranges::find(state.list, steam_id, &friend_entry::steam_id);
    if (found != state.list.end())
      result = *found;
  });
  return result;
}

std::vector<friend_entry> get_friends() {
  std::vector<friend_entry> result;
  friends_data.access([&](const friend_state &state) { result = state.list; });
  const game::XUID own_id = auth::get_guid();
  if (own_id)
    std::erase_if(result, [own_id](const friend_entry &entry) {
      return entry.steam_id == own_id;
    });
  sort_by_presence(result);
  return result;
}

bool invite_to_game(const game::XUID steam_id) {
  if (!steam_id || steam_id == auth::get_guid())
    return false;

  if (!game::get_dvar_bool("friends_open").value_or(false) &&
      !nat::set_open_to_friends(true)) {
    toast::warn("Invite", "Only the host of your lobby can invite players.");
    return false;
  }

  const std::string target_name = get_known_name(steam_id);
  add_friend(steam_id, target_name);

  const std::string_view own_name = name::get_player_name();
  nat::send_invite(steam_id,
                   own_name.empty() ? "Player" : std::string(own_name));
  toast::success("Invite", "Invite sent to " + target_name + ".");
  return true;
}

std::vector<friend_server_info> get_friend_server_addresses() {
  std::vector<friend_server_info> result;
  std::unordered_set<game::XUID> seen_ids;

  const std::vector<friend_entry> all_friends = get_friends();

  for (const friend_entry &entry : all_friends) {
    if (entry.steam_id != 0 && !seen_ids.contains(entry.steam_id)) {
      seen_ids.insert(entry.steam_id);

      const auto [color, description] =
          entry.state == status::in_game ? std::pair{"^2", "Friend match"}
          : entry.state == status::online
              ? std::pair{"^3", "Online, party closed"}
              : std::pair{"^1", "Offline"};
      result.push_back({entry.steam_id, entry.server_address,
                        color + entry.name, description});
    }
  }

  return result;
}

bool connect_to_friend(game::XUID steam_id) {
  if (steam_id == 0) {
    return false;
  }

  // Check if friend is in our list and has a server address
  std::string addr_str;
  std::string join_token;
  auto state = status::offline;
  friends_data.access([&](const friend_state &friends) {
    for (const friend_entry &e : friends.list) {
      if (e.steam_id == steam_id) {
        addr_str = e.server_address;
        join_token = e.join_token;
        state = e.state;
        break;
      }
    }
  });

  if (addr_str.empty()) {
    scheduler::once(
        [state] {
          game::ui::UI_OpenErrorPopupWithMessage(
              game::LOCAL_CLIENT_0, game::errorCode::UI,
              state == status::online
                  ? "Your friend's party is closed. Ask them to open it to "
                    "friends."
                  : "Your friend is offline.");
        },
        scheduler::main);
    return false;
  }

  if (!join_token.empty()) {
    nat::begin_join(join_token, addr_str, steam_id);
    return true;
  }

  // Fallback: raw connect
  const game::net::netadr_t fallback_addr =
      network::address_from_string(addr_str);
  if (network::is_connectable_address(fallback_addr)) {
    const char *sanitized = utils::string::va(
        "%i.%i.%i.%i:%hu", fallback_addr.ipv4.a, fallback_addr.ipv4.b,
        fallback_addr.ipv4.c, fallback_addr.ipv4.d, fallback_addr.port);
    game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                             utils::string::va("connect %s\n", sanitized));
    return true;
  }

  return false;
}

void notify_presence_changed() { queue_social_update(); }

void refresh_presence() {
  reload_from_disk();

  if (presence_refreshing.exchange(true))
    return;

  const auto saved = get_friends();
  std::vector<uint64_t> ids;
  std::unordered_set<uint64_t> queried;
  ids.reserve(saved.size());
  queried.reserve(saved.size());
  for (const auto &entry : saved) {
    ids.push_back(entry.steam_id);
    queried.insert(entry.steam_id);
  }

  nat::refresh_friends(ids, [queried = std::move(queried)](
                                std::vector<nat::friend_presence> live) {
    std::unordered_map<game::XUID, nat::friend_presence> available;
    available.reserve(live.size());
    for (auto &entry : live) {
      if (!entry.steam_id)
        continue;
      if (!entry.token.empty()) {
        const auto address = network::address_from_string(entry.endpoint);
        if (!network::is_connectable_address(address))
          continue;
        entry.endpoint = network::address_to_string(address);
      }
      available.insert_or_assign(entry.steam_id, std::move(entry));
    }

    bool changed{};
    friends_data.access([&](friend_state &state) {
      for (auto &entry : state.list) {
        if (!queried.contains(entry.steam_id))
          continue;
        const auto found = available.find(entry.steam_id);
        const auto next_state = found == available.end()      ? status::offline
                                : found->second.token.empty() ? status::online
                                                              : status::in_game;
        const auto next_address =
            found == available.end() ? std::string{} : found->second.endpoint;
        const auto next_token =
            found == available.end() ? std::string{} : found->second.token;
        if (entry.state != next_state || entry.server_address != next_address ||
            entry.join_token != next_token) {
          entry.state = next_state;
          entry.server_address = next_address;
          entry.join_token = next_token;
          changed = true;
        }
      }
    });

    presence_refreshing = false;
    if (changed)
      notify_presence_changed();
  });
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
    const auto found =
        std::ranges::find(state.list, steam_id, &friend_entry::steam_id);
    if (found == state.list.end())
      return;
    found->state = status::offline;
    found->server_address.clear();
    found->join_token.clear();
  });
}

void set_closed_presence(const game::XUID steam_id) {
  friends_data.access([steam_id](friend_state &state) {
    const auto found =
        std::ranges::find(state.list, steam_id, &friend_entry::steam_id);
    if (found == state.list.end())
      return;
    found->state = status::online;
    found->server_address.clear();
    found->join_token.clear();
  });
}

void set_master_presence(const game::XUID steam_id, const std::string &address,
                         const std::string &join_token) {
  const auto parsed = network::address_from_string(address);
  if (!steam_id || !network::is_connectable_address(parsed)) {
    return;
  }

  friends_data.access([&](friend_state &state) {
    const auto found =
        std::ranges::find(state.list, steam_id, &friend_entry::steam_id);
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

game::XUID find_browser_route(const std::string &address) {
  const game::net::netadr_t parsed = network::address_from_string(address);
  if (!network::is_ip_address(parsed))
    return 0;
  const std::string normalized = network::address_to_string(parsed);
  std::lock_guard lock(browser_routes_mutex);
  if (browser_routes.contains(normalized)) {
    return browser_routes[normalized];
  }
  return 0;
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("friends");

  void post_unpack() override {
    reload_from_disk();
    load_recent_players();
    network::on("friendInvite", receive_invite);
    network::on("friendInviteFailed", receive_invite_failed);
    scheduler::loop(record_recent_players, scheduler::main, 10s);

    game::register_dvar_bool("friends_open", false, game::DVAR_NONE,
                             "Advertise this private match to saved friends");
    command::add("friends_open", [](const command::params &) {
      const bool enabled = !game::get_dvar_bool("friends_open").value_or(false);
      game::Dvar_SetFromStringByName("friends_open", enabled ? "1" : "0", true);
      if (!nat::set_open_to_friends(enabled)) {
        game::Dvar_SetFromStringByName("friends_open", "0", true);
        toast::warn("Friends",
                    "Only the host of your lobby can open it to friends.");
      } else if (enabled) {
        toast::success("FRIENDS", "Friends can now join.");
      } else {
        toast::warn("FRIENDS", "Friends can no longer join.");
      }
    });
  }
};
} // namespace friends

REGISTER_COMPONENT(friends::component)
