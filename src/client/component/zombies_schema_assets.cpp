#include <std_include.hpp>

#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/schema_assets.hpp"
#include "zombies/schema_asset_store.hpp"
#include "zombies/schema_context_policy.hpp"

namespace zombies::coop::schema_assets {
namespace {
// Deliberately process-lifetime storage, independent of the custom-script
// allocator and map shutdown. The native VM may still hold or mutate pointers.
t7::schema_asset_store<script_asset> assets;
std::mutex client_context_mutex;
std::string client_context_map;

std::string_view bounded_path(const char* name) noexcept {
  if (!name) return {};
  for (std::size_t i = 0; i <= 255; ++i)
    if (!name[i]) return {name, i};
  return {};
}
void fail(std::string_view reason) noexcept {
  // Defer Com_Error to the existing main-frame gate, outside the asset/cache
  // locks and engine database call. Never unwind a native engine frame.
  diagnostic_event("schema-asset-failure");
  try { schema_asset_failure(reason); } catch (...) {}
}
bool active() noexcept {
  refresh_schema_session();
  const auto capacity = frozen_schema_capacity();
  return capacity >= 5 && capacity <= 8;
}
void sync_client_map(std::string_view name) {
  if (schema_assets_failed() || assets.exhausted()) return;
  if (!t7::canonical_asset(name).ends_with(".csc")) return;
  bool changed = false;
  (void)prepare_client_generation(frozen_schema_map(), false, changed);
}
void retained_event(std::string_view phase, std::size_t length) noexcept {
  diagnostic_event(phase, static_cast<std::int64_t>(length));
  try {
    const auto [count, bytes] = assets.retained();
    diagnostic_event("schema-asset-retained", static_cast<std::int64_t>(count),
        static_cast<std::int64_t>(bytes));
  } catch (...) {}
}
} // namespace

void begin_vm_generation(bool client) noexcept {
  try {
    if (assets.exhausted()) { fail("Schema asset store exhausted; restart required."); return; }
    if (client) {
      const std::lock_guard lock(client_context_mutex);
      if (!assets.begin_generation(true)) { fail("Schema asset store exhausted; restart required."); return; }
      client_context_map.clear();
    } else if (!assets.begin_generation(false)) { fail("Schema asset store exhausted; restart required."); return; }
    diagnostic_event(client ? "schema-client-vm-generation" : "schema-server-vm-generation");
  } catch (...) { fail("Could not establish a fresh script asset generation."); }
}
bool prepare_client_generation(std::string_view map, bool fresh_request, bool& changed) noexcept {
  changed = false;
  try {
    if (schema_assets_failed() || assets.exhausted()) return false;
    std::string next_map(map);
    const std::lock_guard lock(client_context_mutex);
    if (!client_map_generation_changes(client_context_map, map, fresh_request)) return true;
    if (!assets.begin_generation(true)) { fail("Schema asset store exhausted; restart required."); return false; }
    client_context_map.swap(next_map);
    changed = true;
    diagnostic_event(fresh_request ? "schema-client-request-generation" : "schema-client-map-generation");
    return true;
  } catch (...) { fail("Could not establish a client asset generation."); return false; }
}
void reset_client_session_generation() noexcept {
  try {
    const std::lock_guard lock(client_context_mutex);
    if (!assets.begin_generation(true)) return; // poison is permanent; retain current fallback identity
    client_context_map.clear();
    diagnostic_event("schema-client-frontend-reset");
  } catch (...) { fail("Could not retire client asset generation."); }
}

bool retention_exhausted() noexcept {
  try { return assets.exhausted(); } catch (...) { return true; }
}

bool pin_payloads(std::string_view server, std::string_view client, std::string& error) {
  try {
    const bool result = assets.pin(server, client, error);
    if (result) diagnostic_event("schema-payloads-pinned",
        static_cast<std::int64_t>(server.size()), static_cast<std::int64_t>(client.size()));
    return result;
  }
  catch (const std::exception& e) { error = e.what(); return false; }
  catch (...) { error = "Could not retain schema companion payloads."; return false; }
}

bool handles_companion(const char* name) noexcept {
  if (!active()) return false;
  return t7::fixed_companion_role(bounded_path(name)).has_value();
}

script_asset* lookup_companion(const char* name) noexcept {
  try {
    if (!handles_companion(name)) return nullptr;
    sync_client_map(bounded_path(name));
    const auto lease = assets.capture_generation(bounded_path(name));
    if (auto* existing = assets.cached(bounded_path(name), lease)) return existing;
    std::string error;
    auto* result = assets.companion(bounded_path(name), error, lease);
    if (!result) {
      fail(error);
      result = assets.emergency_companion(bounded_path(name));
    }
    else retained_event(bounded_path(name) == t7::server_companion ?
        "schema-companion-gsc" : "schema-companion-csc", static_cast<std::size_t>(result->len));
    return result;
  } catch (...) {
    fail("Could not allocate fresh schema companion backing.");
    try { return assets.emergency_companion(bounded_path(name)); } catch (...) { return nullptr; }
  }
}

script_asset* transform(const char* name, script_asset* original) noexcept {
  if (!active()) return original;
  try {
    const auto path = bounded_path(name);
    if (!t7::is_schema_anchor(path, frozen_schema_map())) return original;
    sync_client_map(path);
    const auto lease = assets.capture_generation(path);
    if (auto* cached = assets.cached(path, lease)) return cached;
    if (schema_assets_failed()) return original;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) > t7::retention_limit) {
      fail("Missing or oversized raw schema bootstrap anchor.");
      return original;
    }
    const auto saved_template = assets.template_for(original);
    const auto object = saved_template ? std::span<const std::uint8_t>(*saved_template) :
        std::span(reinterpret_cast<const std::uint8_t*>(original->buffer), static_cast<std::size_t>(original->len));
    auto cloned = t7::clone_with_include(object, t7::companion_include);
    if (cloned.result == t7::include_result::already_present) {
      // Isolate each new generation from previously linked/mutated backing.
      cloned.bytes.assign(object.begin(), object.end());
    } else if (cloned.result != t7::include_result::inserted) {
      fail("Schema bootstrap anchor is not a validated raw T7 include object.");
      return original;
    }
    const auto embedded = t7::bounded_name(object, t7::read32(object, 0x34));
    if (!embedded || t7::canonical_script(*embedded) != t7::canonical_script(path)) {
      fail("Schema bootstrap anchor logical name differs from its object.");
      return original;
    }
    std::string error;
    auto* result = assets.retain(path, std::move(cloned.bytes), error, lease);
    if (!result) fail(error);
    else retained_event(path.ends_with(".gsc") ? "schema-anchor-gsc" : "schema-anchor-csc",
        static_cast<std::size_t>(result->len));
    return result ? result : original;
  } catch (...) {
    fail("Could not clone schema bootstrap anchor.");
    return original;
  }
}
} // namespace zombies::coop::schema_assets
