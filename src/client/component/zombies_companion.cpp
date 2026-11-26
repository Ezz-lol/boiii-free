#include <std_include.hpp>
#define BOIII_COOP_EZZ

#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"
#include "zombies/companion.hpp"
#include "zombies/companion_payloads.hpp"
#include "zombies/schema_assets.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/schema_context_policy.hpp"
#include <array>
#include <chrono>
#include <cstring>
#include <utils/cryptography.hpp>
#include <utils/flags.hpp>
#include <utils/nt.hpp>
#include <atomic>
#include <fstream>
#include <mutex>

namespace zombies::coop {
namespace {
std::atomic_int schema_slots{};
std::atomic_bool assets_failed{};
std::mutex context_mutex;
std::string selected_map;
schema_context_policy context_policy;
std::uint64_t peer_request_id{};
std::mutex bundle_mutex;
bool bundle_pinned{};
std::string score_wrapper;

int read_int(const char *name) {
#ifdef BOIII_COOP_EZZ
  return game::get_dvar_int(name).value_or(0);
#else
  return game::get_dvar_int(name);
#endif
}
std::string read_string(const char *name) {
#ifdef BOIII_COOP_EZZ
  return std::string(game::get_dvar_string(name).value_or(""));
#else
  return game::get_dvar_string(name);
#endif
}
bool zombies_mode() {
#ifdef BOIII_COOP_EZZ
  return game::com::Com_SessionMode_GetMode() == game::eModes::ZOMBIES;
#else
  return game::Com_SessionMode_GetMode() == game::MODE_ZOMBIES;
#endif
}
bool valid_map(std::string_view map) {
  if (map.empty() || map.size() > 64 || map == "core_frontend")
    return false;
  for (const auto c : map)
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-'))
      return false;
  return true;
}
std::string digest(std::string_view data) {
  const auto raw = utils::cryptography::sha256::compute(
      reinterpret_cast<const std::uint8_t *>(data.data()), data.size(), false);
  constexpr char hex[] = "0123456789abcdef";
  std::string out;
  out.reserve(raw.size() * 2);
  for (const unsigned char value : raw) {
    out += hex[value >> 4];
    out += hex[value & 15];
  }
  return out;
}
std::string canonical_text(std::string_view bytes) {
  std::string result;
  result.reserve(bytes.size());
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    if (bytes[i] == '\r' && i + 1 < bytes.size() && bytes[i + 1] == '\n')
      continue;
    result += bytes[i];
  }
  return result;
}
std::string read_bounded(const std::filesystem::path &path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open())
    throw std::runtime_error("Missing paired Zombies companion file.");
  std::string bytes(1024 * 1024 + 1, '\0');
  stream.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
  const auto size = stream.gcount();
  if (stream.bad() || (!stream.eof() && stream.fail()) || size <= 0 ||
      size > 1024 * 1024)
    throw std::runtime_error(
        "Invalid or oversized paired Zombies companion file.");
  bytes.resize(static_cast<std::size_t>(size));
  return bytes;
}
void clear_receipt(const char *revision, const char *mask,
                   const char *contract) {
  game::Dvar_SetFromStringByName(revision, "0", true);
  game::Dvar_SetFromStringByName(mask, "0", true);
  game::Dvar_SetFromStringByName(contract, "", true);
}
void set_slots(int capacity) {
#ifdef BOIII_COOP_EZZ
  const auto dvar = game::get_dvar("zm_coopSchemaSlots");
  if (!dvar)
    throw std::runtime_error("Missing native schema capacity dvar.");
  game::Dvar_SetIntFromSource(dvar, capacity, game::DvarSetSource::INTERNAL);
#else
  auto *dvar = game::Dvar_FindVar("zm_coopSchemaSlots");
  if (!dvar)
    throw std::runtime_error("Missing native schema capacity dvar.");
  // Source-backed Ezz binding for the same retail/dedicated PC images. INTERNAL
  // may update our ROM dvar; console/script setters must not own its value.
  static const game::symbol<void(game::dvar_t *, int, unsigned)> set_internal{
      0x14226B3B0, 0x140579740};
  set_internal(dvar, capacity, 0u);
#endif
  if (read_int("zm_coopSchemaSlots") != capacity)
    throw std::runtime_error(
        "Could not freeze the paired Zombies schema capacity.");
}
int expected_mask(std::string_view map) {
  if (map == "zm_castle")
    return 7;
  if (map == "zm_stalingrad")
    return 11;
  if (map == "zm_genesis")
    return 19;
  if (map == "zm_moon")
    return 35;
  if (map == "zm_island")
    return 67;
  return 3;
}
} // namespace

int frozen_schema_capacity() noexcept {
  return schema_slots.load(std::memory_order_acquire);
}
bool schema_assets_failed() noexcept {
  return assets_failed.load(std::memory_order_acquire);
}

std::string frozen_schema_map() {
  // The slot layout is session-fixed; each rotated map has its own field
  // families. Use the engine's current map once travel has replaced the
  // frontend map.
#ifdef BOIII_COOP_EZZ
  const bool frontend = game::com::Com_IsRunningUILevel();
#else
  const bool frontend = game::Com_IsRunningUILevel();
#endif
  const auto actual = frontend ? std::string{} : read_string("mapname");
  if (valid_map(actual))
    return actual;
  const std::lock_guard lock(context_mutex);
  return selected_map;
}

void schema_asset_failure(std::string_view reason) {
  if (assets_failed.exchange(true, std::memory_order_acq_rel))
    return;
  diagnostic_event("schema_asset_failed");
  fprintf(stderr, "[zombies_coop_schema] phase=asset_failed reason=%.*s\n",
          static_cast<int>(std::min<std::size_t>(reason.size(), 256)),
          reason.data());
}

bool companion_files_ready(std::string &error) {
  if (schema_assets::retention_exhausted()) {
    error = "Schema asset store exhausted; restart required.";
    return false;
  }
  const std::lock_guard lock(bundle_mutex);
  if (bundle_pinned)
    return true;
  try {
    const std::filesystem::path roots[] = {
#ifdef BOIII_COOP_EZZ
        game::get_data_path(),
#else
        game::get_appdata_path() / "data",
#endif
        std::filesystem::current_path() / "boiii",
        utils::nt::library{}.get_folder() / "boiii"};
    for (const auto &root : roots) {
      const auto folder = root / "coop_scripts";
      if (!std::filesystem::exists(folder / "eroded_coop_fields.gsc") &&
          !std::filesystem::exists(folder / "eroded_coop_fields.csc") &&
          !std::filesystem::exists(folder / "score_models.lua") &&
          !std::filesystem::exists(folder / "eroded_coop_island_trials.gsc"))
        continue;
      // A partial/stale first bundle is an error; never mix installation roots.
      auto server = read_bounded(folder / "eroded_coop_fields.gsc");
      auto client = read_bounded(folder / "eroded_coop_fields.csc");
      auto wrapper = canonical_text(read_bounded(folder / "score_models.lua"));
      auto island = read_bounded(folder / "eroded_coop_island_trials.gsc");
      if (digest(server) != payloads::server_sha256 ||
          digest(client) != payloads::client_sha256 ||
          digest(wrapper) != payloads::ui_sha256 ||
          digest(island) != payloads::island_trials_sha256)
        throw std::runtime_error("Paired Zombies companion files do not match "
                                 "this executable. Install its matching data.");
      if (!schema_assets::pin_payloads(server, client, island, error))
        return false;
      score_wrapper = std::move(wrapper);
      bundle_pinned = true;
      diagnostic_event("schema_bundle_pinned");
      return true;
    }
    error = "Missing paired Zombies GSC/CSC/UI companion bundle. Install "
            "matching data.";
  } catch (const std::exception &exception) {
    error = exception.what();
  } catch (...) {
    error = "Could not pin the paired Zombies companion bundle.";
  }
  return false;
}

namespace {
std::int64_t monotonic_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
bool prepare_session(int capacity, std::string_view map, bool peer,
                     std::string &error, std::uint64_t request = 0) {
  if (capacity <= 4) {
    reset_schema_session();
    return capacity >= 1;
  }
  if (capacity > 8 || !utils::flags::has_flag("experimental-zombies-coop") ||
      (!peer && !zombies_mode()) || !valid_map(map)) {
    error = "Invalid expanded Zombies schema session or missing experimental "
            "opt-in.";
    return false;
  }
  if (!companion_files_ready(error))
    return false;
  if (schema_assets::retention_exhausted()) {
    error = "Schema asset retention exhausted; restart required before another "
            "expanded session.";
    return false;
  }
  try {
#ifdef BOIII_COOP_EZZ
    const bool frontend = game::com::Com_IsRunningUILevel();
    const bool active_vm = game::com::Com_IsInGame() && !frontend;
#else
    const bool frontend = game::Com_IsRunningUILevel();
    const bool active_vm = game::Com_IsInGame() && !frontend;
#endif
    bool fresh_request = false;
    {
      const std::lock_guard lock(context_mutex);
      if (schema_assets_failed()) {
        error = "Paired Zombies schema failed. Rehost from the frontend with "
                "matching data.";
        return false;
      }
      const bool same_descriptor =
          schema_slots.load(std::memory_order_relaxed) == capacity &&
          selected_map == map;
      if (peer) {
        const auto action = peer_schema_prepare(
            peer_request_id, request, same_descriptor, frontend,
            active_vm || schema_slots.load(std::memory_order_relaxed) >= 5);
        if (action == peer_prepare_action::invalid) {
          error = "Missing verified join request identity.";
          return false;
        }
        if (action == peer_prepare_action::duplicate)
          return true;
        if (action == peer_prepare_action::require_frontend) {
          error = "Return to the frontend before reconnecting to an expanded "
                  "Zombies session; the native CSC reload boundary is not "
                  "verified.";
          return false;
        }
        fresh_request = action == peer_prepare_action::fresh;
      } else if (same_descriptor)
        return true;
    }
    bool client_generation_changed = false;
    if (!schema_assets::prepare_client_generation(map, fresh_request,
                                                  client_generation_changed)) {
      error = "Could not establish a safe client schema generation.";
      return false;
    }
    clear_receipt("zm_coopSchemaServerRevision", "zm_coopSchemaServerMask",
                  "zm_coopSchemaServerContract");
    if (client_generation_changed) {
      clear_receipt("zm_coopSchemaClientRevision", "zm_coopSchemaClientMask",
                    "zm_coopSchemaClientContract");
      game::Dvar_SetFromStringByName("zm_coopSchemaUIRevision", "0", true);
    }
    set_slots(capacity);
    {
      const std::lock_guard lock(context_mutex);
      selected_map = map;
      if (peer) {
        context_policy.peer(monotonic_ms());
        peer_request_id = request;
      } else {
        context_policy.host();
        peer_request_id = 0;
      }
      schema_slots.store(capacity, std::memory_order_release);
    }
    diagnostic_event(peer ? "schema_peer_pending" : "schema_session_frozen",
                     capacity);
    return true;
  } catch (const std::exception &exception) {
    error = exception.what();
  } catch (...) {
    error = "Could not freeze the paired Zombies schema.";
  }
  return false;
}
struct session_prefix {
  std::int32_t module, type, mode, status;
  std::array<std::byte, 0x30> unknown10;
  std::int32_t active;
  std::array<std::byte, 0x9c> unknown44;
  std::int32_t max_clients;
};
static_assert(offsetof(session_prefix, active) == 0x40);
static_assert(offsetof(session_prefix, max_clients) == 0xe0);
static_assert(sizeof(session_prefix) == 0xe4);
int native_capacity(const void *pointer, int module, int *mode = nullptr) {
  if (!pointer)
    return 0;
  session_prefix state{};
  std::memcpy(&state, pointer, sizeof(state));
  // Read-only prefix matches Ezz's verified LobbySession/SessionHost/HostInfo.
  if (state.module != module || state.type != 1 || state.status != 0 ||
      state.active != 2)
    return 0;
  if (state.mode != 0 && state.mode != 1)
    return 0; // PUBLIC or CUSTOM only
  if (mode)
    *mode = state.mode;
  return state.max_clients >= 1 && state.max_clients <= 18 ? state.max_clients
                                                           : 0;
}
} // namespace

bool prepare_schema_session(int capacity, std::string_view map,
                            std::string &error) {
  return prepare_session(capacity, map, false, error);
}
bool prepare_peer_schema_session(int capacity, std::string_view map,
                                 std::string &error, std::uint64_t request_id) {
  return prepare_session(capacity, map, true, error, request_id);
}

void cancel_peer_schema_session() noexcept {
  if (!game::is_client())
    return;
  try {
#ifdef BOIII_COOP_EZZ
    const bool frontend = game::com::Com_IsRunningUILevel();
#else
    const bool frontend = game::Com_IsRunningUILevel();
#endif
    {
      const std::lock_guard lock(context_mutex);
      if (!context_policy.peer_owned())
        return;
      // A cancelled UI query must never remove fields from a live map VM.
      if (!frontend && zombies_mode()) {
        context_policy.active_peer();
        return;
      }
    }
    reset_schema_session();
  } catch (...) {
  }
}

void refresh_schema_session() noexcept {
  if (!game::is_client() ||
      !utils::flags::has_flag("experimental-zombies-coop"))
    return;
  try {
#ifdef BOIII_COOP_EZZ
    const bool frontend = game::com::Com_IsRunningUILevel();
    const bool is_host =
        game::lobby::LobbyHost_IsHost(game::lobby::LobbyType::GAME);
    const auto *host = is_host ? game::lobby::LobbyHostData_GetSession(
                                     game::lobby::LobbyType::GAME)
                               : nullptr;
    const auto *client = game::lobby::client::LobbyClientData_GetSession(
        game::lobby::LobbyType::GAME);
#else
    const bool frontend = game::Com_IsRunningUILevel();
    const bool is_host = game::LobbyHost_IsHosting(game::LOBBY_TYPE_GAME);
    const auto *host =
        is_host ? game::LobbyHost_GetSession(game::LOBBY_TYPE_GAME) : nullptr;
    const auto *client = game::LobbyClient_GetSession(game::LOBBY_TYPE_GAME);
#endif
    const bool zombies = zombies_mode();
    int host_mode = -1;
    const auto host_capacity = native_capacity(host, 0, &host_mode);
    // CUSTOM screen400 includes Solo, whose existing launch guard clamps to4.
    // Never install an expanded client schema in that staging lobby.
    static const game::symbol<int> ui_screen{0x156111F18};
    const int screen = *ui_screen;
    const bool host_eligible =
        schema_host_eligible(frontend, host_mode, screen);
    const auto client_capacity = native_capacity(client, 1);
    const auto capacity = frozen_schema_capacity();
    bool peer = false, retain = false;
    {
      const std::lock_guard lock(context_mutex);
      if (frontend && host_eligible && host_capacity >= 5 &&
          host_capacity <= 8 &&
          context_policy.source() == schema_context_policy::origin::peer_active)
        context_policy.host(); // an actual new local GAME host replaces a
                               // completed peer session
      peer = context_policy.peer_owned();
      retain = context_policy.retain(monotonic_ms(), frontend, zombies,
                                     host_eligible && host_capacity == capacity,
                                     client_capacity == capacity);
    }
    if (peer) {
      if (!retain)
        reset_schema_session();
      return;
    }
    if (zombies && host_eligible && host_capacity >= 5 && host_capacity <= 8) {
      const auto map = read_string(frontend ? "ui_mapname" : "mapname");
      if (!valid_map(map))
        return; // selection has not been committed yet
      if (capacity >= 5 && !frontend && host_capacity != capacity) {
        schema_asset_failure(
            "Native hosted capacity changed after schema allocation.");
        return;
      }
      // Map identity/receipts are re-armed by the actual server VM load seam,
      // not by an intermediate mapname dvar change during travel.
      if (capacity >= 5 && !frontend)
        return;
      std::string error;
      if (!prepare_schema_session(host_capacity, map, error))
        schema_asset_failure(error);
      return;
    }
    if (!retain && (capacity >= 5 || schema_assets_failed()))
      reset_schema_session();
  } catch (...) {
    schema_asset_failure("Could not sample native Zombies schema context.");
  }
}

void schema_begin_server_vm() {
  clear_receipt("zm_coopSchemaServerRevision", "zm_coopSchemaServerMask",
                "zm_coopSchemaServerContract");
  diagnostic_event("schema_server_vm_begin", frozen_schema_capacity());
}

void reset_schema_session() noexcept {
  reset_schema_allocation(
      [] { set_slots(0); },
      [] {
        const std::lock_guard lock(context_mutex);
        schema_assets::reset_client_session_generation();
        selected_map.clear();
        context_policy.reset();
        peer_request_id = 0;
        // Publish stock only after the engine's ROM dvar was verified as zero.
        schema_slots.store(0, std::memory_order_release);
        assets_failed.store(schema_assets::retention_exhausted(),
                            std::memory_order_release);
      },
      [] {
        schema_asset_failure("Could not reset the native schema allocation; "
                             "restart with matching data.");
      });
}

bool schema_server_ready() noexcept {
  if (frozen_schema_capacity() < 5 || schema_assets_failed() ||
      !schema_assets::score_producer_ready())
    return false;
  try {
    return read_int("zm_coopSchemaServerRevision") == companion_revision &&
           read_int("zm_coopSchemaServerMask") ==
               expected_mask(frozen_schema_map()) &&
           read_string("zm_coopSchemaServerContract") == companion_contract;
  } catch (...) {
    return false;
  }
}

std::string score_schema_wrapper() {
  const std::lock_guard lock(bundle_mutex);
  return score_wrapper;
}

struct companion_component final : generic_component {
  void post_unpack() override {
#ifdef BOIII_COOP_EZZ
    constexpr auto read_only = game::dvarFlags_e::ROM;
#else
    constexpr auto read_only = game::DVAR_ROM;
#endif
    (void)game::register_dvar_int("zm_coopSchemaSlots", 0, 0, 8, read_only,
                                  "Native-frozen schema slot allocation; "
                                  "gameplay uses current participants.");
    for (const auto *name :
         {"zm_coopSchemaServerRevision", "zm_coopSchemaServerMask",
          "zm_coopSchemaClientRevision", "zm_coopSchemaClientMask",
          "zm_coopSchemaUIRevision"})
      (void)game::register_dvar_int(name, 0, 0, INT_MAX, 0u,
                                    "Paired schema execution diagnostics.");
  }
};
} // namespace zombies::coop
REGISTER_COMPONENT(zombies::coop::companion_component)
