#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "command.hpp"
#include "console.hpp"
#include "network.hpp"
#include "scheduler.hpp"

#include <game/utils.hpp>

namespace rcon {
namespace {
game::EngineDependentDvar rcon_timeout;

typedef std::unordered_map<game::net::netadr_t, int> rateLimitMap_t;
static rateLimitMap_t rate_limit_map;

std::optional<std::string>
get_and_validate_rcon_command(const std::string &data) {
  const command::params params{data};

  if (params.size() > 1 &&
      params[0] == game::get_rcon_password().value_or("")) {
    return params.join(1);
  }

  return std::nullopt;
}

void rcon_executer(const game::net::netadr_t &target, const std::string &data) {
  const std::optional<std::string> command =
      get_and_validate_rcon_command(data);
  if (command) {
    std::string console_buffer{};

    console::scoped_interceptor _(
        [&console_buffer](const std::string_view &text) {
          console_buffer += text;
        });

    game::cmd::Cmd_ExecuteSingleCommand(0, game::CONTROLLER_INDEX_FIRST,
                                        command->data(), true);

    network::send(target, "print", console_buffer);
  }
}

bool rate_limit_check(const game::net::netadr_t &address, const int time) {
  const int32_t last_time = rate_limit_map[address];

  if (last_time != 0 && (time - last_time) < rcon_timeout.get_int()) {
    return false; // Flooding
  }

  rate_limit_map[address] = time;
  return true;
}

void rate_limit_cleanup(const int time) {
  for (rateLimitMap_t::iterator i = rate_limit_map.begin();
       i != rate_limit_map.end();) {
    // No longer at risk of flooding, remove
    if ((time - i->second) > rcon_timeout.get_int()) {
      i = rate_limit_map.erase(i);
    } else {
      ++i;
    }
  }
}

void rcon_handler(const game::net::netadr_t &target,
                  const network::data_view &data,
                  game::LocalClientNum_t clientNum) {
  const int32_t time = game::sys::Sys_Milliseconds();
  if (rate_limit_check(target, time)) {
    rate_limit_cleanup(time);

    const std::string str_data =
        std::string(reinterpret_cast<const char *>(data.data()), data.size());
    scheduler::once([target, s = str_data] { rcon_executer(target, s); },
                    scheduler::pipeline::main);
  }
}
} // namespace

constexpr int32_t RCON_TIMEOUT_MIN_VALUE = 100;
constexpr int32_t RCON_TIMEOUT_MAX_VALUE = 10'000;
constexpr int32_t RCON_TIMEOUT_DEFAULT_VALUE = 500;
struct component final : server_component {
  DEFINE_COMPONENT_NAME("rcon");

  void post_unpack() override {
    network::on("rcon", rcon_handler);
    rcon_timeout = game::register_dvar_int(
        "rcon_timeout", RCON_TIMEOUT_DEFAULT_VALUE, RCON_TIMEOUT_MIN_VALUE,
        RCON_TIMEOUT_MAX_VALUE, game::dvarFlags_e::NONE, "");
  }
};
} // namespace rcon

REGISTER_COMPONENT(rcon::component)
