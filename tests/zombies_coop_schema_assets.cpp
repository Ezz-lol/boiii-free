#include "../src/client/component/zombies/schema_asset_store.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <new>

std::atomic_bool fail_allocations{false};
std::atomic_int fail_after{-1};
void* operator new(std::size_t size) {
  if (fail_allocations.load(std::memory_order_relaxed)) throw std::bad_alloc();
  if (fail_after.load(std::memory_order_relaxed) >= 0 && fail_after.fetch_sub(1, std::memory_order_relaxed) == 0)
    throw std::bad_alloc();
  if (auto* pointer = std::malloc(size ? size : 1)) return pointer;
  throw std::bad_alloc();
}
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }

using namespace zombies::coop::t7;
struct test_asset { const char* name; int len; const char* buffer; };
std::string load(const char* path) {
  std::ifstream input(path, std::ios::binary);
  assert(input);
  return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

std::string named_object(std::string_view original, std::string_view name) {
  std::vector<std::uint8_t> bytes(original.begin(), original.end());
  const auto offset = static_cast<std::uint32_t>(bytes.size());
  bytes.insert(bytes.end(), name.begin(), name.end()); bytes.push_back(0);
  write32(bytes, 0x34, offset);
  return {reinterpret_cast<const char*>(bytes.data()), bytes.size()};
}
std::string_view object_name(const test_asset* asset) {
  assert(asset && asset->len >= static_cast<int>(object_header_size));
  const auto bytes = std::span(reinterpret_cast<const std::uint8_t*>(asset->buffer), static_cast<std::size_t>(asset->len));
  const auto name = bounded_name(bytes, read32(bytes, 0x34));
  assert(name); return *name;
}
void island_identity_cases(const std::string& gsc, const std::string& csc, const std::string& island) {
  assert(fixed_companion_identity(island_companion) == companion_id::island_trials);
  assert(fixed_companion_identity("SCRIPTS\\ZM\\ERODED_COOP_ISLAND_TRIALS.GSC") == companion_id::island_trials);
  assert(fixed_companion_role(island_companion) == 0u);
  assert(companion_realm(companion_id::server_fields) == 0u && companion_realm(companion_id::client_fields) == 1u);
  assert(companion_realm(companion_id::island_trials) == 0u);
  assert(!fixed_companion_identity("coop_schema/eroded_coop_island_trials.gsc"));
  assert(!fixed_companion_identity("scripts/zm/eroded_coop_island_trials.csc"));
  assert(!fixed_companion_identity("scripts/zm/eroded_coop_island_trials.gsc.bak"));
  assert(is_island_server_anchor("SCRIPTS\\ZM\\ZM_ISLAND.GSC", "zm_island"));
  assert(!is_island_server_anchor("scripts/zm/zm_island.csc", "zm_island"));
  assert(!is_island_server_anchor("scripts/zm/zm_tomb.gsc", "zm_island"));
  assert(!is_island_server_anchor("scripts/zm/zm_island.gsc", "zm_tomb"));
  assert(!is_island_server_anchor("scripts/zm/gametypes/_clientids.gsc", "zm_island"));
  auto map_object = named_object(gsc, "scripts/zm/zm_island.gsc");
  auto common_include = clone_with_include(std::span(reinterpret_cast<const std::uint8_t*>(map_object.data()), map_object.size()), companion_include);
  assert(common_include.result == include_result::inserted);
  auto map_include = clone_with_include(common_include.bytes, island_include);
  assert(map_include.result == include_result::inserted);
  assert(clone_with_include(map_include.bytes, island_include).result == include_result::already_present);
  // Code, source CRC and existing fixups remain untouched by either append.
  const auto source = std::span(reinterpret_cast<const std::uint8_t*>(map_object.data()), map_object.size());
  assert(read32(source,0x08)==read32(map_include.bytes,0x08));
  assert(std::equal(source.begin()+0x10,source.begin()+0x44,map_include.bytes.begin()+0x10));
  assert(std::equal(source.begin()+object_header_size,source.end(),map_include.bytes.begin()+object_header_size));

  schema_asset_store<test_asset> assets;
  std::string error;
  assert(assets.pin(gsc,csc,island,error));
  assert(assets.pin(gsc,csc,island,error));
  auto changed = island; changed.back() ^= 1;
  assert(!assets.pin(gsc,csc,changed,error));
  assert(!assets.pin(gsc,csc,gsc,error)); // mismatched logical identity
  assert(!assets.island_server_authorized());
  assert(!assets.companion(island_companion,error)); // map authorization is required
  auto server_lease = assets.capture_generation(server_companion);
  const auto client_lease = assets.capture_generation(client_companion);
  assert(!assets.authorize_island_server(client_lease));
  assert(!assets.authorize_island_server({2u,server_lease.generation}));
  assert(!assets.island_server_authorized());
  assert(assets.authorize_island_server(server_lease));
  auto* first = assets.companion(island_companion,error,server_lease);
  auto* common = assets.companion(server_companion,error,server_lease);
  assert(first && common && first != common);
  assert(object_name(first) == island_companion && std::string_view(first->name) == island_companion);
  assert(object_name(common) == server_companion);
  const auto retained = assets.retained();
  for(int index=0;index<1000;++index) {
    assert(assets.companion("SCRIPTS\\ZM\\ERODED_COOP_ISLAND_TRIALS.GSC",error)==first);
    assert(assets.companion(server_companion,error)==common);
  }
  assert(assets.retained()==retained); // recursive lookups do not allocate new backing
  assert(assets.begin_generation(true));
  assert(assets.island_server_authorized()); // CSC epochs cannot revoke/expand server scope
  assert(assets.companion(island_companion,error)==first);
  const_cast<char*>(first->buffer)[first->len-1]^=1;
  assert(assets.begin_generation(false));
  assert(!assets.island_server_authorized());
  assert(!assets.authorize_island_server(server_lease));
  assert(!assets.companion(island_companion,error));
  const auto fresh_lease=assets.capture_generation(server_companion);
  assert(assets.authorize_island_server(fresh_lease));
  assert(!assets.companion(island_companion,error,server_lease));
  auto* fresh=assets.companion(island_companion,error,fresh_lease);
  assert(fresh && fresh!=first && fresh->buffer!=first->buffer);
  assert(std::string_view(fresh->buffer,fresh->len)==island); // immutable pre-link template
  assert(!assets.retain(island_companion,{1},error,server_lease));
  fail_allocations.store(true);
  const auto existing=assets.emergency_companion("SCRIPTS\\ZM\\ERODED_COOP_ISLAND_TRIALS.GSC");
  fail_allocations.store(false);
  assert(existing==fresh); // ID2 must not return the GSC common object's ID0 cache hit
  assert(!assets.begin_generation(false) && !assets.begin_generation(true));
  assert(!assets.authorize_island_server(fresh_lease));

  schema_asset_store<test_asset> reserved;
  assert(reserved.pin(gsc,csc,island,error));
  assert(reserved.authorize_island_server(reserved.capture_generation(island_companion)));
  fail_allocations.store(true);
  const auto fallback=reserved.emergency_companion(island_companion);
  const auto server_fallback=reserved.emergency_companion(server_companion);
  const auto client_fallback=reserved.emergency_companion(client_companion);
  const auto unknown=reserved.emergency_companion("scripts/zm/unknown.gsc");
  fail_allocations.store(false);
  assert(fallback && server_fallback && client_fallback && !unknown);
  assert(fallback!=server_fallback && fallback!=client_fallback && server_fallback!=client_fallback);
  assert(object_name(fallback)==island_companion && object_name(server_fallback)==server_companion && object_name(client_fallback)==client_companion);
  assert(std::string_view(fallback->buffer,fallback->len)==island);
  assert(reserved.companion(island_companion,error)==fallback);
  assert(!reserved.begin_generation(false));

  // Pinning and each lookup allocation may fail. No partially pinned identity or
  // wrong-name fallback is permitted; all earlier backing remains retained.
  schema_asset_store<test_asset> pin_failure;
  fail_allocations.store(true);
  bool pin_threw=false;
  try { (void)pin_failure.pin(gsc,csc,island,error); } catch(const std::bad_alloc&) { pin_threw=true; }
  fail_allocations.store(false);
  assert(pin_threw && !pin_failure.exhausted());
  assert(pin_failure.pin(gsc,csc,island,error));
  int rejected_pins=0;
  for(int allocation=0;allocation<48;++allocation) {
    schema_asset_store<test_asset> injected_pin;
    const auto before=injected_pin.retained();
    bool pinned=false;
    fail_after.store(allocation);
    try { pinned=injected_pin.pin(gsc,csc,island,error); }
    catch(const std::bad_alloc&) { ++rejected_pins; }
    fail_after.store(-1);
    assert(!injected_pin.exhausted());
    if(!pinned) assert(injected_pin.retained()==before); // no partially published bundle
    assert(injected_pin.pin(gsc,csc,island,error));
    assert(injected_pin.authorize_island_server(injected_pin.capture_generation(island_companion)));
    for(const auto name:companion_names) {
      const auto* result=injected_pin.companion(name,error);
      assert(result && object_name(result)==name);
    }
  }
  assert(rejected_pins>=12);
  int rejected=0;
  for(int allocation=0;allocation<24;++allocation) {
    schema_asset_store<test_asset> injected;
    assert(injected.pin(gsc,csc,island,error));
    assert(injected.authorize_island_server(injected.capture_generation(island_companion)));
    test_asset* result=nullptr;
    fail_after.store(allocation);
    try { result=injected.companion(island_companion,error); }
    catch(const std::bad_alloc&) { ++rejected; }
    fail_after.store(-1);
    if(!result) result=injected.emergency_companion(island_companion);
    assert(result && object_name(result)==island_companion);
    assert(std::string_view(result->buffer,result->len)==island);
  }
  assert(rejected>=6);
  schema_asset_store<test_asset> count_bound;
  assert(count_bound.pin(gsc,csc,island,error));
  for(int index=0;index<4096;++index)
    assert(count_bound.retain("scripts/zm/capacity_"+std::to_string(index)+".gsc",{1},error));
  assert(!count_bound.retain("scripts/zm/capacity_overflow.gsc",{1},error));
  assert(count_bound.exhausted() && count_bound.retained().first==4096);
}

int main() {
  assert(is_schema_anchor("scripts/zm/gametypes/_clientids.gsc", "zm_zod"));
  assert(is_schema_anchor("scripts/zm/_zm_score.csc", "zm_tomb"));
  assert(is_schema_anchor("scripts/zm/zm_custom.csc", "zm_custom"));
  assert(is_schema_anchor("scripts/zm/zm_custom.gsc", "zm_custom"));
  assert(!is_schema_anchor("scripts/zm/zm_other.gsc", "zm_custom"));
  assert(!is_schema_anchor("scripts/zm/zm_custom.gsc", "../zm_custom"));
  assert(!is_schema_anchor("scripts/zm/gametypes/_clientids.csc", "zm_zod"));
  assert(!is_schema_anchor("scripts/zm/_zm_score.gsc", "zm_zod"));
  const auto gsc = load("data/coop_scripts/eroded_coop_fields.gsc");
  const auto csc = load("data/coop_scripts/eroded_coop_fields.csc");
  // Synthetic object tests storage identity only, not Island script semantics.
  const auto island = named_object(gsc, island_companion);
  island_identity_cases(gsc,csc,island);
  schema_asset_store<test_asset> assets;
  std::string error;
  assert(assets.pin(gsc, csc, island, error));
  assert(assets.pin(gsc, csc, island, error));
  assert(!assets.pin(csc, gsc, island, error));
  auto modified = gsc; modified.back() ^= 1;
  assert(!assets.pin(modified, csc, island, error));
  const auto first = assets.companion(server_companion, error);
  const auto second = assets.companion(server_companion, error);
  assert(first && second && first == second); // native recursive linking observes one identity
  for (int i = 0; i < 1000; ++i) // observed native recursion must not allocate new objects
    assert(assets.companion("SCRIPTS\\ZM\\ERODED_COOP_FIELDS.GSC", error) == first);
  const auto* native_layout = reinterpret_cast<const std::uint8_t*>(first);
  for (std::size_t i = sizeof(void*) + sizeof(int); i < 2 * sizeof(void*); ++i)
    assert(native_layout[i] == 0); // CB's upper length half is deterministic.
  assert(std::string_view(first->buffer, first->len) == gsc);
  auto* linked = const_cast<char*>(first->buffer);
  linked[first->len - 1] ^= 1; // Simulate the engine mutating a linked return.
  assert(assets.companion(server_companion, error) == first); // same generation keeps link state
  assets.begin_generation(false);
  const auto third = assets.companion(server_companion, error);
  assert(third != first && third->buffer != first->buffer);
  assert(std::string_view(third->buffer, third->len) == gsc);
  assert(assets.template_for(first) == std::vector<std::uint8_t>(gsc.begin(), gsc.end()));
  assert(assets.companion("not_a_companion", error) == nullptr);
  test_asset* concurrent[2]{};
  std::thread a([&] { std::string e; concurrent[0] = assets.companion(client_companion, e); });
  std::thread b([&] { std::string e; concurrent[1] = assets.companion(client_companion, e); });
  a.join(); b.join();
  assert(concurrent[0] && concurrent[1] && concurrent[0] == concurrent[1]);
  // A budget accepts the pinned templates and a small bounded number of clones,
  // then fails rather than freeing or reusing any engine-visible memory.
  schema_asset_store<test_asset> bounded(4096 * sizeof(void*) + (gsc.size() + csc.size() + island.size()) * 8);
  assert(bounded.pin(gsc, csc, island, error));
  const auto retained = bounded.companion(server_companion, error);
  assert(retained);
  int accepted = 1;
  for (;;) {
    bounded.begin_generation(false);
    if (!bounded.companion(server_companion, error)) break;
    ++accepted; assert(accepted < 10);
  }
  assert(bounded.exhausted() && !error.empty());
  assert(std::string_view(retained->buffer, retained->len) == gsc);
  const auto emergency = bounded.emergency_companion(server_companion);
  assert(emergency && bounded.companion(server_companion, error) == emergency);
  assert(!bounded.begin_generation(false)); // poisoned reset/new request is rejected atomically
  assert(bounded.exhausted());
  assert(!bounded.pin(gsc, csc, island, error)); // a matching pinned bundle cannot clear poison
  assert(bounded.emergency_companion(server_companion) == emergency); // refused generation kept current identity
  assert(bounded.retain("scripts/zm/new_after_poison.gsc", {1}, error) == nullptr);
  fail_allocations.store(true);
  const auto fixed_gsc = fixed_companion_role("SCRIPTS\\ZM\\ERODED_COOP_FIELDS.GsC");
  const auto fixed_csc = fixed_companion_role("scripts/zm/eroded_coop_fields.CSC");
  const auto not_fixed = fixed_companion_role("scripts/zm/eroded_coop_fields.gsc.bak");
  const auto no_allocation_fallback = bounded.emergency_companion("SCRIPTS\\ZM\\ERODED_COOP_FIELDS.GSC");
  fail_allocations.store(false);
  assert(fixed_gsc == 0u && fixed_csc == 1u && !not_fixed);
  assert(no_allocation_fallback == emergency);
  fail_allocations.store(true);
  const auto existing_no_allocation = assets.emergency_companion(client_companion);
  fail_allocations.store(false);
  assert(existing_no_allocation == concurrent[0]); // cached identity wins over reserved fallback during OOM
  std::atomic_bool poisoned{false};
  std::thread poison([&] {
    assert(assets.emergency_companion(client_companion) == concurrent[0]);
    poisoned.store(true, std::memory_order_release);
  });
  std::thread after_poison([&] {
    while (!poisoned.load(std::memory_order_acquire)) std::this_thread::yield();
    assert(!assets.begin_generation(false));
    std::string e;
    assert(assets.companion(server_companion, e) == third); // cached first even after poison
    assert(assets.retain("scripts/zm/new_after_poison.gsc", {1}, e) == nullptr);
  });
  poison.join(); after_poison.join();
  schema_asset_store<test_asset> leased;
  assert(leased.pin(gsc, csc, island, error));
  const auto old_lease = leased.capture_generation(server_companion);
  assert(leased.begin_generation(false));
  assert(!leased.retain(server_companion, std::vector<std::uint8_t>(gsc.begin(), gsc.end()), error, old_lease));
  assert(leased.cached(server_companion) == nullptr); // old-map bytes never enter the new cache
  assert(!leased.companion(server_companion, error, old_lease));
  assert(leased.cached(server_companion) == nullptr);
  const auto wrong_role = leased.capture_generation(client_companion);
  assert(!leased.retain(server_companion, std::vector<std::uint8_t>(gsc.begin(), gsc.end()), error, wrong_role));
  const auto stable_current = leased.companion(server_companion, error);
  assert(stable_current);
  assert(!leased.retain(server_companion, {1}, error, old_lease)); // stale work cannot return a new-generation hit
  assert(leased.companion(server_companion, error) == stable_current);
  const auto pending_lease = leased.capture_generation(server_companion);
  std::atomic_bool advanced{false};
  std::thread advance([&] {
    assert(leased.begin_generation(false));
    advanced.store(true, std::memory_order_release);
  });
  std::thread stale_work([&] {
    while (!advanced.load(std::memory_order_acquire)) std::this_thread::yield();
    std::string e;
    assert(!leased.retain(server_companion, std::vector<std::uint8_t>(gsc.begin(), gsc.end()), e, pending_lease));
    assert(leased.cached(server_companion) == nullptr);
  });
  advance.join(); stale_work.join();
  schema_asset_store<test_asset> invalid;
  assert(!invalid.pin("truncated", csc, island, error));
  assert(invalid.companion(server_companion, error) == nullptr);
}
