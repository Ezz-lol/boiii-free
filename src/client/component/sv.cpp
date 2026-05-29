#include <cstdint>
#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "sv.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>

namespace sv {
namespace {
using EnterWorldTasks = std::vector<EnterWorldTask>;
utils::concurrency::container<EnterWorldTasks> sv_cliententerworld_tasks;

using RemoveTasks = std::vector<RemoveTask>;
utils::concurrency::container<RemoveTasks> sv_live_removeclient_tasks;

utils::hook::detour SV_ClientEnterWorld_hook;
void SV_ClientEnterWorld_stub(game::sv::client_s *client,
                              game::user::usercmd_t *cmd) {
  SV_ClientEnterWorld_hook.invoke(client, cmd);
  sv_cliententerworld_tasks.access([client, cmd](EnterWorldTasks &tasks) {
    for (const auto &func : tasks) {
      func(client, cmd);
    }
  });
}

utils::hook::detour SV_Live_RemoveClient_hook;
void SV_Live_RemoveClient_stub(game::sv::client_s *client, const char *reason) {
  sv_live_removeclient_tasks.access([client, reason](RemoveTasks &tasks) {
    for (const auto &func : tasks) {
      func(client, reason);
    }
  });
  SV_Live_RemoveClient_hook.invoke(client, reason);
}
} // namespace

void on_cliententerworld(const EnterWorldTask &callback) {
  sv_cliententerworld_tasks.access(
      [&callback](EnterWorldTasks &tasks) { tasks.emplace_back(callback); });
}
void on_removeclient(const RemoveTask &callback) {
  sv_live_removeclient_tasks.access(
      [&callback](RemoveTasks &tasks) { tasks.emplace_back(callback); });
}

bool valid_client_num(game::ClientNum_t c) {
  return static_cast<uint32_t>(c) >= game::lobby::MIN_PLAYERS &&
         static_cast<uint32_t>(c) < game::lobby::MAX_PLAYERS;
}

/*
  All `client_s`s are stored in svs.clients.
  Thus, index of a given client can be calculated by findings its displacement
  relative to `svs.clients` divided by size of `client_s`.

  The engine calculates index of a given client the same way.
*/
game::ClientNum_t get_client_num(game::sv::client_s *client) {
  if (game::valid_engine_ptr(client)) {
    if (game::is_client()) {
      const uint64_t svs_clients_addr =
          reinterpret_cast<uint64_t>(game::sv::svs_clients_cl.get());
      const uint64_t client_addr = reinterpret_cast<uint64_t>(client);
      const uint64_t client_displacement = client_addr - svs_clients_addr;
      if (client_displacement > 0) {
        const game::ClientNum_t client_num = static_cast<game::ClientNum_t>(
            client_displacement / sizeof(game::sv::client_s_cl));
        if (valid_client_num(client_num)) {
          return client_num;
        }
      }

    } else {
      const uint64_t svs_clients_addr =
          reinterpret_cast<uint64_t>(game::sv::svs_clients.get());
      const uint64_t client_addr = reinterpret_cast<uint64_t>(client);
      const uint64_t client_displacement = client_addr - svs_clients_addr;
      if (client_displacement > 0) {
        const game::ClientNum_t client_num = static_cast<game::ClientNum_t>(
            client_displacement / sizeof(game::sv::client_s));
        if (valid_client_num(client_num)) {
          return client_num;
        }
      }
    }
  }
  return game::INVALID_CLIENT_INDEX;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    SV_ClientEnterWorld_hook.create(game::sv::SV_ClientEnterWorld.get(),
                                    SV_ClientEnterWorld_stub);
    SV_Live_RemoveClient_hook.create(game::sv::SV_Live_RemoveClient.get(),
                                     SV_Live_RemoveClient_stub);
  }
};
} // namespace sv

REGISTER_COMPONENT(sv::component)