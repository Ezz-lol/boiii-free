#include "schema_asset_store.hpp"
#include "map_runtime_bridge.hpp"
#include <functional>
#include <iostream>
#include <latch>
#include <stdexcept>
#include <thread>

namespace b = zombies::coop::map_runtime_bridge;
namespace m = zombies::coop::map_import_binding;
namespace t7 = zombies::coop::t7;
static unsigned checks = 0;
static void check(bool okay, const char *label) { ++checks; if (!okay) throw std::runtime_error(label); }
struct asset {
  const char *name{};
#ifdef BOIII_BRIDGE_EZZ
  std::size_t len{};
#else
  std::int32_t len{};
#endif
  const char *buffer{};
};
using store_type = t7::schema_asset_store<asset>;
using request_type = b::request<store_type::generation_lease>;
using proofs_type = b::receipts<asset, 8>;
template <class Store, class Request, class Guard>
inline constexpr bool requires_guarded_emergency = requires(Store &store, Request r, Guard guard) {
  store.emergency_map_companion(t7::map_companion_names[1], r.lease, guard);
};
static std::function<void()> hash_hook, factory_hook;
static m::digest witness(std::span<const std::uint8_t> bytes) {
  if (hash_hook) { auto hook = std::move(hash_hook); hash_hook = {}; hook(); }
  m::digest digest{}; std::uint32_t h = 0x811c9dc5;
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    h = (h ^ bytes[i]) * 0x1000193; digest[i % 32] ^= static_cast<std::uint8_t>(h >> ((i % 4) * 8));
  }
  return digest;
}
static void put16(std::span<std::uint8_t> bytes, std::size_t at, std::uint16_t value) {
  bytes[at] = static_cast<std::uint8_t>(value); bytes[at + 1] = static_cast<std::uint8_t>(value >> 8);
}
static const std::array<std::string_view, 8> module_paths{
 "scripts/zm/zm_temple_pack_a_punch.gsc", "scripts/zm/zm_castle_ee_bossfight.gsc",
 "scripts/zm/zm_factory_teleporter.gsc", "scripts/zm/zm_theater_teleporter.gsc",
 "scripts/zm/zm_stalingrad_dragon.gsc", "scripts/zm/zm_stalingrad_wearables.gsc",
 "scripts/zm/zm_stalingrad.gsc", "scripts/zm/bgbs/_zm_bgb_anywhere_but_here.gsc"};
static const std::array<std::string_view, 8> module_maps{
 "zm_temple", "zm_castle", "zm_factory", "zm_theater", "zm_stalingrad", "zm_stalingrad", "zm_stalingrad", "zm_stalingrad"};
static constexpr unsigned optional_helper = t7::map_companion_names.size() > 6 ? 6u : 5u;
static constexpr b::helper_mask gorod_helpers = 32 | (b::helper_mask{1} << optional_helper);
static const std::array<unsigned, 8> helper_index{1, 2, 3, 4, 5, 5, 5, optional_helper};
static std::array<std::vector<std::uint8_t>, 8> module_data;
static std::array<m::contract, 8> contracts;
static std::array<asset, 8> originals;
static std::array<std::string, t7::map_companion_names.size()> payload_data;
static std::array<std::string_view, t7::map_companion_names.size()> payload_views;
static m::contract fixture_factory(unsigned index) {
  if (factory_hook) { auto hook = std::move(factory_hook); factory_hook = {}; hook(); }
  return contracts[index];
}
template <unsigned Index> static m::contract factory() { return fixture_factory(Index); }
static const std::array<b::descriptor, 8> descriptors{{
 {module_maps[0], module_paths[0], factory<0>, 3, false},
 {module_maps[1], module_paths[1], factory<1>, 5, false},
 {module_maps[2], module_paths[2], factory<2>, 8, false},
 {module_maps[3], module_paths[3], factory<3>, 16, false},
 {module_maps[4], module_paths[4], factory<4>, gorod_helpers, false},
 {module_maps[5], module_paths[5], factory<5>, gorod_helpers, false},
 {module_maps[6], module_paths[6], factory<6>, gorod_helpers, true},
 {"", module_paths[7], factory<7>, b::helper_mask{1} << optional_helper, false, false}}};
static const auto rows = std::span<const b::descriptor>(descriptors);
static std::vector<std::uint8_t> object(std::string_view name, bool module) {
  std::vector<std::uint8_t> bytes(0x400);
  std::copy(t7::object_magic.begin(), t7::object_magic.end(), bytes.begin());
  t7::write32(bytes, 8, 0x12345678); t7::write32(bytes, 0x14, 0x180);
  t7::write32(bytes, 0x30, 0x80); t7::write32(bytes, 0x34, 0x60);
  std::copy(name.begin(), name.end(), bytes.begin() + 0x60);
  if (module) {
    t7::write32(bytes, 0x20, 0x300); put16(bytes, 0x3a, 1);
    t7::write32(bytes, 0x300, 0xabcdef01); t7::write32(bytes, 0x304, 0x180);
    t7::write32(bytes, 0x308, 0x22222222); t7::write32(bytes, 0x30c, 0x33333333);
    t7::write32(bytes, 0x24, 0x340); put16(bytes, 0x3c, 1);
    t7::write32(bytes, 0x340, 0x44444444); t7::write32(bytes, 0x344, 0x33333333);
    put16(bytes, 0x348, 1); bytes[0x34b] = 0x24; t7::write32(bytes, 0x34c, 0x1a0);
    put16(bytes, 0x1a0, 0x1234);
  }
  return bytes;
}
static void initialize() {
  for (unsigned i = 0; i < module_data.size(); ++i) {
    auto &bytes = module_data[i]; bytes = object(module_paths[i], true);
    auto &c = contracts[i]; c.independently_verified = true;
    c.logical_name = module_paths[i]; c.sha256 = witness(bytes); c.object_size = bytes.size();
    c.source_crc = 0x12345678; c.code = 0x180; c.code_size = 0x80;
    c.companion_include = std::string(t7::map_companion_names[helper_index[i]]);
    c.companion_include.resize(c.companion_include.size() - 4);
    m::call_site site{}; site.owner = {0xabcdef01, 0x180, 0x200, 0x22222222, 0x33333333, 0, 0};
    site.reference = 0x1a0; site.opcode = 0x1234; site.instruction.assign(bytes.begin() + 0x1a0, bytes.begin() + 0x1b0);
    c.bindings = {{{0x44444444, 0x33333333, 0, 0x24}, {0x55555555, 0x66666666, 0, 4}, {std::move(site)}}};
    originals[i] = {module_paths[i].data(), static_cast<decltype(asset::len)>(bytes.size()), reinterpret_cast<const char *>(bytes.data())};
  }
  for (unsigned i = 0; i < payload_data.size(); ++i) {
    const auto bytes = object(t7::map_companion_names[i], false);
    payload_data[i].assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    payload_views[i] = payload_data[i];
  }
}
static request_type capture(store_type &store, std::string_view map, int capacity = 8) {
  return {map, capacity, store.capture_generation("scripts/zm/__bridge.gsc")};
}
static void pin(store_type &store) {
  std::string error; check(b::pin_helpers(store, std::span<const std::string_view, t7::map_companion_names.size()>(payload_views), error), error.c_str());
}
static bool has_include(const asset *value, std::string_view dependency) {
  const auto bytes = std::span(reinterpret_cast<const std::uint8_t *>(value->buffer), static_cast<std::size_t>(value->len));
  const auto table = t7::read32(bytes, 0x0c);
  for (unsigned i = 0; i < bytes[0x44]; ++i) {
    const auto name = t7::bounded_name(bytes, t7::read32(bytes, table + i * 4));
    if (name && *name == dependency) return true;
  }
  return false;
}
static void matrix() {
  for (const auto map : {"zm_temple", "zm_castle", "zm_factory", "zm_theater", "zm_stalingrad"})
    for (int capacity = 5; capacity <= 8; ++capacity) {
      store_type store; proofs_type proofs; pin(store); const auto r = capture(store, map, capacity);
      auto guard = [] { return true; };
      auto status = b::ready(store, proofs, rows, r, guard);
      check(status.required && !status.ready, "missing modules/dependencies pending");
      unsigned expected_modules = 0;
      for (unsigned i = 0; i < descriptors.size(); ++i) if (descriptors[i].map == map) {
        ++expected_modules;
        auto changed = b::route_stock(store, proofs, rows, r, module_paths[i], &originals[i], witness, guard);
        check(changed.state == b::outcome::transformed && changed.asset, changed.reason);
        check(store.template_for(changed.asset) == std::optional{module_data[i]}, "immutable raw template retained");
        check(has_include(changed.asset, contracts[i].companion_include), "entry include composed");
        check(has_include(changed.asset, "scripts/zm/eroded_coop_fields") == descriptors[i].add_core_fields_include, "only Stalingrad root composes core include");
        auto *visible = const_cast<char *>(changed.asset->buffer); visible[0x1a0] ^= 1;
        auto repeated = b::route_stock(store, proofs, rows, r, module_paths[i], changed.asset, witness, guard);
        check(repeated.state == b::outcome::cached && repeated.asset == changed.asset, "same VM identity despite linker mutation");
      }
      check(expected_modules == (std::string_view(map) == "zm_stalingrad" ? 3u : 1u), "all exact mandatory descriptors counted");
      check(!b::ready(store, proofs, rows, r, guard).ready, "stock receipts alone cannot ready");
      const auto mask = b::detail::dependencies(rows, map);
      for (unsigned i = 0; i < payload_views.size(); ++i) {
        auto helper = b::lookup_helper(store, proofs, rows, r, t7::map_companion_names[i], static_cast<asset *>(nullptr), guard);
        if (mask & (b::helper_mask{1} << i)) check(helper.state == b::outcome::helper && helper.asset, helper.reason);
        else check(helper.state == b::outcome::untouched && !helper.asset, "unrelated map helper untouched");
      }
      check(b::ready(store, proofs, rows, r, guard).ready, "all current map objects and deps ready");
      auto wrong = r; wrong.capacity = 9;
      check(!b::ready(store, proofs, rows, wrong, guard).ready, "invalid expanded capacity cannot ready");
      check(!b::ready(store, proofs, rows, r, [] { return false; }).ready, "native mismatch revokes readiness");
      check(store.begin_generation(true), "client generation begins");
      check(b::ready(store, proofs, rows, r, guard).ready, "client VM does not revoke server proof");
    }
}
static void generations_and_routing() {
  store_type store; proofs_type proofs; pin(store);
  auto guard = [] { return true; }; const auto old = capture(store, "zm_stalingrad");
  auto first = b::route_stock(store, proofs, rows, old, module_paths[6], &originals[6], witness, guard);
  check(first.okay(), first.reason); const_cast<char *>(first.asset->buffer)[0x1a0] ^= 1;
  check(store.begin_generation(false), "new server VM"); const auto fresh = capture(store, "zm_stalingrad");
  check(!b::ready(store, proofs, rows, fresh, guard).ready, "old receipts not successor authority");
  auto next = b::route_stock(store, proofs, rows, fresh, module_paths[6], first.asset, witness, guard);
  check(next.state == b::outcome::transformed && next.asset != first.asset, "old retained asset restored from pristine in fresh backing");
  check(next.asset->buffer != first.asset->buffer, "old and new backing independent");
  check(store.template_for(next.asset) == std::optional{module_data[6]}, "raw template not prior transformed output");
  auto stale = b::route_stock(store, proofs, rows, old, module_paths[6], first.asset, witness, guard);
  check(stale.state == b::outcome::declined && !stale.asset, "stale stock work inert");
  bool published = false;
  check(!b::publish_error(store, old, guard, [&] { published = true; }) && !published, "stale error cannot poison successor");
  check(b::publish_error(store, fresh, guard, [&] { published = true; }) && published, "current failure publication guarded");
  auto stale_helper = b::lookup_helper(store, proofs, rows, old, t7::map_companion_names[5], static_cast<asset *>(nullptr), guard);
  check(stale_helper.state == b::outcome::declined && !store.exhausted(), "stale helper cannot exhaust successor");
  auto other = capture(store, "zm_zod");
  check(!b::ready(store, proofs, rows, other, guard).required, "other maps unchanged");
  auto bgb_other = b::route_stock(store, proofs, rows, other, module_paths[7], &originals[7], witness, guard);
  check(bgb_other.state == b::outcome::transformed && bgb_other.asset, "global optional BGB routed when requested in owned expanded map");
  check(!b::ready(store, proofs, rows, other, guard).ready, "requested optional BGB demands helper receipt");
  check(b::lookup_helper(store, proofs, rows, other, t7::map_companion_names[optional_helper], static_cast<asset *>(nullptr), guard).okay(), "optional global helper retained");
  check(b::ready(store, proofs, rows, other, guard).ready, "custom map ready after requested optional module and helper");
  check(b::route_stock(store, proofs, rows, other, module_paths[6], &originals[6], witness, guard).state == b::outcome::untouched, "seven map-specific descriptors remain exact-map only");
  auto stock = fresh; stock.capacity = 4;
  check(b::route_stock(store, proofs, rows, stock, module_paths[6], &originals[6], witness, guard).state == b::outcome::untouched, "configured stock unchanged");
  check(b::route_stock(store, proofs, rows, fresh, "scripts/zm/zm_stalingrad.csc", &originals[6], witness, guard).state == b::outcome::untouched, "client script never matched to server path");
}
static void transitive_dependencies() {
  if constexpr (optional_helper == 5) return;
  store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_stalingrad");
  auto guard = [] { return true; };
  for (unsigned index : {4u, 5u, 6u})
    check(b::route_stock(store, proofs, rows, r, module_paths[index], &originals[index], witness, guard).okay(), "Gorod mandatory module retained");
  check(b::lookup_helper(store, proofs, rows, r, t7::map_companion_names[5], static_cast<asset *>(nullptr), guard).okay(), "Gorod direct helper retained");
  check(!b::ready(store, proofs, rows, r, guard).ready, "Gorod direct helper alone misses shared BGB dependency");
  check(b::lookup_helper(store, proofs, rows, r, t7::map_companion_names[6], static_cast<asset *>(nullptr), guard).okay(), "shared BGB dependency retained");
  check(b::ready(store, proofs, rows, r, guard).ready, "Gorod transitive helper receipts complete");
}
static void fail_closed_and_reentrancy() {
  auto guard = [] { return true; };
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple");
    std::string error;
    auto *foreign = store.retain(module_paths[0], module_data[0], error, r.lease);
    check(foreign, "foreign producer fixture seeded");
    auto declined = b::route_stock(store, proofs, rows, r, module_paths[0], foreign, witness, guard);
    check(declined.state == b::outcome::declined && !declined.asset && std::string_view(declined.reason) == "unowned_stock_cache", "foreign stock cache cannot impersonate transform");
    check(!b::ready(store, proofs, rows, r, guard).ready, "foreign cache never readies");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple");
    auto mutation = module_data[0]; mutation[0x1a0] ^= 1;
    asset bad{module_paths[0].data(), static_cast<decltype(asset::len)>(mutation.size()), reinterpret_cast<const char *>(mutation.data())};
    auto declined = b::route_stock(store, proofs, rows, r, module_paths[0], &bad, witness, guard);
    check(declined.state == b::outcome::declined && !declined.asset && !store.cached(module_paths[0], r.lease), "unknown raw fails without publication");
    const auto saved = contracts[0]; contracts[0].independently_verified = false;
    check(!b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, guard).okay(), "unverified fixed factory fails closed");
    contracts[0] = saved;
    check(b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, guard).okay(), "declined preparation releases producer claim");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple"); bool native_current = true;
    hash_hook = [&] { native_current = false; };
    auto declined = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, [&] { return native_current; });
    check(declined.state == b::outcome::declined && !store.cached(module_paths[0], r.lease), "context change during pure work rejects retention");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple"); unsigned samples = 0;
    auto changed = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, [&] { return ++samples < 5; });
    check(changed.state == b::outcome::declined && store.cached(module_paths[0], r.lease), "post-retention context change cannot publish receipt");
    check(!b::ready(store, proofs, rows, r, guard).ready, "unreceipted retained object never ready");
    check(!b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, guard).okay(), "unknown cache requires failure handling");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple"); bool nested_declined = false;
    factory_hook = [&] {
      auto nested = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, guard);
      nested_declined = nested.state == b::outcome::declined && std::string_view(nested.reason) == "producer_already_building";
    };
    auto changed = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, guard);
    check(changed.okay() && nested_declined, "factory outside lock permits safe duplicate producer refusal");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple");
    auto altered = descriptors; altered[0].helpers = 1;
    check(!b::route_stock(store, proofs, std::span<const b::descriptor>(altered), r, module_paths[0], &originals[0], witness, guard).okay(), "companion omitted from descriptor dependencies declines");
    check(b::route_stock(store, proofs, rows, r, module_paths[0], static_cast<asset *>(nullptr), witness, guard).state == b::outcome::declined, "missing raw declines");
    check(!b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, []() -> bool { throw std::runtime_error("guard"); }).okay(), "guard exception never unwinds caller");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "workshop_custom");
    check(!b::ready(store, proofs, rows, r, guard).required, "custom map without optional module unaffected");
    auto bytes = module_data[7]; bytes[0x1a0] ^= 1;
    asset malformed{module_paths[7].data(), static_cast<decltype(asset::len)>(bytes.size()), reinterpret_cast<const char *>(bytes.data())};
    check(!b::route_stock(store, proofs, rows, r, module_paths[7], &malformed, witness, guard).okay(), "optional encountered unknown raw declines");
    const auto status = b::ready(store, proofs, rows, r, guard);
    check(status.required && !status.ready, "optional route failure persists as required current-VM receipt");
  }
  {
    store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "workshop_custom"); bool pending_seen = false;
    factory_hook = [&] { const auto status = b::ready(store, proofs, rows, r, guard); pending_seen = status.required && !status.ready; };
    check(b::route_stock(store, proofs, rows, r, module_paths[7], &originals[7], witness, guard).okay() && pending_seen, "optional in-progress receipt blocks readiness before transform completes");
  }
}
static void emergency() {
  store_type probe; pin(probe); const auto used = probe.retained().second;
  store_type store(used + 1); proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple");
  auto guard = [] { return true; };
  auto result = b::lookup_helper(store, proofs, rows, r, t7::map_companion_names[1], static_cast<asset *>(nullptr), guard);
  check(store.exhausted(), "helper budget exhaustion latched by store");
  if (requires_guarded_emergency<store_type, request_type, decltype(guard)>)
    check(result.state == b::outcome::emergency && result.asset, "v4 guarded emergency returns owned reservation");
  else check(result.state == b::outcome::declined && !result.asset, "v3 never calls unguarded emergency");
  check(!b::ready(store, proofs, rows, r, guard).ready, "emergency never establishes readiness");
}
static void serialized_producer() {
  store_type store; proofs_type proofs; pin(store); const auto r = capture(store, "zm_temple");
  std::mutex component_mutex;
  std::latch first_in_factory{1}, second_started{1};
  std::array<b::operation<asset>, 2> result;
  factory_hook = [&] { first_in_factory.count_down(); second_started.wait(); };
  std::thread first([&] {
    std::scoped_lock lock(component_mutex);
    result[0] = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, [] { return true; });
  });
  first_in_factory.wait();
  std::thread second([&] {
    second_started.count_down();
    std::scoped_lock lock(component_mutex);
    result[1] = b::route_stock(store, proofs, rows, r, module_paths[0], &originals[0], witness, [] { return true; });
  });
  first.join(); second.join();
  check(result[0].state == b::outcome::transformed, "serialized first worker transforms");
  check(result[1].state == b::outcome::cached && result[1].asset == result[0].asset, "serialized parallel worker returns same owned pointer");
  bool published = false;
  check(b::publish_error(store, r, [] { return true; }, [&] { published = true; }) && published,
        "root publishes failure after component lock has been released");
}
int main() {
  try {
    initialize(); matrix(); generations_and_routing(); transitive_dependencies(); fail_closed_and_reentrancy(); emergency(); serialized_producer();
    std::cout << "checks=" << checks << " helper_count=" << t7::map_companion_names.size()
              << " scope=portable-synthetic-eight-module-runtime-bridge-only\n";
  } catch (const std::exception &error) {
    std::cerr << "failed checks=" << checks << " reason=" << error.what() << '\n'; return 1;
  }
}
