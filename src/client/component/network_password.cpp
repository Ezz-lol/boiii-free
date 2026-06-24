#include <cstdint>
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"

#include "network_password.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace network_password {
namespace {
const game::dvar_t *net_password_dvar = nullptr;
}

// FNV1a-64 hash
uint64_t hash_password(const std::string_view &password) {
  uint64_t hash = 14695981039346656037ULL;
  constexpr uint64_t prime = 1099511628211ULL;

  for (const char c : password) {
    hash ^= static_cast<uint64_t>(static_cast<unsigned char>(tolower(c)));
    hash *= prime;
  }

  return hash & 0x7FFFFFFFFFFFFFFFULL;
}

std::string get_password_hash_string() {
  // TODO: Replace FNV1a hash with a salted challenge-response scheme.
  if (!is_password_set()) {
    return "0";
  }

  const std::string_view password = game::get_dvar_string("net_password");
  const uint64_t hash = hash_password(password);
  return utils::string::va("%llu", hash);
}

bool is_password_set() {
  const std::string_view password = game::get_dvar_string("net_password");
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