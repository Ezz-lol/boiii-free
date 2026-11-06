#include <std_include.hpp>

#include "command.hpp"
#include "party.hpp"
#include "scheduler.hpp"
#include "server_list.hpp"
#include "toast.hpp"

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

struct search {
  game::eModes mode{};
  std::string gametype;
  std::optional<bool> hardcore;
  int32_t party_size{};
  size_t pending{};
  bool mode_found{};
  std::optional<candidate> best;
};

utils::concurrency::container<std::optional<search>> current;

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
        if (!best) {
          toast::error("Find Match",
                       mode_found ? "All servers are full" : "No server found");
          return;
        }
        party::connect(best->address);
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
        to_int(info.get("playmode")) == static_cast<int32_t>(s->mode) &&
        (s->gametype.empty() ||
         utils::string::to_lower(info.get("gametype")) == s->gametype) &&
        (!s->hardcore || (to_int(info.get("hc")) != 0) == *s->hardcore);
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

// The gametype and hardcore come from the playlist picked for Find Match. "any"
// or leaving them out matches every server
void start(const command::params &params) {
  const game::eModes mode = game::com::Com_SessionMode_GetMode();
  if (mode != game::eModes::MULTIPLAYER && mode != game::eModes::ZOMBIES) {
    return;
  }

  bool searching = false;
  current.access([&](std::optional<search> &s) {
    searching = s.has_value();
    if (!searching) {
      s.emplace();
      s->mode = mode;
      if (params.size() > 1 && params.get(1) != std::string_view("any")) {
        s->gametype = utils::string::to_lower(params.get(1));
      }
      if (params.size() > 2) {
        s->hardcore = to_int(params.get(2)) != 0;
      }
      s->party_size = std::max(1, game::lobby::LobbyHost_GetClientCount(
                                      game::lobby::LobbyType::PRIVATE,
                                      game::lobby::LobbyClientType::ALL));
    }
  });
  if (searching) {
    return;
  }

  toast::show("Find Match", "Looking for the best server...",
              "t7_icon_connect_overlays");

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
} // namespace

struct component final : client_component {
  DEFINE_COMPONENT_NAME("quick_match");

  void post_unpack() override {
    command::add("quickmatch",
                 [](const command::params &params) { start(params); });
  }
};
} // namespace quick_match

REGISTER_COMPONENT(quick_match::component)
