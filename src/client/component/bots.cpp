#include <std_include.hpp>

#include <game/utils.hpp>

#include <component/bots/spawn_policy.hpp>
#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/getinfo.hpp>
#include <component/scheduler.hpp>
#include <component/zombies/companion.hpp>
#include <component/zombies/coop_runtime.hpp>
#include <component/zombies/diagnostics.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>

#include <str.hpp>

namespace bots {
namespace {
constexpr const char BOT_CONNECT_PACKET_INFOSTRING_FMT[] =
    "connect "
    "\"\\invited\\1\\cg_predictItems\\1"
    "\\cl_anonymous\\0\\color\\4\\head\\default"
    "\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s"
    "\\clanAbbrev\\%s\\xuid\\%s\\xnaddr\\%s\\natType\\2\\protocol"
    "\\%d\\netfieldchk\\%d\\sessionmode\\%s\\qport\\%d\"";
constexpr char DEFAULT_BOT_CLAN_ABBREV[] = "3arc";

static_assert(sizeof(game::playerName_t) == policy::player_name_bytes);
static_assert(sizeof(game::clanAbbrev_t) == policy::clan_abbrev_bytes);

struct BotName {
  game::playerName_t name;
  game::clanAbbrev_t clan_abbrev;
};

std::vector<BotName> load_bots_names() {
  constexpr const BotName DEFAULT_BOT_NAMES[] = {
      {.name = "Skwll", .clan_abbrev = "DEV"},
      {.name = "amkillam", .clan_abbrev = "DEV"},
      {.name = "Frozedy", .clan_abbrev = "DEV"},
      {.name = "Level", .clan_abbrev = "DEV"},

      {.name = "Ava", .clan_abbrev = "WIFE"},

      {.name = "Solar", .clan_abbrev = "Ezz"},
      {.name = "Catsby", .clan_abbrev = "Ezz"},
      {.name = "Hellcat", .clan_abbrev = "Ezz"},
      {.name = "Zyrow", .clan_abbrev = "Ezz"},
      {.name = "ZACK", .clan_abbrev = "Ezz"},
      {.name = "Aranella", .clan_abbrev = "Ezz"},
      {.name = "Clyde", .clan_abbrev = "Ezz"},
      {.name = "Misty", .clan_abbrev = "Ezz"},
      {.name = "TheBot", .clan_abbrev = "Ezz"},
      {.name = "Alenski", .clan_abbrev = "Ezz"},
      {.name = "Boiii", .clan_abbrev = "Ezz"},
      {.name = "Sabino", .clan_abbrev = "Ezz"},
      {.name = "KST", .clan_abbrev = "Ezz"},
      {.name = "Faisal", .clan_abbrev = "Ezz"},

      {.name = "Richtofen", .clan_abbrev = "ZMB"},
      {.name = "Dempsy", .clan_abbrev = "ZMB"},
      {.name = "Samantha", .clan_abbrev = "ZMB"},
      {.name = "Takeo", .clan_abbrev = "ZMB"},
      {.name = "ShadowMan", .clan_abbrev = "ZMB"},

      {.name = "Aimbot.exe", .clan_abbrev = "EXE"},
      {.name = "Wallhack.exe", .clan_abbrev = "EXE"},
      {.name = "NotAScammer.exe", .clan_abbrev = "EXE"},

      {.name = "ThunderCockKiller12", .clan_abbrev = "TF"},

      {.name = "1stPlaceBtw", .clan_abbrev = "YUH"},

      {.name = "NoIamFirstPlace", .clan_abbrev = "FR"},

      {.name = "IDied", .clan_abbrev = "LOL"},

      {.name = "TheLegend27", .clan_abbrev = "GOW"}};

  std::string buffer;
  std::vector<BotName> bot_names;
  if (utils::io::read_file(utils::nt::library{}.get_folder() / "boiii/bots.txt",
                           &buffer) &&
      !buffer.empty()) {
    std::vector<std::string> data = utils::string::split(buffer, '\n');
    for (std::string &entry : data) {
      utils::string::replace(entry, "\r", "");
      utils::string::trim(entry);

      if (!entry.empty()) {
        std::string clan_abbrev;
        // Check if there is a clan tag
        const size_t pos = entry.find(',');
        if (pos != std::string::npos) {
          // Only start copying over from non-null characters (otherwise it can
          // be
          // "<=")
          if ((pos + 1) < entry.size()) {
            clan_abbrev = entry.substr(pos + 1);
          }

          entry = entry.substr(0, pos);
        }

        BotName name;
        const auto safe_name =
            policy::userinfo_component(entry, sizeof(name.name));
        const auto safe_clan =
            policy::userinfo_component(clan_abbrev, sizeof(name.clan_abbrev));
        if (safe_name.empty())
          continue;
        strscpy(name.name, safe_name);
        strscpy(name.clan_abbrev, safe_clan);

        bot_names.emplace_back(name);
      }
    }
  }

  return bot_names.empty() ? std::vector(std::begin(DEFAULT_BOT_NAMES),
                                         std::end(DEFAULT_BOT_NAMES))
                           : bot_names;
}

const std::vector<BotName> &get_bot_names() {
  static const std::vector<BotName> bot_names = [] {
    std::vector<BotName> names = load_bots_names();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::ranges::shuffle(names, gen);
    return names;
  }();

  return bot_names;
}

const char *get_bot_name() {
  static std::atomic<size_t> current = 0;
  const std::vector<BotName> &names = get_bot_names();

  current.store((current.load(std::memory_order_acquire) + 1) % names.size(),
                std::memory_order_release);
  const char *result = names.at(current).name;
  game::trace(
      "[Bot][get_bot_name] returning result \"{}\" for current bot index {}",
      result, current.load(std::memory_order_acquire));
  return result;
}
const char *find_clan_name(const std::string &needle) {
  for (const BotName &entry : get_bot_names()) {
    if (entry.name == needle) {
      return entry.clan_abbrev;
    }
  }

  return DEFAULT_BOT_CLAN_ABBREV;
}

int format_bot_string(char *buffer, [[maybe_unused]] const char *format,
                      const char *name, const char *xuid, const char *xnaddr,
                      int protocol, int net_field_chk, const char *session_mode,
                      int qport) {

  game::trace("[Bot][format_bot_string] Called with name: \"{}\", xuid: "
              "\"{}\", xnaddr: \"{}\", protocol: {}, net_field_chk: {}, "
              "session_mode: \"{}\", qport: {}",
              name, xuid, xnaddr, protocol, net_field_chk, session_mode, qport);
  return sprintf_s(buffer, 0x400, BOT_CONNECT_PACKET_INFOSTRING_FMT, name,
                   find_clan_name(name), xuid, xnaddr, protocol, net_field_chk,
                   session_mode, qport);
}
std::atomic<std::uint64_t> spawn_epoch{1};

void invalidate_spawn_batches() {
  spawn_epoch.fetch_add(1, std::memory_order_acq_rel);
}

template <typename Client>
std::optional<int> occupied_slots(const Client *clients, int capacity) {
  if (!clients)
    return std::nullopt;
  int occupied = 0;
  for (int slot = 0; slot < capacity; ++slot)
    if (static_cast<int>(clients[slot].state) != 0)
      ++occupied;
  return occupied;
}

std::optional<policy::session_snapshot>
read_spawn_session(bool inspect_occupancy) {
  if (!getinfo::is_host())
    return std::nullopt;
  const auto epoch = spawn_epoch.load(std::memory_order_acquire);
  const int capacity = static_cast<int>(+game::get_com_maxclients());
  if (capacity < 1 || capacity > policy::maximum_slots)
    return std::nullopt;
  policy::session_snapshot current;
  current.epoch = epoch;
  current.mode = static_cast<int>(game::com::Com_SessionMode_GetMode());
  current.capacity = capacity;
  current.running = true;
  current.zombies = current.mode == static_cast<int>(game::eModes::ZOMBIES);
  current.map = std::string(game::get_mapname().value_or(""));
  if (current.zombies && capacity > 4) {
    current.schema_capacity = zombies::coop::frozen_schema_capacity();
    current.schema_ready = zombies::coop::schema_server_ready() &&
                           zombies::coop::admission_ready() &&
                           zombies::coop::frozen_schema_map() == current.map;
  }
  // Commands capture identity only. Native client arrays are inspected
  // on the server pipeline immediately before each allocation.
  if (inspect_occupancy) {
    const auto occupied =
        game::is_server() ? occupied_slots(*game::sv::svs_clients, capacity)
                          : occupied_slots(*game::sv::svs_clients_cl, capacity);
    if (!occupied)
      return std::nullopt;
    current.occupied = *occupied;
  }
  if (epoch != spawn_epoch.load(std::memory_order_acquire) ||
      !policy::valid(current))
    return std::nullopt;
  return current;
}

std::string roster_map_token(std::string_view map) {
  std::string result;
  for (std::size_t i = 0; i < map.size() && i < 64; ++i) {
    const auto c = map[i];
    result += (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                      (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'
                  ? c
                  : '_';
  }
  return result;
}

template <typename Client>
bool append_roster_slots(std::string &json, const Client *clients,
                         int capacity) {
  if (!clients)
    return false;
  bool first = true;
  for (int slot = 0; slot < capacity; ++slot) {
    const auto &client = clients[slot];
    if (static_cast<int>(client.state) == 0)
      continue;
    if (!first)
      json += ',';
    first = false;
    json += "{\"slot\":" + std::to_string(slot) + ",\"test\":";
    json += client.bIsTestClient ? "true" : "false";
    json +=
        ",\"state\":" + std::to_string(static_cast<int>(client.state)) + "}";
  }
  return true;
}

void publish_bot_roster(const policy::session_snapshot &requested) {
  const auto current = read_spawn_session(false);
  if (!current || !policy::same_session(requested, *current)) {
    printf("COOP_BOT_ROSTER|{\"version\":1,\"error\":\"stale_session\"}\n");
    fflush(stdout);
    return;
  }
  std::string json =
      "{\"version\":1,\"generation\":" + std::to_string(current->epoch);
  json += ",\"map\":\"" + roster_map_token(current->map) +
          "\",\"capacity\":" + std::to_string(current->capacity);
  json += ",\"slots\":[";
  const bool valid_slots =
      game::is_server()
          ? append_roster_slots(json, *game::sv::svs_clients, current->capacity)
          : append_roster_slots(json, *game::sv::svs_clients_cl,
                                current->capacity);
  json += "]}";
  const auto after = read_spawn_session(false);
  if (valid_slots && after && policy::same_session(requested, *after))
    printf("COOP_BOT_ROSTER|%s\n", json.c_str());
  else
    printf("COOP_BOT_ROSTER|{\"version\":1,\"error\":\"stale_session\"}\n");
  fflush(stdout);
}

bool drop_current_test_client(const policy::session_snapshot &requested,
                              int slot, std::uint64_t generation) {
  const auto current = read_spawn_session(false);
  if (!current || slot < 0 || slot >= current->capacity ||
      !policy::same_session(requested, *current))
    return false;
  game::sv::client_s *target = nullptr;
  if (game::is_server()) {
    auto *clients = *game::sv::svs_clients;
    if (clients)
      target = &clients[slot];
  } else {
    auto *clients = *game::sv::svs_clients_cl;
    if (clients)
      target = &clients[slot];
  }
  if (!target || !policy::may_drop_test_client(
                     requested, *current, generation, slot,
                     static_cast<int>(target->state) >=
                         static_cast<int>(game::net::clientState_t::CONNECTED),
                     target->bIsTestClient))
    return false;
  game::sv::SV_DropClient(target, "Coop test client removal", true, true);
  return true;
}

} // namespace

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("bots");

  void post_unpack() override {
    utils::hook::jump(game::select(0x141653B90, 0x1402732E0), get_bot_name);
    utils::hook::call(game::select(0x1421EC547, 0x14052E53A),
                      format_bot_string);

    if (game::is_client()) {
      utils::hook::jump(game::select(0x1416542A0, 0x0),
                        get_bot_name); // SV_ZombieNameRandom
    }

    game_event::on_g_init_game(invalidate_spawn_batches);
    game_event::on_g_shutdown_game(invalidate_spawn_batches);
    command::add("spawnBot", [](const command::params &params) {
      if (params.size() > 2) {
        printf("spawnBot: use a count from 1 to 18, or all.\n");
        return;
      }
      const auto count = policy::parse_count(
          params.size() > 1 ? std::optional<std::string_view>{params[1]}
                            : std::nullopt);
      if (!count) {
        printf("spawnBot: use a count from 1 to 18, or all.\n");
        return;
      }
      const auto requested = read_spawn_session(false);
      if (!requested)
        return;
      zombies::coop::diagnostic_event("bot_request", *count,
                                      requested->capacity);
      scheduler::once(
          [requested = *requested, count = *count] {
            const int spawned = policy::spawn_batch(
                requested, count, [] { return read_spawn_session(true); },
                [] {
                  const bool added = game::sv::SV_AddTestClient() != nullptr;
                  zombies::coop::diagnostic_event("bot_added", added ? 1 : 0);
                  return added;
                });
            zombies::coop::diagnostic_event("bot_batch", spawned, count);
            printf("spawnBot: added %d bot(s).\n", spawned);
          },
          scheduler::pipeline::server);
    });
    command::add("coopBotRoster", [](const command::params &params) {
      const auto requested =
          params.size() == 1 ? read_spawn_session(false) : std::nullopt;
      if (!requested) {
        printf("COOP_BOT_ROSTER|{\"version\":1,\"error\":\"session_not_ready\"}"
               "\n");
        fflush(stdout);
        return;
      }
      scheduler::once(
          [requested = *requested] { publish_bot_roster(requested); },
          scheduler::pipeline::server);
    });
    command::add("coopKickTestClient", [](const command::params &params) {
      const auto slot = params.size() == 3
                            ? policy::parse_unsigned(params[1], 17)
                            : std::nullopt;
      const auto generation =
          params.size() == 3 ? policy::parse_unsigned(params[2], UINT64_MAX)
                             : std::nullopt;
      const auto requested = read_spawn_session(false);
      if (!slot || !generation || !requested || *generation == 0 ||
          *generation != requested->epoch) {
        printf("COOP_BOT_KICK|{\"error\":\"invalid_or_stale_request\","
               "\"dropped\":false}\n");
        fflush(stdout);
        return;
      }
      scheduler::once(
          [requested = *requested, slot = static_cast<int>(*slot),
           generation = *generation] {
            const bool dropped =
                drop_current_test_client(requested, slot, generation);
            zombies::coop::diagnostic_event("bot_kick", slot, dropped ? 1 : 0);
            printf("COOP_BOT_KICK|{\"slot\":%d,\"generation\":%llu,\"dropped\":"
                   "%s}\n",
                   slot, static_cast<unsigned long long>(generation),
                   dropped ? "true" : "false");
            fflush(stdout);
          },
          scheduler::pipeline::server);
    });
  }
};
} // namespace bots

REGISTER_COMPONENT(bots::component)
