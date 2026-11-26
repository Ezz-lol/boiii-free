#include <std_include.hpp>

#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"
#include "zombies/connected_players.hpp"
#include "zombies/builtin_snapshot_windows.hpp"
#include "zombies/coop_runtime.hpp"
#include "zombies/diagnostics.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>

namespace zombies::coop {
namespace {
std::atomic_bool connected_hook_installed{false};

void report_connected_hook(bool installed, const char* reason) noexcept {
  diagnostic_event(installed ? "connected_bots_installed" : "connected_bots_declined");
  printf("COOP_CONNECTED_BOTS|{\"version\":1,\"family\":\"ezz\",\"installed\":%s,\"reason\":\"%s\",\"scope\":\"dedicated_expanded_zombies_default_only\"}\n",
      installed ? "true" : "false", reason);
  fflush(stdout);
}

// BEGIN CONNECTED PLAYER DEFAULT SHIM
bool connected_player_default_stub(game::scr::scriptInstance_t instance, unsigned index,
                                   bool original_default) {
  // Context acquisition releases all coop locks before the engine getter,
  // whose validation can leave the VM through a nonlocal error path.
  const auto current = connected_players_context();
  const bool chosen_default = connected_players_policy::default_value(
      static_cast<int>(instance), index, original_default, current);
  return game::scr::Scr_GetBoolOptional(instance, index, chosen_default);
}
// END CONNECTED PLAYER DEFAULT SHIM
} // namespace

connected_players_policy::context connected_players_context() noexcept {
  connected_players_policy::context current{};
  current.hook_ready = connected_hook_installed.load(std::memory_order_acquire);
  if (!current.hook_ready) return current;
  current.dedicated = game::is_server();
  if (!current.dedicated) return current;
  current.opted_in = utils::flags::has_flag("experimental-zombies-coop");
  if (!current.opted_in) return current;
  current.zombies = game::com::Com_SessionMode_GetMode() == game::eModes::ZOMBIES;
  if (!current.zombies) return current;
  current.frontend = game::com::Com_IsRunningUILevel();
  if (current.frontend) return current;
  current.capacity = game::get_dvar_int("com_maxclients").value_or(0);
  if (current.capacity < 5 || current.capacity > 8) return current;
  current.map_armed = expanded_host();
  return current;
}

struct connected_players_component final : generic_component {
  DEFINE_COMPONENT_NAME("zombies/connected_players");
  void post_unpack() override {
    if (!game::is_server()) { report_connected_hook(false, "unsupported_client"); return; }
    if (!utils::flags::has_flag("experimental-zombies-coop")) {
      report_connected_hook(false, "experimental_disabled"); return;
    }
    builtin_snapshot::windows_reader reader;
    const auto captured = builtin_snapshot::capture(reader, game::get_base());
    if (!connected_players_policy::verified_body(captured)) {
      report_connected_hook(false, captured.valid ? "body_mismatch" : captured.reason);
      return;
    }
    try {
      // Only this verified CALL's default argument changes. The native ACTIVE
      // filter, explicit optional argument, count loop and Scr_AddInt remain.
      utils::hook::call(captured.action_address + connected_players_policy::optional_call_offset,
                        connected_player_default_stub);
      connected_hook_installed.store(true, std::memory_order_release);
      report_connected_hook(true, "verified_default_only");
    } catch (...) {
      report_connected_hook(false, "install_failed");
    }
  }
};
} // namespace zombies::coop

REGISTER_COMPONENT(zombies::coop::connected_players_component)
