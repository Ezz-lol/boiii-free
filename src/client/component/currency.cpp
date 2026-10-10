#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "com.hpp"
#include "command.hpp"
#include "currency.hpp"
#include "local_db.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <atomic>
#include <ctime>
#include <random>

namespace currency {
namespace {
using game::ControllerIndex_t;
using game::StorageFileType;
using game::live::inventory::InventoryItem;
using namespace game::ddl;
using namespace game::live::storage;
using namespace game::loot;

game::EngineDependentDvarMut local_currency;
game::EngineDependentDvarMut points_per_minute, points_per_match_cap;
game::EngineDependentDvarMut divinium_per_match, last_divinium_award;
game::EngineDependentDvarMut max_local_currencies;
game::EngineDependentDvarMut free_distills, paid_distills, free_distill_expiry;
utils::hook::detour currency_hook, spend_hook, increment_hook, consume_hook;
utils::hook::detour inventory_update_hook, buy_crate_hook, purchase_hook,
    get_item_hook;

// `bool Loot_RewardIsProcessing(const ControllerIndex_t controllerIndex)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t controllerIndex)> Loot_RewardIsProcessing{
    0x141E74120, 0x141E80BB0, 0x0};
// `bool __fastcall LiveInventory_IsValid(const ControllerIndex_t
// controllerIndex)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t controllerIndex)> LiveInventory_IsValid{
    0x141DFD920, 0x141E0A3B0, 0x0};
// `bool LiveInventory_UpdatePlayerBalance(const ControllerIndex_t
// controllerIndex, InventoryCurrency currencyType, uint32_t newValue)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t, int, int)>
    LiveInventory_UpdatePlayerBalance{0x141DFE0C0, 0x141E0AB50, 0x0};
// `bool LiveInventory_UpdateItemQuantity(const ControllerIndex_t
// controllerIndex, const uint32_t itemId, const uint32_t newAmount, const
// uint32_t modDateTime, const uint32_t collisionField)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t, uint32_t, uint32_t, uint32_t, uint32_t,
                  uint16_t)>
    LiveInventory_UpdateItemQuantity{0x141DFDFA0, 0x141E0AA30, 0x0};
// `const char * StringTable_GetColumnValueForRow(const StringTable *table,
// const int32_t row, const int32_t column)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<const char *(const StringTable *, int, int)>
    StringTable_GetColumnValueForRow{0x142251B40, 0x1422AE660, 0x0};
// `const char * StringTable_Lookup(const StringTable *table, const
// int32_t comparisonColumn, const char *value, const int32_t valueColumn)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<const char *(const StringTable *, int, const char *, int)>
    StringTable_Lookup{0x142251CD0, 0x1422AE7F0, 0x0};

inline bool enabled() { return local_currency && local_currency.get_bool(); }

StorageFileType stats_type(bool zombies) {
  const bool online = game::com::Com_SessionMode_GetNetworkMode() ==
                      game::eNetworkModes::ONLINE;
  if (zombies) {
    return online ? StorageFileType::ZM_STATS_ONLINE
                  : StorageFileType::ZM_STATS_OFFLINE;
  }
  return online ? StorageFileType::MP_STATS_ONLINE
                : StorageFileType::MP_STATS_OFFLINE;
}

struct stats {
  ControllerIndex_t controller;
  StorageFileType file;
  DDLContext *context;
  struct change {
    DDLState state;
    uint32_t before, after;
  };
  std::vector<change> changes;

  stats(ControllerIndex_t index, bool zombies)
      : controller(index), file(stats_type(zombies)),
        context(index >= 0 && index < 2 ? Storage_GetDDLContext(index, file, 0)
                                        : nullptr) {}

  std::optional<DDLState> find(std::initializer_list<const char *> path,
                               int32_t item = -1) const {
    if (!context || !context->buff || !context->def) {
      return std::nullopt;
    }
    const game::ddl::DDLState *root = Storage_GetDDLRootState(file);
    // The native lookup returns address 80 when the file type is missing.
    if (reinterpret_cast<uintptr_t>(root) < 0x1000) {
      return std::nullopt;
    }
    game::ddl::DDLState state = *root;
    if (state.ddlDef != context->def) {
      return std::nullopt;
    }
    for (const char *name : path) {
      DDLState next{};
      if (!DDL_MoveToName(&state, &next, name)) {
        return std::nullopt;
      }
      state = next;
      if (item >= 0 && std::string_view(name) == "ItemStats") {
        if (!DDL_MoveToIndex(&state, &next, item)) {
          return std::nullopt;
        }
        state = next;
      }
    }
    if (!state.isValid || !state.member || state.member->arraySize != 1 ||
        (state.member->type != 0 && state.member->type != 2)) {
      return std::nullopt;
    }
    return state;
  }

  uint32_t get(const DDLState &state) const {
    for (const change &entry : changes) {
      if (entry.state.offset == state.offset) {
        return entry.after;
      }
    }
    return DDL_GetUInt(&state, context);
  }

  void set(const DDLState &state, uint32_t value) {
    for (change &entry : changes) {
      if (entry.state.offset == state.offset) {
        entry.after = value;
        return;
      }
    }
    changes.push_back({state, get(state), value});
  }

  bool commit() {
    size_t written = 0;
    for (const change &entry : changes) {
      if (entry.after > entry.state.member->rangeLimit ||
          !DDL_SetUInt(&entry.state, context, entry.after)) {
        break;
      }
      ++written;
    }
    if (written == changes.size() && Storage_Write(controller, file, 0)) {
      return true;
    }
    while (written) {
      const change &entry = changes[--written];
      DDL_SetUInt(&entry.state, context, entry.before);
    }
    return false;
  }
};

std::optional<DDLState> player_stat(const stats &data, const char *name) {
  return data.find({"PlayerStatsList", name, "statValue"});
}

std::optional<uint32_t> vials(const stats &data) {
  const std::optional<DDLState> gained = player_stat(data, "BGB_TOKENS_GAINED");
  const std::optional<DDLState> used = player_stat(data, "BGB_TOKENS_USED");
  if (!gained || !used) {
    return std::nullopt;
  }
  return accounting::remaining(data.get(*gained), data.get(*used));
}

const StringTable *table(const char *name) {
  return reinterpret_cast<const StringTable *>(
      game::db::xasset::DB_FindXAssetHeader(
          game::db::xasset::XAssetType::STRINGTABLE, name, false, 0)
          .named);
}

struct gum {
  uint32_t id;
  uint32_t stat_index;
  std::string reference;
};

std::atomic<uint64_t> gum_generation{1};

void invalidate_gum_pool(const char *) { ++gum_generation; }

const std::vector<gum> &gum_pool() {
  thread_local std::vector<gum> pool;
  thread_local uint64_t generation = 0;
  const uint64_t current_generation = gum_generation.load();
  if (generation != current_generation || pool.empty()) {
    pool.clear();
    generation = current_generation;
    const StringTable *items = table("gamedata/loot/zmlootitems.csv");
    const StringTable *stat_table =
        table("gamedata/stats/zm/zm_statsTable.csv");
    if (items && stat_table) {
      for (int32_t row = 0; row < stat_table->rowCount; ++row) {
        const char *type = StringTable_GetColumnValueForRow(stat_table, row, 2);
        if (type && std::string_view(type) == "bubblegum_consumable") {
          const char *index_text =
              StringTable_GetColumnValueForRow(stat_table, row, 0);
          const char *ref =
              StringTable_GetColumnValueForRow(stat_table, row, 4);
          if (index_text && ref && ref[0] && strlen(ref) < 256) {
            const std::optional<uint32_t> index =
                accounting::parse_amount(index_text);
            const char *id_text = StringTable_Lookup(items, 0, ref, 1);
            if (id_text) {
              uint32_t id = 0;
              const char *end = id_text + strlen(id_text);
              const std::from_chars_result parsed =
                  std::from_chars(id_text, end, id);
              if (index && *index < 256 && *ref && strlen(ref) < 256 && id &&
                  parsed.ec == std::errc{} && parsed.ptr == end) {
                pool.push_back({id, *index, ref});
              }
            }
          }
        }
      }
    }
  }
  return pool;
}

std::optional<DDLState> gum_stat(const stats &data, const gum &item,
                                 const char *name) {
  return data.find({"ItemStats", "stats", name, "statValue"}, item.stat_index);
}

std::optional<uint32_t> quantity(const stats &data, const gum &item) {
  const std::optional<DDLState> gained =
      gum_stat(data, item, "bgbconsumablesgained");
  const std::optional<DDLState> used =
      gum_stat(data, item, "bgbconsumablesused");

  return gained && used ? std::optional(accounting::remaining(data.get(*gained),
                                                              data.get(*used)))
                        : std::nullopt;
}

// Multiplayer Black Market.
constexpr int32_t cryptokeys_currency =
    static_cast<int32_t>(game::lua::InventoryCurrency::MP_CRYPTO_KEYS);
constexpr int32_t loot_xp_currency =
    static_cast<int32_t>(game::lua::InventoryCurrency::MP_LOOT_XP);
using game::lua::LootCrateType;
using game::lua::LootRarityType;
constexpr uint32_t crate_items = 3;
// The rare crate bonus and the drop odds were decided by DW, so the client has
// no table or dvar for them.
constexpr uint32_t rare_bonus_min = 2, rare_bonus_max = 5;
constexpr size_t mp_rarities = static_cast<size_t>(LootRarityType::EPIC) + 1;
// Odds per rarity (common, rare, legendary, epic).
constexpr std::array<uint32_t, mp_rarities> common_crate_odds{50, 35, 12, 3};
constexpr std::array<uint32_t, mp_rarities> rare_crate_odds{20, 45, 25, 10};

using loot_dvar = const game::symbol<game::EngineDependentDvarMut>;

// Six Pack and Daily Double add crates to a drop item and set a cooldown on a
// consumable item.
struct bundle {
  loot_dvar &sku;
  loot_dvar &drop_id;
  loot_dvar &consumable_id;
  loot_dvar &crates;
  loot_dvar &cost;
  loot_dvar &cooldown;
};
const bundle six_pack{
    game::loot_sixPack_crate_dwid,    game::loot_sixPack_drop_id,
    game::loot_sixPack_consumable_id, game::loot_sixPack_final_count,
    game::loot_sixPack_cryptoCost,    game::loot_sixPack_cooloffSeconds};
const bundle daily_double{game::loot_dailyDouble_dwid,
                          game::loot_dailyDouble_drop_id,
                          game::loot_dailyDouble_consumable_id,
                          game::loot_dailyDouble_final_count,
                          game::loot_dailyDouble_cryptoCost,
                          game::loot_dailyDouble_cooloffSeconds};

// Loot XP from the server, credited when the player returns to the menu.
std::optional<uint32_t> server_loot_xp;

uint32_t dvar_value(loot_dvar &dvar) { return *dvar ? dvar->get_uint() : 0; }

uint32_t item_count(uint32_t item_id) {
  const std::optional<local_db::item> item = local_db::get_item(item_id);
  return item ? item->quantity : 0;
}

uint32_t add_loot_xp(uint32_t amount) {
  const uint32_t loot_xp_per_key = dvar_value(game::loot_cryptokeyCost);
  if (!loot_xp_per_key) {
    return 0;
  }
  uint32_t earned = 0;
  local_db::transaction([&] {
    const uint64_t total =
        static_cast<uint64_t>(local_db::balance(loot_xp_currency)) + amount;
    const uint32_t keys = local_db::balance(cryptokeys_currency);
    const uint32_t updated = static_cast<uint32_t>(std::min<uint64_t>(
        keys + total / loot_xp_per_key, accounting::max_balance));
    earned = updated - keys;
    return local_db::set_balance(cryptokeys_currency, updated) &&
           local_db::set_balance(
               loot_xp_currency,
               static_cast<uint32_t>(total % loot_xp_per_key));
  });
  return earned;
}

// Mirrors the balances into the engine, which refreshes the menus showing them.
bool update_cryptokey_balances(ControllerIndex_t controller) {
  return LiveInventory_UpdatePlayerBalance(
             controller, cryptokeys_currency,
             local_db::balance(cryptokeys_currency)) &&
         LiveInventory_UpdatePlayerBalance(controller, loot_xp_currency,
                                           local_db::balance(loot_xp_currency));
}

void announce_cryptokeys(uint32_t keys) {
  if (keys) {
    toast::reward("BLACK MARKET",
                  keys == 1 ? std::string("+1 Cryptokey")
                            : utils::string::va("+%u Cryptokeys", keys),
                  "uie_t7_blackmarket_promo_cryptokeys");
  }
}

struct loot_item {
  uint32_t id;
  LootRarityType rarity;
  std::string name;
};

LootRarityType rarity_type(std::string_view rarity) {
  if (rarity == "common") {
    return LootRarityType::COMMON;
  }
  if (rarity == "rare") {
    return LootRarityType::RARE;
  }
  if (rarity == "legendary") {
    return LootRarityType::LEGENDARY;
  }
  // epic and the limited editions
  return LootRarityType::EPIC;
}

// Rebuilt when the string tables are reloaded, like the gum pool.
const std::vector<loot_item> &mp_loot_pool() {
  thread_local std::vector<loot_item> pool;
  thread_local uint64_t generation = 0;
  const uint64_t current_generation = gum_generation.load();
  if (generation == current_generation && !pool.empty()) {
    return pool;
  }
  pool.clear();
  generation = current_generation;
  const StringTable *items = table("gamedata/loot/mplootitems.csv");
  const StringTable *unreleased = table("gamedata/loot/mpunreleasedloot.csv");
  if (!items) {
    return pool;
  }
  for (int32_t row = 0; row < items->rowCount; ++row) {
    const char *name = StringTable_GetColumnValueForRow(items, row, 0);
    const char *id_text = StringTable_GetColumnValueForRow(items, row, 1);
    const char *category = StringTable_GetColumnValueForRow(items, row, 2);
    const char *rarity = StringTable_GetColumnValueForRow(items, row, 3);
    if (!name || !*name || !id_text || !category || !rarity ||
        std::string_view(category) == "c2" || strlen(name) >= 256) {
      continue;
    }
    const std::string base(name, strcspn(name, ";"));
    if (unreleased) {
      const char *full = StringTable_Lookup(unreleased, 0, name, 0);
      const char *short_name =
          StringTable_Lookup(unreleased, 0, base.data(), 0);
      if ((full && *full) || (short_name && *short_name)) {
        continue;
      }
    }
    uint32_t id = 0;
    const char *end = id_text + strlen(id_text);
    const std::from_chars_result parsed = std::from_chars(id_text, end, id);
    if (id && parsed.ec == std::errc{} && parsed.ptr == end) {
      pool.push_back({id, rarity_type(rarity), name});
    }
  }
  return pool;
}

bool buy_crate(ControllerIndex_t controller, int32_t crate, int32_t currency) {
  if (!enabled()) {
    return buy_crate_hook.invoke<bool>(controller, crate, currency);
  }
  const bool with_keys = currency == cryptokeys_currency;
  const bool with_points =
      currency ==
      static_cast<int32_t>(game::lua::InventoryCurrency::COD_POINTS);
  // Crates opened from a Six Pack or Daily Double.
  uint32_t bundle_drop = 0;
  if (currency ==
      static_cast<int32_t>(game::lua::InventoryCurrency::MP_BUNDLE_ITEM)) {
    if (crate == static_cast<int32_t>(LootCrateType::COMMON)) {
      bundle_drop = dvar_value(six_pack.drop_id);
    } else if (static_cast<uint32_t>(crate) ==
               dvar_value(game::loot_dailyDouble_rare_crate_dwid)) {
      bundle_drop = dvar_value(daily_double.drop_id);
      crate = static_cast<int32_t>(LootCrateType::RARE);
    }
  }
  if (!valid_controller_index(controller) ||
      (crate != static_cast<int32_t>(LootCrateType::COMMON) &&
       crate != static_cast<int32_t>(LootCrateType::RARE)) ||
      (!with_keys && !with_points && !bundle_drop) ||
      Loot_RewardIsProcessing(controller)) {
    return false;
  }
  const bool rare = crate == static_cast<int32_t>(LootCrateType::RARE);
  const uint32_t key_cost =
      dvar_value(rare ? game::loot_rareCrate_cryptoCost
                      : game::loot_commonCrate_cryptoCost);
  const std::vector<loot_item> &pool = mp_loot_pool();
  if (pool.empty()) {
    return false;
  }
  if (with_points) {
    const uint32_t cost = dvar_value(rare ? game::loot_rareCrate_cpCost
                                          : game::loot_commonCrate_cpCost);
    stats data(controller, false);
    const std::optional<DDLState> points = player_stat(data, "CODPOINTS");
    if (!cost || !points) {
      return false;
    }
    const uint32_t balance =
        std::min(data.get(*points), accounting::max_balance);
    if (cost > balance) {
      return false;
    }
    data.set(*points, balance - cost);
    if (!data.commit()) {
      return false;
    }
    LiveInventory_UpdatePlayerBalance(controller, 0, balance - cost);
  }

  static std::mt19937 random(std::random_device{}());
  const std::array<uint32_t, mp_rarities> &odds =
      rare ? rare_crate_odds : common_crate_odds;
  std::vector<const loot_item *> rolled;
  uint32_t bonus = 0;
  const bool bought = local_db::transaction([&] {
    const uint32_t keys = local_db::balance(cryptokeys_currency);
    const uint32_t crates_left = bundle_drop ? item_count(bundle_drop) : 0;
    if ((with_keys && keys < key_cost) || (bundle_drop && !crates_left)) {
      return false;
    }

    std::array<std::vector<const loot_item *>, mp_rarities> unowned;
    for (const loot_item &item : pool) {
      if (!item_count(item.id)) {
        unowned[static_cast<size_t>(item.rarity)].push_back(&item);
      }
    }
    const auto roll = [&](LootRarityType min_rarity) -> const loot_item * {
      uint32_t total = 0;
      for (size_t r = static_cast<size_t>(min_rarity); r < odds.size(); ++r) {
        total += unowned[r].empty() ? 0 : odds[r];
      }
      if (!total) {
        return nullptr;
      }
      uint32_t pick =
          std::uniform_int_distribution<uint32_t>(0, total - 1)(random);
      for (size_t r = static_cast<size_t>(min_rarity); r < odds.size(); ++r) {
        if (unowned[r].empty()) {
          continue;
        }
        if (pick >= odds[r]) {
          pick -= odds[r];
          continue;
        }
        std::vector<const loot_item *> &bucket = unowned[r];
        const size_t index =
            std::uniform_int_distribution<size_t>(0, bucket.size() - 1)(random);
        const loot_item *item = bucket[index];
        bucket.erase(bucket.begin() + static_cast<ptrdiff_t>(index));
        return item;
      }
      return nullptr;
    };
    for (uint32_t i = 0; i < crate_items; ++i) {
      // A rare crate always has at least one rare or better item.
      const loot_item *item =
          rare && i == 0 ? roll(LootRarityType::RARE) : nullptr;
      if (!item) {
        item = roll(LootRarityType::COMMON);
      }
      if (!item) {
        // Everything is owned: show a duplicate instead of an empty slot.
        item = &pool[std::uniform_int_distribution<size_t>(0, pool.size() -
                                                                  1)(random)];
      }
      rolled.push_back(item);
      if (!local_db::set_item(item->id, 1)) {
        return false;
      }
    }

    bonus = rare ? std::uniform_int_distribution<uint32_t>(
                       rare_bonus_min, rare_bonus_max)(random)
                 : 0;
    return local_db::set_balance(
               cryptokeys_currency,
               std::min(keys - (with_keys ? key_cost : 0) + bonus,
                        accounting::max_balance)) &&
           (!bundle_drop || local_db::set_item(bundle_drop, crates_left - 1));
  });
  if (!bought) {
    return false;
  }

  *game::loot::s_lastResult.get() = {};
  for (size_t i = 0; i < rolled.size(); ++i) {
    LootResultItem &entry = game::loot::s_lastResult->granted[i];
    entry.itemId = rolled[i]->id;
    entry.itemQuantity = 1;
    strscpy(entry.itemName, rolled[i]->name);
    game::loot::s_lastResult->all[i] = entry;
  }
  game::loot::s_lastResult->bonusCryptoKeys = static_cast<int32_t>(bonus);
  game::loot::s_lastResult->result = LootResultType::SUCCESS;
  game::loot::s_lastResult->isValid = true;
  update_cryptokey_balances(controller);
  return true;
}

void *purchase_skus(ControllerIndex_t controller, uint32_t *skus, int32_t count,
                    uint32_t *quantities, int32_t currency, bool consume,
                    void *on_success, void *on_failure) {
  const bundle *bought = nullptr;
  if (enabled() && valid_controller_index(controller) && skus && count == 1) {
    for (const bundle *candidate : {&six_pack, &daily_double}) {
      const uint32_t sku = dvar_value(candidate->sku);
      if (sku && skus[0] == sku) {
        bought = candidate;
      }
    }
  }
  if (!bought) {
    return purchase_hook.invoke<void *>(controller, skus, count, quantities,
                                        currency, consume, on_success,
                                        on_failure);
  }
  const uint32_t drop = dvar_value(bought->drop_id);
  const uint32_t consumable = dvar_value(bought->consumable_id);
  const uint32_t cost = dvar_value(bought->cost);
  const uint32_t crates = dvar_value(bought->crates);
  const uint32_t cooldown_seconds = dvar_value(bought->cooldown);
  const uint32_t now = static_cast<uint32_t>(std::time(nullptr));
  const bool purchased = local_db::transaction([&] {
    const uint32_t keys = local_db::balance(cryptokeys_currency);
    const std::optional<local_db::item> cooldown =
        local_db::get_item(consumable);
    if (currency != cryptokeys_currency || !drop || !consumable || !crates ||
        keys < cost || (cooldown && cooldown->expire_time > now)) {
      return false;
    }
    return local_db::set_balance(cryptokeys_currency, keys - cost) &&
           local_db::set_item(drop, item_count(drop) + crates) &&
           local_db::set_item(consumable, 1, now + cooldown_seconds);
  });
  if (!purchased) {
    return nullptr;
  }
  update_cryptokey_balances(controller);
  // The menu only checks that the purchase started.
  static bool started;
  return &started;
}

// The menu reads the once a day cooldown from the expiry of the bundle's
// consumable inventory item.
const InventoryItem *get_inventory_item(ControllerIndex_t controller,
                                        uint32_t item_id) {
  if (enabled() && item_id) {
    const std::optional<local_db::item> item = local_db::get_item(item_id);
    if (item && item->expire_time) {
      const uint32_t cooldown_seconds =
          item_id == dvar_value(daily_double.consumable_id)
              ? dvar_value(daily_double.cooldown)
              : dvar_value(six_pack.cooldown);
      static InventoryItem entry{};
      entry = {item_id, item->quantity, item->expire_time - cooldown_seconds,
               item->expire_time, 0};
      return &entry;
    }
  }
  return get_item_hook.invoke<const InventoryItem *>(controller, item_id);
}

// The client only accepts Loot XP from public online and arena matches.
bool accept_server_loot_xp() { return true; }

uint32_t get_currency(ControllerIndex_t controller, int32_t kind) {
  if (enabled() && (kind == cryptokeys_currency || kind == loot_xp_currency)) {
    return local_db::balance(kind);
  }
  if (!enabled() || (kind != 0 && kind != 3)) {
    return currency_hook.invoke<int32_t>(controller, kind);
  }
  const stats data(controller, kind == 3);
  if (kind == 3) {
    return vials(data).value_or(0);
  }
  const std::optional<DDLState> state = player_stat(data, "CODPOINTS");
  return state ? std::min(data.get(*state), accounting::max_balance) : 0;
}

constexpr uint32_t ROLL_SELECTION_POOL_SIZE = 3;
bool spend_vials(ControllerIndex_t controller, uint32_t count) {
  if (!enabled()) {
    return spend_hook.invoke<bool>(controller, count);
  }

  if (!valid_controller_index(controller) || count < 1 || count > 3 ||
      /* FIXME: We have this stored in a global. Why are we needlessly looking
          up the dvar by name? This needs to be fixed. */
      !game::get_dvar_bool("loot_enabled").value_or(false) ||
      !LiveInventory_IsValid(controller) ||
      Loot_RewardIsProcessing(controller) || game::com::Com_IsInGame() ||
      game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES) {
    return false;
  }
  stats data(controller, true);
  const std::optional<uint32_t> balance = vials(data);
  const std::optional<DDLState> used = player_stat(data, "BGB_TOKENS_USED");
  const std::vector<gum> &pool = gum_pool();
  if (!balance || *balance < count || !used || pool.empty()) {
    return false;
  }
  data.set(*used, data.get(*used) + count);
  static std::mt19937 random(std::random_device{}());
  std::uniform_int_distribution<size_t> roll(0, pool.size() - 1);
  std::array<gum, ROLL_SELECTION_POOL_SIZE> rolled;
  std::array<uint32_t, ROLL_SELECTION_POOL_SIZE> quantities{};
  for (uint32_t i = 0; i < ROLL_SELECTION_POOL_SIZE; ++i) {
    rolled[i] = pool[roll(random)];
    if (i < count) {
      const std::optional<DDLState> gained =
          gum_stat(data, rolled[i], "bgbconsumablesgained");
      if (!gained || !quantity(data, rolled[i])) {
        return false;
      }
      const std::optional<uint32_t> next =
          accounting::credit(data.get(*gained), 1);
      if (!next) {
        return false;
      }
      data.set(*gained, *next);
    }
  }
  for (uint32_t i = 0; i < count; ++i) {
    const std::optional<uint32_t> value = quantity(data, rolled[i]);
    if (!value) {
      return false;
    }
    quantities[i] = *value;
  }
  if (!data.commit()) {
    return false;
  }
  *game::loot::s_lastResult.get() = {};
  for (uint32_t i = 0; i < ROLL_SELECTION_POOL_SIZE; ++i) {
    LootResultItem &entry = game::loot::s_lastResult->all[i];
    entry.itemId = rolled[i].id;
    entry.itemQuantity = 1;
    strscpy(entry.itemName, rolled[i].reference);
    if (i < count) {
      game::loot::s_lastResult->granted[i] = entry;
      game::loot::s_lastResult->granted[i].itemQuantity = 1;
      LiveInventory_UpdateItemQuantity(controller, rolled[i].id, quantities[i],
                                       0, 0, 0);
    }
  }
  LiveInventory_UpdatePlayerBalance(controller, 3, *balance - count);
  game::loot::s_lastResult->result = LootResultType::SUCCESS;
  game::loot::s_lastResult->isValid = true;

  const std::string message =
      count == 1 ? "Obtained 1 gum"
                 : "Obtained " + std::to_string(count) + " gums";
  scheduler::once(
      [message] {
        toast::reward("Doctor Monty's Factory", message,
                      "uie_t7_menu_gobblegum_comsumable");
      },
      scheduler::pipeline::main, 3000ms);

  return true;
}

bool increment_currency(ControllerIndex_t controller, int32_t kind,
                        uint32_t amount) {
  if (enabled() && kind == loot_xp_currency) {
    server_loot_xp = server_loot_xp.value_or(0) + amount;
    return true;
  }
  if (!enabled() || kind != 3) {
    return increment_hook.invoke<bool>(controller, kind, amount);
  }
  stats data(controller, true);
  const std::optional<DDLState> pending = data.find({"vialsOwed"});
  const std::optional<uint32_t> balance = vials(data);
  if (!pending || !balance || !amount || data.get(*pending) > amount) {
    return false;
  }
  // _zm_bgb_token.gsc already increments BGB_TOKENS_GAINED before
  // ReportLootReward.
  data.set(*pending, 0);
  if (!data.commit()) {
    return false;
  }
  LiveInventory_UpdatePlayerBalance(controller, 3, *balance);
  using namespace game::ui;
  const uint16_t root = UI_Model_GetModelForController(controller);
  const uint16_t tokens = UI_Model_CreateModelFromPath(root, "MegaChewTokens");
  UI_Model_SetInt(UI_Model_CreateModelFromPath(tokens, "remainingTokens"),
                  *balance);
  const uint16_t aar = UI_Model_CreateModelFromPath(
      root, "aarStats.performanceTabStats.bgbTokensGainedThisGame");
  UI_Model_SetInt(aar, amount);
  const uint16_t notify = UI_Model_CreateModelFromPath(root, "scriptNotify");
  UI_Model_SetInt(UI_Model_CreateModelFromPath(notify, "numArgs"), 0);
  UI_Model_SetString(notify, "zombie_bgb_token_notification");
  UI_Model_ForceNotify(notify);
  return true;
}

bool increment_from_lua(ControllerIndex_t controller, int32_t kind,
                        uint32_t amount) {
  if (!enabled() || kind != 3) {
    return increment_currency(controller, kind, amount);
  }
  // Unlike ReportLootReward, the Lua increment has not credited the earned
  // stat.
  stats data(controller, true);
  const std::optional<DDLState> gained = player_stat(data, "BGB_TOKENS_GAINED");
  const std::optional<uint32_t> balance = vials(data);
  if (!gained || !balance || !amount ||
      amount > accounting::max_balance - *balance) {
    return false;
  }
  const std::optional<uint32_t> next =
      accounting::credit(data.get(*gained), amount);
  if (!next) {
    return false;
  }
  data.set(*gained, *next);
  if (!data.commit()) {
    return false;
  }
  LiveInventory_UpdatePlayerBalance(controller, 3, *balance + amount);
  return true;
}

void *consume_items(ControllerIndex_t controller, uint32_t *ids, int32_t count,
                    uint32_t *amounts) {
  if (!enabled()) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  if (!valid_controller_index(controller) || !ids || !amounts || count < 1 ||
      count > 4000) {
    return nullptr;
  }
  const std::vector<gum> &pool = gum_pool();
  // FIXME: what is "ponytail:"? This needs to be cleaned up.
  // ponytail: stock callers consume one item; batch support needs a native task
  // completion.
  if (count != 1) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  const std::ranges::borrowed_iterator_t<
      const std::vector<gum, std::allocator<gum>> &>
      found = std::find_if(pool.begin(), pool.end(),
                           [&](const gum &item) { return item.id == ids[0]; });
  if (found == pool.end()) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  if (/* FIXME: We have this stored in a global. Why are we needlessly looking
         up the dvar by name? This needs to be fixed. */
      game::get_dvar_bool("cg_unlockall_gobblegums").value_or(false)) {
    return nullptr;
  }
  stats data(controller, true);
  const std::optional<DDLState> gained =
      gum_stat(data, *found, "bgbconsumablesgained");
  const std::optional<DDLState> used =
      gum_stat(data, *found, "bgbconsumablesused");
  if (gained && used) {
    const std::optional<uint32_t> next =
        accounting::debit(data.get(*gained), data.get(*used), amounts[0]);
    if (next) {
      data.set(*used, *next);
      if (data.commit()) {
        LiveInventory_UpdateItemQuantity(
            controller, found->id, quantity(data, *found).value(), 0, 0, 0);
        return nullptr;
      }
    }
  }
  toast::error("GobbleGum", "Could not save consumption to native stats.");
  return nullptr;
}

void inventory_update(ControllerIndex_t controller) {
  if (!valid_controller_index(controller)) {
    return;
  }
  inventory_update_hook.invoke<void>(controller);
  static game::LocalClientPool<bool> keys_restored{};
  if (!enabled() || !LiveInventory_IsValid(controller)) {
    keys_restored[controller] = false;
  } else if (!keys_restored[controller]) {
    keys_restored[controller] = update_cryptokey_balances(controller);
  }
  static game::LocalClientPool<bool> restored{};
  static game::LocalClientPool<bool> points_restored{};
  static game::LocalClientPool<StorageFileType> restored_file{};
  static game::LocalClientPool<game::XUID> restored_user{};
  if (!enabled() || !LiveInventory_IsValid(controller)) {
    restored[controller] = false;
    points_restored[controller] = false;
    return;
  }
  stats data(controller, true);
  const game::XUID user = game::live::user::LiveUser_GetXuid(controller);
  if (restored_file[controller] != data.file ||
      restored_user[controller] != user) {
    restored[controller] = false;
    points_restored[controller] = false;
  }
  restored_file[controller] = data.file;
  restored_user[controller] = user;
  if (!data.context) {
    restored[controller] = false;
  }
  const stats mp(controller, false);
  const std::optional<DDLState> points = player_stat(mp, "CODPOINTS");
  if (!points) {
    points_restored[controller] = false;
  } else if (!points_restored[controller]) {
    points_restored[controller] = LiveInventory_UpdatePlayerBalance(
        controller, 0, std::min(mp.get(*points), accounting::max_balance));
  }
  if (!data.context || restored[controller]) {
    return;
  }
  const std::optional<uint32_t> balance = vials(data);
  const std::vector<gum> &pool = gum_pool();
  if (!balance || pool.empty()) {
    return;
  }
  for (const gum &item : pool) {
    const std::optional<uint32_t> count = quantity(data, item);
    if (!count || !LiveInventory_UpdateItemQuantity(controller, item.id, *count,
                                                    0, 0, 0)) {
      return;
    }
  }
  LiveInventory_UpdatePlayerBalance(controller, 3, *balance);
  restored[controller] = true;
}

void balance_command(const char *name, bool zombies) {
  command::add(name, [zombies](const command::params &args) {
    if (!enabled() || game::com::Com_IsInGame()) {
      toast::error("Currency",
                   "Return to the menu with local currency enabled.");
      return;
    }
    const game::ControllerIndex_t controller =
        game::com::Com_ControllerIndexes_GetPrimary();
    stats data(controller, zombies);
    const std::optional<DDLState> state =
        player_stat(data, zombies ? "BGB_TOKENS_GAINED" : "CODPOINTS");
    const std::optional<DDLState> used =
        zombies ? player_stat(data, "BGB_TOKENS_USED") : state;
    if (!state || !used) {
      toast::error("Currency", "Native stats are not ready.");
      return;
    }
    if (args.size() == 1) {
      const uint32_t value = zombies ? vials(data).value() : data.get(*state);
      toast::info("Currency", utils::string::va("Balance: %u", value));
      return;
    }
    const std::optional<uint32_t> amount =
        accounting::parse_amount(args.get(1));
    const std::optional<uint32_t> next =
        amount ? accounting::credit(zombies ? data.get(*used) : 0, *amount)
               : std::nullopt;
    if (args.size() != 2 || !next) {
      toast::error("Currency", "Expected an integer from 0 to 9999999.");
      return;
    }
    data.set(*state, *next);
    if (!data.commit()) {
      toast::error("Currency", "Could not queue the native stats save.");
      return;
    }
    LiveInventory_UpdatePlayerBalance(controller, zombies ? 3 : 0, *amount);
    toast::success("Currency", utils::string::va("Balance set to %u", *amount));
  });
}

// Credits the Loot XP of a session when the player returns to the menu, so
// matches on a server that changes maps add up.
void award_loot_xp() {
  static std::optional<std::chrono::steady_clock::time_point> started;
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();
  const game::eModes mode = game::com::Com_SessionMode_GetMode();
  const bool in_game =
      game::com::Com_IsInGame() && !game::com::Com_IsRunningUILevel();
  if (!enabled()) {
    started.reset();
    server_loot_xp.reset();
    return;
  }
  if (in_game) {
    if (!started && mode == game::eModes::MULTIPLAYER) {
      started = now;
    }
    return;
  }
  if (!started && !server_loot_xp) {
    return;
  }
  // Same rate as the ranked reward, for sessions whose server sent none.
  const int64_t seconds =
      started ? std::chrono::duration_cast<std::chrono::seconds>(now - *started)
                    .count()
              : 0;
  const uint32_t earn_seconds = dvar_value(game::loot_earnTime);
  const uint32_t xp = server_loot_xp.value_or(
      earn_seconds
          ? static_cast<uint32_t>(
                seconds * dvar_value(game::loot_cryptokeyCost) / earn_seconds)
          : 0);
  started.reset();
  server_loot_xp.reset();
  if (xp) {
    announce_cryptokeys(add_loot_xp(xp));
    update_cryptokey_balances(game::com::Com_ControllerIndexes_GetPrimary());
  }
}

void award_match_points() {
  using clock = std::chrono::steady_clock;
  struct match {
    ControllerIndex_t controller;
    game::XUID xuid;
    StorageFileType file;
    bool zombies;
    clock::time_point started;
  };
  struct match_reward {
    match session;
    uint32_t points;
    uint32_t vials;
    bool points_saved;
    bool vials_saved;
  };
  static std::optional<match> playing;
  static std::optional<match_reward> pending;
  const std::chrono::steady_clock::time_point now = clock::now();
  const game::ControllerIndex_t controller =
      game::com::Com_ControllerIndexes_GetPrimary();
  if (!enabled() || !valid_controller_index(controller)) {
    playing.reset();
    pending.reset();
    return;
  }
  const game::XUID xuid = game::live::user::LiveUser_GetXuid(controller);
  const StorageFileType file = stats_type(false);
  const auto same_user = [&](const match &entry) -> bool {
    return entry.controller == controller && entry.xuid == xuid &&
           entry.file == file;
  };
  const game::eModes mode = game::com::Com_SessionMode_GetMode();
  const std::string_view map = game::get_mapname().value_or("");
  const bool zombies = mode == game::eModes::ZOMBIES || map.starts_with("zm_");
  const bool in_game =
      game::com::Com_IsInGame() && !game::com::Com_IsRunningUILevel();
  if (playing && !same_user(*playing)) {
    playing.reset();
  }
  if (pending && !same_user(pending->session)) {
    pending.reset();
  }
  if (in_game && !playing &&
      (mode == game::eModes::MULTIPLAYER || mode == game::eModes::ZOMBIES)) {
    playing = match{controller, xuid, file, zombies, now};
    last_divinium_award.set(0);
  } else if (in_game && playing && zombies) {
    playing->zombies = true;
  } else if (!in_game && playing) {
    const int64_t seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now - playing->started)
            .count();
    const uint32_t earned_points = accounting::match_reward(
        seconds, points_per_minute.get_uint(), points_per_match_cap.get_uint());
    const uint32_t earned_vials =
        playing->zombies && seconds >= 60 ? divinium_per_match.get_uint() : 0;
    pending = match_reward{*playing, earned_points, earned_vials,
                           earned_points == 0, earned_vials == 0};
    pending->session.started = now;
    playing.reset();
  }
  if (!pending || in_game) {
    return;
  }

  if (!pending->points_saved) {
    stats data(controller, false);
    const std::optional<DDLState> points = player_stat(data, "CODPOINTS");
    if (points) {
      const uint32_t before =
          std::min(data.get(*points), accounting::max_balance);
      pending->points =
          std::min(pending->points, accounting::max_balance - before);
      data.set(*points, before + pending->points);
      if (!pending->points || data.commit()) {
        pending->points_saved = true;
        LiveInventory_UpdatePlayerBalance(controller, 0,
                                          before + pending->points);
      }
    }
  }

  if (!pending->vials_saved) {
    stats data(controller, true);
    const std::optional<DDLState> gained =
        player_stat(data, "BGB_TOKENS_GAINED");
    const std::optional<uint32_t> balance = vials(data);
    if (gained && balance) {
      const uint32_t before = data.get(*gained);
      const uint32_t limit =
          std::min(accounting::max_balance, gained->member->rangeLimit);
      pending->vials = std::min(pending->vials, limit - before);
      data.set(*gained, before + pending->vials);
      if (!pending->vials || data.commit()) {
        pending->vials_saved = true;
        last_divinium_award.set(pending->vials);
        LiveInventory_UpdatePlayerBalance(controller, 3,
                                          *balance + pending->vials);
        using namespace game::ui;
        const uint16_t root = UI_Model_GetModelForController(controller);
        const uint16_t tokens =
            UI_Model_CreateModelFromPath(root, "MegaChewTokens");
        UI_Model_SetInt(UI_Model_CreateModelFromPath(tokens, "remainingTokens"),
                        *balance + pending->vials);
        const uint16_t aar = UI_Model_CreateModelFromPath(
            root, "aarStats.performanceTabStats.bgbTokensGainedThisGame");
        UI_Model_SetInt(aar, pending->vials);
      }
    }
  }

  if (pending->points_saved && pending->vials_saved) {
    std::string summary;
    if (pending->points) {
      summary = utils::string::va("+%u CoD Points", pending->points);
    }
    if (pending->vials) {
      summary +=
          summary.empty()
              ? utils::string::va("+%u Liquid Divinium", pending->vials)
              : utils::string::va(", +%u Liquid Divinium", pending->vials);
    }
    if (!summary.empty()) {
      toast::reward("MATCH REWARDS", summary,
                    pending->vials ? "t7_hud_zm_vial_256"
                                   : "uie_t7_icon_codpoints");
    }
    pending.reset();
    return;
  }

  if (now - pending->session.started > 30s) {
    pending.reset();
    toast::error("Match Rewards",
                 "Native stats were unavailable; rewards were not saved.");
  }
}
} // namespace

std::optional<uint32_t> add_cod_points(ControllerIndex_t controller,
                                       uint32_t amount) {
  if (!enabled() || !valid_controller_index(controller) || !amount ||
      game::com::Com_IsInGame()) {
    return std::nullopt;
  }

  stats data(controller, false);
  const std::optional<DDLState> points = player_stat(data, "CODPOINTS");
  if (!points) {
    return std::nullopt;
  }

  const uint32_t balance = std::min(data.get(*points), accounting::max_balance);
  if (amount > accounting::max_balance - balance) {
    return std::nullopt;
  }

  const uint32_t updated = balance + amount;
  data.set(*points, updated);
  if (!data.commit()) {
    return std::nullopt;
  }

  LiveInventory_UpdatePlayerBalance(controller, 0, updated);
  return updated;
}

bool purchase_vials(ControllerIndex_t controller, uint32_t cost,
                    uint32_t amount) {
  if (!enabled() || !valid_controller_index(controller) || !cost || !amount ||
      game::com::Com_IsInGame()) {
    return false;
  }

  const ControllerIndex_t index = controller;
  stats points_data(index, false);
  stats vials_data(index, true);
  const std::optional<DDLState> points = player_stat(points_data, "CODPOINTS");
  const std::optional<DDLState> gained =
      player_stat(vials_data, "BGB_TOKENS_GAINED");
  const std::optional<uint32_t> vial_balance = vials(vials_data);
  if (!points || !gained || !vial_balance) {
    return false;
  }

  const uint32_t point_balance =
      std::min(points_data.get(*points), accounting::max_balance);
  if (cost > point_balance ||
      amount > accounting::max_balance - *vial_balance) {
    return false;
  }

  const std::optional<uint32_t> next_gained =
      accounting::credit(vials_data.get(*gained), amount);
  if (!next_gained) {
    return false;
  }

  points_data.set(*points, point_balance - cost);
  if (!points_data.commit()) {
    return false;
  }

  vials_data.set(*gained, *next_gained);
  if (!vials_data.commit()) {
    points_data.set(*points, point_balance);
    points_data.commit();
    return false;
  }

  LiveInventory_UpdatePlayerBalance(index, 0, point_balance - cost);
  LiveInventory_UpdatePlayerBalance(index, 3, *vial_balance + amount);
  return true;
}

bool purchase_distills(ControllerIndex_t controller, std::string_view kind,
                       uint32_t currency) {
  if (!enabled() || !valid_controller_index(controller) ||
      game::com::Com_IsInGame()) {
    return false;
  }

  const bool free = kind == "free";
  if (!free && kind != "x3" && kind != "x6" && kind != "x9") {
    return false;
  }
  if (!free && currency != 0 && currency != 3) {
    return false;
  }
  if (free && free_distill_cooldown() > 0) {
    return false;
  }

  const char *amount_name =
      free ? "loot_distill_free_quantity"
           : utils::string::va("loot_distill_paid_%.*s_quantity",
                               static_cast<int32_t>(kind.size()), kind.data());
  const uint32_t amount = game::get_dvar_uint(amount_name).value_or(30);
  game::EngineDependentDvarMut &balance_dvar =
      free ? free_distills : paid_distills;
  const uint32_t balance = balance_dvar.get_uint();
  if (amount <= 0 || balance < 0 ||
      amount > accounting::max_balance - balance || (free && balance != 0)) {
    return false;
  }

  const ControllerIndex_t index = controller;
  int32_t remaining_currency = 0;
  if (!free) {
    const char *cost_name = utils::string::va(
        "loot_distill_paid_%.*s_%sCost", static_cast<int32_t>(kind.size()),
        kind.data(), currency == 3 ? "vial" : "cp");
    const std::optional<uint32_t> cost = game::get_dvar_uint(cost_name);
    if (!cost.has_value()) {
      return false;
    }

    stats data(index, currency == 3);
    if (currency == 3) {
      const std::optional<uint32_t> current = vials(data);
      const std::optional<DDLState> used = player_stat(data, "BGB_TOKENS_USED");
      if (!current || !used || cost.value() > *current) {
        return false;
      }
      data.set(*used, data.get(*used) + cost.value());
      remaining_currency = *current - cost.value();
    } else {
      const std::optional<DDLState> points = player_stat(data, "CODPOINTS");
      if (!points || cost.value() > data.get(*points)) {
        return false;
      }
      remaining_currency = data.get(*points) - cost.value();
      data.set(*points, remaining_currency);
    }
    if (!data.commit()) {
      return false;
    }
  }

  const std::optional<int32_t> previous_balance =
      balance_dvar.set(balance + amount);
  if (!previous_balance) {
    return false;
  }
  if (free) {
    constexpr int32_t cooldown_seconds = 24 * 60 * 60;
    const int32_t now = static_cast<int32_t>(std::time(nullptr));
    if (!free_distill_expiry.set(now + cooldown_seconds)) {
      balance_dvar.set(*previous_balance);
      return false;
    }
  }
  if (!free) {
    LiveInventory_UpdatePlayerBalance(index, currency, remaining_currency);
  }
  return true;
}

uint32_t free_distill_cooldown() {
  if (!free_distill_expiry) {
    return 0;
  }
  const int32_t now = static_cast<int32_t>(std::time(nullptr));
  return std::max(0, free_distill_expiry.get_int() - now);
}

uint32_t distill_balance(bool free) {
  if (!enabled()) {
    return -1;
  }
  return (free ? free_distills : paid_distills).get_int();
}

const gum *find_gum_by_name(const std::vector<gum> &pool,
                            const char *reference) {
  if (reference && reference[0]) {

    const std::ranges::borrowed_iterator_t<
        const std::vector<gum, std::allocator<gum>> &>
        found = std::ranges::find_if(pool, [reference](const gum &item) {
          return item.reference == reference;
        });

    if (found != pool.end()) {
      return &*found;
    }
  }

  return nullptr;
}

bool cook_recipe(ControllerIndex_t controller, uint32_t recipe,
                 bool use_free_distills) {
  if (!enabled() || !valid_controller_index(controller) ||
      game::com::Com_IsInGame() ||
      game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES) {
    return false;
  }

  const StringTable *recipes =
      table("gamedata/tables/zm/zm_gobblegumrecipes.csv");
  const std::vector<gum> &pool = gum_pool();
  if (!recipes || pool.empty()) {
    return false;
  }

  int32_t recipe_row = -1;
  for (int32_t row = 0; row < recipes->rowCount; ++row) {
    const char *text = StringTable_GetColumnValueForRow(recipes, row, 0);
    const std::optional<uint32_t> value =
        text ? accounting::parse_amount(text) : std::nullopt;
    if (value && *value == recipe) {
      recipe_row = row;
      break;
    }
  }
  if (recipe_row < 0) {
    return false;
  }

  const gum *result_gum = find_gum_by_name(
      pool, StringTable_GetColumnValueForRow(recipes, recipe_row, 1));
  const char *result_count_text =
      StringTable_GetColumnValueForRow(recipes, recipe_row, 2);
  const std::optional<uint32_t> result_count =
      result_count_text ? accounting::parse_amount(result_count_text)
                        : std::nullopt;
  if (!result_gum || !result_count || !*result_count) {
    return false;
  }

  const int32_t distill_cost =
      /* FIXME: We have this stored in a global. Why are we needlessly looking
         up the dvar by name? This needs to be fixed. */
      game::get_dvar_int("loot_recipe_distill_cost").value_or(10);
  game::EngineDependentDvarMut &distill_balance =
      use_free_distills ? free_distills : paid_distills;
  if (distill_cost <= 0 || distill_balance.get_int() < distill_cost) {
    return false;
  }

  /* FIXME: We have this stored in a global. Why are we needlessly looking
   up the dvar by name? This needs to be fixed. */
  const bool unlockall =
      game::get_dvar_bool("cg_unlockall_gobblegums").value_or(false);

  stats data(controller, true);
  for (int32_t column = 3;; column += 2) {
    const char *reference =
        StringTable_GetColumnValueForRow(recipes, recipe_row, column);
    if (!reference || !*reference || *reference == '#') {
      break;
    }
    const gum *ingredient = find_gum_by_name(pool, reference);
    const char *count_text =
        StringTable_GetColumnValueForRow(recipes, recipe_row, column + 1);
    const std::optional<uint32_t> count =
        count_text ? accounting::parse_amount(count_text) : std::nullopt;
    if (!ingredient || !count || !*count) {
      return false;
    }
    if (unlockall) {
      continue;
    }
    const std::optional<DDLState> gained =
        gum_stat(data, *ingredient, "bgbconsumablesgained");
    const std::optional<DDLState> used =
        gum_stat(data, *ingredient, "bgbconsumablesused");
    if (!gained || !used) {
      return false;
    }
    const std::optional<uint32_t> next =
        accounting::debit(data.get(*gained), data.get(*used), *count);
    if (!next) {
      return false;
    }
    data.set(*used, *next);
  }

  const std::optional<DDLState> result_gained =
      gum_stat(data, *result_gum, "bgbconsumablesgained");
  if (!result_gained) {
    return false;
  }
  const std::optional<uint32_t> next_result =
      accounting::credit(data.get(*result_gained), *result_count);
  if (!next_result) {
    return false;
  }
  data.set(*result_gained, *next_result);
  if (!data.commit() ||
      !distill_balance.set(distill_balance.get_int() - distill_cost)) {
    return false;
  }

  *game::loot::s_lastResult.get() = {};
  game::loot::s_lastResult->granted[0].itemId = result_gum->id;
  game::loot::s_lastResult->granted[0].itemQuantity = *result_count;
  strscpy(game::loot::s_lastResult->granted[0].itemName, result_gum->reference);
  game::loot::s_lastResult->all[0] = game::loot::s_lastResult->granted[0];
  game::loot::s_lastResult->result = game::loot::LootResultType::SUCCESS;
  game::loot::s_lastResult->isValid = true;

  const uint32_t granted_count = *result_count;
  const std::string message =
      granted_count == 1
          ? "Obtained 1 gum"
          : "Obtained " + std::to_string(granted_count) + " gums";
  toast::reward("Recipe Cooked", message, "uie_t7_menu_gobblegum_comsumable");

  return true;
}

std::optional<uint32_t> item_quantity(ControllerIndex_t controller,
                                      uint32_t inventory_id) {
  if (!enabled() || !valid_controller_index(controller)) {
    return std::nullopt;
  }
  if (const std::optional<local_db::item> item =
          local_db::get_item(inventory_id)) {
    return item->quantity;
  }
  const std::optional<uint32_t> free_id =
      /* FIXME: We have this stored in a global. Why are we needlessly looking
         up the dvar by name? This needs to be fixed. */
      game::get_dvar_uint("loot_distill_free_balance_id");
  const std::optional<uint32_t> paid_id =
      /* FIXME: We have this stored in a global. Why are we needlessly looking
         up the dvar by name? This needs to be fixed. */
      game::get_dvar_uint("loot_distill_paid_balance_id");
  if (free_id.has_value() && inventory_id == *free_id) {
    return free_distills.get_uint();
  }
  if (paid_id.has_value() && inventory_id == *paid_id) {
    return paid_distills.get_uint();
  }
  const std::vector<gum> &pool = gum_pool();
  for (const gum &item : pool) {
    if (item.id == inventory_id) {
      if (/* FIXME: We have this stored in a global. Why are we needlessly
             looking up the dvar by name? This needs to be fixed. */
          game::get_dvar_bool("cg_unlockall_gobblegums").value_or(false)) {
        return 999;
      }
      const stats data(controller, true);
      return quantity(data, item).value_or(0);
    }
  }
  return std::nullopt;
}

bool reset_gobblegums(ControllerIndex_t controller) {
  if (!enabled() || !valid_controller_index(controller) ||
      game::com::Com_IsInGame()) {
    return false;
  }

  const ControllerIndex_t index = controller;
  stats data(index, true);
  const std::vector<gum> &pool = gum_pool();
  if (pool.empty()) {
    return false;
  }
  for (const gum &item : pool) {
    const std::optional<DDLState> gained =
        gum_stat(data, item, "bgbconsumablesgained");
    const std::optional<DDLState> used =
        gum_stat(data, item, "bgbconsumablesused");
    if (!gained || !used) {
      return false;
    }
    data.set(*gained, 0);
    data.set(*used, 0);
  }
  if (!data.commit()) {
    return false;
  }
  for (const gum &item : pool) {
    LiveInventory_UpdateItemQuantity(index, item.id, 0, 0, 0, 0);
  }
  return true;
}

bool set_currencies_maxed(ControllerIndex_t controller, bool maxed) {
  if (!enabled() || !valid_controller_index(controller) ||
      game::com::Com_IsInGame()) {
    return false;
  }

  const ControllerIndex_t index = controller;
  stats points_data(index, false);
  stats vials_data(index, true);
  const std::optional<DDLState> points = player_stat(points_data, "CODPOINTS");
  const std::optional<DDLState> gained =
      player_stat(vials_data, "BGB_TOKENS_GAINED");
  const std::optional<DDLState> used =
      player_stat(vials_data, "BGB_TOKENS_USED");
  if (!points || !gained || !used) {
    return false;
  }

  const uint32_t point_balance =
      maxed ? std::min(accounting::max_balance, points->member->rangeLimit) : 0;
  const uint32_t vial_balance =
      maxed ? std::min(accounting::max_balance, gained->member->rangeLimit) : 0;
  const uint32_t distill_balance = maxed ? accounting::max_balance : 0;
  points_data.set(*points, point_balance);
  vials_data.set(*used, 0);
  vials_data.set(*gained, vial_balance);
  if (!points_data.commit() || !vials_data.commit() ||
      !free_distills.set(distill_balance) ||
      !paid_distills.set(distill_balance) ||
      (!maxed && !free_distill_expiry.set(0))) {
    return false;
  }

  LiveInventory_UpdatePlayerBalance(index, 0, point_balance);
  LiveInventory_UpdatePlayerBalance(index, 3, vial_balance);
  return true;
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("currency");

  void post_unpack() override {
    local_currency = game::register_dvar_bool(
        "cg_local_currency", true, game::DvarFlags{.archive = 1},
        "Use native stats for local currency and GobbleGum inventory");
    com::on_level_load(invalidate_gum_pool);
    com::on_level_unload(invalidate_gum_pool);
    points_per_minute = game::register_dvar_int(
        "cg_codpoints_per_minute", 120, 0, 10000, game::DvarFlags{.archive = 1},
        "Local CoD Points per completed minute of Multiplayer or Zombies play");
    points_per_match_cap = game::register_dvar_int(
        "cg_currency_award_cap", 5000, 0, 100000, game::DvarFlags{.archive = 1},
        "Maximum local CoD Points awarded per match");
    divinium_per_match = game::register_dvar_int(
        "cg_divinium_per_match", 3, 0, 100, game::DvarFlags{.archive = 1},
        "Local Liquid Divinium awarded when a Zombies match ends");
    last_divinium_award = game::register_dvar_int(
        "cg_last_divinium_award", 0, 0, 100, game::DvarFlags{}, "");
    max_local_currencies = game::register_dvar_bool(
        "cg_max_local_currencies", false, game::DvarFlags{.archive = 1},
        "Whether local currency balances were last set to maximum");
    free_distills = game::register_dvar_int(
        "cg_free_distills", 0, 0, accounting::max_balance,
        game::DvarFlags{.archive = 1}, "Saved free Newton's Cookbook Distills");
    paid_distills = game::register_dvar_int(
        "cg_paid_distills", 0, 0, accounting::max_balance,
        game::DvarFlags{.archive = 1}, "Saved paid Newton's Cookbook Distills");
    free_distill_expiry = game::register_dvar_int(
        "cg_free_distill_expiry", 0, 0, std::numeric_limits<int32_t>::max(),
        game::DvarFlags{.archive = 1}, "Next free Distill claim time");

    // Loot_GetCurrency
    // FIXME: these inline offsets should be a symbol - strongly typed, named,
    // and properly namespaced.
    currency_hook.create(game::select(0x141DFC680, 0x141E09110, 0x0),
                         get_currency);
    // Loot_SpendVials
    // FIXME: these inline offsets should be a symbol - strongly typed, named,
    // and properly namespaced.
    spend_hook.create(game::select(0x141E76DD0, 0x141E83860, 0x0), spend_vials);
    // Loot_IncCurrency
    // FIXME: these inline offsets should be a symbol - strongly typed, named,
    // and properly namespaced.
    increment_hook.create(game::select(0x141E761C0, 0x141E82C50, 0x0),
                          increment_currency);

    // `Loot_IncCurrency` call in `Lua_CoD_LuaCall_IncrementCurrency`
    utils::hook::call(game::select(0x141F14842, 0x141F20FC2, 0x0),
                      increment_from_lua);
    // LiveInventory_ConsumeItem
    // FIXME: these inline offsets should be a symbol - strongly typed, named,
    // and properly namespaced.
    consume_hook.create(game::select(0x141DFC180, 0x141E08C10, 0x0),
                        consume_items);
    // LiveInventory_Update
    // FIXME: these inline offsets should be a symbol - strongly typed, named,
    // and properly namespaced.
    inventory_update_hook.create(game::select(0x141DFDF10, 0x141E0A9A0, 0x0),
                                 inventory_update);

    buy_crate_hook.create(game::loot::Loot_BuyCrate.get(), buy_crate);
    purchase_hook.create(
        game::live::inventory::LiveInventory_PurchaseSkus.get(), purchase_skus);
    get_item_hook.create(game::live::inventory::LiveInventory_GetItem.get(),
                         get_inventory_item);
    // `Com_SessionMode_IsOnlineArenaGame` call before `Loot_IncCurrency` in
    // the Loot XP server command.
    utils::hook::call(game::select(0x140F7B2E3, 0x140F7B2E3, 0x0),
                      accept_server_loot_xp);

    balance_command("codpoints", false);
    balance_command("divinium", true);
    command::add("cryptokeys", [](const command::params &args) {
      if (args.size() == 1) {
        toast::info("Black Market",
                    utils::string::va("Cryptokeys: %u",
                                      local_db::balance(cryptokeys_currency)));
        return;
      }
      const std::optional<uint32_t> amount =
          accounting::parse_amount(args.get(1));
      if (args.size() != 2 || !amount ||
          !local_db::set_balance(cryptokeys_currency, *amount)) {
        toast::error("Black Market", "Expected an integer from 0 to 9999999.");
        return;
      }
      update_cryptokey_balances(game::com::Com_ControllerIndexes_GetPrimary());
      toast::success("Black Market",
                     utils::string::va("Cryptokeys set to %u", *amount));
    });
    scheduler::loop(award_match_points, scheduler::pipeline::main, 1s);
    scheduler::loop(award_loot_xp, scheduler::pipeline::main, 1s);
  }
};
} // namespace currency

REGISTER_COMPONENT(currency::component)
