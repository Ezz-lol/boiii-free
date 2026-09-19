#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>
#include <std_include.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "com.hpp"
#include "command.hpp"
#include "currency.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <atomic>
#include <random>

namespace currency {
namespace {
using game::ControllerIndex_t;
using game::StorageFileType;
using namespace game::ddl;
using namespace game::live::storage;

game::EngineDependentDvarMut local_currency;
game::EngineDependentDvarMut points_per_minute, points_per_match_cap;
utils::hook::detour currency_hook, spend_hook, increment_hook, consume_hook;
utils::hook::detour inventory_update_hook;

game::symbol<bool(ControllerIndex_t)> loot_busy{0x141E80BB0};
game::symbol<bool(ControllerIndex_t)> inventory_valid{0x141E0A3B0};
game::symbol<bool(ControllerIndex_t, int, int)> update_currency{0x141E0AB50};
game::symbol<bool(ControllerIndex_t, uint32_t, uint32_t, uint32_t, uint32_t,
                  uint16_t)>
    update_item{0x141E0AA30};

struct string_table;
game::symbol<int(const string_table *)> table_rows{0x1422AEDE0};
game::symbol<const char *(const string_table *, int, int)> table_cell{
    0x1422AE660};
game::symbol<const char *(const string_table *, int, const char *, int)>
    table_lookup{0x1422AE7F0};

struct loot_item {
  uint32_t id;
  uint32_t quantity;
  char reference[256];
};
static_assert(sizeof(loot_item) == 264);

// Shared with the native factory completion and GetLootResults, not a Lua
// override.
struct loot_results {
  bool ready;
  uint8_t padding[3];
  int result;
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
                               int item = -1) const {
    if (!context || !context->buff || !context->def) {
      return std::nullopt;
    }
    const auto *root = Storage_GetDDLRootState(file);
    // The native lookup returns address 80 when the file type is missing.
    if (reinterpret_cast<uintptr_t>(root) < 0x1000) {
      return std::nullopt;
    }
    auto state = *root;
    if (state.ddlDef != context->def) {
      return std::nullopt;
    }
    for (const auto name : path) {
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
    for (const auto &entry : changes) {
      if (entry.state.offset == state.offset) {
        return entry.after;
      }
    }
    return DDL_GetUInt(&state, context);
  }

  void set(const DDLState &state, uint32_t value) {
    for (auto &entry : changes) {
      if (entry.state.offset == state.offset) {
        entry.after = value;
        return;
      }
    }
    changes.push_back({state, get(state), value});
  }

  bool commit() {
    size_t written = 0;
    for (const auto &entry : changes) {
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
      const auto &entry = changes[--written];
      DDL_SetUInt(&entry.state, context, entry.before);
    }
    return false;
  }
};

std::optional<DDLState> player_stat(const stats &data, const char *name) {
  return data.find({"PlayerStatsList", name, "statValue"});
}

std::optional<uint32_t> vials(const stats &data) {
  const auto gained = player_stat(data, "BGB_TOKENS_GAINED");
  const auto used = player_stat(data, "BGB_TOKENS_USED");
  if (!gained || !used) {
    return std::nullopt;
  }
  return accounting::remaining(data.get(*gained), data.get(*used));
}

const string_table *table(const char *name) {
  return reinterpret_cast<const string_table *>(
      game::db::xasset::DB_FindXAssetHeader(
          game::db::xasset::XAssetType::STRINGTABLE, name, false, 0)
          .named);
}

struct gum {
  uint32_t id;
  int stat_index;
  std::string reference;
};

std::atomic<uint64_t> gum_generation{1};

void invalidate_gum_pool(const char *) { ++gum_generation; }

const std::vector<gum> &gum_pool() {
  thread_local std::vector<gum> pool;
  thread_local uint64_t generation = 0;
  const auto current_generation = gum_generation.load();
  if (generation == current_generation && !pool.empty()) {
    return pool;
  }
  pool.clear();
  generation = current_generation;
  const auto *items = table("gamedata/loot/zmlootitems.csv");
  const auto *stat_table = table("gamedata/stats/zm/zm_statsTable.csv");
  if (!items || !stat_table) {
    return pool;
  }
  for (int row = 0; row < table_rows(stat_table); ++row) {
    const auto *type = table_cell(stat_table, row, 2);
    if (!type || std::string_view(type) != "bubblegum_consumable") {
      continue;
    }
    const auto *index_text = table_cell(stat_table, row, 0);
    const char *ref = table_cell(stat_table, row, 4);
    if (!index_text || !ref || !*ref || strlen(ref) >= 256) {
      continue;
    }
    const auto index = accounting::parse_amount(index_text);
    const char *id_text = table_lookup(items, 0, ref, 1);
    if (!id_text) {
      continue;
    }
    uint32_t id = 0;
    const auto end = id_text + strlen(id_text);
    const auto parsed = std::from_chars(id_text, end, id);
    if (index && *index < 256 && *ref && strlen(ref) < 256 && id &&
        parsed.ec == std::errc{} && parsed.ptr == end) {
      pool.push_back({id, static_cast<int>(*index), ref});
    }
  }
  return pool;
}

std::optional<DDLState> gum_stat(const stats &data, const gum &item,
                                 const char *name) {
  return data.find({"ItemStats", "stats", name, "statValue"}, item.stat_index);
}

std::optional<uint32_t> quantity(const stats &data, const gum &item) {
  const auto gained = gum_stat(data, item, "bgbconsumablesgained");
  const auto used = gum_stat(data, item, "bgbconsumablesused");
  if (!gained || !used) {
    return std::nullopt;
  }
  return accounting::remaining(data.get(*gained), data.get(*used));
}

int get_currency(ControllerIndex_t controller, int kind) {
  if (!enabled() || (kind != 0 && kind != 3)) {
    return currency_hook.invoke<int>(controller, kind);
  }
  const stats data(controller, kind == 3);
  if (kind == 3) {
    return static_cast<int>(vials(data).value_or(0));
  }
  const auto state = player_stat(data, "CODPOINTS");
  return state ? static_cast<int>(
                     std::min(data.get(*state), accounting::max_balance))
               : 0;
}

bool spend_vials(ControllerIndex_t controller, int count) {
  if (!enabled()) {
    return spend_hook.invoke<bool>(controller, count);
  }
  if (controller < 0 || controller >= 2 || count < 1 || count > 3 ||
      !game::get_dvar_bool("loot_enabled").value_or(false) ||
      !inventory_valid(controller) || loot_busy(controller) ||
      game::com::Com_IsInGame() ||
      game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES) {
    return false;
  }
  stats data(controller, true);
  const auto balance = vials(data);
  const auto used = player_stat(data, "BGB_TOKENS_USED");
  const auto &pool = gum_pool();
  if (!balance || *balance < static_cast<uint32_t>(count) || !used ||
      pool.empty()) {
    return false;
  }
  data.set(*used, data.get(*used) + count);
  static std::mt19937 random(std::random_device{}());
  std::uniform_int_distribution<size_t> roll(0, pool.size() - 1);
  std::array<gum, 3> rolled;
  std::array<uint32_t, 3> quantities{};
  for (int i = 0; i < 3; ++i) {
    rolled[i] = pool[roll(random)];
    if (i < count) {
      const auto gained = gum_stat(data, rolled[i], "bgbconsumablesgained");
      if (!gained || !quantity(data, rolled[i])) {
        return false;
      }
      const auto next = accounting::credit(data.get(*gained), 1);
      if (!next) {
        return false;
      }
      data.set(*gained, *next);
    }
  }
  for (int i = 0; i < count; ++i) {
    const auto value = quantity(data, rolled[i]);
    if (!value) {
      return false;
    }
    quantities[i] = *value;
  }
  if (!data.commit()) {
    return false;
  }
  *results.get() = {};
  for (int i = 0; i < 3; ++i) {
    auto &entry = results->all[i];
    entry.id = rolled[i].id;
    entry.quantity = 1;
    memcpy(entry.reference, rolled[i].reference.c_str(),
           rolled[i].reference.size() + 1);
    if (i < count) {
      results->granted[i] = entry;
      results->granted[i].quantity = 1;
      update_item(controller, rolled[i].id, quantities[i], 0, 0, 0);
    }
  }
  update_currency(controller, 3, *balance - count);
  results->result = static_cast<int>(game::lua::LootResultType::SUCCESS);
  results->ready = true;
  return true;
}

bool increment_currency(ControllerIndex_t controller, int kind,
                        uint32_t amount) {
  if (!enabled() || kind != 3) {
    return increment_hook.invoke<bool>(controller, kind, amount);
  }
  stats data(controller, true);
  const auto pending = data.find({"vialsOwed"});
  const auto balance = vials(data);
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
  const auto root = UI_Model_GetModelForController(controller);
  const auto notify = UI_Model_CreateModelFromPath(root, "scriptNotify");
  UI_Model_SetInt(UI_Model_CreateModelFromPath(notify, "numArgs"), 0);
  if (!UI_Model_SetString(notify, "zombie_bgb_token_notification")) {
    UI_Model_ForceNotify(notify);
  }
  return true;
}

bool increment_from_lua(ControllerIndex_t controller, int kind,
                        uint32_t amount) {
  if (!enabled() || kind != 3) {
    return increment_currency(controller, kind, amount);
  }
  // Unlike ReportLootReward, the Lua increment has not credited the earned
  // stat.
  stats data(controller, true);
  const auto gained = player_stat(data, "BGB_TOKENS_GAINED");
  const auto balance = vials(data);
  if (!gained || !balance || !amount ||
      amount > accounting::max_balance - *balance) {
    return false;
  }
  const auto next = accounting::credit(data.get(*gained), amount);
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

void *consume_items(ControllerIndex_t controller, uint32_t *ids, int count,
                    uint32_t *amounts) {
  if (!enabled()) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  if (controller < 0 || controller >= 2 || !ids || !amounts || count < 1 ||
      count > 4000) {
    return nullptr;
  }
  const auto &pool = gum_pool();
  // ponytail: stock callers consume one item; batch support needs a native task
  // completion.
  if (count != 1) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  const auto found =
      std::find_if(pool.begin(), pool.end(),
                   [&](const gum &item) { return item.id == ids[0]; });
  if (found == pool.end()) {
    return consume_hook.invoke<void *>(controller, ids, count, amounts);
  }
  if (game::get_dvar_bool("cg_unlockall_loot").value_or(false)) {
    return nullptr;
  }
  stats data(controller, true);
  const auto gained = gum_stat(data, *found, "bgbconsumablesgained");
  const auto used = gum_stat(data, *found, "bgbconsumablesused");
  if (gained && used) {
    const auto next =
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
  if (controller < 0 || controller >= 2) {
    return;
  }
  inventory_update_hook.invoke<void>(controller);
  static bool restored[2]{};
  static bool points_restored[2]{};
  static StorageFileType restored_file[2]{};
  static game::XUID restored_user[2]{};
  if (!enabled() || !inventory_valid(controller)) {
    restored[controller] = false;
    points_restored[controller] = false;
    return;
  }
  stats data(controller, true);
  const auto user = game::live::user::LiveUser_GetXuid(controller);
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
  const auto points = player_stat(mp, "CODPOINTS");
  if (!points) {
    points_restored[controller] = false;
  } else if (!points_restored[controller]) {
    points_restored[controller] = update_currency(
        controller, 0,
        static_cast<int>(std::min(mp.get(*points), accounting::max_balance)));
  }
  if (!data.context || restored[controller]) {
    return;
  }
  const auto balance = vials(data);
  const auto &pool = gum_pool();
  if (!balance || pool.empty()) {
    return;
  }
  for (const auto &item : pool) {
    const auto count = quantity(data, item);
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
    const auto controller = game::com::Com_ControllerIndexes_GetPrimary();
    stats data(controller, zombies);
    const auto state =
        player_stat(data, zombies ? "BGB_TOKENS_GAINED" : "CODPOINTS");
    const auto used = zombies ? player_stat(data, "BGB_TOKENS_USED") : state;
    if (!state || !used) {
      toast::error("Currency", "Native stats are not ready.");
      return;
    }
    if (args.size() == 1) {
      const auto value = zombies ? vials(data).value() : data.get(*state);
      toast::info("Currency", utils::string::va("Balance: %u", value));
      return;
    }
    const auto amount = accounting::parse_amount(args.get(1));
    const auto next =
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
    clock::time_point started;
  };
  static std::optional<match> playing;
  static std::optional<match> pending;
  static uint32_t reward = 0;
  const auto now = clock::now();
  const auto controller = game::com::Com_ControllerIndexes_GetPrimary();
  if (!enabled() || controller < 0 || controller >= 2) {
    playing.reset();
    pending.reset();
    return;
  }
  const auto xuid = game::live::user::LiveUser_GetXuid(controller);
  const auto file = stats_type(false);
  const auto same_user = [&](const match &entry) {
    return entry.controller == controller && entry.xuid == xuid &&
           entry.file == file;
  };
  const auto mode = game::com::Com_SessionMode_GetMode();
  const bool in_game =
      game::com::Com_IsInGame() && !game::com::Com_IsRunningUILevel();
  if (playing && !same_user(*playing)) {
    playing.reset();
  }
  if (pending && !same_user(*pending)) {
    pending.reset();
  }
  if (in_game && !playing &&
      (mode == game::eModes::MULTIPLAYER || mode == game::eModes::ZOMBIES)) {
    playing = match{controller, xuid, file, now};
  } else if (!in_game && playing) {
    const auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now - playing->started)
            .count();
    const auto earned = accounting::match_reward(
        seconds, points_per_minute.get_int(), points_per_match_cap.get_int());
    reward =
        pending ? std::min(reward + earned, accounting::max_balance) : earned;
    pending = *playing;
    pending->started = now;
    playing.reset();
  }
  if (!pending || in_game) {
    return;
  }
  stats data(controller, false);
  const auto points = player_stat(data, "CODPOINTS");
  if (points) {
    const auto before = std::min(data.get(*points), accounting::max_balance);
    const auto amount = std::min(reward, accounting::max_balance - before);
    data.set(*points, before + amount);
    if (!amount || data.commit()) {
      pending.reset();
      if (amount) {
        update_currency(controller, 0, before + amount);
        toast::reward("MATCH REWARDS",
                      utils::string::va("+%u CoD Points", amount),
                      "uie_t7_icon_codpoints");
      }
      return;
    }
  }
  if (now - pending->started > 30s) {
    pending.reset();
    toast::error("Match Rewards",
                 "Native stats were unavailable; CoD Points were not saved.");
  }
}
} // namespace

std::optional<int> item_quantity(int controller, int inventory_id) {
  if (!enabled()) {
    return std::nullopt;
  }
  const auto &pool = gum_pool();
  for (const auto &item : pool) {
    if (item.id == static_cast<uint32_t>(inventory_id)) {
      const stats data(static_cast<ControllerIndex_t>(controller), true);
      return static_cast<int>(quantity(data, item).value_or(0));
    }
  }
  return std::nullopt;
}

struct component final : client_component {
  void post_unpack() override {
    local_currency = game::register_dvar_bool(
        "cg_local_currency", false, game::DvarFlags{.archive = 1},
        "Use native stats for local currency and GobbleGum inventory");
    com::on_level_load(invalidate_gum_pool);
    com::on_level_unload(invalidate_gum_pool);
    points_per_minute = game::register_dvar_int(
        "cg_codpoints_per_minute", 120, 0, 10000, game::DvarFlags{.archive = 1},
        "Local CoD Points per completed minute of Multiplayer or Zombies play");
    points_per_match_cap = game::register_dvar_int(
        "cg_currency_award_cap", 5000, 0, 100000, game::DvarFlags{.archive = 1},
        "Maximum local CoD Points awarded per match");
    currency_hook.create(0x141E09110_g, get_currency);
    spend_hook.create(0x141E83860_g, spend_vials);
    increment_hook.create(0x141E82C50_g, increment_currency);
    utils::hook::call(0x141F20FC2_g, increment_from_lua);
    consume_hook.create(0x141E08C10_g, consume_items);
    inventory_update_hook.create(0x141E0A9A0_g, inventory_update);
    balance_command("codpoints", false);
    balance_command("divinium", true);
    scheduler::loop(award_match_points, scheduler::pipeline::main, 1s);
  }
};
} // namespace currency

REGISTER_COMPONENT(currency::component)
