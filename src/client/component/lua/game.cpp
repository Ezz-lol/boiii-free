#include <std_include.hpp>

#include <game/ui_scripting/execution.hpp>
#include <game/utils.hpp>

#include <component/currency.hpp>
#include <component/discord.hpp>
#include <component/download_overlay.hpp>
#include <component/friends.hpp>
#include <component/getinfo.hpp>
#include <component/gpu.hpp>
#include <component/lua/lua_state.hpp>
#include <component/name.hpp>
#include <component/scheduler.hpp>
#include <component/toast.hpp>
#include <component/workshop/workshop.hpp>
#include <loader/component_loader.hpp>

#include <steam/interfaces/matchmaking_servers.hpp>
#include <steam/steam.hpp>
#include <steam/voice.hpp>

#include <utils/http.hpp>
#include <utils/string.hpp>

namespace game_lua {
using namespace game::db;
using namespace game::db::xasset;
using namespace game::lua;
using namespace game::lua::hks;
using namespace ui_scripting;

template <typename F> luaReturnCount_e call(lua_State *luaVM, F f) {
  try {
    const arguments results = wrap_function(f)(get_return_values());
    for (const script_value &result : results) {
      push_value(result);
    }
    return static_cast<luaReturnCount_e>(results.size());
  } catch (const std::exception &ex) {
    hksi_luaL_error(luaVM, ex.what());
  }
  return luaReturnCount_e::NONE;
}

bool is_mod_rawfile(const std::string &name) {
  const void *data =
      DB_FindXAssetHeader(XAssetType::RAWFILE, name.c_str(), false, 0).data;
  if (!data) {
    return false;
  }
  for (const pool::XAssetEntryPoolEntry &item : pool::g_assetEntryPool->pool) {
    const XAssetEntry &entry = item.entry;
    if (entry.inuse && entry.asset.type == XAssetType::RAWFILE &&
        entry.asset.header.data == data) {
      return (*xzone::g_zoneNames)[entry.zoneIndex].zoneType ==
             game::ZoneType::MOD;
    }
  }
  return false;
}

table social_player(const game::XUID id, const std::string &name) {
  const std::optional<friends::friend_entry> entry = friends::find_friend(id);
  table player = table();
  player.set("steam_id", utils::string::va("%llu", id));
  player.set("name", name);
  player.set("friend", entry.has_value());
  player.set("status", entry ? static_cast<int>(entry->state) : 0);
  player.set("server", entry ? entry->server_address : std::string());
  player.set("invited", friends::has_invited_us(id));
  return player;
}

std::string time_ago(const int64_t timestamp) {
  const int64_t minutes =
      std::max<int64_t>(std::time(nullptr) - timestamp, 0) / 60;
  if (minutes < 1)
    return "just now";
  if (minutes < 60)
    return std::format("{}m ago", minutes);
  if (minutes < 1440)
    return std::format("{}h ago", minutes / 60);
  return std::format("{}d ago", minutes / 1440);
}

luaReturnCount_e getvoiceinputdevices(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    table devices = table();
    int index = 1;
    for (const std::string &name : steam::voice::InputDevices()) {
      devices.set(index++, name);
    }
    return devices;
  });
}

luaReturnCount_e getgpupreference(lua_State *luaVM) {
  lua_pushinteger(luaVM, gpu::get_preference());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e setgpupreference(lua_State *luaVM) {
  if (lua_gettop(luaVM) > 0 && lua_isnumber(luaVM, 1)) {
    gpu::set_preference(lua_tonumber(luaVM, 1));
  }
  return luaReturnCount_e::NONE;
}

luaReturnCount_e getfriendcount(lua_State *luaVM) {
  lua_pushinteger(luaVM, friends::get_friend_count());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e getfriend(lua_State *luaVM) {
  return call(luaVM, [](int index) -> table {
    const friends::friend_entry f = friends::get_friend_by_index(index);
    table t = table();
    t.set("steam_id", utils::string::va("%llu", f.steam_id));
    t.set("name", std::string(f.name));
    t.set("status", static_cast<int>(f.state));
    t.set("server", std::string(f.server_address));
    t.set("invited", friends::has_invited_us(f.steam_id));
    return t;
  });
}

luaReturnCount_e getrecentplayer(lua_State *luaVM) {
  return call(luaVM, [](const int index) -> table {
    const std::vector<friends::recent_player> players =
        friends::get_recent_players();
    if (index < 0 || index >= static_cast<int>(players.size()))
      return table();
    const friends::recent_player &recent = players[index];
    table player = social_player(recent.steam_id, recent.name);
    player.set("seen", time_ago(recent.last_seen));
    return player;
  });
}

luaReturnCount_e getrecentcount(lua_State *luaVM) {
  return call(luaVM, []() -> int32_t {
    return static_cast<int32_t>(friends::get_recent_count());
  });
}

luaReturnCount_e refreshfriends(lua_State *luaVM) {
  return call(luaVM, [] { friends::refresh_presence(); });
}

luaReturnCount_e addfriend(lua_State *luaVM) {
  return call(luaVM,
              [](const std::string &steam_id_str, const std::string &name) {
                uint64_t steam_id = 0;
                try {
                  steam_id = std::stoull(steam_id_str);
                } catch (...) {
                  return;
                }
                friends::add_friend(steam_id, name);
              });
}

luaReturnCount_e removefriend(lua_State *luaVM) {
  return call(luaVM, [](const std::string &steam_id_str) {
    uint64_t steam_id = 0;
    try {
      steam_id = std::stoull(steam_id_str);
    } catch (...) {
      return;
    }
    friends::remove_friend(steam_id);
  });
}

luaReturnCount_e invitefriend(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id_str) -> bool {
    uint64_t id = 0;
    try {
      id = std::stoull(id_str);
    } catch (...) {
      return false;
    }
    return friends::invite_to_game(id);
  });
}

luaReturnCount_e connecttofriend(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id_str) -> bool {
    uint64_t id = 0;
    try {
      id = std::stoull(id_str);
    } catch (...) {
      return false;
    }
    return friends::connect_to_friend(id);
  });
}

luaReturnCount_e getsocialplayer(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id_hex) -> table {
    const game::XUID id = std::strtoull(id_hex.c_str(), nullptr, 16);
    return social_player(id, friends::get_known_name(id));
  });
}

luaReturnCount_e issocialfriend(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id_hex) -> bool {
    try {
      return friends::is_friend(std::stoull(id_hex, nullptr, 16));
    } catch (...) {
      return false;
    }
  });
}

luaReturnCount_e connectsocialfriend(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id_hex) -> bool {
    try {
      return friends::connect_to_friend(std::stoull(id_hex, nullptr, 16));
    } catch (...) {
      return false;
    }
  });
}

luaReturnCount_e ishost(lua_State *luaVM) {
  return call(luaVM, []() -> bool { return getinfo::is_host(); });
}

luaReturnCount_e purchasecodpoints(lua_State *luaVM) {
  return call(luaVM,
              [](const game::ControllerIndex_t controller,
                 const uint32_t amount) -> int32_t {
                constexpr std::array allowed_amounts{200, 1100, 2400, 5000,
                                                     13000};
                if (std::find(allowed_amounts.begin(), allowed_amounts.end(),
                              amount) == allowed_amounts.end()) {
                  return -1;
                }

                const std::optional<uint32_t> balance =
                    currency::add_cod_points(controller, amount);
                return balance ? static_cast<int>(*balance) : -1;
              });
}

luaReturnCount_e setstoreprice(lua_State *luaVM) {
  return call(luaVM, [](const uint32_t sku, const uint32_t cost) -> bool {
    return currency::set_store_price(sku, cost);
  });
}

luaReturnCount_e ismodrawfile(lua_State *luaVM) {
  return call(luaVM, is_mod_rawfile);
}

luaReturnCount_e israwfile(lua_State *luaVM) {
  return call(luaVM, [](const std::string &name) {
    return DB_FindXAssetHeader(XAssetType::RAWFILE, name.c_str(), false, 0)
               .data != nullptr;
  });
}

luaReturnCount_e purchasevials(lua_State *luaVM) {
  return call(luaVM,
              [](const game::ControllerIndex_t controller, const int32_t cost,
                 const int32_t amount) -> bool {
                if (cost <= 0 || amount <= 0) {
                  return false;
                }
                return currency::purchase_vials(controller, cost, amount);
              });
}

luaReturnCount_e purchasedistills(lua_State *luaVM) {
  return call(luaVM,
              [](const game::ControllerIndex_t controller,
                 const std::string &kind, const uint32_t payment) -> bool {
                return currency::purchase_distills(controller, kind, payment);
              });
}

luaReturnCount_e getfreedistillcooldown(lua_State *luaVM) {
  return call(luaVM,
              []() -> uint32_t { return currency::free_distill_cooldown(); });
}

luaReturnCount_e getdistillbalance(lua_State *luaVM) {
  return call(luaVM, [](const bool free) -> int {
    return currency::distill_balance(free);
  });
}

luaReturnCount_e cookgobblegumrecipe(lua_State *luaVM) {
  return call(luaVM,
              [](const game::ControllerIndex_t controller, const int recipe,
                 const bool free_distills) -> bool {
                return currency::cook_recipe(controller, recipe, free_distills);
              });
}

luaReturnCount_e resetgobblegums(lua_State *luaVM) {
  return call(luaVM, [](const game::ControllerIndex_t controller) -> bool {
    return currency::reset_gobblegums(controller);
  });
}

luaReturnCount_e resetblackmarket(lua_State *luaVM) {
  return call(luaVM, [](const game::ControllerIndex_t controller) -> bool {
    return currency::reset_black_market(controller);
  });
}

luaReturnCount_e getblackmarketdrops(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    table drops = table();
    int index = 1;
    for (const currency::loot_drop &drop : currency::black_market_drops()) {
      table entry = table();
      // A string like the game's own loot item IDs, HKS numbers are floats
      entry.set("id", std::to_string(drop.item_id));
      entry.set("time", drop.time);
      drops.set(index++, entry);
    }
    return drops;
  });
}

luaReturnCount_e iscurrencymaxed(lua_State *luaVM) {
  return call(
      luaVM,
      [](const game::ControllerIndex_t controller, const int32_t kind) -> bool {
        return currency::is_currency_maxed(controller, kind);
      });
}

luaReturnCount_e setcurrencymaxed(lua_State *luaVM) {
  return call(luaVM,
              [](const game::ControllerIndex_t controller, const int32_t kind,
                 const bool maxed) -> bool {
                return currency::set_currency_maxed(controller, kind, maxed);
              });
}

luaReturnCount_e savestats(lua_State *luaVM) {
  return call(
      luaVM,
      [](const game::ControllerIndex_t controller, const int mode) -> bool {
        if (controller < 0 || controller >= 2 || mode < 0 || mode > 2 ||
            game::com::Com_IsInGame()) {
          return false;
        }

        const bool online = game::com::Com_SessionMode_GetNetworkMode() ==
                            game::eNetworkModes::ONLINE;
        game::StorageFileType file;
        if (mode == static_cast<int>(game::eModes::ZOMBIES)) {
          file = online ? game::StorageFileType::ZM_STATS_ONLINE
                        : game::StorageFileType::ZM_STATS_OFFLINE;
        } else if (mode == static_cast<int>(game::eModes::MULTIPLAYER)) {
          file = online ? game::StorageFileType::MP_STATS_ONLINE
                        : game::StorageFileType::MP_STATS_OFFLINE;
        } else {
          file = online ? game::StorageFileType::CP_STATS_ONLINE
                        : game::StorageFileType::CP_STATS_OFFLINE;
        }
        return game::live::storage::Storage_Write(
            static_cast<game::ControllerIndex_t>(controller), file, 0);
      });
}

luaReturnCount_e httpget(lua_State *luaVM) {
  return call(luaVM, [](const std::string &url) -> std::string {
    const std::optional<std::string> result = utils::http::get_data(url);
    return result.value_or("");
  });
}

luaReturnCount_e httppost(lua_State *luaVM) {
  return call(
      luaVM,
      [](const std::string &url, const std::string &body) -> std::string {
        const std::optional<std::string> result =
            utils::http::post_data(url, body);
        return result.value_or("");
      });
}

luaReturnCount_e getworkshopcatalog(lua_State *luaVM) {
  return call(luaVM,
              [](const std::string &query, const int page,
                 const std::string &kind) -> table {
                workshop::request_catalog_page(query, page, kind);
                table result{};
                int index = 1;
                for (const auto &item : workshop::get_catalog_page()) {
                  table row{};
                  row.set("id", item.id);
                  row.set("title", item.title);
                  row.set("description", item.description);
                  row.set("kind", item.kind);
                  row.set("file_size", static_cast<double>(item.file_size));
                  row.set("preview", item.preview_url);
                  result.set(index++, row);
                }
                return result;
              });
}

luaReturnCount_e requestworkshopcatalog(lua_State *luaVM) {
  return call(luaVM, [](const std::string &query, const int page,
                        const std::string &kind) {
    workshop::request_catalog_page(query, page, kind);
  });
}

luaReturnCount_e getworkshopcatalogstatus(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    const auto status = workshop::get_catalog_status();
    table row{};
    row.set("loading", status.loading);
    row.set("page", status.page);
    row.set("generation", static_cast<double>(status.generation));
    row.set("error", status.error);
    return row;
  });
}

luaReturnCount_e requestworkshoppreview(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id, const std::string &url) {
    workshop::request_preview(id, url);
  });
}

luaReturnCount_e setworkshopcover(lua_State *luaVM) {
  return call(luaVM,
              [](const std::string &id) { workshop::set_preview_cover(id); });
}

luaReturnCount_e getworkshopdownload(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    const auto state = download_overlay::get_state();
    table row{};
    row.set("active", state.active);
    row.set("id", workshop::queue::current_id());
    row.set("item_name", state.item_name);
    row.set("downloaded_bytes", static_cast<double>(state.downloaded_bytes));
    row.set("total_bytes", static_cast<double>(state.total_bytes));
    row.set("speed_bps", static_cast<double>(state.speed_bps));
    row.set("eta_seconds", static_cast<double>(state.eta_seconds));
    row.set("status_line", state.status_line);
    row.set("fraction",
            state.total_bytes
                ? std::min(1.0, static_cast<double>(state.downloaded_bytes) /
                                    static_cast<double>(state.total_bytes))
                : 0.0);
    return row;
  });
}

luaReturnCount_e queueworkshopitem(lua_State *luaVM) {
  return call(luaVM,
              [](const std::string &id, const std::string &kind,
                 const std::string &title, const std::string &size) -> bool {
                if (!workshop::queue::add(
                        id, kind, title,
                        std::strtoull(size.c_str(), nullptr, 10), false))
                  return false;
                const int position = workshop::queue::position(id);
                const auto name = title.empty() ? id : title;
                toast::info("Workshop",
                            position <= 1 && !workshop::is_any_download_active()
                                ? "Downloading " + name
                                : "Added to queue (#" +
                                      std::to_string(position) + "): " + name);
                return true;
              });
}

luaReturnCount_e getworkshopqueue(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    table result{};
    int index = 1;
    for (const auto &entry : workshop::queue::list()) {
      table row{};
      row.set("id", entry.id);
      row.set("kind", entry.kind);
      row.set("title", entry.title);
      row.set("file_size", static_cast<double>(entry.file_size));
      row.set("state", std::string(workshop::queue::state_name(entry.state)));
      row.set("position", entry.position);
      result.set(index++, row);
    }
    return result;
  });
}

luaReturnCount_e getworkshopqueuegeneration(lua_State *luaVM) {
  return call(luaVM, []() -> int {
    return static_cast<int>(workshop::queue::generation() & 0x7FFFFFFF);
  });
}

luaReturnCount_e getworkshopqueueposition(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id) -> int {
    return workshop::queue::position(id);
  });
}

luaReturnCount_e removeworkshopqueue(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id) -> bool {
    return workshop::queue::remove(id);
  });
}

luaReturnCount_e moveworkshopqueue(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id, const int delta) -> bool {
    return workshop::queue::move(id, delta);
  });
}

luaReturnCount_e clearworkshopqueue(lua_State *luaVM) {
  return call(luaVM, []() { workshop::queue::clear(); });
}

luaReturnCount_e getworkshopinstalled(lua_State *luaVM) {
  return call(luaVM, []() -> table {
    table result{};
    int index = 1;
    for (const auto &item : workshop::list_installed()) {
      table row{};
      row.set("id", item.id);
      row.set("title", item.title);
      row.set("description", item.description);
      row.set("kind", item.kind);
      row.set("path", item.path);
      row.set("file_size", static_cast<double>(item.file_size));
      row.set("installed", true);
      result.set(index++, row);
    }
    return result;
  });
}

luaReturnCount_e isworkshopinstalled(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id) -> bool {
    return !workshop::find_installed(id).empty();
  });
}

luaReturnCount_e deleteworkshopitem(lua_State *luaVM) {
  return call(luaVM, [](const std::string &id) -> bool {
    std::string error;
    const bool ok = workshop::delete_installed(id, error);
    if (ok)
      toast::success("Workshop", "Item deleted.");
    else
      toast::error("Workshop", error);
    return ok;
  });
}

luaReturnCount_e cancelworkshopdownload(lua_State *luaVM) {
  return call(luaVM, []() -> bool { return workshop::cancel_download(); });
}

luaReturnCount_e setDiscordPlayerScore(lua_State *luaVM) {
  return call(luaVM, [](int score) { discord::set_player_score(score); });
}

luaReturnCount_e setDiscordEnemyScore(lua_State *luaVM) {
  return call(luaVM, [](int score) { discord::set_enemy_score(score); });
}

luaReturnCount_e setDiscordRoundsPlayed(lua_State *luaVM) {
  return call(luaVM, [](int round) { discord::set_rounds_played(round); });
}

luaReturnCount_e hotreloadcheck(lua_State *luaVM) {
  return call(luaVM, [] { ui_scripting::check_hot_reload(); });
}

luaReturnCount_e hotreloadstart(lua_State *luaVM) {
  return call(luaVM, [](const std::string &path) {
    ui_scripting::start_hot_reload(path);
  });
}

luaReturnCount_e getappdatapath(lua_State *luaVM) {
  return call(luaVM, []() -> std::string {
    return game::get_appdata_path().generic_string();
  });
}

luaReturnCount_e getclientoverridename(lua_State *luaVM) {
  return call(luaVM, [](const game::ClientNum_t client_num) -> std::string {
    return name::get_name_override(client_num).value_or("");
  });
}

luaReturnCount_e getclientoverridetag(lua_State *luaVM) {
  return call(luaVM, [](const game::ClientNum_t client_num) -> std::string {
    return name::get_clan_abbrev_override(client_num).value_or("");
  });
}

luaReturnCount_e getrawservercount(lua_State *luaVM) {
  return call(luaVM, []() -> int32_t {
    return steam::get_raw_internet_server_count();
  });
}

luaReturnCount_e isserverlistrefreshing(lua_State *luaVM) {
  return call(luaVM,
              []() -> bool { return steam::is_server_list_refreshing(); });
}

luaReturnCount_e getrawserverinfo(lua_State *luaVM) {
  return call(luaVM, [](const int32_t index) -> table {
    table t = table();
    const steam::gameserveritem_t *item =
        steam::get_raw_internet_server_item(index);
    if (!item) {
      return t;
    }

    t.set("name", std::string(item->m_szServerName));
    t.set("map", std::string(item->m_szMap));
    t.set("desc", std::string(item->m_szGameDescription));
    t.set("ping", item->m_nPing);
    t.set("playerCount", item->m_nPlayers);
    t.set("maxPlayers", item->m_nMaxPlayers);
    t.set("botCount", item->m_nBotPlayers);
    t.set("password", item->m_bPassword);
    t.set("secure", item->m_bSecure);

    const std::string tags = std::string(item->m_szGameTags);
    const auto get_tag = [&](const char *key) -> std::string {
      const char *val = game::info::Info_ValueForKey(tags.c_str(), key);
      return val ? val : "";
    };

    t.set("gametype", get_tag("gametype"));
    t.set("dedicated", get_tag("dedicated") == "true");
    t.set("ranked", get_tag("ranked") == "true");
    t.set("hardcore", get_tag("hardcore") == "true");
    t.set("zombies", get_tag("zombies") == "true");
    t.set("campaign", get_tag("campaign") == "true" ? 1 : 0);
    t.set("rounds", std::atoi(get_tag("rounds").c_str()));
    t.set("modName", get_tag("modName"));

    const uint32_t ip = item->m_NetAdr.m_unIP;
    const uint16_t port = item->m_NetAdr.m_usConnectionPort;
    t.set("connectAddr", utils::string::va("%u.%u.%u.%u:%u", (ip >> 24) & 0xFF,
                                           (ip >> 16) & 0xFF, (ip >> 8) & 0xFF,
                                           ip & 0xFF, port));
    t.set("serverIndex", index);

    return t;
  });
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("lua/game");

public:
  void post_unpack() override {
    static constexpr const luaL_Reg GameLibrary[] = {
        lua_state::luaL_LoggedReg<"game", "getfriendcount", getfriendcount>(),
        lua_state::luaL_LoggedReg<"game", "getfriend", getfriend>(),
        lua_state::luaL_LoggedReg<"game", "getrecentplayer", getrecentplayer>(),
        lua_state::luaL_LoggedReg<"game", "getrecentcount", getrecentcount>(),
        lua_state::luaL_LoggedReg<"game", "refreshfriends", refreshfriends>(),
        lua_state::luaL_LoggedReg<"game", "addfriend", addfriend>(),
        lua_state::luaL_LoggedReg<"game", "removefriend", removefriend>(),
        lua_state::luaL_LoggedReg<"game", "invitefriend", invitefriend>(),
        lua_state::luaL_LoggedReg<"game", "connecttofriend", connecttofriend>(),
        lua_state::luaL_LoggedReg<"game", "getsocialplayer", getsocialplayer>(),
        lua_state::luaL_LoggedReg<"game", "issocialfriend", issocialfriend>(),
        lua_state::luaL_LoggedReg<"game", "connectsocialfriend",
                                  connectsocialfriend>(),
        lua_state::luaL_LoggedReg<"game", "ishost", ishost>(),
        lua_state::luaL_LoggedReg<"game", "purchasecodpoints",
                                  purchasecodpoints>(),
        lua_state::luaL_LoggedReg<"game", "setstoreprice", setstoreprice>(),
        lua_state::luaL_LoggedReg<"game", "ismodrawfile", ismodrawfile>(),
        lua_state::luaL_LoggedReg<"game", "israwfile", israwfile>(),
        lua_state::luaL_LoggedReg<"game", "purchasevials", purchasevials>(),
        lua_state::luaL_LoggedReg<"game", "purchasedistills",
                                  purchasedistills>(),
        lua_state::luaL_LoggedReg<"game", "getfreedistillcooldown",
                                  getfreedistillcooldown>(),
        lua_state::luaL_LoggedReg<"game", "getdistillbalance",
                                  getdistillbalance>(),
        lua_state::luaL_LoggedReg<"game", "cookgobblegumrecipe",
                                  cookgobblegumrecipe>(),
        lua_state::luaL_LoggedReg<"game", "resetgobblegums", resetgobblegums>(),
        lua_state::luaL_LoggedReg<"game", "resetblackmarket",
                                  resetblackmarket>(),
        lua_state::luaL_LoggedReg<"game", "getblackmarketdrops",
                                  getblackmarketdrops>(),
        lua_state::luaL_LoggedReg<"game", "iscurrencymaxed", iscurrencymaxed>(),
        lua_state::luaL_LoggedReg<"game", "setcurrencymaxed",
                                  setcurrencymaxed>(),
        lua_state::luaL_LoggedReg<"game", "savestats", savestats>(),
        lua_state::luaL_LoggedReg<"game", "httpget",
                                  lua_state::unsafe_function<httpget>>(),
        lua_state::luaL_LoggedReg<"game", "httppost",
                                  lua_state::unsafe_function<httppost>>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopcatalog",
                                  getworkshopcatalog>(),
        lua_state::luaL_LoggedReg<"game", "requestworkshopcatalog",
                                  requestworkshopcatalog>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopcatalogstatus",
                                  getworkshopcatalogstatus>(),
        lua_state::luaL_LoggedReg<"game", "requestworkshoppreview",
                                  requestworkshoppreview>(),
        lua_state::luaL_LoggedReg<"game", "setworkshopcover",
                                  setworkshopcover>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopdownload",
                                  getworkshopdownload>(),
        lua_state::luaL_LoggedReg<"game", "queueworkshopitem",
                                  queueworkshopitem>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopqueue",
                                  getworkshopqueue>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopqueuegeneration",
                                  getworkshopqueuegeneration>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopqueueposition",
                                  getworkshopqueueposition>(),
        lua_state::luaL_LoggedReg<"game", "removeworkshopqueue",
                                  removeworkshopqueue>(),
        lua_state::luaL_LoggedReg<"game", "moveworkshopqueue",
                                  moveworkshopqueue>(),
        lua_state::luaL_LoggedReg<"game", "clearworkshopqueue",
                                  clearworkshopqueue>(),
        lua_state::luaL_LoggedReg<"game", "getworkshopinstalled",
                                  getworkshopinstalled>(),
        lua_state::luaL_LoggedReg<"game", "isworkshopinstalled",
                                  isworkshopinstalled>(),
        lua_state::luaL_LoggedReg<"game", "deleteworkshopitem",
                                  deleteworkshopitem>(),
        lua_state::luaL_LoggedReg<"game", "cancelworkshopdownload",
                                  cancelworkshopdownload>(),
        lua_state::luaL_LoggedReg<"game", "setDiscordPlayerScore",
                                  setDiscordPlayerScore>(),
        lua_state::luaL_LoggedReg<"game", "setDiscordEnemyScore",
                                  setDiscordEnemyScore>(),
        lua_state::luaL_LoggedReg<"game", "setDiscordRoundsPlayed",
                                  setDiscordRoundsPlayed>(),
        lua_state::luaL_LoggedReg<"game", "hotreloadcheck",
                                  lua_state::unsafe_function<hotreloadcheck>>(),
        lua_state::luaL_LoggedReg<"game", "hotreloadstart",
                                  lua_state::unsafe_function<hotreloadstart>>(),
        lua_state::luaL_LoggedReg<"game", "getappdatapath", getappdatapath>(),
        lua_state::luaL_LoggedReg<"game", "getclientoverridename",
                                  getclientoverridename>(),
        lua_state::luaL_LoggedReg<"game", "getclientoverridetag",
                                  getclientoverridetag>(),
        lua_state::luaL_LoggedReg<"game", "getrawservercount",
                                  getrawservercount>(),
        lua_state::luaL_LoggedReg<"game", "isserverlistrefreshing",
                                  isserverlistrefreshing>(),
        lua_state::luaL_LoggedReg<"game", "getrawserverinfo",
                                  getrawserverinfo>(),
        lua_state::luaL_LoggedReg<"game", "getvoiceinputdevices",
                                  getvoiceinputdevices>(),
        lua_state::luaL_LoggedReg<"game", "getgpupreference",
                                  getgpupreference>(),
        lua_state::luaL_LoggedReg<"game", "setgpupreference",
                                  setgpupreference>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("game", GameLibrary);

    if (game::is_client()) {
      scheduler::once(
          [] {
            game::register_dvar_string("voice_input_device", "",
                                       game::DVAR_ARCHIVE,
                                       "Microphone used for voice chat");
          },
          scheduler::main);
    }
  }
};
} // namespace game_lua

REGISTER_COMPONENT(game_lua::component)
