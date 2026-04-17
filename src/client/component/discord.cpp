#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "scheduler.hpp"
#include "discord.hpp"
#include "party.hpp"

#include <discord_rpc.h>
#include <utils/string.hpp>

#include <ctime>
#include <unordered_map>

static __declspec(noinline) bool seh_dvar_string(const char *name, char *buf,
                                                 size_t sz) {
  __try {
    const auto *dvar = game::dvar::Dvar_FindVar(name);
    if (!dvar) {
      buf[0] = '\0';
      return true;
    }
    const char *val = game::dvar::Dvar_GetString(dvar);
    if (!val) {
      buf[0] = '\0';
      return true;
    }
    strncpy_s(buf, sz, val, _TRUNCATE);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    buf[0] = '\0';
    return false;
  }
}

static __declspec(noinline) bool seh_dvar_int(const char *name, int *out) {
  __try {
    const auto *dvar = game::dvar::Dvar_FindVar(name);
    if (!dvar) {
      *out = 0;
      return true;
    }
    *out = dvar->current.value.integer;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = 0;
    return false;
  }
}

static __declspec(noinline) bool seh_Com_IsInGame(bool *out) {
  __try {
    *out = game::com::Com_IsInGame();
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = false;
    return false;
  }
}

static __declspec(noinline) bool seh_Com_IsRunningUILevel(bool *out) {
  __try {
    *out = game::com::Com_IsRunningUILevel();
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = false;
    return false;
  }
}

static __declspec(noinline) bool seh_SessionMode_IsMode(game::eModes mode,
                                                        bool *out) {
  __try {
    *out = game::com::Com_SessionMode_IsMode(mode);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = false;
    return false;
  }
}

static __declspec(noinline) bool seh_get_client_count(int max_clients,
                                                      int *out) {
  __try {
    int count = 0;
    char name_buf[64];
    for (int i = 0; i < max_clients; ++i) {
      name_buf[0] = '\0';
      if (game::cl::CL_GetClientName(0, i, name_buf, sizeof(name_buf), false) &&
          name_buf[0] != '\0')
        ++count;
    }
    *out = count;
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    *out = 0;
    return false;
  }
}

namespace discord {
namespace {
constexpr auto DISCORD_APP_ID = "967371125573177474";

time_t start_time = 0;
time_t match_time = 0;
bool was_in_game = false;

std::atomic_int s_player_score{0};
std::atomic_int s_enemy_score{0};
std::atomic_int s_rounds_played{0};

static const std::unordered_map<std::string, const char *> map_names = {
    {"mp_combine", "Combine"},
    {"mp_biodome", "Aquarium"},
    {"mp_redwood", "Redwood"},
    {"mp_stronghold", "Stronghold"},
    {"mp_nuketown_x", "Nuk3town"},
    {"mp_apartments", "Evac"},
    {"mp_havoc", "Havoc"},
    {"mp_ethiopia", "Hunted"},
    {"mp_infection", "Infection"},
    {"mp_metro", "Metro"},
    {"mp_exodus", "Exodus"},
    {"mp_sector", "Breach"},
    {"mp_spire", "Spire"},
    {"mp_veiled", "Rift"},
    {"mp_crucible", "Verge"},
    {"mp_rise", "Rise"},
    {"mp_skyjacked", "Skyjacked"},
    {"mp_waterpark", "Splash"},
    {"mp_chinatown", "Knockout"},
    {"mp_kung_fu", "Empire"},
    {"mp_arena", "Gauntlet"},
    {"mp_ruins", "Ruins"},
    {"mp_rome", "Citadel"},
    {"mp_miniature", "Micro"},
    {"mp_banzai", "Berserk"},
    {"mp_platform", "Cryogen"},
    {"mp_conduit", "Rumble"},
    {"mp_aerospace2", "Outlaw"},
    {"zm_zod", "Shadows of Evil"},
    {"zm_factory", "The Giant"},
    {"zm_castle", "Der Eisendrache"},
    {"zm_island", "Zetsubou No Shima"},
    {"zm_stalingrad", "Gorod Krovi"},
    {"zm_genesis", "Revelations"},
    {"zm_prototype", "Nacht Der Untoten"},
    {"zm_asylum", "Verruckt"},
    {"zm_sumpf", "Shi No Numa"},
    {"zm_theater", "Kino Der Toten"},
    {"zm_cosmodrome", "Ascension"},
    {"zm_temple", "Shangri-La"},
    {"zm_moon", "Moon"},
    {"zm_tomb", "Origins"},
};

static const std::unordered_map<std::string, const char *> gametype_names = {
    {"tdm", "Team Deathmatch"},    {"dm", "Free-For-All"},
    {"ffa", "Free-For-All"},       {"dom", "Domination"},
    {"sd", "Search & Destroy"},    {"hp", "Hardpoint"},
    {"ctf", "Capture The Flag"},   {"kc", "Kill Confirmed"},
    {"conf", "Kill Confirmed"},    {"gun", "Gun Game"},
    {"sas", "Safeguard"},          {"snipe", "One Shot"},
    {"oic", "One in the Chamber"}, {"sharp", "Sharpshooter"},
    {"prop", "Prop Hunt"},         {"ball", "Uplink"},
    {"infect", "Infected"},        {"dem", "Demolition"},
    {"clean", "Search & Rescue"},  {"zom", "Zombies"},
    {"zclassic", "Zombies"},
};

const char *get_mode_name(bool is_mp, bool is_zm, bool is_cp) {
  if (is_mp)
    return "Multiplayer";
  if (is_zm)
    return "Zombies";
  if (is_cp)
    return "Campaign";
  return "Playing";
}

std::string get_display_map(const std::string &raw, bool is_mp, bool is_zm,
                            bool is_cp) {
  if (const auto it = map_names.find(raw); it != map_names.end())
    return it->second;
  if (raw.empty())
    return get_mode_name(is_mp, is_zm, is_cp);
  return raw;
}

std::string get_display_gametype(const std::string &raw) {
  if (const auto it = gametype_names.find(raw); it != gametype_names.end())
    return it->second;
  return raw.empty() ? "" : raw;
}

std::string strip_colors(const std::string &src) {
  std::string out;
  out.reserve(src.size());
  for (size_t i = 0; i < src.size(); ++i) {
    if (src[i] == '^' && i + 1 < src.size()) {
      char c = src[i + 1];
      if ((c >= '0' && c <= '9') || c == ':' || c == ';') {
        ++i;
        continue;
      }
    }
    out += src[i];
  }
  return out;
}

std::string truncate(const std::string &s, size_t max_len) {
  if (s.size() <= max_len)
    return s;
  return s.substr(0, max_len - 3) + "...";
}

std::string safe_dvar_string(const char *name) {
  char buf[128] = {};
  seh_dvar_string(name, buf, sizeof(buf));
  return buf;
}

void update_discord() {
  try {
    DiscordRichPresence dp{};
    ZeroMemory(&dp, sizeof(dp));
    dp.instance = 1;
    dp.button1_label = "Join our Discord \xF0\x9F\x91\x8D";
    dp.button1_url = "https://discord.gg/ezz";
    dp.button2_label = "Ezz Forum \xF0\x9F\xA4\x96";
    dp.button2_url = "https://forum.ezz.lol";

    bool in_game = false;
    seh_Com_IsInGame(&in_game);

    if (!in_game) {
      if (was_in_game) {
        was_in_game = false;
        s_player_score = 0;
        s_enemy_score = 0;
        s_rounds_played = 0;
        party::clear_server_info();
      }

      bool ui_level = false;
      seh_Com_IsRunningUILevel(&ui_level);

      dp.startTimestamp = start_time;
      dp.details = "BO3 via Ezz";
      dp.state = ui_level ? "Main Menu" : "Loading...";
      dp.largeImageKey = "logo";
      dp.largeImageText = "Playing BO3 via Ezz!";
      dp.smallImageKey = "sexy";
      Discord_UpdatePresence(&dp);
      return;
    }

    if (!was_in_game) {
      match_time = time(nullptr);
      was_in_game = true;
      s_player_score = 0;
      s_enemy_score = 0;
      s_rounds_played = 0;
    }

    dp.startTimestamp = match_time;

    bool is_mp = false, is_zm = false, is_cp = false;
    seh_SessionMode_IsMode(game::MODE_MULTIPLAYER, &is_mp);
    seh_SessionMode_IsMode(game::MODE_ZOMBIES, &is_zm);
    seh_SessionMode_IsMode(game::MODE_CAMPAIGN, &is_cp);

    auto mapname = safe_dvar_string("mapname");
    if (mapname == "core_frontend")
      mapname.clear();
    const auto display_map = get_display_map(mapname, is_mp, is_zm, is_cp);

    auto gametype = safe_dvar_string("g_gametype");
    if (gametype.empty() || gametype == "frontend")
      gametype = safe_dvar_string("ui_gametype");
    const auto display_type = get_display_gametype(gametype);

    std::string server_name;
    try {
      server_name = party::get_server_hostname();
    } catch (...) {
    }
    if (!server_name.empty())
      server_name = truncate(strip_colors(server_name), 32);

    const bool is_private = server_name.empty();

    std::string details;
    std::string state;

    if (is_cp) {
      details = is_private ? display_map : server_name;
      state = is_private ? "" : display_map;
    } else if (is_mp) {
      int ps = s_player_score.load();
      int es = s_enemy_score.load();
      const std::string score_str =
          std::to_string(ps) + " - " + std::to_string(es);
      const std::string map_info = !mapname.empty()
                                       ? (display_type + " on " + display_map)
                                       : display_type;

      if (is_private) {
        details = map_info;
        state = score_str;
      } else {
        details = server_name;
        state = map_info + " | " + score_str;
      }
    } else {
      int round = s_rounds_played.load();
      const std::string round_str =
          round > 0 ? ("Round " + std::to_string(round)) : "";

      if (is_private) {
        details = std::string(get_mode_name(is_mp, is_zm, is_cp)) + " on " +
                  display_map;
        state = round_str;
      } else {
        details = server_name;
        state = round > 0 ? (display_map + " | " + round_str) : display_map;
      }
    }

    dp.details = details.c_str();
    dp.state = state.c_str();

    const bool known_map = map_names.count(mapname) > 0;
    dp.largeImageKey = known_map ? mapname.c_str() : "logo";

    std::string large_text =
        std::string(get_mode_name(is_mp, is_zm, is_cp)) + " - " + display_map;
    dp.largeImageText = large_text.c_str();
    dp.smallImageKey = "logo";
    dp.smallImageText = "Playing BO3 via Ezz!";

    int max_clients = 0;
    try {
      max_clients = party::get_server_max_clients();
    } catch (...) {
    }
    if (max_clients <= 0)
      seh_dvar_int("com_maxclients", &max_clients);

    int party_size = 1;
    if (max_clients > 0)
      seh_get_client_count(max_clients, &party_size);
    if (party_size < 1)
      party_size = 1;

    if (max_clients > 0) {
      dp.partySize = party_size;
      dp.partyMax = max_clients;
    }

    Discord_UpdatePresence(&dp);
  } catch (...) {
  }
}

void ready(const DiscordUser *request) {
  SetEnvironmentVariableA("discord_user", request->userId);
  printf("Discord: Ready: %s - %s\n", request->userId, request->username);
}

void errored(const int error_code, const char *message) {
  printf("Discord: Error (%i): %s\n", error_code, message);
}
} // namespace

void set_player_score(const int score) { s_player_score = score; }
void set_enemy_score(const int score) { s_enemy_score = score; }
void set_rounds_played(const int round) { s_rounds_played = round; }

class component final : public client_component {
public:
  void post_load() override {
    start_time = time(nullptr);

    DiscordEventHandlers handlers{};
    ZeroMemory(&handlers, sizeof(handlers));
    handlers.ready = ready;
    handlers.errored = errored;
    handlers.disconnected = errored;

    Discord_Initialize(DISCORD_APP_ID, &handlers, 1, nullptr);

    scheduler::loop(Discord_RunCallbacks, scheduler::pipeline::async, 1s);
    scheduler::loop(update_discord, scheduler::pipeline::main, 5s);
  }

  void pre_destroy() override { Discord_Shutdown(); }
};
} // namespace discord

REGISTER_COMPONENT(discord::component)