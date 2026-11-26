#include <std_include.hpp>
#define BOIII_COOP_EZZ

#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/schema_assets.hpp"
#include "zombies/schema_asset_store.hpp"
#include "zombies/schema_context_policy.hpp"
#include "zombies/castle_selector.hpp"
#include "zombies/score_events.hpp"
#include "game/utils.hpp"
#include <utils/cryptography.hpp>

namespace zombies::coop::schema_assets {
namespace {
// Deliberately process-lifetime storage, independent of the custom-script
// allocator and map shutdown. The native VM may still hold or mutate pointers.
t7::schema_asset_store<script_asset> assets;
std::mutex client_context_mutex;
std::string client_context_map;

std::string_view bounded_path(const char *name) noexcept {
  if (!name)
    return {};
  for (std::size_t i = 0; i <= 255; ++i)
    if (!name[i])
      return {name, i};
  return {};
}
void fail(std::string_view reason) noexcept {
  // Defer Com_Error to the existing main-frame gate, outside the asset/cache
  // locks and engine database call. Never unwind a native engine frame.
  diagnostic_event("schema-asset-failure");
  try {
    schema_asset_failure(reason);
  } catch (...) {
  }
}
bool active() noexcept {
  refresh_schema_session();
  const auto capacity = frozen_schema_capacity();
  return capacity >= 5 && capacity <= 8;
}
void sync_client_map(std::string_view name) {
  if (schema_assets_failed() || assets.exhausted())
    return;
  if (!t7::canonical_asset(name).ends_with(".csc"))
    return;
  bool changed = false;
  (void)prepare_client_generation(frozen_schema_map(), false, changed);
}
void retained_event(std::string_view phase, std::size_t length) noexcept {
  diagnostic_event(phase, static_cast<std::int64_t>(length));
  try {
    const auto [count, bytes] = assets.retained();
    diagnostic_event("schema-asset-retained", static_cast<std::int64_t>(count),
                     static_cast<std::int64_t>(bytes));
  } catch (...) {
  }
}
struct castle_request {
  int capacity{};
  std::string map;
  t7::schema_asset_store<script_asset>::generation_lease lease;
};
bool castle_zombies_mode() {
#ifdef BOIII_COOP_EZZ
  return game::com::Com_SessionMode_GetMode() == game::eModes::ZOMBIES;
#else
  return game::Com_SessionMode_GetMode() == game::MODE_ZOMBIES;
#endif
}
std::string_view castle_current_map() {
  // Bound the native string without allocating. Use existing CB/Ezz dvar APIs;
  // a peer's frozen allocation alone does not establish the active game mode.
  auto dvar = game::get_dvar("mapname");
  if (!dvar)
    return {};
  auto resolved = dvar.resolve();
  if (!resolved || resolved.type() != game::dvarType_t::STRING)
    return {};
  const auto *value = resolved.get_cstring();
  if (!value)
    return {};
  for (std::size_t i = 0; i <= 64; ++i)
    if (!value[i])
      return {value, i};
  return {};
}
castle_selector::context
castle_snapshot(const castle_request &request) noexcept {
  return {true,
          request.capacity,
          request.lease.role,
          request.lease.generation,
          request.map,
          request.map};
}
bool castle_current(const castle_request &request) noexcept {
  try {
    if (!castle_zombies_mode())
      return false;
    const auto actual = castle_current_map();
    // Called while the store generation lock is held: never take the companion
    // context mutex here. The frozen map was verified outside store locks at
    // capture. Recheck native mode/map and atomic capacity against that
    // snapshot; the store independently checks and holds its current server VM
    // lease.
    return castle_selector::same_context(
        castle_snapshot(request),
        {castle_zombies_mode(), frozen_schema_capacity(), request.lease.role,
         request.lease.generation, actual, request.map});
  } catch (...) {
    return false;
  }
}
std::optional<castle_request> capture_castle_request(std::string_view path) {
  if (!castle_zombies_mode())
    return std::nullopt;
  castle_request request{frozen_schema_capacity(),
                         std::string(castle_current_map()),
                         assets.capture_generation(path)};
  const auto frozen = frozen_schema_map();
  if (!castle_selector::valid_context(
          {castle_zombies_mode(), request.capacity, request.lease.role,
           request.lease.generation, request.map, frozen}))
    return std::nullopt;
  return request;
}
void castle_failure(const castle_request &request,
                    std::string_view reason) noexcept {
  try {
    // A late worker from an old VM must never poison its successor. Hold the
    // generation lock across the native-context check and failure publication.
    (void)assets.if_current(request.lease, [&] {
      if (castle_current(request))
        fail(reason);
    });
  } catch (...) {
  }
}
castle_selector::digest castle_digest(std::span<const std::uint8_t> bytes) {
  const auto raw =
      utils::cryptography::sha256::compute(bytes.data(), bytes.size(), false);
  if (raw.size() != 32)
    throw std::runtime_error("Invalid raw SHA-256 result.");
  castle_selector::digest result{};
  std::memcpy(result.data(), raw.data(), result.size());
  return result;
}
script_asset *transform_castle(std::string_view path,
                               script_asset *original) noexcept {
  std::optional<castle_request> request;
  try {
    request = capture_castle_request(path);
    if (!request)
      return original;
    if (auto *cached = assets.cached(path, request->lease)) {
      bool current = false;
      (void)assets.if_current(request->lease,
                              [&] { current = castle_current(*request); });
      return current ? cached : original;
    }
    if (schema_assets_failed())
      return original;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) >
            castle_selector::input_limit) {
      castle_failure(*request, "Missing or oversized raw Castle VO script.");
      return original;
    }
    // Older VMs can offer their linked/post-transform object. Always recover
    // the pristine PRE-transform template before the fixed raw digest gate.
    auto pristine = assets.template_for(original);
    if (!pristine) {
      const auto *raw =
          reinterpret_cast<const std::uint8_t *>(original->buffer);
      pristine = std::vector<std::uint8_t>(raw, raw + original->len);
    }
    auto cloned = castle_selector::rewrite(*pristine, castle_digest);
    if (!cloned.changed) {
      castle_failure(*request, "Castle VO script differs from the verified "
                               "pre-link import contract.");
      return original;
    }
    std::string error;
    auto *result = assets.retain_if(path, std::move(cloned.bytes), error,
                                    request->lease, std::move(pristine),
                                    [&] { return castle_current(*request); });
    if (!result)
      castle_failure(*request, error);
    else
      retained_event("schema-castle-import-gsc",
                     static_cast<std::size_t>(result->len));
    return result ? result : original;
  } catch (...) {
    if (request)
      castle_failure(*request,
                     "Could not retain the verified Castle VO import repair.");
    return original;
  }
}
script_asset *transform_score(std::string_view path,
                              script_asset *original) noexcept {
  std::optional<castle_request> request;
  try {
    request = capture_castle_request(path);
    if (!request)
      return original;
    if (auto *cached = assets.cached(path, request->lease)) {
      bool current = false;
      (void)assets.if_current(request->lease,
                              [&] { current = castle_current(*request); });
      return current ? cached : original;
    }
    if (schema_assets_failed())
      return original;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) > score_events::input_limit) {
      castle_failure(*request,
                     "Missing or oversized raw stock score-event script.");
      return original;
    }
    // Older VMs can offer their linked/post-transform object. Always recover
    // the pristine PRE-transform template before the fixed raw digest gate.
    auto pristine = assets.template_for(original);
    if (!pristine) {
      const auto *raw =
          reinterpret_cast<const std::uint8_t *>(original->buffer);
      pristine = std::vector<std::uint8_t>(raw, raw + original->len);
    }
    auto cloned = score_events::rewrite(*pristine, castle_digest);
    if (!cloned.changed) {
      castle_failure(*request, "stock score-event script differs from the "
                               "verified pre-link import contract.");
      return original;
    }
    std::string error;
    auto *result = assets.retain_if(path, std::move(cloned.bytes), error,
                                    request->lease, std::move(pristine),
                                    [&] { return castle_current(*request); });
    if (!result)
      castle_failure(*request, error);
    else
      retained_event("schema-score-import-gsc",
                     static_cast<std::size_t>(result->len));
    return result ? result : original;
  } catch (...) {
    if (request)
      castle_failure(
          *request,
          "Could not retain the verified stock score-event import repair.");
    return original;
  }
}
struct score_source_sample {
  bool verified{};
  std::string map;
  int capacity{};
  std::uint64_t generation{};
};
score_source_sample capture_score_source() noexcept {
  try {
    if (schema_assets_failed() || assets.exhausted())
      return {};
    auto request = capture_castle_request(score_events::asset_name);
    if (!request)
      return {};
    // The only producer of this private cache entry is the fixed,
    // digest-checked score transform. A stale lease cannot turn a successor VM
    // ready.
    const auto *retained =
        assets.cached(score_events::asset_name, request->lease);
    bool current = false;
    (void)assets.if_current(request->lease,
                            [&] { current = castle_current(*request); });
    if (!current)
      return {};
    return {retained != nullptr, request->map, request->capacity,
            request->lease.generation};
  } catch (...) {
    return {};
  }
}

} // namespace

bool score_producer_ready() noexcept { return capture_score_source().verified; }
std::string score_source_diagnostic() {
  const auto sample = capture_score_source();
  // Map tokens have already passed the strict native schema context predicate;
  // no user/network strings, pointers, names or credentials enter this receipt.
  std::string result = "{\"version\":1,\"verified\":";
  result += sample.verified ? "true" : "false";
  result += ",\"map\":\"" + sample.map +
            "\",\"capacity\":" + std::to_string(sample.capacity);
  result += ",\"realm\":0,\"generation\":" + std::to_string(sample.generation);
  result += ",\"revision\":" + std::to_string(companion_revision);
  result += ",\"contract\":\"" + std::string(companion_contract) + "\"}";
  return result;
}

void begin_vm_generation(bool client) noexcept {
  try {
    if (assets.exhausted()) {
      fail("Schema asset store exhausted; restart required.");
      return;
    }
    if (client) {
      const std::lock_guard lock(client_context_mutex);
      if (!assets.begin_generation(true)) {
        fail("Schema asset store exhausted; restart required.");
        return;
      }
      client_context_map.clear();
    } else if (!assets.begin_generation(false)) {
      fail("Schema asset store exhausted; restart required.");
      return;
    }
    diagnostic_event(client ? "schema-client-vm-generation"
                            : "schema-server-vm-generation");
  } catch (...) {
    fail("Could not establish a fresh script asset generation.");
  }
}
bool prepare_client_generation(std::string_view map, bool fresh_request,
                               bool &changed) noexcept {
  changed = false;
  try {
    if (schema_assets_failed() || assets.exhausted())
      return false;
    std::string next_map(map);
    const std::lock_guard lock(client_context_mutex);
    if (!client_map_generation_changes(client_context_map, map, fresh_request))
      return true;
    if (!assets.begin_generation(true)) {
      fail("Schema asset store exhausted; restart required.");
      return false;
    }
    client_context_map.swap(next_map);
    changed = true;
    diagnostic_event(fresh_request ? "schema-client-request-generation"
                                   : "schema-client-map-generation");
    return true;
  } catch (...) {
    fail("Could not establish a client asset generation.");
    return false;
  }
}
void reset_client_session_generation() noexcept {
  try {
    const std::lock_guard lock(client_context_mutex);
    if (!assets.begin_generation(true))
      return; // poison is permanent; retain current fallback identity
    client_context_map.clear();
    diagnostic_event("schema-client-frontend-reset");
  } catch (...) {
    fail("Could not retire client asset generation.");
  }
}

bool retention_exhausted() noexcept {
  try {
    return assets.exhausted();
  } catch (...) {
    return true;
  }
}

bool pin_payloads(std::string_view server, std::string_view client,
                  std::string_view island, std::string &error) {
  try {
    const bool result = assets.pin(server, client, island, error);
    if (result)
      diagnostic_event("schema-payloads-pinned",
                       static_cast<std::int64_t>(server.size()),
                       static_cast<std::int64_t>(client.size()));
    return result;
  } catch (const std::exception &e) {
    error = e.what();
    return false;
  } catch (...) {
    error = "Could not retain schema companion payloads.";
    return false;
  }
}

bool handles_companion(const char *name) noexcept {
  if (!active())
    return false;
  const auto identity = t7::fixed_companion_identity(bounded_path(name));
  if (!identity)
    return false;
  return *identity != t7::companion_id::island_trials ||
         assets.island_server_authorized();
}

script_asset *lookup_companion(const char *name) noexcept {
  try {
    if (!handles_companion(name))
      return nullptr;
    sync_client_map(bounded_path(name));
    const auto lease = assets.capture_generation(bounded_path(name));
    if (auto *existing = assets.cached(bounded_path(name), lease))
      return existing;
    std::string error;
    auto *result = assets.companion(bounded_path(name), error, lease);
    if (!result) {
      fail(error);
      result = assets.emergency_companion(bounded_path(name));
    } else if (t7::fixed_companion_identity(bounded_path(name)) ==
               t7::companion_id::island_trials)
      retained_event("schema-companion-island",
                     static_cast<std::size_t>(result->len));
    else
      retained_event(bounded_path(name) == t7::server_companion
                         ? "schema-companion-gsc"
                         : "schema-companion-csc",
                     static_cast<std::size_t>(result->len));
    return result;
  } catch (...) {
    fail("Could not allocate fresh schema companion backing.");
    try {
      return assets.emergency_companion(bounded_path(name));
    } catch (...) {
      return nullptr;
    }
  }
}

script_asset *transform(const char *name, script_asset *original) noexcept {
  if (!active())
    return original;
  try {
    const auto path = bounded_path(name);
    if (castle_selector::is_asset(path))
      return transform_castle(path, original);
    if (score_events::is_asset(path))
      return transform_score(path, original);
    const auto map = frozen_schema_map();
    if (!t7::is_schema_anchor(path, map))
      return original;
    const bool island_anchor = t7::is_island_server_anchor(path, map);
    sync_client_map(path);
    const auto lease = assets.capture_generation(path);
    if (auto *cached = assets.cached(path, lease))
      return cached;
    if (schema_assets_failed())
      return original;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) > t7::retention_limit) {
      fail("Missing or oversized raw schema bootstrap anchor.");
      return original;
    }
    const auto saved_template = assets.template_for(original);
    const auto object =
        saved_template ? std::span<const std::uint8_t>(*saved_template)
                       : std::span(reinterpret_cast<const std::uint8_t *>(
                                       original->buffer),
                                   static_cast<std::size_t>(original->len));
    auto cloned = t7::clone_with_include(object, t7::companion_include);
    if (cloned.result == t7::include_result::already_present) {
      // Isolate each new generation from previously linked/mutated backing.
      cloned.bytes.assign(object.begin(), object.end());
    } else if (cloned.result != t7::include_result::inserted) {
      fail("Schema bootstrap anchor is not a validated raw T7 include object.");
      return original;
    }
    if (island_anchor) {
      auto island_clone =
          t7::clone_with_include(cloned.bytes, t7::island_include);
      if (island_clone.result == t7::include_result::inserted)
        cloned.bytes = std::move(island_clone.bytes);
      else if (island_clone.result != t7::include_result::already_present) {
        fail("Island bootstrap anchor could not retain its scoped companion "
             "include.");
        return original;
      }
    }
    const auto embedded = t7::bounded_name(object, t7::read32(object, 0x34));
    if (!embedded ||
        t7::canonical_script(*embedded) != t7::canonical_script(path)) {
      fail("Schema bootstrap anchor logical name differs from its object.");
      return original;
    }
    if (island_anchor && !assets.authorize_island_server(lease)) {
      fail("Island bootstrap generation changed before authorization.");
      return original;
    }
    std::string error;
    auto *result = assets.retain(path, std::move(cloned.bytes), error, lease);
    if (!result)
      fail(error);
    else
      retained_event(path.ends_with(".gsc") ? "schema-anchor-gsc"
                                            : "schema-anchor-csc",
                     static_cast<std::size_t>(result->len));
    return result ? result : original;
  } catch (...) {
    fail("Could not clone schema bootstrap anchor.");
    return original;
  }
}
} // namespace zombies::coop::schema_assets
