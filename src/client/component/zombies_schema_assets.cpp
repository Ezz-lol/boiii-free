#include <std_include.hpp>
#define BOIII_COOP_EZZ

#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/schema_assets.hpp"
#include "zombies/schema_asset_store.hpp"
#include "zombies/schema_context_policy.hpp"
#include "zombies/castle_selector.hpp"
#include "zombies/score_events.hpp"
#include "zombies/powerup_events.hpp"
#include "zombies/coop_runtime.hpp"
#include "zombies/lobby_policy.hpp"
#include "zombies/map_runtime_bridge.hpp"
#include "zombies/map_mechanics_payloads.hpp"
#include "zombies/map_mechanics_contracts.hpp"
#include <type_traits>
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

// Map mechanic imports use process-lived, current-VM backing.
namespace map_bridge = zombies::coop::map_runtime_bridge;
using map_store_lease = t7::schema_asset_store<script_asset>::generation_lease;
using map_bridge_request = map_bridge::request<map_store_lease>;
using map_bridge_proofs = map_bridge::receipts<script_asset, 8>;
std::mutex map_operation_mutex; // component -> store; never acquired by a store callback
map_bridge_proofs map_proofs;

struct map_owned_request {
  int capacity{};
  std::string map;
  common_helper::native_ticket ticket;
  map_store_lease lease{};
};
bool map_identity_owned(const map_owned_request &r) noexcept {
  // Identity only. Revocation/failure cannot disable owned emergency backing.
  // All called APIs are atomic-only; no engine/dvar/context/runtime mutex here.
  return r.ticket && r.capacity >= 5 && r.capacity <= 8 && r.lease.role == 0 &&
         r.ticket->epoch > 0 && r.ticket->capacity == r.capacity &&
         frozen_schema_capacity() == r.capacity &&
         common_helper::native_session_owned(r.ticket);
}
map_bridge_request map_bridge_view(const map_owned_request &r) noexcept {
  // Construct only after the owned std::string is at its final address. Never
  // save this view in a struct which is subsequently moved (including SSO).
  return {r.map, r.capacity, r.lease};
}
std::span<const map_bridge::descriptor> map_descriptors() noexcept {
  return map_mechanics::descriptors;
}
std::optional<map_owned_request> capture_map_request(std::string_view path,
                                                    bool allow_failed = false) {
  // Every engine/frozen-map read occurs before map_operation_mutex/store locks.
  if (!castle_zombies_mode()) return {};
#ifdef BOIII_COOP_EZZ
  if (game::com::Com_IsRunningUILevel()) return {};
#else
  if (game::Com_IsRunningUILevel()) return {};
#endif
  const auto capacity = frozen_schema_capacity();
  if (capacity < 5 || capacity > 8 || (!allow_failed && schema_assets_failed())) return {};
  const std::string actual(castle_current_map());
  const auto frozen = frozen_schema_map();
  const auto ticket = common_helper::capture_native_session();
  if (!ticket || !t7::supported_map_name(actual) || actual == "core_frontend" ||
      actual != frozen || ticket->map != actual || ticket->capacity != capacity ||
      ticket->epoch != active_startup_epoch() || !common_helper::native_session_owned(ticket))
    return {};
  const auto lease = assets.capture_generation(path);
  if (lease.role != 0 || !lease.generation) return {};
  map_owned_request r{capacity, actual, ticket, lease};
  return map_identity_owned(r) ? std::optional<map_owned_request>(std::move(r)) : std::nullopt;
}
std::optional<map_owned_request> capture_map_backing_identity(std::string_view path) {
  // Read-only loader/replacefunc paths use already-published native identity.
  // They authorize no new stock transformation and perform no engine calls.
  const auto ticket = common_helper::capture_native_session();
  if (!ticket || !t7::supported_map_name(ticket->map) || ticket->map == "core_frontend" ||
      ticket->capacity < 5 || ticket->capacity > 8 ||
      !common_helper::native_session_owned(ticket)) return {};
  map_owned_request r{ticket->capacity, ticket->map, ticket, assets.capture_generation(path)};
  return map_identity_owned(r) ? std::optional<map_owned_request>(std::move(r)) : std::nullopt;
}

struct map_failure_publication { bool current{}, first{}; };
map_failure_publication map_latch_failure_locked(const map_owned_request &r) noexcept {
  // Call ONLY from assets.if_current. No logging/engine/other locks here.
  if (!map_identity_owned(r)) return {};
  const bool first = latch_schema_asset_failure();
  (void)common_helper::revoke_native_session(r.ticket);
  return {true, first};
}
void map_report_failure(map_failure_publication publication, std::string_view reason) noexcept {
  // Caller has released map_operation_mutex and the retained-store lock.
  if (!publication.first) return;
  try { report_schema_asset_failure(reason); } catch (...) {}
}
void map_publish_failure(const map_owned_request &r, std::string_view reason) noexcept {
  map_failure_publication publication;
  try {
    (void)assets.if_current(r.lease, [&] { publication = map_latch_failure_locked(r); });
  } catch (...) {}
  map_report_failure(publication, reason);
}
std::string map_operation_reason(const map_bridge::operation<script_asset> &result) {
  std::string reason(result.reason);
  if (!result.detail.empty()) { reason += ": "; reason += result.detail; }
  return reason;
}
bool map_known_stock(const char *name) noexcept {
  try {
    const auto path = t7::canonical_asset(bounded_path(name));
    if (!path.ends_with(".gsc")) return false;
    for (const auto &row : map_descriptors()) if (row.path == path) return true;
    return false;
  } catch (...) { return false; }
}
bool map_known_helper(const char *name) noexcept {
  return t7::map_companion_identity(bounded_path(name)).has_value();
}
bool map_handles_stock(const char *name) noexcept {
  try {
    if (!map_known_stock(name)) return false;
    const auto ticket = common_helper::capture_native_session();
    if (!ticket || !common_helper::native_session_owned(ticket) ||
        ticket->capacity < 5 || ticket->capacity > 8) return false;
    const auto path = t7::canonical_asset(bounded_path(name));
    for (const auto &row : map_descriptors())
      if (row.path == path && map_bridge::detail::applies(row, ticket->map)) return true;
    return false;
  } catch (...) { return false; }
}
bool map_handles_helper(const char *name) noexcept {
  try {
    const auto index = t7::map_companion_identity(bounded_path(name));
    if (!index) return false;
    const auto ticket = common_helper::capture_native_session();
    return ticket && ticket->capacity >= 5 && ticket->capacity <= 8 &&
        common_helper::native_session_owned(ticket) &&
        (map_bridge::detail::dependencies(map_descriptors(), ticket->map) &
         (map_bridge::helper_mask{1} << *index));
  } catch (...) { return false; }
}
bool map_pin_all_helpers(std::string &error) noexcept {
  try {
    static_assert(std::tuple_size_v<std::remove_cvref_t<decltype(map_mechanics::compiled_payloads)>> == 7);
    static_assert(t7::map_companion_names.size() == 7);
    static_assert(std::tuple_size_v<std::remove_cvref_t<decltype(map_mechanics::descriptors)>> == 8);
    std::array<std::string_view, 7> views{};
    for (unsigned i = 0; i < views.size(); ++i) {
      const auto &payload = map_mechanics::compiled_payloads[i];
      if (payload.name != t7::map_companion_names[i] || payload.bytes.empty()) {
        error = "Fixed map helper payload identity differs from its reserved code name.";
        return false;
      }
      views[i] = {reinterpret_cast<const char *>(payload.bytes.data()), payload.bytes.size()};
    }
    bool pinned = false;
    {
      const std::lock_guard lock(map_operation_mutex);
      pinned = map_bridge::pin_helpers(assets, std::span<const std::string_view, 7>(views), error);
    }
    return pinned;
  } catch (...) {
    try { error = "Could not pin immutable map helper code objects."; } catch (...) {}
    return false;
  }
}
script_asset *map_transform_stock(const char *name, script_asset *original) noexcept {
  std::optional<map_owned_request> r;
  try {
    if (!map_known_stock(name)) return original;
    const auto path = bounded_path(name);
    r = capture_map_request(path);
    if (!r) return original; // rejected identities cannot publish a successor failure
    map_bridge::operation<script_asset> result;
    {
      const std::lock_guard lock(map_operation_mutex);
      const auto view = map_bridge_view(*r);
      result = map_bridge::route_stock(assets, map_proofs, map_descriptors(), view, path,
          original, castle_digest, [&]() noexcept { return map_identity_owned(*r); });
    }
    if (result.state == map_bridge::outcome::untouched) return original;
    if (result.okay()) return result.asset;
    // Return stock only for native unwinding AFTER expansion is atomically
    // withdrawn. The outer startup/admission gate must observe the failure.
    const auto reason = map_operation_reason(result); // allocation/log text outside both locks
    map_publish_failure(*r, reason);
    return original;
  } catch (...) {
    if (r) map_publish_failure(*r, "map_stock_native_glue_exception");
    return original;
  }
}
script_asset *map_lookup_helper(const char *name) noexcept {
  std::optional<map_owned_request> r;
  try {
    if (!map_known_helper(name)) return nullptr;
    const auto path = bounded_path(name);
    r = capture_map_request(path, true);
    if (!r) return nullptr;
    map_bridge::operation<script_asset> result;
    {
      const std::lock_guard lock(map_operation_mutex);
      const auto view = map_bridge_view(*r);
      result = map_bridge::lookup_helper(assets, map_proofs, map_descriptors(), view, path,
          static_cast<script_asset *>(nullptr), [&]() noexcept { return map_identity_owned(*r); });
    }
    if (result.okay() || result.state == map_bridge::outcome::untouched) return result.asset;
    // lookup_helper obtains guarded emergency backing before this publication.
    const auto reason = map_operation_reason(result);
    map_publish_failure(*r, reason);
    return result.state == map_bridge::outcome::emergency ? result.asset : nullptr;
  } catch (...) {
    // Capture can throw before a usable identity exists. Never use an unscoped
    // emergency or publish a failure against an uncaptured successor.
    script_asset *emergency = nullptr;
    if (r) {
      try {
        emergency = assets.emergency_map_companion(bounded_path(name), r->lease,
            [&]() noexcept { return map_identity_owned(*r); });
      } catch (...) {}
      map_publish_failure(*r, "map_helper_native_glue_exception");
    }
    return emergency;
  }
}
script_asset *map_owned_stock_backing(const map_owned_request &r, std::string_view path) {
  auto *cached = assets.cached(path, r.lease);
  if (!cached) return nullptr;
  script_asset *result = nullptr;
  const auto descriptor_view = map_descriptors();
  (void)assets.if_current(r.lease, [&] {
    if (!map_identity_owned(r)) return;
    for (std::size_t i = 0; i < descriptor_view.size(); ++i) {
      const auto &row = descriptor_view[i];
      if (row.path != path || !map_bridge::detail::applies(row, r.map)) continue;
      const auto &proof = map_proofs.stock[i];
      if (proof.generation == r.lease.generation && proof.identity == &row && proof.requested &&
          proof.state == map_bridge_proofs::stage::ready && proof.asset == cached) result = cached;
    }
  });
  return result;
}
script_asset *map_current_backing(const char *name) noexcept {
  try {
    const auto path = t7::canonical_asset(bounded_path(name));
    if (!map_known_stock(name) && !map_known_helper(name)) return nullptr;
    auto r = capture_map_backing_identity(path);
    if (!r) return nullptr;
    if (const auto index = t7::map_companion_identity(path)) {
      auto *cached = assets.cached(path, r->lease);
      script_asset *result = nullptr;
      (void)assets.if_current(r->lease, [&] {
        if (!map_identity_owned(*r)) return;
        const auto &proof = map_proofs.helpers[*index];
        if (cached && proof.generation == r->lease.generation && proof.asset == cached) result = cached;
      });
      return result;
    }
    return map_owned_stock_backing(*r, path);
  } catch (...) { return nullptr; }
}
int map_temple_pap_epoch() {
  // Native dvar read on the normal server readiness/diagnostic path only.
  // Never execute this inside map_operation_mutex or a store callback.
#ifdef BOIII_COOP_EZZ
  auto found = game::get_dvar("zm_coopTemplePapEpoch");
  auto value = found ? found.resolve() : found;
  return value && value.type() == game::dvarType_t::INT ? value.get_int() : 0;
#else
  const auto *value = game::Dvar_FindVar("zm_coopTemplePapEpoch");
  if (value && value->type == game::DVAR_TYPE_SESSIONMODE_BASE_DVAR) {
    const auto mode = game::Com_SessionMode_GetMode();
    if (mode < 0 || mode >= game::MODE_COUNT) return 0;
    value = value->current.value.indirect[mode];
  }
  return value && value->type == game::DVAR_TYPE_INT ? value->current.value.integer : 0;
#endif
}
struct map_readiness_sample {
  std::optional<map_owned_request> identity;
  bool required{}, ready{};
  const char *reason{"no_current_context"};
};
map_readiness_sample map_readiness_snapshot() {
  map_readiness_sample sample;
  const auto capacity = frozen_schema_capacity();
  if (capacity < 5) {
    sample.ready = !schema_assets_failed();
    sample.reason = sample.ready ? "stock_capacity" : "schema_asset_failed";
    return sample;
  }
  sample.required = true;
  sample.identity = capture_map_request("scripts/zm/__map_mechanics_ready.gsc", true);
  if (!sample.identity) return sample;
  const auto &r = *sample.identity;
  if (schema_assets_failed()) { sample.reason = "schema_asset_failed"; return sample; }
  const auto view = map_bridge_view(r);
  const auto result = map_bridge::ready(assets, map_proofs, map_descriptors(), view,
      [&]() noexcept { return map_identity_owned(r); });
  sample.required = result.required;
  sample.ready = result.ready;
  sample.reason = result.reason;
  if (listing_official_map(r.map)) {
    sample.required = true;
    // All official maps load stock Gums before readiness. Optional global BGB
    // remains optional only for custom maps which never encounter that module.
    if (!map_owned_stock_backing(r, "scripts/zm/bgbs/_zm_bgb_anywhere_but_here.gsc")) {
      sample.ready = false; sample.reason = "official_bgb_not_retained";
    }
  }
  if (sample.ready && r.map == "zm_temple" && map_temple_pap_epoch() != r.ticket->epoch) {
    sample.ready = false; sample.reason = "temple_pap_not_armed";
  }
  if (!map_identity_owned(r) || schema_assets_failed()) {
    sample.ready = false; sample.reason = "readiness_identity_changed";
  }
  return sample;
}
bool map_ready() noexcept {
  try { return map_readiness_snapshot().ready; } catch (...) { return false; }
}
std::string map_diagnostic() {
  try {
    const auto sample = map_readiness_snapshot();
    const auto *r = sample.identity ? &*sample.identity : nullptr;
    // Exactly ten fields. Map strings passed the strict native map predicate;
    // reasons are fixed source labels. No addresses or user/network data.
    return "{\"version\":1,\"map\":\"" + (r ? r->map : std::string{}) +
      "\",\"capacity\":" + std::to_string(r ? r->capacity : frozen_schema_capacity()) +
      ",\"epoch\":" + std::to_string(r ? r->ticket->epoch : active_startup_epoch()) +
      ",\"realm\":0,\"generation\":" + std::to_string(r ? r->lease.generation : 0) +
      ",\"required\":" + (sample.required ? "true" : "false") +
      ",\"ready\":" + (sample.ready ? "true" : "false") +
      ",\"reason\":\"" + sample.reason + "\",\"linked_body_authenticated\":false}";
  } catch (...) {
    return "{\"version\":1,\"map\":\"\",\"capacity\":0,\"epoch\":0,\"realm\":0,\"generation\":0,\"required\":true,\"ready\":false,\"reason\":\"diagnostic_exception\",\"linked_body_authenticated\":false}";
  }
}
bool map_observe_helper_detour(std::uintptr_t target) noexcept {
  try {
    auto r = capture_map_backing_identity(t7::map_companion_names[0]);
    if (!r) return false;
    struct protected_range { script_asset *asset{}; std::uintptr_t begin{}, end{}; };
    std::array<protected_range, 7> ranges{};
    for (unsigned i = 0; i < ranges.size(); ++i) {
      const auto bytes = map_mechanics::compiled_payloads[i].bytes;
      if (bytes.size() < t7::object_header_size) continue;
      const auto code = t7::read32(bytes, 0x14), size = t7::read32(bytes, 0x30);
      if (code < t7::object_header_size || !size || code > bytes.size() || size > bytes.size() - code) continue;
      auto *cached = assets.cached(t7::map_companion_names[i], r->lease);
      if (!cached || !cached->buffer) continue;
      const auto base = reinterpret_cast<std::uintptr_t>(cached->buffer);
      if (base > (std::numeric_limits<std::uintptr_t>::max)() - code) continue;
      const auto begin = base + code;
      if (begin > (std::numeric_limits<std::uintptr_t>::max)() - size) continue;
      ranges[i] = {cached, begin, begin + size};
    }
    map_failure_publication publication;
    bool rejected = false;
    (void)assets.if_current(r->lease, [&] {
      if (!map_identity_owned(*r)) return;
      for (unsigned i = 0; i < ranges.size(); ++i) {
        const auto &range = ranges[i]; const auto &proof = map_proofs.helpers[i];
        if (range.asset && proof.generation == r->lease.generation && proof.asset == range.asset &&
            target >= range.begin && target < range.end) {
          rejected = true;
          publication = map_latch_failure_locked(*r); break;
        }
      }
    });
    map_report_failure(publication, "replacefunc_target_inside_owned_map_helper");
    return rejected;
  } catch (...) { return false; }
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

common_lifecycle common_sources;
bool common_current(const common_request &request) noexcept {
  return common_helper::native_session_current(request.session);
}
bool common_owned(const common_request &request) noexcept {
  return common_helper::native_session_owned(request.session);
}
void common_revoke(const common_request &request) noexcept {
  (void)common_helper::revoke_native_session(request.session);
}
std::optional<common_request>
capture_common_request(bool require_proof = true) {
  auto token = common_helper::capture_native_session();
  if (!token || !castle_zombies_mode())
    return {};
#ifdef BOIII_COOP_EZZ
  if (game::com::Com_IsRunningUILevel())
    return {};
#else
  if (game::Com_IsRunningUILevel())
    return {};
#endif
  const std::string actual(castle_current_map());
  const auto frozen = frozen_schema_map();
  if (actual != token->map || frozen != token->map ||
      token->capacity != frozen_schema_capacity())
    return {};
  const auto lease = assets.capture_generation(common_helper::logical_name);
  common_request request{lease, token->epoch, token->capacity, token->map,
                         token};
  if (lease.role != 0 ||
      !(require_proof ? common_current(request) : common_owned(request)))
    return {};
  return request;
}
script_asset *transform_powerup(script_asset *original) noexcept {
  std::optional<common_request> request;
  try {
    request = capture_common_request(false);
    if (!request)
      return original;
    if (auto *backing = common_sources.lookup_powerup(
            assets, *request, [&] { return common_owned(*request); }))
      return backing;
    if (!common_current(*request))
      return original;
    // Pending/missing common cannot authorize a stock clone for expanded play.
    if (!common_sources.lookup_common(assets, *request,
                                      [&] { return common_owned(*request); })) {
      common_revoke(*request);
      return original;
    }
    if (auto *cached = assets.cached(powerup_events::asset_name, request->vm))
      return cached;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) > powerup_events::input_limit) {
      common_revoke(*request);
      return original;
    }
    auto pristine = assets.template_for(original);
    if (!pristine) {
      const auto *begin =
          reinterpret_cast<const std::uint8_t *>(original->buffer);
      pristine = std::vector<std::uint8_t>(begin, begin + original->len);
    }
    auto changed = powerup_events::rewrite(*pristine, castle_digest);
    if (!changed.changed) {
      common_revoke(*request);
      return original;
    }
    std::string error;
    auto *retained =
        assets.retain_if(powerup_events::asset_name, std::move(changed.bytes),
                         error, request->vm, std::move(pristine),
                         [&] { return common_current(*request); });
    if (!retained) {
      common_revoke(*request);
      return original;
    }
    const auto base = reinterpret_cast<std::uintptr_t>(retained->buffer);
    if (!common_sources.pair_transformed_powerup(
            assets, *request, retained,
            {{base + 0x5070, base + 0x5198}, {base + 20888, base + 21432}},
            [&] { return common_current(*request); })) {
      common_revoke(*request);
      return original;
    }
    return retained;
  } catch (...) {
    if (request)
      common_revoke(*request);
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

bool handles_map_stock(const char *name) noexcept { return map_handles_stock(name); }
bool handles_map_helper(const char *name) noexcept { return map_handles_helper(name); }
script_asset *transform_map_stock(const char *name, script_asset *original) noexcept {
  return map_transform_stock(name, original);
}
script_asset *lookup_map_helper(const char *name) noexcept { return map_lookup_helper(name); }
bool pin_all_map_helpers(std::string &error) noexcept { return map_pin_all_helpers(error); }
script_asset *current_map_backing(const char *name) noexcept { return map_current_backing(name); }
bool map_mechanics_ready() noexcept { return map_ready(); }
std::string map_source_diagnostic() { return map_diagnostic(); }
bool reject_map_helper_detour(std::uintptr_t target) noexcept {
  return map_observe_helper_detour(target);
}

bool common_required() noexcept {
  try {
    return castle_zombies_mode() && frozen_schema_capacity() >= 5;
  } catch (...) {
    return false;
  }
}
std::optional<common_request> common_begin_load() noexcept {
  try {
    return capture_common_request();
  } catch (...) {
    return {};
  }
}
std::optional<common_binding> bind_common_loader(
    script_asset *selected, std::span<const std::uint8_t> compiled,
    std::span<const std::uint8_t> readable,
    std::span<const std::uint8_t> cleaned, bool readable_success) noexcept {
  try {
#ifdef BOIII_COOP_EZZ
    constexpr auto family = common_helper::family::ezz;
#else
    constexpr auto family = common_helper::family::cb;
#endif
    return common_helper::bind_loader_object(family, selected, compiled,
                                             readable, cleaned,
                                             readable_success, castle_digest);
  } catch (...) {
    return {};
  }
}
std::optional<common_seal> seal_common_loader(const common_binding &binding,
                                              script_asset *selected) noexcept {
  try {
    return common_helper::seal_selected_winner(binding, selected,
                                               castle_digest);
  } catch (...) {
    return {};
  }
}
bool preregister_common(const common_request &request,
                        const common_seal &seal) noexcept {
  try {
    std::string error;
    return common_sources.preregister(
               assets, request, seal, castle_digest, error,
               [&] { return common_current(request); }) != nullptr;
  } catch (...) {
    common_revoke(request);
    return false;
  }
}
void common_load_failed(const common_request &request) noexcept {
  common_revoke(request);
}
void common_load_returned(const common_request &request) noexcept {
  try {
    if (!common_sources.note_common_load_returned(
            assets, request, [&] { return common_current(request); }))
      common_revoke(request);
  } catch (...) {
    common_revoke(request);
  }
}
void common_finish_load(const common_request &request) noexcept {
  try {
    if (!common_sources.finish_loader(assets, request,
                                      [&] { return common_current(request); }))
      common_revoke(request);
  } catch (...) {
    common_revoke(request);
  }
}
bool common_source_pair_ready() noexcept {
  try {
    auto request = capture_common_request();
    return request && common_sources.ready(assets, *request, [&] {
      return common_current(*request);
    });
  } catch (...) {
    return false;
  }
}
script_asset *common_backing(const char *name) noexcept {
  try {
    if (t7::canonical_asset(bounded_path(name)) != common_helper::logical_name)
      return nullptr;
    auto request = capture_common_request(false);
    return request
               ? common_sources.lookup_common(
                     assets, *request, [&] { return common_owned(*request); })
               : nullptr;
  } catch (...) {
    return nullptr;
  }
}

script_asset *powerup_backing(const char *name) noexcept {
  try {
    if (t7::canonical_asset(bounded_path(name)) != powerup_events::asset_name)
      return nullptr;
    auto request = capture_common_request(false);
    return request
               ? common_sources.lookup_powerup(
                     assets, *request, [&] { return common_owned(*request); })
               : nullptr;
  } catch (...) {
    return nullptr;
  }
}

void observe_common_replacement(const char *name,
                                std::span<const std::uint8_t> compiled,
                                std::span<const std::uint8_t> readable,
                                std::span<const std::uint8_t> cleaned,
                                bool readable_success) noexcept {
  std::optional<common_request> request;
  try {
    if (t7::canonical_asset(bounded_path(name)) != common_helper::logical_name)
      return;
    // Loader workers need no engine calls or companion/runtime mutex here.
    auto token = common_helper::capture_native_session();
    if (!token)
      return;
    const auto lease = assets.capture_generation(common_helper::logical_name);
    request =
        common_request{lease, token->epoch, token->capacity, token->map, token};
    if (!common_owned(*request))
      return;
    auto *current = common_sources.lookup_common(
        assets, *request, [&] { return common_owned(*request); });
    if (!current)
      return; // Initial registry scans have not sealed a winner yet.
    const auto immutable = assets.template_for(current);
    bool identical = immutable &&
                     compiled.size() <= common_helper::common_limit &&
                     castle_digest(*immutable) == castle_digest(compiled);
#ifdef BOIII_COOP_EZZ
    identical = identical && readable_success &&
                readable.size() <= common_helper::common_limit &&
                cleaned.size() <= common_helper::common_limit &&
                castle_digest(readable) == common_helper::source_pin &&
                castle_digest(cleaned) == common_helper::source_pin;
#else
    (void)readable;
    (void)cleaned;
    (void)readable_success;
    identical =
        identical && castle_digest(compiled) == common_helper::cb_compiled_pin;
#endif
    if (!identical)
      common_revoke(*request);
    // An identical duplicate never clears a prior revocation or resets load
    // state.
  } catch (...) {
    if (request)
      common_revoke(*request);
  }
}
void observe_common_detour(std::uintptr_t target) noexcept {
  (void)map_observe_helper_detour(target);
  try {
    auto request = capture_common_request(false);
    if (!request)
      return;
    (void)common_sources.observe_detour(
        assets, target, [&] { return common_owned(*request); },
        [&](int) { common_revoke(*request); });
  } catch (...) {
  }
}
std::string common_source_pair_diagnostic() {
  auto request = capture_common_request(false);
  const bool verified = request && common_sources.ready(assets, *request, [&] {
    return common_current(*request);
  });
  return "{\"version\":1,\"verified\":" +
         std::string(verified ? "true" : "false") + ",\"map\":\"" +
         (request ? request->map : std::string{}) +
         "\",\"capacity\":" + std::to_string(request ? request->capacity : 0) +
         ",\"realm\":0,\"generation\":" +
         std::to_string(request ? request->vm.generation : 0) +
         ",\"revision\":" + std::to_string(companion_revision) +
         ",\"contract\":\"" + std::string(companion_contract) +
         "\",\"linked_body_authenticated\":false}";
}

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
  if (!client)
    common_helper::invalidate_native_session();
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
    const bool result = assets.pin(server, client, island, error) &&
        pin_all_map_helpers(error);
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
    // Sole producer for mapped stock modules, including the Stalingrad root anchor.
    if (map_known_stock(name)) return map_transform_stock(name, original);
    const auto path = bounded_path(name);
    if (t7::canonical_asset(path) == powerup_events::asset_name)
      return transform_powerup(original);
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
