#include <std_include.hpp>

#include "command.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"
#include "toast.hpp"

#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <utils/concurrency.hpp>
#include <utils/info_string.hpp>
#include <utils/string.hpp>

namespace quick_match {
namespace {
struct candidate {
  game::net::netadr_t address{};
  uint32_t ping{};
};

struct filter {
  game::eModes mode{};
  std::string gametype;
  std::optional<bool> hardcore;
};

struct search {
  filter wanted;
  int32_t party_size{};
  size_t pending{};
  bool mode_found{};
  std::optional<candidate> best;
};

struct watch {
  filter wanted;
  std::chrono::steady_clock::time_point armed_at;
  std::chrono::steady_clock::time_point next_search;
  bool entered{};
  bool ready{};
  bool notified{};
};

constexpr auto SEARCH_INTERVAL = 15s;
constexpr auto LOBBY_TIMEOUT = 30s;

utils::concurrency::container<std::optional<search>> current;
std::optional<watch> watching;
game::EngineDependentDvarMut quickmatch_active;

void stop() {
  watching.reset();
  quickmatch_active.set(false);
}

int32_t to_int(const std::string &value) { return std::atoi(value.c_str()); }

void finish() {
  std::optional<candidate> best;
  bool mode_found = false;
  current.access([&](std::optional<search> &s) {
    if (s) {
      best = s->best;
      mode_found = s->mode_found;
    }
    s.reset();
  });

  scheduler::once(
      [best, mode_found] {
        if (!watching || !watching->ready) {
          return;
        }
        if (best) {
          stop();
          party::connect(best->address);
          return;
        }
        if (!watching->notified) {
          watching->notified = true;
          toast::show("Find Match",
                      mode_found ? "All servers are full, waiting in the lobby"
                                 : "No server found, waiting in the lobby",
                      "t7_icon_connect_overlays");
        }
      },
      scheduler::main);
}

void handle_response(const bool success, const game::net::netadr_t &host,
                     const utils::info_string &info, const uint32_t ping) {
  bool done = false;
  current.access([&](std::optional<search> &s) {
    if (!s) {
      return;
    }
    // Servers drop bots to make room for players
    const int32_t players =
        to_int(info.get("clients")) - to_int(info.get("bots"));
    const int32_t free_slots = to_int(info.get("sv_maxclients")) - players;
    const bool mode_matches =
        success && info.get("dedicated") == "1" &&
        info.get("isPrivate") != "1" &&
        to_int(info.get("protocol")) == PROTOCOL &&
        to_int(info.get("playmode")) == static_cast<int32_t>(s->wanted.mode) &&
        (s->wanted.gametype.empty() ||
         utils::string::to_lower(info.get("gametype")) == s->wanted.gametype) &&
        (!s->wanted.hardcore ||
         (to_int(info.get("hc")) != 0) == *s->wanted.hardcore);
    s->mode_found |= mode_matches;
    if (mode_matches && free_slots >= s->party_size &&
        (!s->best || ping < s->best->ping)) {
      s->best = candidate{host, ping};
    }
    done = --s->pending == 0;
  });

  if (done) {
    finish();
  }
}

void search_servers(const filter &wanted) {
  bool searching = false;
  current.access([&](std::optional<search> &s) {
    searching = s.has_value();
    if (!searching) {
      s.emplace();
      s->wanted = wanted;
      s->party_size = std::max(1, game::lobby::LobbyHost_GetClientCount(
                                      game::lobby::LobbyType::PRIVATE,
                                      game::lobby::LobbyClientType::ALL));
    }
  });
  if (searching) {
    return;
  }

  server_list::request_servers(
      [](const bool success,
         const std::unordered_set<game::net::netadr_t> &servers) {
        if (!success || servers.empty()) {
          finish();
          return;
        }
        current.access([&](std::optional<search> &s) {
          if (s) {
            s->pending = servers.size();
          }
        });
        for (const game::net::netadr_t &server : servers) {
          party::query_server(server, handle_response);
        }
      });
}

const game::lobby::session::LobbySession *public_lobby() {
  const game::lobby::session::LobbySession *session =
      game::lobby::LobbyHostData_GetSession(game::lobby::LobbyType::GAME);
  if (!session ||
      session->active == game::lobby::session::SessionActive::INACTIVE ||
      session->mode != game::lobby::LobbyMode::PUBLIC ||
      !game::com::Com_IsRunningUILevel()) {
    return nullptr;
  }
  return session;
}

bool everyone_ready(const game::lobby::session::LobbySession &session) {
  bool any = false;
  for (size_t i = 0; i < static_cast<size_t>(game::lobby::MAX_PLAYERS); ++i) {
    if (!session.client[i].activeClient) {
      continue;
    }
    if (!session.lobbyData.lobbyGame.client[i].readyUp) {
      return false;
    }
    any = true;
  }
  return any;
}

void update() {
  if (!watching) {
    return;
  }
  const auto now = std::chrono::steady_clock::now();
  const game::lobby::session::LobbySession *session = public_lobby();
  if (!session) {
    if (watching->entered || now - watching->armed_at > LOBBY_TIMEOUT) {
      stop();
    }
    return;
  }
  watching->entered = true;

  const bool ready = everyone_ready(*session);
  if (ready != watching->ready) {
    watching->ready = ready;
    watching->notified = false;
    if (ready) {
      watching->next_search = now;
      toast::show("Find Match", "Looking for a server...",
                  "t7_icon_connect_overlays");
    }
  }
  if (ready && now >= watching->next_search) {
    watching->next_search = now + SEARCH_INTERVAL;
    search_servers(watching->wanted);
  }
}

// The gametype and hardcore come from the playlist picked for Find Match. "any"
// or leaving them out matches every server
void start(const command::params &params) {
  const game::eModes mode = game::com::Com_SessionMode_GetMode();
  if (mode != game::eModes::MULTIPLAYER && mode != game::eModes::ZOMBIES) {
    return;
  }

  watch w{};
  w.wanted.mode = mode;
  if (params.size() > 1 && params.get(1) != std::string_view("any")) {
    w.wanted.gametype = utils::string::to_lower(params.get(1));
  }
  if (params.size() > 2) {
    w.wanted.hardcore = to_int(params.get(2)) != 0;
  }
  w.armed_at = std::chrono::steady_clock::now();
  watching = std::move(w);
  quickmatch_active.set(true);

  toast::show("Find Match", "Ready up to look for a server",
              "t7_icon_connect_overlays");
}
} // namespace

struct component final : client_component {
  DEFINE_COMPONENT_NAME("quick_match");

  void post_unpack() override {
    quickmatch_active = game::register_dvar_bool(
        "quickmatch_active", false, game::dvarFlags_e::NONE,
        "Find Match is waiting in the public lobby for a server");
    command::add("quickmatch",
                 [](const command::params &params) { start(params); });
    scheduler::loop(update, scheduler::main, 1s);
  }
};
} // namespace quick_match

REGISTER_COMPONENT(quick_match::component)
