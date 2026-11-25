#include <std_include.hpp>

#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/schema_assets.hpp"
#include "zombies/schema_asset_store.hpp"

namespace zombies::coop::schema_assets {
namespace {
// Deliberately process-lifetime storage, independent of the custom-script
// allocator and map shutdown. The native VM may still hold or mutate pointers.
t7::schema_asset_store<script_asset> assets;

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
void retained_event(std::string_view phase, std::size_t length) noexcept {
  diagnostic_event(phase, static_cast<std::int64_t>(length));
  try {
    const auto [count, bytes] = assets.retained();
    diagnostic_event("schema-asset-retained", static_cast<std::int64_t>(count),
        static_cast<std::int64_t>(bytes));
  } catch (...) {}
}
} // namespace

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
  const auto path = bounded_path(name);
  return path == t7::server_companion || path == t7::client_companion;
}

script_asset* lookup_companion(const char* name) noexcept {
  try {
    if (!handles_companion(name) || schema_assets_failed()) return nullptr;
    std::string error;
    auto* result = assets.companion(bounded_path(name), error);
    if (!result) fail(error);
    else retained_event(bounded_path(name) == t7::server_companion ?
        "schema-companion-gsc" : "schema-companion-csc", static_cast<std::size_t>(result->len));
    return result;
  } catch (...) {
    fail("Could not allocate fresh schema companion backing.");
    return nullptr;
  }
}

script_asset* transform(const char* name, script_asset* original) noexcept {
  if (!active()) return original;
  try {
    const auto path = bounded_path(name);
    if (!t7::is_schema_anchor(path, frozen_schema_map())) return original;
    if (schema_assets_failed()) return nullptr;
    if (!original || !original->buffer || original->len <= 0 ||
        static_cast<std::size_t>(original->len) > t7::retention_limit) {
      fail("Missing or oversized raw schema bootstrap anchor.");
      return nullptr;
    }
    const auto object = std::span(reinterpret_cast<const std::uint8_t*>(original->buffer),
        static_cast<std::size_t>(original->len));
    auto cloned = t7::clone_with_include(object, t7::companion_include);
    if (cloned.result == t7::include_result::already_present) {
      // Still isolate every lookup from a previously linked/mutated return.
      cloned.bytes.assign(object.begin(), object.end());
    } else if (cloned.result != t7::include_result::inserted) {
      fail("Schema bootstrap anchor is not a validated raw T7 include object.");
      return nullptr;
    }
    const auto embedded = t7::bounded_name(object, t7::read32(object, 0x34));
    if (!embedded || t7::canonical_script(*embedded) != t7::canonical_script(path)) {
      fail("Schema bootstrap anchor logical name differs from its object.");
      return nullptr;
    }
    std::string error;
    auto* result = assets.retain(path, std::move(cloned.bytes), error);
    if (!result) fail(error);
    else retained_event(path.ends_with(".gsc") ? "schema-anchor-gsc" : "schema-anchor-csc",
        static_cast<std::size_t>(result->len));
    return result;
  } catch (...) {
    fail("Could not clone schema bootstrap anchor.");
    return nullptr;
  }
}
} // namespace zombies::coop::schema_assets
