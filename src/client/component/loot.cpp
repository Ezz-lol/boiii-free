#include <std_include.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <charconv>
#include <cstring>
#include <deque>

#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include "command.hpp"
#include "currency.hpp"
#include "loot.hpp"
#include "loot_pool.hpp"
#include "scheduler.hpp"
#include "toast.hpp"
#include <game/game.hpp>
#include <game/utils.hpp>

namespace toast {
void precache_icon(const std::string &material);
}

namespace loot {
namespace {
game::EngineDependentDvarMut dvar_cg_unlockall_loot;
game::EngineDependentDvarMut dvar_cg_unlockall_gobblegums;
game::EngineDependentDvarMut dvar_cg_unlockall_purchases;
game::EngineDependentDvarMut dvar_cg_unlockall_attachments;
game::EngineDependentDvarMut dvar_cg_unlockall_camos_and_reticles;
game::EngineDependentDvarMut dvar_cg_unlockall_calling_cards;
game::EngineDependentDvarMut dvar_cg_unlockall_specialists_outfits;
game::EngineDependentDvarMut dvar_cg_unlockall_cac_slots;

utils::hook::detour loot_getitemquantity_hook;
utils::hook::detour liveinventory_getitemquantity_hook;
utils::hook::detour liveinventory_areextraslotspurchased_hook;
utils::hook::detour bg_unlockablesisitempurchased_hook;
utils::hook::detour bg_unlockablesisitemattachmentlocked_hook;
utils::hook::detour bg_unlockablesisattachmentslotlocked_hook;
utils::hook::detour bg_unlockablesemblemorbackinglockedbychallenge_hook;
utils::hook::detour bg_unlockablesitemoptionlocked_hook;
utils::hook::detour bg_unlockedgetchallengeunlockedforindex_hook;
utils::hook::detour bg_unlockablescharactercustomizationitemlocked_hook;
utils::hook::detour bg_emblemisentitlementbackgroundgranted_hook;
utils::hook::detour liveentitlements_isentitlementactiveforcontroller_hook;
utils::hook::detour bg_unlockablesgetcustomclasscount_hook;
utils::hook::detour GScr_IsItemPurchasedForClientNum_hook;

constexpr const char *OWNED_LOOT_FILE = "boiii_players/user/loot_owned.txt";

game::EngineDependentDvarMut dvar_cg_loot_progression;
game::EngineDependentDvarMut dvar_cg_loot_drop;
game::EngineDependentDvarMut dvar_cg_loot_toast_style;
game::EngineDependentDvarMut dvar_cg_loot_toast_chars;
game::EngineDependentDvarMut dvar_cg_loot_toast_individual;
game::EngineDependentDvarMut dvar_cg_loot_keys;
// Economy: edit and rebuild to tune.
constexpr int KEY_MINUTES = 1;       // minutes of play per Cryptokey
constexpr int COMMON_DROP_COST = 10; // Cryptokeys per Common Supply Drop
constexpr int RARE_DROP_COST = 30;   // Cryptokeys per Rare Supply Drop
constexpr int COMMON_DROP_ITEMS = 3; // items per Common Supply Drop
constexpr int RARE_DROP_ITEMS = 3;   // items per Rare Supply Drop
constexpr int RARE_BONUS_MIN = 2;    // bonus Cryptokeys from a Rare drop...
constexpr int RARE_BONUS_MAX = 5;    // ...between these two
game::EngineDependentDvarMut dvar_cg_loot_open_sound;
game::EngineDependentDvarMut dvar_cg_loot_card_style;
game::EngineDependentDvarMut dvar_cg_loot_debug;
game::EngineDependentDvarMut dvar_cg_loot_server_rewards;
game::EngineDependentDvarMut dvar_cg_loot_server_max_keys;
game::EngineDependentDvarMut dvar_cg_loot_server_max_items;
game::EngineDependentDvarMut dvar_cg_loot_hide_blackmarket;
game::EngineDependentDvarMut dvar_cg_loot_reveal_sound;

std::mutex owned_loot_mutex;
std::unordered_set<uint32_t> owned_loot;

bool owns_loot(const uint32_t item_id) {
  std::lock_guard _(owned_loot_mutex);
  return owned_loot.contains(item_id);
}

void load_owned_loot() {
  std::string data;
  if (!utils::io::read_file(OWNED_LOOT_FILE, &data)) {
    return;
  }

  std::lock_guard _(owned_loot_mutex);
  std::istringstream stream(data);
  std::string line;
  while (std::getline(stream, line)) {
    uint32_t id = 0;
    const auto [end, error] =
        std::from_chars(line.data(), line.data() + line.size(), id);
    if (error == std::errc{} && id) {
      owned_loot.insert(id);
    }
  }
}

void save_owned_loot() {
  std::string data;
  {
    std::lock_guard _(owned_loot_mutex);
    for (const uint32_t id : owned_loot) {
      data += std::to_string(id);
      data += '\n';
    }
  }
  utils::io::write_file(OWNED_LOOT_FILE, data);
}

constexpr const char *RECENT_LOOT_FILE = "boiii_players/user/loot_recent.txt";
constexpr size_t MAX_RECENT_DROPS = 50;

struct recent_entry {
  uint32_t id;
  bool seen;
};

std::mutex recent_mutex;
std::deque<recent_entry> recent_drops;

void load_recent_drops() {
  std::string data;
  if (!utils::io::read_file(RECENT_LOOT_FILE, &data)) {
    return;
  }

  std::lock_guard _(recent_mutex);
  std::istringstream stream(data);
  uint32_t id = 0;
  int seen = 0;
  while (stream >> id >> seen) {
    recent_drops.push_back({id, seen != 0});
  }
  while (recent_drops.size() > MAX_RECENT_DROPS) {
    recent_drops.pop_front();
  }
}

void save_recent_drops() {
  std::string data;
  {
    std::lock_guard _(recent_mutex);
    for (const recent_entry &entry : recent_drops) {
      data += std::to_string(entry.id);
      data += entry.seen ? " 1\n" : " 0\n";
    }
  }
  utils::io::write_file(RECENT_LOOT_FILE, data);
}

void remember_drop(const uint32_t id) {
  std::lock_guard _(recent_mutex);
  recent_drops.push_back({id, false});
  while (recent_drops.size() > MAX_RECENT_DROPS) {
    recent_drops.pop_front();
  }
}

const pool::entry *find_item_by_id(const uint32_t id) {
  for (const pool::entry &item : pool::items) {
    if (item.id == id) {
      return &item;
    }
  }
  return nullptr;
}

int rarity_rank(const pool::rarity tier) {
  switch (tier) {
  case pool::common:
    return 0;
  case pool::rare:
    return 1;
  case pool::epic:
    return 2;
  case pool::legendary:
    return 3;
  default:
    return 4;
  }
}

constexpr std::array<uint32_t, 4> COMMON_DROP_ODDS{50, 35, 12, 3};
constexpr std::array<uint32_t, 4> RARE_DROP_ODDS{20, 45, 25, 10};

const pool::entry *roll_unowned_item(const bool boosted_odds = false,
                                     const int min_rank = 0) {
  std::array<std::vector<const pool::entry *>, 4> buckets;
  {
    std::lock_guard _(owned_loot_mutex);
    for (const pool::entry &item : pool::items) {
      const int rank = std::min(rarity_rank(item.tier), 3);
      if (rank >= min_rank && !owned_loot.contains(item.id)) {
        buckets[rank].push_back(&item);
      }
    }
  }

  const auto &odds = boosted_odds ? RARE_DROP_ODDS : COMMON_DROP_ODDS;
  uint32_t total = 0;
  for (size_t rank = 0; rank < buckets.size(); ++rank) {
    if (!buckets[rank].empty()) {
      total += odds[rank];
    }
  }
  if (!total) {
    return nullptr;
  }

  static std::mt19937_64 rng{std::random_device{}()};
  uint32_t roll = std::uniform_int_distribution<uint32_t>(0, total - 1)(rng);
  for (size_t rank = 0; rank < buckets.size(); ++rank) {
    if (buckets[rank].empty()) {
      continue;
    }
    if (roll < odds[rank]) {
      const auto &bucket = buckets[rank];
      return bucket[std::uniform_int_distribution<size_t>(0, bucket.size() -
                                                                 1)(rng)];
    }
    roll -= odds[rank];
  }
  return nullptr;
}

void play_ui_sound(const char *alias) {
  toast::run_lua(std::string("if Engine.PlaySound then Engine.PlaySound(\"") +
                 alias + "\") end");
}

void show_toast(const std::string &title, const std::string &body,
                const char *icon) {
  const bool in_match = game::com::Com_IsInGame();
  const std::string image = (icon && *icon && !in_match)
                                ? std::string(icon)
                                : std::string("blacktransparent");
  toast::precache_icon(image);

  scheduler::once(
      [title, body, image] {
        if (dvar_cg_loot_toast_style &&
            dvar_cg_loot_toast_style.get_int() == 1) {
          toast::reward(title, body, image);
        } else {
          toast::show(title, body, image);
        }
      },
      scheduler::pipeline::main, 400ms);
}

const char *toast_icon(const pool::entry &item) {
  const std::string_view category = item.category;
  if (category == "decal" || category == "emblem") {
    return "t7_icon_menu_simple_emblems";
  }
  return item.icon;
}

void log_item(const pool::entry &item) {
  printf("[loot] granted %s: %s  [%u %s]\n", item.title, item.display, item.id,
         item.name);
}

void add_owned(const pool::entry &item) {
  std::lock_guard _(owned_loot_mutex);
  owned_loot.insert(item.id);
}

#ifndef NDEBUG
void grant_item(const pool::entry &item) {
  add_owned(item);
  save_owned_loot();
  remember_drop(item.id);
  save_recent_drops();
  log_item(item);
  show_toast(item.title, item.display, toast_icon(item));
}
#endif

std::vector<const pool::entry *> roll_and_grant(const int count,
                                                const bool boosted_odds,
                                                const bool guarantee_rare) {
  std::vector<const pool::entry *> granted;
  for (int i = 0; i < count; ++i) {
    const pool::entry *item = nullptr;
    if (i == 0 && guarantee_rare) {
      item = roll_unowned_item(boosted_odds, 1);
    }
    if (!item) {
      item = roll_unowned_item(boosted_odds);
    }
    if (!item) {
      break;
    }
    add_owned(*item);
    remember_drop(item->id);
    log_item(*item);
    granted.push_back(item);
  }

  if (!granted.empty()) {
    save_owned_loot();
    save_recent_drops();
  }
  return granted;
}

void announce_drops(std::vector<const pool::entry *> granted) {
  if (granted.empty()) {
    show_toast("Black Market", "You own every item!",
               "uie_t7_blackmarket_promo_cryptokeys");
    return;
  }

  if (granted.size() == 1) {
    show_toast(granted[0]->title, granted[0]->display, toast_icon(*granted[0]));
    return;
  }

  std::stable_sort(granted.begin(), granted.end(),
                   [](const pool::entry *a, const pool::entry *b) {
                     return rarity_rank(a->tier) > rarity_rank(b->tier);
                   });
  printf("[loot] %zu drops received\n", granted.size());

  const int individual_limit = dvar_cg_loot_toast_individual
                                   ? dvar_cg_loot_toast_individual.get_int()
                                   : 3;
  if (static_cast<int>(granted.size()) <= individual_limit) {
    for (const pool::entry *item : granted) {
      show_toast(item->title, item->display, toast_icon(*item));
    }
    return;
  }

  const size_t max_chars = static_cast<size_t>(std::max(
      10, dvar_cg_loot_toast_chars ? dvar_cg_loot_toast_chars.get_int() : 28));

  struct toast_page {
    std::string body;
    const pool::entry *featured;
  };
  std::vector<toast_page> pages;
  std::vector<std::pair<const pool::entry *, int>> unique_items;
  for (const pool::entry *item : granted) {
    auto existing = std::find_if(
        unique_items.begin(), unique_items.end(), [&](const auto &entry) {
          return std::strcmp(entry.first->display, item->display) == 0;
        });
    if (existing != unique_items.end()) {
      ++existing->second;
    } else {
      unique_items.emplace_back(item, 1);
    }
  }

  for (const auto &[item, copies] : unique_items) {
    std::string label = item->display;
    if (copies > 1) {
      label += " x" + std::to_string(copies);
    }
    const size_t length = label.size();
    bool placed = false;
    for (toast_page &page : pages) {
      if (page.body.size() + 2 + length <= max_chars) {
        page.body += ", ";
        page.body += label;
        placed = true;
        break;
      }
    }
    if (!placed) {
      pages.push_back({label, item});
    }
  }

  const std::string base_title =
      "x" + std::to_string(granted.size()) + " Black Market Drops";
  for (size_t i = 0; i < pages.size(); ++i) {
    std::string title = base_title;
    if (pages.size() > 1) {
      title += " (" + std::to_string(i + 1) + "/" +
               std::to_string(pages.size()) + ")";
    }
    show_toast(title, pages[i].body, toast_icon(*pages[i].featured));
  }
}

int grant_random_drops(const int count) {
  auto granted = roll_and_grant(count, false, false);
  const int amount = static_cast<int>(granted.size());
  announce_drops(std::move(granted));
  return amount;
}

#ifndef NDEBUG
const pool::entry *find_item(const std::string_view query) {
  uint32_t id = 0;
  const auto [end, error] =
      std::from_chars(query.data(), query.data() + query.size(), id);
  const bool is_id = error == std::errc{} && end == query.data() + query.size();

  for (const pool::entry &item : pool::items) {
    if (is_id ? item.id == id
              : (query == item.name ||
                 query == std::string_view(item.name).substr(
                              0, std::string_view(item.name).find(';')))) {
      return &item;
    }
  }
  return nullptr;
}
#endif

constexpr const char *KEYS_FILE = "boiii_players/user/loot_keys.txt";

std::mutex keys_mutex;
int cryptokeys = 0;
std::atomic<int> key_seconds{0}; // playtime toward the next Cryptokey
int keys_earned_this_match = 0;
std::vector<const pool::entry *> items_earned_this_match;
int server_keys_this_match = 0;
int server_items_this_match = 0;

void load_keys() {
  std::string data;
  if (!utils::io::read_file(KEYS_FILE, &data)) {
    return;
  }
  std::istringstream stream(data);
  int value = 0;
  int seconds = 0;
  if (stream >> value && value >= 0) {
    std::lock_guard _(keys_mutex);
    cryptokeys = value;
  }
  if (stream >> seconds && seconds >= 0) {
    key_seconds = seconds;
  }
}

void save_keys() {
  int value;
  {
    std::lock_guard _(keys_mutex);
    value = cryptokeys;
  }
  utils::io::write_file(KEYS_FILE, std::to_string(value) + " " +
                                       std::to_string(key_seconds.load()));
}

int get_keys() {
  std::lock_guard _(keys_mutex);
  return cryptokeys;
}

void add_keys(const int amount) {
  if (amount <= 0) {
    return;
  }
  {
    std::lock_guard _(keys_mutex);
    cryptokeys = std::min(cryptokeys + amount, 99999);
  }
  save_keys();
}

bool spend_keys(const int amount) {
  {
    std::lock_guard _(keys_mutex);
    if (amount < 0 || cryptokeys < amount) {
      return false;
    }
    cryptokeys -= amount;
  }
  save_keys();
  return true;
}

int random_between(const int low, const int high) {
  static std::mt19937 rng{std::random_device{}()};
  return std::uniform_int_distribution<int>(low, std::max(low, high))(rng);
}

int supply_drop_cost(const bool rare) {
  return rare ? RARE_DROP_COST : COMMON_DROP_COST;
}

struct opened_drop {
  std::vector<uint32_t> items;
  int bonus_keys = 0;
  bool rare = false;
};
std::mutex last_drop_mutex;
opened_drop last_drop;

bool open_supply_drop(const bool rare, const bool quiet = false) {
  const int cost = supply_drop_cost(rare);
  if (!roll_unowned_item()) {
    show_toast("Black Market", "You own every item!",
               "uie_t7_blackmarket_promo_cryptokeys");
    return false;
  }
  if (!spend_keys(cost)) {
    play_ui_sound("uin_bm_denied");
    show_toast("Not enough Cryptokeys",
               "Need " + std::to_string(cost) + ", have " +
                   std::to_string(get_keys()),
               "uie_t7_blackmarket_promo_cryptokeys");
    return false;
  }

  const int items = rare ? RARE_DROP_ITEMS : COMMON_DROP_ITEMS;
  printf("[loot] opened %s supply drop (-%d Cryptokeys)\n",
         rare ? "rare" : "common", cost);
  auto granted = roll_and_grant(items, rare, rare);

  int bonus = 0;
  if (rare) {
    bonus = random_between(RARE_BONUS_MIN, RARE_BONUS_MAX);
    add_keys(bonus);
  }

  {
    std::lock_guard _(last_drop_mutex);
    last_drop.items.clear();
    for (const pool::entry *item : granted) {
      last_drop.items.push_back(item->id);
    }
    last_drop.bonus_keys = bonus;
    last_drop.rare = rare;
  }

  if (!quiet) {
    announce_drops(granted);
    if (bonus > 0) {
      show_toast("Bonus Cryptokeys", "+" + std::to_string(bonus),
                 "uie_t7_blackmarket_promo_cryptokeys");
    }
  }
  return true;
}

void loot_progression_frame() {
  if (!dvar_cg_loot_progression || !dvar_cg_loot_progression.get_bool()) {
    return;
  }

  const int pending = dvar_cg_loot_drop.get_int();
  if (pending > 0) {
    dvar_cg_loot_drop.set(0);
    grant_random_drops(pending);
  }

  const int pending_keys = dvar_cg_loot_keys.get_int();
  if (pending_keys > 0) {
    dvar_cg_loot_keys.set(0);
    add_keys(pending_keys);
    play_ui_sound("uin_bm_key_earned");
    show_toast("Cryptokeys Earned",
               "+" + std::to_string(pending_keys) + " (" +
                   std::to_string(get_keys()) + " total)",
               "uie_t7_blackmarket_promo_cryptokeys");
  }

  const bool in_match = game::com::Com_IsInGame();

  static bool was_in_match = false;
  if (was_in_match && !in_match) {
    if (keys_earned_this_match > 0) {
      play_ui_sound("uin_bm_key_earned");
      show_toast("Cryptokeys",
                 "+" + std::to_string(keys_earned_this_match) + " (" +
                     std::to_string(get_keys()) + " total)",
                 "uie_t7_blackmarket_promo_cryptokeys");
      keys_earned_this_match = 0;
    }
    if (!items_earned_this_match.empty()) {
      announce_drops(std::move(items_earned_this_match));
      items_earned_this_match.clear();
    }
    save_keys(); // keep partial progress toward the next key
    server_keys_this_match = 0;
    server_items_this_match = 0;
  }
  was_in_match = in_match;

  if (!in_match) {
    return;
  }

  const int key_minutes = KEY_MINUTES;
  if (key_minutes > 0 && ++key_seconds >= key_minutes * 60) {
    key_seconds = 0;
    add_keys(1); // also saves progress; announced when the match ends
    ++keys_earned_this_match;
  } else if (key_minutes > 0 && key_seconds % 15 == 0) {
    save_keys();
  }
}

int loot_getitemquantity_stub(const game::ControllerIndex_t controller_index,
                              const game::eModes mode, const int item_id) {
  if (mode == game::eModes::ZOMBIES) {
    const std::optional<uint32_t> quantity =
        currency::item_quantity(controller_index, item_id);
    if (quantity.has_value()) {
      return quantity.value();
    }
  }

  if (dvar_cg_unlockall_loot.get_bool()) {
    return 1;
  }

  if (dvar_cg_loot_progression && dvar_cg_loot_progression.get_bool() &&
      owns_loot(static_cast<uint32_t>(item_id))) {
    return 1;
  }

  return loot_getitemquantity_hook.invoke<uint32_t>(controller_index, mode,
                                                    item_id);
}

uint32_t liveinventory_getitemquantity_stub(
    const game::ControllerIndex_t controller_index, const uint32_t item_id) {
  if (dvar_cg_unlockall_loot.get_bool() &&
      (item_id == 99003 || (item_id >= 99018 && item_id <= 99021) ||
       item_id == 99025 || (item_id >= 90047 && item_id <= 90064))) {
    return 1;
  }

  if (dvar_cg_unlockall_cac_slots.get_bool() && item_id == 99003) {
    return 1;
  }

  if (const auto quantity =
          currency::item_quantity(controller_index, item_id)) {
    return *quantity;
  }

  return liveinventory_getitemquantity_hook.invoke<int>(controller_index,
                                                        item_id);
}

bool liveinventory_areextraslotspurchased_stub(
    const game::ControllerIndex_t controller_index) {
  if (dvar_cg_unlockall_cac_slots.get_bool()) {
    return true;
  }

  return liveinventory_areextraslotspurchased_hook.invoke<bool>(
      controller_index);
}

bool bg_unlockablesisitempurchased_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index) {
  if (dvar_cg_unlockall_purchases.get_bool()) {
    return true;
  }

  return bg_unlockablesisitempurchased_hook.invoke<bool>(mode, controller_index,
                                                         item_index);
}

bool bg_unlockablesisitemattachmentlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index, int attachment_num) {
  if (dvar_cg_unlockall_attachments.get_bool()) {
    return false;
  }

  return bg_unlockablesisitemattachmentlocked_hook.invoke<bool>(
      mode, controller_index, item_index, attachment_num);
}

bool bg_unlockablesisattachmentslotlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controller_index,
    int item_index, int attachment_slot_index) {
  if (dvar_cg_unlockall_attachments.get_bool()) {
    return false;
  }

  return bg_unlockablesisattachmentslotlocked_hook.invoke<bool>(
      mode, controller_index, item_index, attachment_slot_index);
}

bool bg_unlockablesitemoptionlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    int itemIndex, int optionIndex) {
  if (dvar_cg_unlockall_camos_and_reticles.get_bool()) {
    return false;
  }

  return bg_unlockablesitemoptionlocked_hook.invoke<bool>(
      mode, controllerIndex, itemIndex, optionIndex);
}

bool bg_unlockablesemblemorbackinglockedbychallenge_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    game::emblemChallengeLookup_t *challengeLookup, bool otherPlayer) {
  if (dvar_cg_unlockall_calling_cards.get_bool()) {
    return false;
  }

  return bg_unlockablesemblemorbackinglockedbychallenge_hook.invoke<bool>(
      mode, controllerIndex, challengeLookup, otherPlayer);
}

bool bg_unlockedgetchallengeunlockedforindex_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    unsigned __int16 index, int itemIndex) {
  if (dvar_cg_unlockall_camos_and_reticles.get_bool()) {
    return true;
  }

  return bg_unlockedgetchallengeunlockedforindex_hook.invoke<bool>(
      mode, controllerIndex, index, itemIndex);
}

bool bg_unlockablescharactercustomizationitemlocked_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex,
    uint32_t characterIndex, game::CharacterItemType itemType, int itemIndex) {
  if (dvar_cg_unlockall_specialists_outfits.get_bool()) {
    return false;
  }

  return bg_unlockablescharactercustomizationitemlocked_hook.invoke<bool>(
      mode, controllerIndex, characterIndex, itemType, itemIndex);
}

bool bg_emblemisentitlementbackgroundgranted_stub(
    const game::ControllerIndex_t controllerIndex,
    game::BGEmblemBackgroundID backgroundId) {
  if (dvar_cg_unlockall_calling_cards.get_bool() &&
      (backgroundId != 684 && backgroundId != 685 && backgroundId != 687 &&
       backgroundId != 693 && backgroundId != 695 && backgroundId != 701 &&
       backgroundId != 703 && backgroundId != 707 && backgroundId != 708)) {
    return true;
  }

  return bg_emblemisentitlementbackgroundgranted_hook.invoke<bool>(
      controllerIndex, backgroundId);
}

bool liveentitlements_isentitlementactiveforcontroller_stub(
    const game::ControllerIndex_t controllerIndex, uint32_t incentiveId) {
  if (dvar_cg_unlockall_calling_cards.get_bool() && incentiveId != 29) {
    return true;
  }

  return liveentitlements_isentitlementactiveforcontroller_hook.invoke<bool>(
      controllerIndex, incentiveId);
}

int bg_unlockablesgetcustomclasscount_stub(
    game::eModes mode, const game::ControllerIndex_t controllerIndex) {
  if (dvar_cg_unlockall_cac_slots.get_bool()) {
    return 10;
  }

  return bg_unlockablesgetcustomclasscount_hook.invoke<int>(mode,
                                                            controllerIndex);
}

bool GScr_IsItemPurchasedForClientNum_AlwaysTrue(
    [[maybe_unused]] game::ClientNum_t clientNum,
    [[maybe_unused]] int itemIndex) {
  return true;
}
}; // namespace

namespace {
enum class reward_type : uint8_t {
  keys = 1,
  drops = 2,
};

void apply_reward(const reward_type type, int amount) {
  if (!dvar_cg_loot_progression || !dvar_cg_loot_progression.get_bool() ||
      amount <= 0) {
    return;
  }
  if (dvar_cg_loot_server_rewards && !dvar_cg_loot_server_rewards.get_bool()) {
    debug_log("ignored a server reward (cg_loot_server_rewards is 0)");
    return;
  }

  if (type == reward_type::keys) {
    const int cap = dvar_cg_loot_server_max_keys
                        ? dvar_cg_loot_server_max_keys.get_int()
                        : 100;
    amount = std::min(amount, std::max(0, cap - server_keys_this_match));
    server_keys_this_match += amount;
  } else if (type == reward_type::drops) {
    const int cap = dvar_cg_loot_server_max_items
                        ? dvar_cg_loot_server_max_items.get_int()
                        : 10;
    amount = std::min(amount, std::max(0, cap - server_items_this_match));
    server_items_this_match += amount;
  }
  if (amount <= 0) {
    debug_log("server reward over this match's limit, ignored");
    return;
  }

  if (game::com::Com_IsInGame()) {
    if (type == reward_type::keys) {
      add_keys(amount);
      keys_earned_this_match += amount;
    } else if (type == reward_type::drops) {
      for (const pool::entry *item : roll_and_grant(amount, false, false)) {
        items_earned_this_match.push_back(item);
      }
    }
    return;
  }

  if (type == reward_type::keys) {
    add_keys(amount);
    play_ui_sound("uin_bm_key_earned");
    show_toast("Cryptokeys Earned",
               "+" + std::to_string(amount) + " (" +
                   std::to_string(get_keys()) + " total)",
               "uie_t7_blackmarket_promo_cryptokeys");
  } else if (type == reward_type::drops) {
    grant_random_drops(amount);
  }
}

} // namespace

std::vector<recent_drop> get_recent_drops(const size_t max_count) {
  std::vector<recent_drop> result;
  std::lock_guard _(recent_mutex);
  for (auto it = recent_drops.rbegin();
       it != recent_drops.rend() && result.size() < max_count; ++it) {
    const pool::entry *item = find_item_by_id(it->id);
    if (!item) {
      continue;
    }
    result.push_back({item->title, item->display, item->icon, item->category,
                      rarity_rank(item->tier), !it->seen});
  }
  return result;
}

void mark_recent_drops_seen() {
  bool changed = false;
  {
    std::lock_guard _(recent_mutex);
    for (recent_entry &entry : recent_drops) {
      changed |= !entry.seen;
      entry.seen = true;
    }
  }
  if (changed) {
    save_recent_drops();
  }
}

size_t unseen_drop_count() {
  std::lock_guard _(recent_mutex);
  return static_cast<size_t>(
      std::count_if(recent_drops.begin(), recent_drops.end(),
                    [](const recent_entry &entry) { return !entry.seen; }));
}

size_t owned_item_count() {
  std::lock_guard _(owned_loot_mutex);
  return owned_loot.size();
}

size_t total_item_count() { return std::size(pool::items); }

void receive_server_keys(const int amount) {
  apply_reward(reward_type::keys, std::clamp(amount, 0, 1000));
}

void receive_server_drops(const int amount) {
  apply_reward(reward_type::drops, std::clamp(amount, 0, 100));
}

void debug_log(const std::string &message) {
  printf("[loot] %s\n", message.c_str());
}

int cryptokey_balance() { return get_keys(); }

float next_cryptokey_progress() {
  const int minutes = KEY_MINUTES;
  if (minutes <= 0) {
    return 0.0f;
  }
  return std::clamp(static_cast<float>(key_seconds.load()) / (minutes * 60.0f),
                    0.0f, 1.0f);
}

int supply_drop_price(const bool rare) { return supply_drop_cost(rare); }

bool open_supply_drop_from_ui(const bool rare) {
  return open_supply_drop(rare, true);
}

namespace {
std::string item_subtitle(const pool::entry &item) {
  const std::string_view title = item.title;
  const size_t dash = title.find(" - ");
  const std::string context =
      dash == std::string_view::npos ? "" : std::string(title.substr(dash + 3));
  const std::string_view category = item.category;
  const auto with_context = [&](const char *suffix) {
    return context.empty() ? std::string(suffix) : context + " " + suffix;
  };

  if (category == "camo")
    return with_context("Camo");
  if (category == "specialist_outfit")
    return with_context("Theme");
  if (category == "gesture")
    return with_context("Gesture");
  if (category == "taunt")
    return with_context("Taunt");
  if (category == "attachment_variant")
    return with_context("Attachment");
  if (category == "weapon")
    return "Weapon";
  if (category == "melee_weapon")
    return "Melee Weapon";
  if (category == "calling_card")
    return "Calling Card";
  if (category == "decal")
    return "Decal";
  if (category == "emblem")
    return "Emblem";
  if (category == "reticle")
    return "Reticle";
  if (category == "material")
    return "Paintshop Material";
  return std::string(category);
}

const char *rank_name(const int rank) {
  static constexpr const char *names[] = {"Common", "Rare", "Epic", "Legendary",
                                          "Limited"};
  return names[std::clamp(rank, 0, 4)];
}
} // namespace

last_supply_drop get_last_supply_drop() {
  last_supply_drop result;
  std::lock_guard _(last_drop_mutex);
  result.bonus_keys = last_drop.bonus_keys;
  result.rare = last_drop.rare;
  for (const uint32_t id : last_drop.items) {
    const pool::entry *item = find_item_by_id(id);
    if (!item) {
      continue;
    }
    const int rank = rarity_rank(item->tier);
    result.items.push_back({item->display, item_subtitle(*item), item->icon,
                            rank_name(rank), item->category, rank});
  }
  return result;
}

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "loot"; }
#endif

  void post_unpack() override {
    GScr_IsItemPurchasedForClientNum_hook.create(
        game::scr::gscr::GScr_IsItemPurchasedForClientNum,
        GScr_IsItemPurchasedForClientNum_AlwaysTrue);

    if (game::is_server()) {
      return;
    }

    dvar_cg_unlockall_loot = game::register_dvar_bool(
        "cg_unlockall_loot", false, game::DvarFlags{.archive = 1},
        "Unlocks blackmarket loot");
    dvar_cg_unlockall_gobblegums = game::register_dvar_bool(
        "cg_unlockall_gobblegums", false, game::DvarFlags{.archive = 1},
        "Provides unlimited GobbleGums without changing saved inventory");
    dvar_cg_unlockall_purchases = game::register_dvar_bool(
        "cg_unlockall_purchases", false, game::DvarFlags{.archive = 1},
        "Unlock all purchases with tokens");
    dvar_cg_unlockall_attachments = game::register_dvar_bool(
        "cg_unlockall_attachments", false, game::DvarFlags{.archive = 1},
        "Unlocks all attachments");
    dvar_cg_unlockall_camos_and_reticles = game::register_dvar_bool(
        "cg_unlockall_camos_and_reticles", false, game::DvarFlags{.archive = 1},
        "Unlocks all camos and reticles");
    dvar_cg_unlockall_calling_cards = game::register_dvar_bool(
        "cg_unlockall_calling_cards", false, game::DvarFlags{.archive = 1},
        "Unlocks all calling cards");
    dvar_cg_unlockall_specialists_outfits = game::register_dvar_bool(
        "cg_unlockall_specialists_outfits", false,
        game::DvarFlags{.archive = 1}, "Unlocks all specialists outfits");
    dvar_cg_unlockall_cac_slots = game::register_dvar_bool(
        "cg_unlockall_cac_slots", false, game::DvarFlags{.archive = 1},
        "Unlocks all Create a Class Slots");

    dvar_cg_loot_progression = game::register_dvar_bool(
        "cg_loot_progression", true, game::DvarFlags{.archive = 1},
        "Earn individual Black Market items through drops");
    dvar_cg_loot_drop = game::register_dvar_int(
        "cg_loot_drop", 0, 0, 100, game::DvarFlags{},
        "Pending Black Market drops to grant (settable by servers)");

    dvar_cg_loot_toast_style = game::register_dvar_int(
        "cg_loot_toast_style", 1, 0, 1, game::DvarFlags{.archive = 1},
        "Black Market drop toast: 0 = compact, 1 = Black Market style");

    dvar_cg_loot_toast_chars = game::register_dvar_int(
        "cg_loot_toast_chars", 28, 10, 80, game::DvarFlags{.archive = 1},
        "Max characters of item names per multi-drop toast line");

    dvar_cg_loot_toast_individual = game::register_dvar_int(
        "cg_loot_toast_individual", 3, 0, 100, game::DvarFlags{.archive = 1},
        "Batches up to this size show one detailed toast per item");

    dvar_cg_loot_keys = game::register_dvar_int(
        "cg_loot_keys", 0, 0, 10000, game::DvarFlags{},
        "Pending Cryptokeys to award (settable by servers)");

    dvar_cg_loot_hide_blackmarket = game::register_dvar_int(
        "cg_loot_hide_blackmarket", 1, 0, 1, game::DvarFlags{.archive = 1},
        "1 = our Black Market replaces the original button, 0 = show both");

    dvar_cg_loot_debug =
        game::register_dvar_bool("cg_loot_debug", false, game::DvarFlags{},
                                 "Log Black Market reward events (server "
                                 "script notifies) to the console");

    dvar_cg_loot_server_rewards = game::register_dvar_bool(
        "cg_loot_server_rewards", true, game::DvarFlags{.archive = 1},
        "Accept Cryptokeys/items given by servers (0 = ignore them all)");
    dvar_cg_loot_server_max_keys = game::register_dvar_int(
        "cg_loot_server_max_keys", 100, 0, 100000,
        game::DvarFlags{.archive = 1},
        "Most Cryptokeys a server can give you per match");
    dvar_cg_loot_server_max_items = game::register_dvar_int(
        "cg_loot_server_max_items", 10, 0, 1000, game::DvarFlags{.archive = 1},
        "Most Black Market items a server can give you per match");

    dvar_cg_loot_card_style = game::register_dvar_int(
        "cg_loot_card_style", 1, 0, 1, game::DvarFlags{.archive = 1},
        "Supply drop reveal cards: 1 = Black Market art, 0 = drawn cards");

    scheduler::once(
        [] {
          dvar_cg_loot_open_sound = game::register_dvar_string(
              "cg_loot_open_sound", "", game::DvarFlags{.archive = 1},
              "Supply drop open sound (empty = BO3 default, \"none\" = off)");
          dvar_cg_loot_reveal_sound = game::register_dvar_string(
              "cg_loot_reveal_sound", "", game::DvarFlags{.archive = 1},
              "Reveal card sound (empty = BO3 default, \"none\" = off)");
        },
        scheduler::pipeline::main);

    load_owned_loot();
    load_recent_drops();
    load_keys();
    scheduler::loop(loot_progression_frame, scheduler::pipeline::main, 1s);

#ifndef NDEBUG
    command::add("lootdrop", [](const command::params &params) {
      int count = 1;
      if (params.size() > 1) {
        count = std::clamp(std::atoi(params.get(1)), 1, 100);
      }
      grant_random_drops(count);
    });
#endif

    command::add("lootstatus", [](const command::params &) {
      size_t owned = 0;
      {
        std::lock_guard _(owned_loot_mutex);
        owned = owned_loot.size();
      }
      printf("[loot] owned %zu / %zu Black Market items\n", owned,
             std::size(pool::items));
    });

#ifndef NDEBUG
    command::add("lootdumpstrings", [](const command::params &) {
      std::string out;
      game::db::xasset::DB_EnumXAssets(
          game::db::xasset::XAssetType::LOCALIZE_ENTRY,
          [](game::db::xasset::XAssetHeader header, void *data) {
            const auto *entry = header.localize;
            if (!entry || !entry->name || !entry->value) {
              return;
            }
            auto &buffer = *static_cast<std::string *>(data);
            buffer += entry->name;
            buffer += '\t';
            for (const char *c = entry->value; *c; ++c) {
              buffer += (*c == '\n' || *c == '\r' || *c == '\t') ? ' ' : *c;
            }
            buffer += '\n';
          },
          &out, true);

      const char *path = "boiii_players/user/localize_dump.txt";
      if (utils::io::write_file(path, out)) {
        printf("[loot] wrote %zu bytes of localized strings to %s\n",
               out.size(), path);
      } else {
        printf("[loot] failed to write %s\n", path);
      }
    });
#endif

#ifndef NDEBUG
    command::add("lootgive", [](const command::params &params) {
      if (params.size() < 2) {
        printf("[loot] usage: lootgive <item id or internal name>\n");
        return;
      }
      const pool::entry *item = find_item(params.get(1));
      if (!item) {
        printf("[loot] no Black Market item named %s\n", params.get(1));
        return;
      }
      grant_item(*item);
    });
#endif

    command::add("lootkeys", [](const command::params &params) {
      if (params.size() > 1) {
        add_keys(std::atoi(params.get(1)));
      }
      printf("[loot] %d Cryptokeys\n", get_keys());
    });

    command::add("lootopen", [](const command::params &params) {
      const bool rare =
          params.size() > 1 && std::string_view(params.get(1)) == "rare";
      open_supply_drop(rare);
    });

    command::add("lootlist", [](const command::params &) {
      std::lock_guard _(owned_loot_mutex);
      for (const pool::entry &item : pool::items) {
        if (owned_loot.contains(item.id)) {
          printf("[loot] %s: %s\n", item.title, item.display);
        }
      }
      printf("[loot] owned %zu / %zu Black Market items\n", owned_loot.size(),
             std::size(pool::items));
    });

#ifndef NDEBUG
    command::add("lootdumpimages", [](const command::params &) {
      std::string out;
      game::db::xasset::DB_EnumXAssets(
          game::db::xasset::XAssetType::IMAGE,
          [](game::db::xasset::XAssetHeader header, void *data) {
            const auto *image = header.image;
            if (!image || !image->name) {
              return;
            }
            auto &buffer = *static_cast<std::string *>(data);
            buffer += image->name;
            buffer += '\n';
          },
          &out, true);

      const char *path = "boiii_players/user/image_dump.txt";
      if (utils::io::write_file(path, out)) {
        printf("[loot] wrote %zu bytes of image names to %s\n", out.size(),
               path);
      } else {
        printf("[loot] failed to write %s\n", path);
      }
    });
#endif

#ifndef NDEBUG
    command::add("lootsound", [](const command::params &params) {
      if (params.size() < 2) {
        printf("[loot] usage: lootsound <sound alias>\n");
        return;
      }
      std::string alias = params.get(1);
      alias.erase(std::remove_if(alias.begin(), alias.end(),
                                 [](const char c) {
                                   return c == '"' || c == '\\' || c == '\n';
                                 }),
                  alias.end());
      toast::run_lua("if Engine.PlaySound then Engine.PlaySound(\"" + alias +
                     "\") end");
      printf("[loot] playing %s\n", alias.c_str());
    });
#endif

#ifndef NDEBUG
    command::add("lootdumpsounds", [](const command::params &) {
      std::string out;
      game::db::xasset::DB_EnumXAssets(
          game::db::xasset::XAssetType::SOUND,
          [](game::db::xasset::XAssetHeader header, void *data) {
            const auto *bank = header.sound;
            if (!bank || !bank->alias) {
              return;
            }
            auto &buffer = *static_cast<std::string *>(data);
            for (uint32_t i = 0; i < bank->aliasCount; ++i) {
              const auto &list = bank->alias[i];
              const char *name = list.name;
              if (!name && list.head) {
                name = list.head->name;
              }
              if (name && *name) {
                buffer += name;
                buffer += '\n';
              }
            }
          },
          &out, true);

      const char *path = "boiii_players/user/sound_dump.txt";
      if (utils::io::write_file(path, out)) {
        printf("[loot] wrote %zu bytes of sound aliases to %s\n", out.size(),
               path);
      } else {
        printf("[loot] failed to write %s\n", path);
      }
    });
#endif

    command::add("lootreset", [](const command::params &) {
      {
        std::lock_guard _(owned_loot_mutex);
        owned_loot.clear();
      }
      save_owned_loot();
      printf("[loot] owned Black Market items cleared\n");
    });

    command::add("unlockall", [](const command::params &) {
      if (game::com::Com_IsInGame()) {
        toast::error(
            "Unlock All",
            "Cannot use unlockall while in-game. Return to main menu first.");
        return;
      }

      const game::eModes mode = game::com::Com_SessionMode_GetMode();
      if (mode != game::eModes::MULTIPLAYER && mode != game::eModes::ZOMBIES &&
          mode != game::eModes::CAMPAIGN) {
        toast::error(
            "Unlock All",
            "Open Multiplayer, Zombies, or Campaign before using unlockall.");
        return;
      }

      dvar_cg_unlockall_loot.set(true);
      dvar_cg_unlockall_gobblegums.set(true);
      dvar_cg_unlockall_purchases.set(true);
      dvar_cg_unlockall_attachments.set(true);
      dvar_cg_unlockall_camos_and_reticles.set(true);
      dvar_cg_unlockall_calling_cards.set(true);
      dvar_cg_unlockall_specialists_outfits.set(true);
      dvar_cg_unlockall_cac_slots.set(true);
      game::ui_enableAllHeroes->set(true);

      const char *mode_name = nullptr;

      if (mode == game::eModes::MULTIPLAYER) {
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "PrestigeStatsMaster 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname plevel 11\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname hasprestiged 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname rank 54\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rank 944\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rankxp 56800000\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "uploadstats 1\n");
        mode_name = "Multiplayer";
      } else if (mode == game::eModes::ZOMBIES) {
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "PrestigeStatsMaster 0\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname plevel 11\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname hasprestiged 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname rank 34\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rank 999\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rankxp 56800000\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_zod_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_zod_super_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_factory_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_factory_super_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_castle_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_castle_super_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_island_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_island_super_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_stalingrad_ee 1\n");
        game::cbuf::Cbuf_AddText(
            game::LOCAL_CLIENT_0,
            "statsetbyname darkops_stalingrad_super_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname darkops_genesis_ee 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname DARKOPS_GENESIS_SUPER_EE 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "uploadstats 0\n");
        mode_name = "Zombies";
      } else {
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "PrestigeStatsMaster 2\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname plevel 11\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname hasprestiged 1\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname rank 19\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rank 999\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0,
                                 "statsetbyname paragon_rankxp 0\n");
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "uploadstats 2\n");
        mode_name = "Campaign";
      }

      toast::success("Unlock All",
                     std::string(mode_name) + " unlocks applied.");
    });

    loot_getitemquantity_hook.create(
        game::select(0x141E76170, 0x141E82C00, 0x0), loot_getitemquantity_stub);
    liveinventory_getitemquantity_hook.create(
        game::select(0x141DFC5A0, 0x141E09030, 0x0),
        liveinventory_getitemquantity_stub);
    liveinventory_areextraslotspurchased_hook.create(
        game::select(0x141DFBEC0, 0x141E08950, 0x0),
        liveinventory_areextraslotspurchased_stub);
    bg_unlockablesisitempurchased_hook.create(
        game::select(0x1426304B0, 0x1426A9620, 0x0),
        bg_unlockablesisitempurchased_stub);
    bg_unlockablesisitemattachmentlocked_hook.create(
        game::select(0x14262F760, 0x1426A88D0, 0x0),
        bg_unlockablesisitemattachmentlocked_stub);
    bg_unlockablesisattachmentslotlocked_hook.create(
        game::select(0x14262F560, 0x1426A86D0, 0x0),
        bg_unlockablesisattachmentslotlocked_stub);
    bg_unlockablesitemoptionlocked_hook.create(
        game::select(0x142631550, 0x1426AA6C0, 0x0),
        bg_unlockablesitemoptionlocked_stub);
    bg_unlockablesemblemorbackinglockedbychallenge_hook.create(
        game::select(0x14262A970, 0x1426A3AE0, 0x0),
        bg_unlockablesemblemorbackinglockedbychallenge_stub);
    bg_unlockedgetchallengeunlockedforindex_hook.create(
        game::select(0x142636480, 0x1426AF5F0, 0x0),
        bg_unlockedgetchallengeunlockedforindex_stub);
    bg_unlockablescharactercustomizationitemlocked_hook.create(
        game::select(0x142628EC0, 0x1426A2030, 0x0),
        bg_unlockablescharactercustomizationitemlocked_stub);
    bg_emblemisentitlementbackgroundgranted_hook.create(
        game::select(0x1425EE3B0, 0x142667520, 0x0),
        bg_emblemisentitlementbackgroundgranted_stub);
    liveentitlements_isentitlementactiveforcontroller_hook.create(
        game::select(0x141E05A50, 0x141E124E0, 0x0),
        liveentitlements_isentitlementactiveforcontroller_stub);
    bg_unlockablesgetcustomclasscount_hook.create(
        game::select(0x14262C790, 0x1426A5900, 0x0),
        bg_unlockablesgetcustomclasscount_stub);

    scheduler::once(
        []() {
          if (dvar_cg_unlockall_loot.get_bool()) {
            game::ui_enableAllHeroes->set(true);
          }
        },
        scheduler::pipeline::dvars_loaded);
  }
};
}; // namespace loot

REGISTER_COMPONENT(loot::component)
