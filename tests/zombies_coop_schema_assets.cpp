#include "../src/client/component/zombies/schema_asset_store.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <thread>
#include <atomic>
#include <cstdlib>
#include <new>

std::atomic_bool fail_allocations{false};
void* operator new(std::size_t size) {
  if (fail_allocations.load(std::memory_order_relaxed)) throw std::bad_alloc();
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
  schema_asset_store<test_asset> assets;
  std::string error;
  assert(assets.pin(gsc, csc, error));
  assert(assets.pin(gsc, csc, error));
  assert(!assets.pin(csc, gsc, error));
  auto modified = gsc; modified.back() ^= 1;
  assert(!assets.pin(modified, csc, error));
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
  schema_asset_store<test_asset> bounded(4096 * sizeof(void*) + (gsc.size() + csc.size()) * 8);
  assert(bounded.pin(gsc, csc, error));
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
  assert(!bounded.pin(gsc, csc, error)); // a matching pinned bundle cannot clear poison
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
  assert(leased.pin(gsc, csc, error));
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
  assert(!invalid.pin("truncated", csc, error));
  assert(invalid.companion(server_companion, error) == nullptr);
}
