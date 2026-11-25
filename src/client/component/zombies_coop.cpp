#include <std_include.hpp>

#include "game/game.hpp"
#include "game/utils.hpp"
#include "loader/component_loader.hpp"
#include "zombies/coop_policy.hpp"
#include "zombies/coop_runtime.hpp"
#include "zombies/startup_gate.hpp"
#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies_lobby_vm.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <utils/flags.hpp>

namespace zombies::coop {
namespace {
std::mutex state_mutex;
startup_gate gate;
std::atomic_bool blocked{false};
int epoch = 0;
int applied_limit = 0;
bool reported = false;
bool map_armed = false;
std::string active_map;
std::chrono::steady_clock::time_point next_poll{};

const char *preflight_error(int configured, int requested) {
  if (configured < 1 || configured > player_limit)
    return "Invalid Zombies coop capacity. Select one through eight before "
           "hosting.";
  if (!requested)
    return "Invalid native Zombies lobby capacity or missing experimental opt-in. "
           "Use one through eight players and enable experimental coop before "
           "hosting an expanded lobby.";
  if (applied_limit && requested != applied_limit)
    return "Zombies coop capacity changed during a hosted session. Return to "
           "the frontend, or restart the dedicated process, to apply it.";
  if (requested <= stock_limit)
    return nullptr;
  if (game::is_client() && !client_lobby_hooks_ready())
    return "Expanded Zombies hosting requires matching lobby scripts loaded in the client Lobby VM. Check zombies_coop_lobby_vm diagnostics and install matching data.";
  // Map/mod ownership is established by the paired early schema and exact
  // stock callback comparisons in GSC. Official profiles and compatible custom
  // stock callbacks are accepted; unknown callback/gametype owners decline.
  return nullptr;
}

const char *poll_startup() {
  const std::lock_guard<std::mutex> lock(state_mutex);
  if (game::com::Com_IsRunningUILevel() ||
      game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES) {
    gate.reset();
    blocked.store(false, std::memory_order_release);
    applied_limit = 0;
    reported = false;
    map_armed = false;
    return nullptr;
  }
  if (applied_limit <= stock_limit || reported)
    return nullptr;
  if (!map_armed) {
    blocked.store(true, std::memory_order_release);
    return nullptr;
  }
  const auto now = std::chrono::steady_clock::now();
  if (now < next_poll)
    return nullptr;
  next_poll = now + std::chrono::milliseconds(100);
  if (schema_assets_failed()) {
    reported = true;
    blocked.store(true, std::memory_order_release);
    return "Paired Zombies GSC/CSC/UI schema failed. Use matching data and rehost; native checksums remain enforced.";
  }
  const auto fault = gate.observe(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch())
          .count(),
      game::server_running(),
      game::get_dvar_string("zm_coopStatus").value_or(""),
      game::get_dvar_int("zm_coopReadyEpoch").value_or(0),
      (game::get_dvar_string("zm_coopReadyMap").value_or("") == active_map &&
       game::get_dvar_string("mapname").value_or("") == active_map),
      game::get_dvar_int("zm_coopHeartbeat").value_or(0), schema_server_ready());
  blocked.store(!gate.ready(), std::memory_order_release);
  if (fault == startup_gate::fault::none)
    return nullptr;
  reported = true; // set before Com_Error's nonlocal exit
  blocked.store(true, std::memory_order_release);
  return "Zombies coop adapter did not validate this map or lost its lifecycle "
         "state. Expansion was stopped; check zm_coopStatus and use matching "
         "data.";
}
} // namespace

bool admission_ready() noexcept {
  return !blocked.load(std::memory_order_acquire);
}

bool expanded_host() noexcept {
  const std::lock_guard<std::mutex> lock(state_mutex);
  return applied_limit > stock_limit && map_armed;
}

void shutdown_map() noexcept {
  const std::lock_guard<std::mutex> lock(state_mutex);
  // Revoke before script memory is released. The next VM load must explicitly
  // arm a fresh map; old same-map readiness cannot reopen admission meanwhile.
  map_armed = false;
  blocked.store(applied_limit > stock_limit, std::memory_order_release);
  gate.reset();
  active_map.clear();
  // Capacity remains latched across rotations until returning to the frontend.
}

const char *begin_startup() {
  const std::lock_guard<std::mutex> lock(state_mutex);
  if (game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES)
    return nullptr;
  const int configured =
      game::get_dvar_int("zm_coopPlayerLimit").value_or(stock_limit);
  const int requested = resolve_session_limit(
      configured, game::get_dvar_int("com_maxclients").value_or(0),
      utils::flags::has_flag("experimental-zombies-coop"));
  blocked.store(true, std::memory_order_release);
  map_armed = false;
  // All temporary strings are destroyed before invoking engine longjmp error
  // paths.
  if (const char *error = preflight_error(configured, requested)) {
    reported = true;
    return error;
  }
  if (requested > stock_limit) {
    std::string schema_error;
    if (!prepare_schema_session(requested, std::string(game::get_dvar_string("mapname").value_or("")), schema_error)) {
      schema_asset_failure(schema_error);
      reported = true;
      return "Could not prepare the paired Zombies schema before script linking. Install matching companion data and rehost.";
    }
    schema_begin_server_vm();
  } else if (frozen_schema_capacity() >= 5 || schema_assets_failed()) {
    reset_schema_session();
  }
  applied_limit = requested;
  blocked.store(requested > stock_limit, std::memory_order_release);
  epoch = epoch == INT_MAX ? 1 : epoch + 1;
  active_map = std::string(game::get_dvar_string("mapname").value_or(""));
  reported = false;
  gate.arm(epoch, requested);
  map_armed = true;
  next_poll = {};
  game::Dvar_SetFromStringByName("zm_coopStatus", "initializing", true);
  (void)game::set_dvar_int("zm_coopHeartbeat", 0, false);
  (void)game::set_dvar_int("zm_coopStartupEpoch", epoch, false);
  (void)game::set_dvar_int("zm_coopAppliedLimit", requested, false);
  (void)game::set_dvar_int("zm_coopReadyEpoch", 0, false);
  game::Dvar_SetFromStringByName("zm_coopReadyMap", "", true);
  return nullptr;
}

void validate_startup() {
  if (const char *error = begin_startup()) {
    game::com::Com_Error(game::errorParm::DROP, "%s", error);
  }
}

void check_startup() {
  refresh_schema_session();
  // Called only after scheduler task locks have been released.
  if (const char *error = poll_startup()) {
    game::com::Com_Error(game::errorParm::DROP, "%s", error);
  }
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("zombies/coop");
  void post_unpack() override {
    (void)game::register_dvar_int(
        "zm_coopPlayerLimit", stock_limit, 1,
        utils::flags::has_flag("experimental-zombies-coop") ? player_limit
                                                            : stock_limit,
        game::dvarFlags_e::ARCHIVE,
        "Experimental Zombies capacity; change only before hosting. Default "
        "4.");
    (void)game::register_dvar_int("zm_coopHeartbeat", 0, 0, INT_MAX, 0u,
                                  "Coop lifecycle generation.");
    (void)game::register_dvar_int("zm_coopStartupEpoch", 0, 0, INT_MAX, 0u,
                                  "Coop lifecycle generation.");
    (void)game::register_dvar_int("zm_coopAppliedLimit", 4, 1, 8, 0u,
                                  "Applied coop session capacity.");
    (void)game::register_dvar_int("zm_coopReadyEpoch", 0, 0, INT_MAX, 0u,
                                  "Coop lifecycle generation.");
  }
};
} // namespace zombies::coop
REGISTER_COMPONENT(zombies::coop::component)
