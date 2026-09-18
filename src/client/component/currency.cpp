#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/ui_scripting/execution.hpp>
#include <game/utils.hpp>

#include "command.hpp"
#include "currency.hpp"
#include "scheduler.hpp"
#include "toast.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <map>
#include <random>

using namespace game::lua;
using namespace game::lua::cod;
using namespace game::lua::hks;

namespace currency {
namespace {
constexpr int64_t MAX_BALANCE = 9999999;

constexpr int32_t MAX_STOCK = 999;

game::EngineDependentDvarMut dvar_cg_local_currency;
game::EngineDependentDvarMut dvar_cg_codpoints_per_minute;
game::EngineDependentDvarMut dvar_cg_divinium_per_match;
game::EngineDependentDvarMut dvar_cg_currency_award_cap;
game::EngineDependentDvarMut dvar_cg_last_divinium_award;
game::EngineDependentDvarMut dvar_cg_megachew_roller;
game::EngineDependentDvarMut dvar_cg_megachew_grants;

utils::hook::detour lua_getcodpoints_hook;
utils::hook::detour lua_getzmvials_hook;
utils::hook::detour lua_spendzmvials_hook;
utils::hook::detour lua_lootresultsready_hook;
utils::hook::detour lua_lootfailurereason_hook;
utils::hook::detour lua_getlootresults_hook;
utils::hook::detour lua_getlootitemquantity_hook;

struct bgb_t {
  int32_t index;
  const char *reference;
};

constexpr bgb_t MEGA_GOBBLEGUMS[] = {
    {191, "zm_bgb_soda_fountain"},
    {192, "zm_bgb_reign_drops"},
    {193, "zm_bgb_power_vacuum"},
    {194, "zm_bgb_idle_eyes"},
    {195, "zm_bgb_flavor_hexed"},
    {197, "zm_bgb_extra_credit"},
    {216, "zm_bgb_aftertaste"},
    {217, "zm_bgb_burned_out"},
    {218, "zm_bgb_cache_back"},
    {219, "zm_bgb_dead_of_nuclear_winter"},
    {220, "zm_bgb_ephemeral_enhancement"},
    {221, "zm_bgb_im_feelin_lucky"},
    {222, "zm_bgb_immolation_liquidation"},
    {223, "zm_bgb_kill_joy"},
    {224, "zm_bgb_killing_time"},
    {225, "zm_bgb_licensed_contractor"},
    {226, "zm_bgb_on_the_house"},
    {227, "zm_bgb_perkaholic"},
    {228, "zm_bgb_phoenix_up"},
    {229, "zm_bgb_pop_shocks"},
    {230, "zm_bgb_respin_cycle"},
    {231, "zm_bgb_unquenchable"},
    {232, "zm_bgb_wall_power"},
    {233, "zm_bgb_whos_keeping_score"},
    {234, "zm_bgb_crawl_space"},
    {235, "zm_bgb_fatal_contraption"},
    {236, "zm_bgb_head_drama"},
    {237, "zm_bgb_undead_man_walking"},
    {238, "zm_bgb_fear_in_headlights"},
    {239, "zm_bgb_secret_shopper"},
    {240, "zm_bgb_temporal_gift"},
    {241, "zm_bgb_unbearable"},
    {242, "zm_bgb_crate_power"},
    {243, "zm_bgb_disorderly_combat"},
    {245, "zm_bgb_shopping_free"},
    {246, "zm_bgb_slaughter_slide"},
    {247, "zm_bgb_bullet_boost"},
    {248, "zm_bgb_mind_blown"},
    {249, "zm_bgb_near_death_experience"},
    {251, "zm_bgb_profit_sharing"},
    {252, "zm_bgb_round_robbin"},
    {253, "zm_bgb_self_medication"},
    {254, "zm_bgb_board_games"},
    {255, "zm_bgb_board_to_death"},
};

struct megachew_t {
  bool pending{false};
  std::vector<const bgb_t *> rolled{};
};

utils::concurrency::container<megachew_t> megachew;

struct wallet_t {
  int64_t cod_points{0};
  int64_t divinium{0};
  std::map<uint32_t, std::map<int32_t, int32_t>> owned{};
};

utils::concurrency::container<wallet_t> wallet;

const std::filesystem::path &wallet_path() {
  static const std::filesystem::path path =
      std::filesystem::path("boiii_players") / "user" / "wallet.json";
  return path;
}

int64_t clamp_balance(const int64_t value) {
  return std::clamp<int64_t>(value, 0, MAX_BALANCE);
}

int64_t &balance_ref(wallet_t &w, const kind k) {
  return k == kind::cod_points ? w.cod_points : w.divinium;
}

void save_locked(const wallet_t &w) {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

  doc.AddMember("cod_points", w.cod_points, allocator);
  doc.AddMember("divinium", w.divinium, allocator);

  rapidjson::Value owned(rapidjson::kObjectType);
  for (const auto &[mode, items] : w.owned) {
    rapidjson::Value mode_items(rapidjson::kObjectType);
    for (const auto &[item_id, quantity] : items) {
      const std::string item_key = std::to_string(item_id);
      mode_items.AddMember(
          rapidjson::Value(item_key.c_str(),
                           static_cast<rapidjson::SizeType>(item_key.size()),
                           allocator),
          rapidjson::Value(quantity), allocator);
    }

    const std::string mode_key = std::to_string(mode);
    owned.AddMember(
        rapidjson::Value(mode_key.c_str(),
                         static_cast<rapidjson::SizeType>(mode_key.size()),
                         allocator),
        mode_items, allocator);
  }
  doc.AddMember("owned", owned, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);

  utils::io::create_directory(wallet_path().parent_path());
  utils::io::write_file(wallet_path(),
                        std::string(buffer.GetString(), buffer.GetSize()));
}

bool parse_key(const char *key, int64_t *out) {
  const std::string_view view{key};
  return std::from_chars(view.data(), view.data() + view.size(), *out).ec ==
         std::errc{};
}

void load() {
  std::string data;
  if (!utils::io::read_file(wallet_path(), &data) || data.empty()) {
    return;
  }

  rapidjson::Document doc;
  if (doc.Parse(data.c_str()).HasParseError() || !doc.IsObject()) {
    return;
  }

  wallet.access([&doc](wallet_t &w) {
    if (const auto it = doc.FindMember("cod_points");
        it != doc.MemberEnd() && it->value.IsInt64()) {
      w.cod_points = clamp_balance(it->value.GetInt64());
    }
    if (const auto it = doc.FindMember("divinium");
        it != doc.MemberEnd() && it->value.IsInt64()) {
      w.divinium = clamp_balance(it->value.GetInt64());
    }

    const auto owned = doc.FindMember("owned");
    if (owned == doc.MemberEnd() || !owned->value.IsObject()) {
      return;
    }

    for (const auto &mode_entry : owned->value.GetObject()) {
      int64_t mode = 0;
      if (!mode_entry.value.IsObject() ||
          !parse_key(mode_entry.name.GetString(), &mode) ||
          !game::valid_mode(static_cast<game::eModes>(mode))) {
        continue;
      }

      for (const auto &item_entry : mode_entry.value.GetObject()) {
        int64_t item_id = 0;
        if (!item_entry.value.IsInt() ||
            !parse_key(item_entry.name.GetString(), &item_id)) {
          continue;
        }

        w.owned[static_cast<uint32_t>(mode)][static_cast<int32_t>(item_id)] =
            item_entry.value.GetInt();
      }
    }
  });
}

luaReturnCount_e lua_getcodpoints_stub(lua_State *s) {
  if (!enabled()) {
    return lua_getcodpoints_hook.invoke<luaReturnCount_e>(s);
  }

  lua_pushinteger(s, static_cast<int32_t>(get(kind::cod_points)));
  return luaReturnCount_e::ONE;
}

luaReturnCount_e lua_getzmvials_stub(lua_State *s) {
  if (!enabled()) {
    return lua_getzmvials_hook.invoke<luaReturnCount_e>(s);
  }

  lua_pushinteger(s, static_cast<int32_t>(get(kind::divinium)));
  return luaReturnCount_e::ONE;
}

bool roller_enabled() {
  return enabled() && dvar_cg_megachew_roller &&
         dvar_cg_megachew_roller.get_bool();
}

bool grants_enabled() {
  return enabled() && dvar_cg_megachew_grants &&
         dvar_cg_megachew_grants.get_bool();
}

const bgb_t *find_gobblegum(const char *reference) {
  if (!reference) {
    return nullptr;
  }

  for (const bgb_t &gobblegum : MEGA_GOBBLEGUMS) {
    if (std::strcmp(gobblegum.reference, reference) == 0) {
      return &gobblegum;
    }
  }

  return nullptr;
}

const bgb_t &roll_gobblegum() {
  static thread_local std::mt19937 engine{std::random_device{}()};
  std::uniform_int_distribution<size_t> pick(0, std::size(MEGA_GOBBLEGUMS) - 1);
  return MEGA_GOBBLEGUMS[pick(engine)];
}

luaReturnCount_e lua_spendzmvials_stub(lua_State *s) {
  if (!enabled()) {
    return lua_spendzmvials_hook.invoke<luaReturnCount_e>(s);
  }

  const int32_t amount = lua_gettop(s) >= 2 ? lua_tointeger(s, 2) : 0;

  const bool ok = amount > 0 && spend(kind::divinium, amount);
  if (ok && roller_enabled()) {
    megachew.access([amount](megachew_t &m) {
      m.rolled.clear();
      m.rolled.reserve(static_cast<size_t>(amount));
      for (int32_t i = 0; i < amount; ++i) {
        m.rolled.push_back(&roll_gobblegum());
      }
      m.pending = true;
    });
  }

  lua_pushboolean(s, ok);
  return luaReturnCount_e::ONE;
}

bool has_pending_run() {
  return megachew.access<bool>([](const megachew_t &m) { return m.pending; });
}

luaReturnCount_e lua_lootresultsready_stub(lua_State *s) {
  if (!has_pending_run()) {
    return lua_lootresultsready_hook.invoke<luaReturnCount_e>(s);
  }

  lua_pushboolean(s, true);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e lua_lootfailurereason_stub(lua_State *s) {
  if (!has_pending_run()) {
    return lua_lootfailurereason_hook.invoke<luaReturnCount_e>(s);
  }

  lua_pushinteger(s, static_cast<int32_t>(game::lua::LootResultType::SUCCESS));
  return luaReturnCount_e::ONE;
}

luaReturnCount_e lua_getlootresults_stub(lua_State *s) {
  std::vector<const bgb_t *> won;
  const bool ours = megachew.access<bool>([&won](megachew_t &m) {
    if (!m.pending) {
      return false;
    }

    won = std::move(m.rolled);
    m.rolled.clear();
    m.pending = false;
    return true;
  });

  if (!ours) {
    return lua_getlootresults_hook.invoke<luaReturnCount_e>(s);
  }

  const bool record = grants_enabled();

  const ui_scripting::table results{};
  int32_t slot = 1;
  for (const bgb_t *gobblegum : won) {
    results.set(slot++, std::string(gobblegum->reference));
    if (record) {
      add_owned(game::eModes::ZOMBIES, gobblegum->index);
    }
  }

  HksObject value{};
  value.t = HksObjectType::TTABLE;
  value.v.table = results.ptr;
  lua_push(s, value);

  return luaReturnCount_e::ONE;
}

luaReturnCount_e lua_getlootitemquantity_stub(lua_State *s) {
  const bool ours = grants_enabled() && lua_gettop(s) >= 2;
  const bgb_t *gobblegum = ours ? find_gobblegum(lua_tostring(s, 2)) : nullptr;
  const int32_t owned =
      gobblegum ? owned_quantity(game::eModes::ZOMBIES, gobblegum->index) : 0;

  if (owned <= 0) {
    return lua_getlootitemquantity_hook.invoke<luaReturnCount_e>(s);
  }

  lua_pushinteger(s, owned);
  return luaReturnCount_e::ONE;
}

std::chrono::steady_clock::time_point match_start{};
bool in_match = false;
game::eModes match_mode = game::eModes::INVALID;

void award_for_match(const std::chrono::seconds duration) {
  const int64_t minutes = duration.count() / 60;
  if (minutes <= 0) {
    return;
  }

  const int64_t cap = dvar_cg_currency_award_cap.get_int();
  const int64_t points =
      std::min(minutes * dvar_cg_codpoints_per_minute.get_int(), cap);

  std::string summary;
  int32_t awarded_vials = 0;
  if (points > 0) {
    add(kind::cod_points, points);
    summary = utils::string::va("+%lld CoD Points", points);
  }

  if (match_mode == game::eModes::ZOMBIES) {
    const int64_t vials = dvar_cg_divinium_per_match.get_int();
    if (vials > 0) {
      add(kind::divinium, vials);
      awarded_vials = static_cast<int32_t>(vials);
      summary += summary.empty()
                     ? utils::string::va("+%lld Liquid Divinium", vials)
                     : utils::string::va(", +%lld Liquid Divinium", vials);
    }
  }

  dvar_cg_last_divinium_award.set(awarded_vials);

  if (!summary.empty()) {
    const char *icon = awarded_vials > 0 ? "t7_hud_zm_vial_256"
                                         : "uie_t7_icon_codpoints";
    toast::reward("MATCH REWARDS", summary, icon);
  }
}

void poll_match_state() {
  const bool now_in_match = game::com::Com_IsInGame();
  if (now_in_match == in_match) {
    return;
  }

  in_match = now_in_match;

  if (in_match) {
    dvar_cg_last_divinium_award.set(0);
    match_start = std::chrono::steady_clock::now();
    match_mode = game::com::Com_SessionMode_GetMode();
    return;
  }

  if (!enabled()) {
    return;
  }

  award_for_match(std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - match_start));
  match_mode = game::eModes::INVALID;
}

int64_t parse_amount(const char *raw, bool *ok) {
  const std::string_view view{raw};
  int64_t value = 0;
  *ok = std::from_chars(view.data(), view.data() + view.size(), value).ec ==
        std::errc{};
  return value;
}

void add_balance_command(const char *name, const kind k,
                         const char *display_name) {
  command::add(name, [k, display_name](const command::params &params) {
    if (params.size() < 2) {
      toast::info(display_name, utils::string::va("Balance: %lld", get(k)));
      return;
    }

    bool ok = false;
    const int64_t amount = parse_amount(params.get(1), &ok);
    if (!ok) {
      toast::error(display_name,
                   utils::string::va("'%s' is not a number.", params.get(1)));
      return;
    }

    set(k, amount);
    toast::success(display_name,
                   utils::string::va("Balance set to %lld", get(k)));
  });
}
}

bool enabled() {
  return dvar_cg_local_currency && dvar_cg_local_currency.get_bool();
}

int64_t get(const kind k) {
  return wallet.access<int64_t>([k](wallet_t &w) { return balance_ref(w, k); });
}

void set(const kind k, const int64_t amount) {
  wallet.access([k, amount](wallet_t &w) {
    balance_ref(w, k) = clamp_balance(amount);
    save_locked(w);
  });
}

void add(const kind k, const int64_t amount) {
  wallet.access([k, amount](wallet_t &w) {
    int64_t &balance = balance_ref(w, k);
    balance = clamp_balance(balance + amount);
    save_locked(w);
  });
}

bool spend(const kind k, const int64_t amount) {
  return wallet.access<bool>([k, amount](wallet_t &w) {
    int64_t &balance = balance_ref(w, k);
    if (amount < 0 || balance < amount) {
      return false;
    }

    balance -= amount;
    save_locked(w);
    return true;
  });
}

int32_t owned_quantity(const game::eModes mode, const int32_t item_id) {
  if (!enabled()) {
    return 0;
  }

  return wallet.access<int32_t>([mode, item_id](wallet_t &w) -> int32_t {
    const auto mode_it = w.owned.find(static_cast<uint32_t>(mode));
    if (mode_it == w.owned.end()) {
      return 0;
    }

    const auto item_it = mode_it->second.find(item_id);
    return item_it == mode_it->second.end() ? 0 : item_it->second;
  });
}

void add_owned(const game::eModes mode, const int32_t item_id,
               const int32_t quantity) {
  if (!game::valid_mode(mode) || quantity <= 0) {
    return;
  }

  wallet.access([mode, item_id, quantity](wallet_t &w) {
    int32_t &owned = w.owned[static_cast<uint32_t>(mode)][item_id];
    owned = std::clamp(owned + quantity, 0, MAX_STOCK);
    save_locked(w);
  });
}

void save() {
  wallet.access([](const wallet_t &w) { save_locked(w); });
}

struct component final : client_component {
  void post_unpack() override {
    dvar_cg_local_currency = game::register_dvar_bool(
        "cg_local_currency", true, game::DvarFlags{.archive = 1},
        "Emulate CoD Points and Liquid Divinium locally, since the "
        "Demonware marketplace they normally come from is not available");
    dvar_cg_codpoints_per_minute = game::register_dvar_int(
        "cg_codpoints_per_minute", 120, 0, 10000, game::DvarFlags{.archive = 1},
        "CoD Points awarded per minute of play when a match ends");
    dvar_cg_divinium_per_match = game::register_dvar_int(
        "cg_divinium_per_match", 3, 0, 100, game::DvarFlags{.archive = 1},
        "Liquid Divinium awarded when a Zombies match ends");
    dvar_cg_currency_award_cap = game::register_dvar_int(
        "cg_currency_award_cap", 5000, 0, 100000, game::DvarFlags{.archive = 1},
        "Maximum CoD Points awarded from a single match");
    dvar_cg_last_divinium_award = game::register_dvar_int(
        "cg_last_divinium_award", 0, 0, 100, game::DvarFlags{}, "");
    dvar_cg_megachew_roller = game::register_dvar_bool(
        "cg_megachew_roller", true, game::DvarFlags{.archive = 1},
        "Emulate Dr. Monty's Factory, which otherwise errors out because the "
        "loot results it waits on come from Demonware");
    dvar_cg_megachew_grants = game::register_dvar_bool(
        "cg_megachew_grants", false, game::DvarFlags{.archive = 1},
        "Keep the gobblegums rolled at Dr. Monty's Factory, so the frontend "
        "shows a real count and using one spends it. Gobblegums that have "
        "never been rolled stay unrestricted either way");

    lua_getcodpoints_hook.create(api::Lua_CoD_LuaCall_GetCoDPoints,
                                 lua_getcodpoints_stub);
    lua_getzmvials_hook.create(api::Lua_CoD_LuaCall_GetZMVials,
                               lua_getzmvials_stub);
    lua_spendzmvials_hook.create(api::Lua_CoD_LuaCall_SpendZMVials,
                                 lua_spendzmvials_stub);
    lua_lootresultsready_hook.create(api::Lua_CoD_LuaCall_LootResultsReady,
                                     lua_lootresultsready_stub);
    lua_lootfailurereason_hook.create(api::Lua_CoD_LuaCall_LootFailureReason,
                                      lua_lootfailurereason_stub);
    lua_getlootresults_hook.create(api::Lua_CoD_LuaCall_GetLootResults,
                                   lua_getlootresults_stub);
    lua_getlootitemquantity_hook.create(
        api::Lua_CoD_LuaCall_GetLootItemQuantity, lua_getlootitemquantity_stub);

    add_balance_command("codpoints", kind::cod_points, "CoD Points");
    add_balance_command("divinium", kind::divinium, "Liquid Divinium");

    command::add("wallet", [](const command::params &) {
      toast::info("Wallet", utils::string::va(
                                "%lld CoD Points, %lld Liquid Divinium",
                                get(kind::cod_points), get(kind::divinium)));
    });

    command::add("wallet_reset", [](const command::params &) {
      wallet.access([](wallet_t &w) {
        w = wallet_t{};
        save_locked(w);
      });
      toast::success("Wallet", "Balances and purchased items cleared.");
    });

    load();

    scheduler::loop(poll_match_state, scheduler::pipeline::main, 1s);
  }

  void pre_destroy() override {
    if (dvar_cg_local_currency) {
      save();
    }
  }
};
}

REGISTER_COMPONENT(currency::component)
