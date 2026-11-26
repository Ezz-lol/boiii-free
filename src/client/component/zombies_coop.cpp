#include <std_include.hpp>

#include "game/game.hpp"
#include "game/utils.hpp"
#include "loader/component_loader.hpp"
#include "zombies/coop_policy.hpp"
#include "zombies/coop_runtime.hpp"
#include "zombies/startup_gate.hpp"
#include "zombies/companion.hpp"
#include "zombies/common_helper_native_context.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies_lobby_vm.hpp"
#include "scheduler.hpp"
#include <atomic>
#include <chrono>
#include <mutex>
#include <utils/flags.hpp>

namespace zombies::coop {
namespace {
std::mutex state_mutex;
startup_gate gate;
std::atomic_bool blocked{false};
ending_policy::admission_lifecycle admission_state;
common_helper::native_publication common_native_context;
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
    return "Invalid native Zombies lobby capacity or missing experimental "
           "opt-in. "
           "Use one through eight players and enable experimental coop before "
           "hosting an expanded lobby.";
  if (applied_limit && requested != applied_limit)
    return "Zombies coop capacity changed during a hosted session. Return to "
           "the frontend, or restart the dedicated process, to apply it.";
  if (requested <= stock_limit)
    return nullptr;
  if (game::is_client() && !client_lobby_hooks_ready())
    return "Expanded Zombies hosting requires matching lobby scripts loaded in "
           "the client Lobby VM. Check zombies_coop_lobby_vm diagnostics and "
           "install matching data.";
  // Map/mod ownership is established by the paired early schema and exact
  // stock callback comparisons in GSC. Official profiles and compatible custom
  // stock callbacks are accepted; unknown callback/gametype owners decline.
  return nullptr;
}

const char *poll_startup() {
  const std::lock_guard<std::mutex> lock(state_mutex);
  if (game::com::Com_IsRunningUILevel() ||
      game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES) {
    common_native_context.invalidate();
    gate.reset();
    admission_state.reset();
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
  // Readiness sampling can itself latch a just-published terminal receipt.
  // Reuse this benign handler before the throttle AND after collecting the
  // complete observation, before startup_gate can classify it as a fault.
  const auto close_ending = [&] {
    if (schema_assets_failed() || !ending_session_current())
      return false;
    const bool already_ending = gate.ending();
    if (!gate.close_for_ending(epoch, true, ending_policy::receipt_revision))
      return false;
    blocked.store(true, std::memory_order_release);
    if (!already_ending)
      diagnostic_event("session_ending", epoch);
    return true;
  };
  if (close_ending())
    return nullptr;
  const auto now = std::chrono::steady_clock::now();
  if (now < next_poll)
    return nullptr;
  next_poll = now + std::chrono::milliseconds(100);
  if (schema_assets_failed()) {
    reported = true;
    blocked.store(true, std::memory_order_release);
    return "Paired Zombies GSC/CSC/UI schema failed. Use matching data and "
           "rehost; native checksums remain enforced.";
  }
  const bool running = game::server_running();
  const std::string status(game::get_dvar_string("zm_coopStatus").value_or(""));
  const int ready_epoch = game::get_dvar_int("zm_coopReadyEpoch").value_or(0);
  const bool same_map =
      game::get_dvar_string("zm_coopReadyMap").value_or("") == active_map &&
      game::get_dvar_string("mapname").value_or("") == active_map;
  const int heartbeat = game::get_dvar_int("zm_coopHeartbeat").value_or(0);
  const bool schema_ready = schema_server_ready();
  if (close_ending())
    return nullptr;
  const auto fault = gate.observe(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          now.time_since_epoch())
          .count(),
      running, status, ready_epoch, same_map, heartbeat, schema_ready);
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

int active_startup_epoch() noexcept { return admission_state.epoch(); }
bool session_ending() noexcept { return admission_state.ending(); }
bool mark_session_ending(int expected_epoch) noexcept {
  return admission_state.finish(expected_epoch);
}

bool admission_ready() noexcept {
  return !blocked.load(std::memory_order_acquire) &&
         !admission_state.ending() && common_helper::native_admission_current();
}

namespace common_helper {
void invalidate_native_session() noexcept {
  common_native_context.invalidate();
}
native_ticket capture_native_session() noexcept {
  return common_native_context.capture();
}
bool native_session_owned(const native_ticket &saved) noexcept {
  return common_native_context.owned(saved, active_startup_epoch(),
                                     frozen_schema_capacity());
}
bool native_session_current(const native_ticket &saved) noexcept {
  return common_native_context.provenance_current(saved, active_startup_epoch(),
                                                  frozen_schema_capacity(),
                                                  schema_assets_failed());
}
bool native_admission_current() noexcept {
  return frozen_schema_capacity() < 5 ||
         (!session_ending() &&
          native_session_current(capture_native_session()));
}
bool revoke_native_session(const native_ticket &saved) noexcept {
  return saved && saved->epoch == active_startup_epoch() &&
         saved->capacity == frozen_schema_capacity() &&
         common_native_context.revoke(saved);
}
} // namespace common_helper

bool expanded_host() noexcept {
  const std::lock_guard<std::mutex> lock(state_mutex);
  return applied_limit > stock_limit && map_armed;
}

void shutdown_map() noexcept {
  const std::lock_guard<std::mutex> lock(state_mutex);
  common_native_context.invalidate();
  // Revoke before script memory is released. The next VM load must explicitly
  // arm a fresh map; old same-map readiness cannot reopen admission meanwhile.
  map_armed = false;
  blocked.store(applied_limit > stock_limit, std::memory_order_release);
  gate.reset();
  admission_state.reset();
  active_map.clear();
  // Capacity remains latched across rotations until returning to the frontend.
}

const char *begin_startup() {
  const std::lock_guard<std::mutex> lock(state_mutex);
  common_native_context.invalidate();
  if (game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES)
    return nullptr;
  const int configured =
      game::get_dvar_int("zm_coopPlayerLimit").value_or(stock_limit);
  const int requested = resolve_session_limit(
      configured, game::get_dvar_int("com_maxclients").value_or(0),
      utils::flags::has_flag("experimental-zombies-coop"));
  blocked.store(true, std::memory_order_release);
  map_armed = false;
  admission_state.reset();
  // All temporary strings are destroyed before invoking engine longjmp error
  // paths.
  if (const char *error = preflight_error(configured, requested)) {
    reported = true;
    return error;
  }
  if (requested > stock_limit) {
    std::string schema_error;
    if (!prepare_schema_session(
            requested,
            std::string(game::get_dvar_string("mapname").value_or("")),
            schema_error)) {
      schema_asset_failure(schema_error);
      reported = true;
      return "Could not prepare the paired Zombies schema before script "
             "linking. Install matching companion data and rehost.";
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
  // Revoke stale receipts before publishing the new native identity.
  (void)game::Dvar_SetFromStringByName("zm_coopEndingRevision", "0", true);
  (void)game::Dvar_SetFromStringByName("zm_coopEndingMap", "", true);
  (void)game::Dvar_SetFromStringByName("zm_coopEndingEpoch", "0", true);
  admission_state.arm(epoch, requested > stock_limit);
  if (requested > stock_limit &&
      !common_native_context.publish(epoch, requested, active_map)) {
    admission_state.reset();
    map_armed = false;
    blocked.store(true, std::memory_order_release);
    reported = true;
    return "Could not publish the owned common-helper native session.";
  }
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
    (void)game::register_dvar_int("zm_coopEndingRevision", 0, 0,
                                  ending_policy::receipt_revision, 0u,
                                  "Local coop ending revision.");
    (void)game::register_dvar_int("zm_coopEndingEpoch", 0, 0, INT_MAX, 0u,
                                  "Local coop ending epoch.");
    // String dvars follow the existing main-pipeline registration boundary.
    // An early map command may already have created the receipt via Dvar_Set;
    // never re-register/reset its current value after a fresh epoch was armed.
    scheduler::once(
        [] {
          if (!game::Dvar_FindVar("zm_coopEndingMap"))
            (void)game::register_dvar_string("zm_coopEndingMap", "", 0u,
                                             "Local coop ending map.");
        },
        scheduler::pipeline::main);
  }
};
} // namespace zombies::coop
REGISTER_COMPONENT(zombies::coop::component)
