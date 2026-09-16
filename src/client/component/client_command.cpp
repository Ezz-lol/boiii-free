#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/utils.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "client_command.hpp"

namespace client_command {
namespace {
std::unordered_map<std::string, callback> handlers;
utils::hook::detour client_command_hook;

void invoke_original(const game::ClientNum_t client_num) {
  if (game::is_server()) {
    game::ClientCommand(client_num);
  } else {
    client_command_hook.invoke<void>(client_num);
  }
}

void client_command_stub(const game::ClientNum_t client_num) {
  game::level::gentity_t *ent = game::level::client_ent(client_num);

  if (ent == nullptr || ent->client == nullptr) {
    return;
  }

  const command::params_sv params;

  const auto command = utils::string::to_lower(params.get(0));
  if (const auto got = handlers.find(command); got != handlers.end()) {
    got->second(ent, params);
    return;
  }

  invoke_original(client_num);
}
} // namespace

void add(const std::string &name, const callback &cmd) {
  const std::string command = utils::string::to_lower(name);
  handlers[command] = cmd;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    if (game::is_server()) {
      utils::hook::call(0x14052F81B_g, client_command_stub);
    } else {
      client_command_hook.create(game::ClientCommand.get(),
                                 client_command_stub);
    }
  }
};
} // namespace client_command

REGISTER_COMPONENT(client_command::component)
