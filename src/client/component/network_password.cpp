#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>
#include <game/utils.hpp>

#include "network_password.hpp"
#include "game/impl/hash.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace network_password {
namespace {
game::EngineDependentDvar net_password_dvar;
}

// FNV1a-64 hash
uint64_t hash_password(const std::string_view &password) {

  return ::fnv1a64<14695981039346656037ULL, 1099511628211ULL>(password.data()) &
         0x7FFFFFFFFFFFFFFFULL;
}

std::string get_password_hash_string() {
  // TODO: Replace FNV1a hash with a salted challenge-response scheme.
  if (!is_password_set()) {
    return "0";
  }

  const std::string_view password = net_password_dvar.get_string().value_or("");
  const uint64_t hash = hash_password(password);
  return utils::string::va("%llu", hash);
}

bool is_password_set() {
  const std::string_view password = net_password_dvar.get_string().value_or("");
  return !password.empty();
}

struct component final : generic_component {
  void post_unpack() override {
    scheduler::once(
        [] {
          net_password_dvar = game::register_dvar_string(
              "net_password", "", game::DVAR_NONE,
              "Network password for private server isolation");
        },
        scheduler::pipeline::main);
  }
};
} // namespace network_password

REGISTER_COMPONENT(network_password::component)