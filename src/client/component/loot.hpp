#pragma once

#include <std_include.hpp>

namespace loot {
struct recent_drop {
  std::string title;    // e.g. "Common Camo - Razorback"
  std::string display;  // e.g. "Royal"
  std::string icon;     // UI image name, empty if none
  std::string category; // e.g. "camo"
  int tier;    // rank: 0 common, 1 rare, 2 epic, 3 legendary, 4 limited
  bool is_new; // not yet seen in the drops menu
};

std::vector<recent_drop> get_recent_drops(size_t max_count);
void mark_recent_drops_seen();
size_t unseen_drop_count();
size_t owned_item_count();
size_t total_item_count();

int cryptokey_balance();
void debug_log(const std::string &message); // console line prefixed [loot]
void receive_server_keys(int amount);       // from a server's luinotifyevent
void receive_server_drops(int amount);      // from a server's luinotifyevent
float next_cryptokey_progress();            // 0..1 toward the next playtime key
int supply_drop_price(bool rare);
bool open_supply_drop_from_ui(bool rare);

struct revealed_item {
  std::string name;     // e.g. "Caterpillar"
  std::string subtitle; // e.g. "Prophet Head Theme"
  std::string icon;
  std::string rarity;   // e.g. "Legendary"
  std::string category; // e.g. "decal"
  int rank;             // 0 common, 1 rare, 2 epic, 3 legendary, 4 limited
};

struct last_supply_drop {
  std::vector<revealed_item> items;
  int bonus_keys = 0;
  bool rare = false;
};

last_supply_drop get_last_supply_drop();
} // namespace loot
