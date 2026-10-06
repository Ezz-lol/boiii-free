#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <game/game.hpp>

namespace friends {
enum class status : int {
  offline = 0,
  online = 1,
  in_game = 2,
};

struct friend_entry {
  game::XUID steam_id{};
  std::string name;
  status state{status::offline};
  std::string server_address;
  std::string join_token;
};

struct recent_player {
  game::XUID steam_id{};
  std::string name;
  int64_t last_seen{};
};

struct friend_server_info {
  game::XUID steam_id{};
  std::string address;
  std::string player_name;
  std::string description;
};

void add_friend(uint64_t steam_id, const std::string &name);
void remove_friend(uint64_t steam_id);
bool is_friend(uint64_t steam_id);
void reload_from_disk();

int get_friend_count();
friend_entry get_friend_by_index(int index);
std::optional<friend_entry> find_friend(uint64_t steam_id);
std::vector<friend_entry> get_friends();

bool invite_to_game(uint64_t steam_id);
bool has_invited_us(uint64_t steam_id);

std::vector<recent_player> get_recent_players();
size_t get_recent_count();
std::string get_known_name(uint64_t steam_id);

bool connect_to_friend(uint64_t steam_id);
void refresh_presence();
void notify_presence_changed();

// Live routes discovered through the configured Ezz master servers.
void reset_master_presence();
void clear_master_presence(uint64_t steam_id);
void set_closed_presence(uint64_t steam_id);
void set_master_presence(uint64_t steam_id, const std::string &address,
                         const std::string &join_token);

// The game browser only gives party::connect_stub an address. Keep a stable
// friend-id route so selecting an offline row never becomes connect 0.0.0.0.
void clear_browser_routes();
void forget_browser_routes(uint64_t steam_id);
void remember_browser_route(uint64_t steam_id, const std::string &address);
uint64_t find_browser_route(const std::string &address);

std::vector<friend_server_info> get_friend_server_addresses();
} // namespace friends
