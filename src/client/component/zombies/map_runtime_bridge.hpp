#pragma once
// Server map-mechanics bridge. The caller owns one immutable descriptor table and
// routes these stock paths here BEFORE any generic anchor producer.
#include "map_import_binding.hpp"
#include "map_companion_names.hpp"
#include "map_failure_diagnostic.hpp"
#include <limits>

namespace zombies::coop::map_runtime_bridge {
namespace binding = zombies::coop::map_import_binding;
namespace t7 = zombies::coop::t7;
using helper_mask = std::uint64_t;
inline constexpr helper_mask all_helpers = (helper_mask{1} << t7::map_companion_names.size()) - 1;
using contract_factory = binding::contract (*)();
struct descriptor {
  std::string_view map, path;
  contract_factory factory{};
  helper_mask helpers{}; // includes every transitive map helper dependency
  bool add_core_fields_include{}; // compose Stalingrad root anchor once
  bool required_for_map{true}; // false: required only after this path is requested
};
template <class Lease> struct request {
  std::string_view map;
  int capacity{};
  Lease lease{};
};
enum class outcome { untouched, cached, transformed, helper, emergency, declined };
template <class Asset> struct operation {
  outcome state{outcome::declined};
  Asset *asset{}; // declined has no asset; emergency is for failure unwinding only
  const char *reason{"bridge_declined"};
  std::string detail;
  map_diagnostics::source_observation source{}; // metadata from the existing hash callback only
  bool okay() const noexcept {
    return state == outcome::cached || state == outcome::transformed || state == outcome::helper;
  }
};
struct readiness { bool required{}, ready{}; const char *reason{}; };

// Every read/write of these fields MUST occur inside the SAME store's
// if_current callback. No additional mutex or atomic is needed. Do not share
// this table between stores. Descriptors stay immutable for process lifetime.
template <class Asset, std::size_t N> struct receipts {
  enum class stage { empty, building, ready, declined };
  struct stock_receipt {
    std::uint64_t generation{}, ticket{};
    const descriptor *identity{};
    Asset *asset{};
    stage state{stage::empty};
    bool requested{};
  };
  struct helper_receipt { std::uint64_t generation{}; Asset *asset{}; };
  std::array<stock_receipt, N> stock{};
  std::array<helper_receipt, t7::map_companion_names.size()> helpers{};
};

namespace detail {
inline bool expanded(int capacity) noexcept { return capacity >= 5 && capacity <= 8; }
template <class Request> bool valid_server(const Request &r) noexcept {
  return expanded(r.capacity) && r.lease.role == 0u && r.lease.generation != 0;
}
inline std::optional<std::string> canonical_server_asset(std::string_view path) {
  if (path.size() < 4) return {};
  std::string extension(path.substr(path.size() - 4));
  for (auto &ch : extension) if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch + ('a' - 'A'));
  if (extension != ".gsc") return {};
  auto name = t7::canonical_script(path);
  return name ? std::optional<std::string>(*name + ".gsc") : std::nullopt;
}
inline bool valid_row(const descriptor &row) {
  const auto path = canonical_server_asset(row.path);
  return path && *path == row.path && row.factory &&
         row.helpers && !(row.helpers & ~all_helpers);
}
inline bool applies(const descriptor &row, std::string_view map) noexcept {
  return row.map.empty() || row.map == map;
}
inline helper_mask dependencies(std::span<const descriptor> rows, std::string_view map) noexcept {
  helper_mask mask = 0;
  for (const auto &row : rows) if (applies(row, map)) mask |= row.helpers;
  return mask;
}
template <class Store, class Request, class Guard, class Action>
bool current(Store &store, const Request &r, Guard &guard, Action action) {
  bool accepted = false;
  (void)store.if_current(r.lease, [&] {
    if (guard()) { action(); accepted = true; }
  });
  return accepted;
}
template <class Asset> operation<Asset> declined(const char *reason, std::string detail = {}) {
  return {outcome::declined, nullptr, reason, std::move(detail)};
}
template <class Store, class Request, class Guard>
auto guarded_emergency(Store &store, std::string_view name, const Request &r, Guard &guard) {
  // NEVER fall back to the unguarded v3 overload: stale work must not exhaust
  // its successor VM. v4 checks lease+native guard before its exhaustion latch.
  if constexpr (requires { store.emergency_map_companion(name, r.lease, guard); })
    return store.emergency_map_companion(name, r.lease, guard);
  else {
    using pointer = decltype(store.emergency_map_companion(name));
    return pointer{};
  }
}
} // namespace detail

// Pin all fixed server code objects before a VM exposes any of their pointers.
// Failure may leave earlier immutable reservations pinned; no readiness is
// published and an identical retry is safe. Payload identity is store-owned.
template <class Store>
bool pin_helpers(Store &store,
                 std::span<const std::string_view, t7::map_companion_names.size()> payloads,
                 std::string &error) noexcept {
  try {
    for (unsigned index = 0; index < payloads.size(); ++index)
      if (!store.pin_map_companion(index, payloads[index], error)) return false;
    return true;
  } catch (...) { return false; }
}

template <class Store, class Asset, std::size_t N, class Request, class Guard>
operation<Asset> route_stock_worker(Store &store, receipts<Asset, N> &proofs,
    std::span<const descriptor> rows, const Request &r, std::string_view path,
    Asset *original, binding::hash_function hash, Guard guard,
    map_diagnostics::source_observation *observation) noexcept {
  std::optional<std::size_t> chosen;
  std::uint64_t ticket = 0;
  const auto release = [&]() noexcept {
    if (!chosen || !ticket) return;
    try {
      (void)store.if_current(r.lease, [&] {
        auto &entry = proofs.stock[*chosen];
        if (entry.generation == r.lease.generation && entry.ticket == ticket &&
            entry.state == receipts<Asset, N>::stage::building)
          entry.state = receipts<Asset, N>::stage::declined;
      });
    } catch (...) {}
  };
  try {
    if (rows.size() != N) return detail::declined<Asset>("descriptor_count");
    const auto canonical = detail::canonical_server_asset(path);
    if (!canonical) return {outcome::untouched, original, "not_a_server_script", {}};
    for (std::size_t i = 0; i < rows.size(); ++i)
      if (detail::applies(rows[i], r.map) && rows[i].path == *canonical) {
        if (chosen) return detail::declined<Asset>("duplicate_descriptor");
        chosen = i;
      }
    if (!chosen || r.capacity < 5) return {outcome::untouched, original, "not_required", {}};
    const auto mark_declined = [&] {
      if (!detail::valid_server(r)) return;
      (void)detail::current(store, r, guard, [&] {
        auto &entry = proofs.stock[*chosen];
        entry.generation = r.lease.generation; entry.identity = &rows[*chosen];
        entry.requested = true; entry.asset = nullptr; entry.state = receipts<Asset, N>::stage::declined;
      });
    };
    if (!detail::valid_server(r) || !detail::valid_row(rows[*chosen])) {
      mark_declined();
      return detail::declined<Asset>("invalid_request_or_descriptor");
    }
    if (store.exhausted()) { mark_declined(); return detail::declined<Asset>("store_exhausted"); }
    const auto &row = rows[*chosen];
    auto *cached = store.cached(row.path, r.lease);
    bool owned = false, claimed = false;
    const bool live = detail::current(store, r, guard, [&] {
      auto &entry = proofs.stock[*chosen];
      if (entry.generation != r.lease.generation) {
        entry.generation = r.lease.generation; entry.state = receipts<Asset, N>::stage::empty;
        entry.identity = &row; entry.asset = nullptr; entry.requested = false;
      }
      entry.requested = true;
      if (cached) {
        owned = entry.generation == r.lease.generation && entry.identity == &row &&
                entry.state == receipts<Asset, N>::stage::ready && entry.asset == cached;
        if (!owned) entry.state = receipts<Asset, N>::stage::declined;
      } else if (entry.state != receipts<Asset, N>::stage::building) {
        if (entry.ticket == (std::numeric_limits<std::uint64_t>::max)()) return;
        ticket = ++entry.ticket; entry.generation = r.lease.generation;
        entry.identity = &row; entry.asset = nullptr;
        entry.state = receipts<Asset, N>::stage::building; claimed = true;
      }
    });
    if (!live) return detail::declined<Asset>("stale_context");
    if (cached) return owned ? operation<Asset>{outcome::cached, cached, "verified_cache", {}}
                             : detail::declined<Asset>("unowned_stock_cache");
    if (!claimed) return detail::declined<Asset>("producer_already_building");
    // Contract construction and every allocation/hash/transform happen outside
    // the store lock. Guard callbacks below only sample native context.
    const auto contract = row.factory();
    const auto dependency = t7::map_companion_identity(contract.companion_include + ".gsc");
    if (contract.logical_name != row.path || !dependency ||
        !(row.helpers & (helper_mask{1} << *dependency))) {
      release(); return detail::declined<Asset>("contract_descriptor_mismatch");
    }
    auto pristine = original ? store.template_for(original) : std::nullopt;
    if (!pristine) {
      if (!original || !original->buffer || original->len <= 0 ||
          static_cast<std::size_t>(original->len) > binding::input_limit) {
        release(); return detail::declined<Asset>("missing_or_oversized_original");
      }
      const auto *begin = reinterpret_cast<const std::uint8_t *>(original->buffer);
      pristine = std::vector<std::uint8_t>(begin, begin + original->len);
    }
    auto changed = binding::transform(*pristine, contract, hash, observation);
    if (!changed.changed) { release(); return detail::declined<Asset>("raw_contract_declined", std::move(changed.reason)); }
    if (row.add_core_fields_include) {
      auto included = t7::clone_with_include(changed.bytes, "scripts/zm/eroded_coop_fields");
      if (included.result == t7::include_result::inserted) changed.bytes = std::move(included.bytes);
      else if (included.result != t7::include_result::already_present) {
        release(); return detail::declined<Asset>("core_include_declined");
      }
      if (changed.bytes.size() > binding::output_limit) {
        release(); return detail::declined<Asset>("composed_output_limit");
      }
    }
    std::string error;
    auto *retained = store.retain_if(row.path, std::move(changed.bytes), error,
                                     r.lease, std::move(pristine), guard);
    if (!retained) { release(); return detail::declined<Asset>("retention_declined", std::move(error)); }
    bool published = false;
    (void)detail::current(store, r, guard, [&] {
      auto &entry = proofs.stock[*chosen];
      if (entry.generation == r.lease.generation && entry.ticket == ticket &&
          entry.identity == &row && entry.state == receipts<Asset, N>::stage::building) {
        entry.asset = retained; entry.state = receipts<Asset, N>::stage::ready; published = true;
      }
    });
    if (!published) { release(); return detail::declined<Asset>("context_changed_before_receipt"); }
    return {outcome::transformed, retained, "retained_from_pristine", {}};
  } catch (...) { release(); return {outcome::declined, nullptr, "bridge_exception", {}}; }
}

// Observe the exact pristine bytes passed to the existing digest gate. No extra
// hashing/engine call is introduced, and no diagnostic work occurs under store
// callbacks. Failed hashing explicitly leaves the digest unavailable.
template <class Store, class Asset, std::size_t N, class Request, class Guard>
operation<Asset> route_stock(Store &store, receipts<Asset, N> &proofs,
    std::span<const descriptor> rows, const Request &r, std::string_view path,
    Asset *original, binding::hash_function hash, Guard guard) noexcept {
  map_diagnostics::source_observation observed{};
  if (original && original->len > 0)
    observed.bytes = static_cast<std::uint64_t>(original->len);
  try {
    auto result = route_stock_worker(store, proofs, rows, r, path, original,
                                     hash, std::move(guard), &observed);
    result.source = observed;
    return result;
  } catch (...) {
    return {outcome::declined, nullptr, "bridge_exception", {}, observed};
  }
}

template <class Store, class Asset, std::size_t N, class Request, class Guard>
operation<Asset> lookup_helper(Store &store, receipts<Asset, N> &proofs,
    std::span<const descriptor> rows, const Request &r, std::string_view path,
    Asset *original, Guard guard) noexcept {
  const auto index = t7::map_companion_identity(path);
  if (!index || !(detail::dependencies(rows, r.map) & (helper_mask{1} << *index)) || r.capacity < 5)
    return {outcome::untouched, original, "helper_not_required", {}};
  if (rows.size() != N || !detail::valid_server(r)) return detail::declined<Asset>("invalid_helper_request");
  const auto emergency = [&](const char *reason, std::string detail_text = {}) noexcept -> operation<Asset> {
    try {
      auto *backing = detail::guarded_emergency(store, path, r, guard);
      return {backing ? outcome::emergency : outcome::declined, backing, reason, std::move(detail_text)};
    } catch (...) { return {outcome::declined, nullptr, "emergency_declined", {}}; }
  };
  try {
    if (!detail::current(store, r, guard, [] {})) return detail::declined<Asset>("stale_helper_context");
    std::string error;
    auto *asset = store.map_companion(t7::map_companion_names[*index], error, r.lease);
    if (!asset) return emergency("helper_retention_declined", std::move(error));
    if (!detail::current(store, r, guard, [&] {
          proofs.helpers[*index] = {r.lease.generation, asset};
        })) return detail::declined<Asset>("helper_context_changed");
    return {outcome::helper, asset, "verified_pinned_helper", {}};
  } catch (...) { return emergency("helper_exception"); }
}

template <class Store, class Asset, std::size_t N, class Request, class Guard>
readiness ready(Store &store, receipts<Asset, N> &proofs,
    std::span<const descriptor> rows, const Request &r, Guard guard) noexcept {
  try {
    if (r.capacity < 5) return {false, true, "not_required"};
    if (rows.size() != N || !detail::valid_server(r))
      return {true, false, "invalid_readiness_request"};
    helper_mask mask = 0;
    std::array<bool, N> needed{};
    bool required = false;
    if (!detail::current(store, r, guard, [&] {
      for (std::size_t i = 0; i < N; ++i) if (detail::applies(rows[i], r.map)) {
        const auto &entry = proofs.stock[i];
        needed[i] = rows[i].required_for_map ||
                    (entry.generation == r.lease.generation && entry.requested);
        if (needed[i]) { required = true; mask |= rows[i].helpers; }
      }
    })) return {true, false, "readiness_context_changed"};
    if (!required) return {false, true, "not_required"};
    if (mask & ~all_helpers) return {true, false, "invalid_readiness_dependencies"};
    if (store.exhausted()) return {true, false, "store_exhausted"};
    std::array<Asset *, N> stock{};
    std::array<Asset *, t7::map_companion_names.size()> helpers{};
    for (std::size_t i = 0; i < N; ++i) if (needed[i]) {
      if (!detail::valid_row(rows[i])) return {true, false, "invalid_descriptor"};
      stock[i] = store.cached(rows[i].path, r.lease);
    }
    for (unsigned i = 0; i < helpers.size(); ++i)
      if (mask & (helper_mask{1} << i)) helpers[i] = store.cached(t7::map_companion_names[i], r.lease);
    bool complete = true;
    const bool live = detail::current(store, r, guard, [&] {
      helper_mask final_mask = 0;
      for (std::size_t i = 0; i < N; ++i) if (detail::applies(rows[i], r.map)) {
        const auto &entry = proofs.stock[i];
        const bool now_needed = rows[i].required_for_map ||
            (entry.generation == r.lease.generation && entry.requested);
        if (!now_needed) continue;
        final_mask |= rows[i].helpers;
        if (!stock[i] || entry.generation != r.lease.generation || entry.identity != &rows[i] ||
            entry.state != receipts<Asset, N>::stage::ready || entry.asset != stock[i]) complete = false;
      }
      for (unsigned i = 0; i < helpers.size(); ++i) if (final_mask & (helper_mask{1} << i)) {
        const auto &entry = proofs.helpers[i];
        if (!helpers[i] || entry.generation != r.lease.generation || entry.asset != helpers[i]) complete = false;
      }
    });
    if (!live || store.exhausted()) return {true, false, "readiness_context_changed"};
    return {true, complete, complete ? "map_sources_retained" : "map_sources_pending"};
  } catch (...) { return {true, false, "readiness_exception"}; }
}

// Publication callbacks must only update native failure state. Do not enter
// the store, engine, linker, or another mutex from guard/action callbacks.
template <class Store, class Request, class Guard, class Action>
bool publish_error(Store &store, const Request &r, Guard guard, Action action) noexcept {
  try { return detail::current(store, r, guard, action); }
  catch (...) { return false; }
}
} // namespace zombies::coop::map_runtime_bridge
