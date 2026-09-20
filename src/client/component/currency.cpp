#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "com.hpp"
#include "command.hpp"
#include "currency.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <atomic>
#include <ctime>
#include <random>

namespace currency {
namespace {
using game::ControllerIndex_t;
using game::StorageFileType;
using namespace game::ddl;
using namespace game::live::storage;

game::EngineDependentDvarMut local_currency;
game::EngineDependentDvarMut points_per_minute, points_per_match_cap;
game::EngineDependentDvarMut divinium_per_match, last_divinium_award;
game::EngineDependentDvarMut max_local_currencies;
game::EngineDependentDvarMut free_distills, paid_distills, free_distill_expiry;
utils::hook::detour currency_hook, spend_hook, increment_hook, consume_hook;
utils::hook::detour inventory_update_hook;

// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t)> loot_busy{0x141E80BB0};
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t)> inventory_valid{0x141E0A3B0};
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t, int, int)> update_currency{0x141E0AB50};
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<bool(ControllerIndex_t, uint32_t, uint32_t, uint32_t, uint32_t,
                  uint16_t)>
    update_item{0x141E0AA30};

// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<const char *(const StringTable *, int, int)> table_cell{
    0x1422AE660};
// `const char * StringTable_Lookup(const StringTable *table, const
// int32_t comparisonColumn, const char *value, const int32_t valueColumn)`
// FIXME: this symbol should be moved to the proper namespace and file under
// src/client/game/symbols, named with function name matching that used by the
// engine, and typed correctly with parameter names
game::symbol<const char *(const StringTable *, int, const char *, int)>
    table_lookup{0x1422AE7F0};

/*
  FIXME: this is not the real engine struct definition.
  The struct definition should be completed, defined, properly named and
  namespaced, and placed in the appropriate file under the
  "src/client/game/structs" tree.
*/
struct loot_item {
  uint32_t id;
  uint32_t quantity;
  str256_t reference;
};
ASSERT_SIZE(loot_item, 0x108);

/*
  FIXME: this is not the real engine struct definition.
  The struct definition should be completed, defined, properly named and
  namespaced, and placed in the appropriate file under the
  "src/client/game/structs" tree.
*/
// Shared with the native factory completion and
// GetLootResults, not a Lua override.
struct loot_results {
  bool ready;
  uint8_t padding[3];
  game::lua::LootResultType result;
  loot_item granted[5];
  loot_item all[3];
  uint8_t rest[0x954 - 8 - 8 * sizeof(loot_item)];
};
static_assert(offsetof(loot_results, all) == 0x530);
static_assert(sizeof(loot_results) == 0x954);
game::symbol<loot_results> results{0x1512F1BB0};

bool enabled() {
  return local_currency && local_currency.get_bool() &&
         !game::ugc::UGC_ActiveMod_Loaded();
}

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
  if (generation == current_generation && !pool.empty()) {
    return pool;
  }
  pool.clear();
  generation = current_generation;
  const StringTable *items = table("gamedata/loot/zmlootitems.csv");
  const StringTable *stat_table = table("gamedata/stats/zm/zm_statsTable.csv");
  if (!items || !stat_table) {
    return pool;
  }
  for (int32_t row = 0; row < stat_table->rowCount; ++row) {
    const char *type = table_cell(stat_table, row, 2);
    if (!type || std::string_view(type) != "bubblegum_consumable") {
      continue;
    }
    const char *index_text = table_cell(stat_table, row, 0);
    const char *ref = table_cell(stat_table, row, 4);
    if (!index_text || !ref || !*ref || strlen(ref) >= 256) {
      continue;
    }
    const std::optional<uint32_t> index = accounting::parse_amount(index_text);
    const char *id_text = table_lookup(items, 0, ref, 1);
    if (!id_text) {
      continue;
    }
    uint32_t id = 0;
    const char *end = id_text + strlen(id_text);
    const std::from_chars_result parsed = std::from_chars(id_text, end, id);
    if (index && *index < 256 && *ref && strlen(ref) < 256 && id &&
        parsed.ec == std::errc{} && parsed.ptr == end) {
      pool.push_back({id, *index, ref});
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
  if (!gained || !used) {
    return std::nullopt;
  }
  return accounting::remaining(data.get(*gained), data.get(*used));
}

uint32_t get_currency(ControllerIndex_t controller, int32_t kind) {
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
      !inventory_valid(controller) || loot_busy(controller) ||
      game::com::Com_IsInGame() ||
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
  *results.get() = {};
  for (uint32_t i = 0; i < ROLL_SELECTION_POOL_SIZE; ++i) {
    loot_item &entry = results->all[i];
    entry.id = rolled[i].id;
    entry.quantity = 1;
    strscpy(entry.reference, rolled[i].reference);
    if (i < count) {
      results->granted[i] = entry;
      results->granted[i].quantity = 1;
      update_item(controller, rolled[i].id, quantities[i], 0, 0, 0);
    }
  }
  update_currency(controller, 3, *balance - count);
  results->result = game::lua::LootResultType::SUCCESS;
  results->ready = true;
  return true;
}

bool increment_currency(ControllerIndex_t controller, int32_t kind,
                        uint32_t amount) {
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
  update_currency(controller, 3, *balance);
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
  update_currency(controller, 3, *balance + amount);
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
        update_item(controller, found->id, quantity(data, *found).value(), 0, 0,
                    0);
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
  static game::LocalClientPool<bool> restored{};
  static game::LocalClientPool<bool> points_restored{};
  static game::LocalClientPool<StorageFileType> restored_file{};
  static game::LocalClientPool<game::XUID> restored_user{};
  if (!enabled() || !inventory_valid(controller)) {
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
    points_restored[controller] = update_currency(
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
    if (!count || !update_item(controller, item.id, *count, 0, 0, 0)) {
      return;
    }
  }
  update_currency(controller, 3, *balance);
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
    update_currency(controller, zombies ? 3 : 0, *amount);
    toast::success("Currency", utils::string::va("Balance set to %u", *amount));
  });
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
        update_currency(controller, 0, before + pending->points);
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
        update_currency(controller, 3, *balance + pending->vials);
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

  update_currency(controller, 0, updated);
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

  update_currency(index, 0, point_balance - cost);
  update_currency(index, 3, *vial_balance + amount);
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
  if (free || (!free && currency != 0 && currency != 3)) {
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
    update_currency(index, currency, remaining_currency);
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
  if (!enabled() || !valid_controller_index(controller) || recipe < 0 ||
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
    const char *text = table_cell(recipes, row, 0);
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

  const gum *result_gum =
      find_gum_by_name(pool, table_cell(recipes, recipe_row, 1));
  const char *result_count_text = table_cell(recipes, recipe_row, 2);
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

  stats data(controller, true);
  for (int32_t column = 3;; column += 2) {
    const char *reference = table_cell(recipes, recipe_row, column);
    if (!reference || !*reference || *reference == '#') {
      break;
    }
    const gum *ingredient = find_gum_by_name(pool, reference);
    const char *count_text = table_cell(recipes, recipe_row, column + 1);
    const std::optional<uint32_t> count =
        count_text ? accounting::parse_amount(count_text) : std::nullopt;
    if (!ingredient || !count || !*count) {
      return false;
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

  *results.get() = {};
  results->granted[0].id = result_gum->id;
  results->granted[0].quantity = *result_count;
  strscpy(results->granted[0].reference, result_gum->reference);
  results->all[0] = results->granted[0];
  results->result = game::lua::LootResultType::SUCCESS;
  results->ready = true;
  return true;
}

std::optional<uint32_t> item_quantity(ControllerIndex_t controller,
                                      uint32_t inventory_id) {
  if (!enabled() || !valid_controller_index(controller)) {
    return std::nullopt;
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
    update_item(index, item.id, 0, 0, 0, 0);
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

  update_currency(index, 0, point_balance);
  update_currency(index, 3, vial_balance);
  return true;
}

struct component final : client_component {
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

    balance_command("codpoints", false);
    balance_command("divinium", true);
    scheduler::loop(award_match_points, scheduler::pipeline::main, 1s);
  }
};
} // namespace currency

REGISTER_COMPONENT(currency::component)