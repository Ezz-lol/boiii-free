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
void *ClientCommand_original{};

bool ClientCommand_ExecuteRegistered(game::ClientNum_t client_num) {
  game::level::gentity_t *ent = game::level::client_ent(client_num);
  if (ent == nullptr || ent->client == nullptr) {
    return true;
  }

  const command::params_sv params;
  const auto got = handlers.find(utils::string::to_lower(params.get(0)));
  if (got == handlers.end()) {
    return false;
  }

  got->second(ent, params);
  return true;
}

void ClientCommand_stub(utils::hook::assembler &a) {
  const asmjit::Label original = a.get().new_label();
  a.get().push(rcx);
  a.get().sub(rsp, 0x20);
  a.get().mov(rax,
              reinterpret_cast<uintptr_t>(ClientCommand_ExecuteRegistered));
  a.get().call(rax);
  a.get().add(rsp, 0x20);
  a.get().pop(rcx);
  a.get().test(al, al);
  a.get().jz(original);
  a.get().ret();
  a.get().bind(original);
  a.get().mov(rax, reinterpret_cast<uintptr_t>(&ClientCommand_original));
  a.get().jmp(qword_ptr(rax));
}
} // namespace

void register_handler(std::string name, clientCommandHandler_t cmd) {
  const std::string command = utils::string::to_lower(std::move(name));
  handlers[command] = cmd;
}

class component final : public server_component {
  DEFINE_COMPONENT_NAME("client_command");

public:
  void post_unpack() override {
    ClientCommand_hook.create(game::ClientCommand.get(),
                              utils::hook::assemble(ClientCommand_stub));
    ClientCommand_original = ClientCommand_hook.get_original();
  }
};
} // namespace client_command

REGISTER_COMPONENT(client_command::component)
