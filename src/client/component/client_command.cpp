#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/utils.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "client_command.hpp"

namespace client_command {
namespace {
std::unordered_map<std::string, clientCommandHandler_t> handlers;

utils::hook::detour ClientCommand_hook;
void ClientCommand_ExecuteRegistered(game::ClientNum_t client_num) {
  game::level::gentity_t *ent = game::level::client_ent(client_num);

  if (ent && ent->client) {
    const command::params_sv params;
    const std::string command = utils::string::to_lower(params.get(0));
    const std::unordered_map<std::string, clientCommandHandler_t>::iterator
        got = handlers.find(command);

    if (got == handlers.end()) {
      ClientCommand_hook.invoke(client_num);
    } else {
      got->second(ent, params);
    }
  }
}
} // namespace

void register_handler(std::string name, clientCommandHandler_t cmd) {
  const std::string command = utils::string::to_lower(name);
  handlers[command] = cmd;
}

class component final : public generic_component {
#ifndef NDEBUG
  std::string name() override { return "client_command"; }
#endif

public:
  void post_unpack() override {
    ClientCommand_hook.create(game::ClientCommand,
                              ClientCommand_ExecuteRegistered);
  }
};
} // namespace client_command

REGISTER_COMPONENT(client_command::component)
