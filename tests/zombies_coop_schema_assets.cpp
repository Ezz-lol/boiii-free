#include "../src/client/component/zombies/schema_asset_store.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <thread>

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
  assert(first && second && first != second && first->buffer != second->buffer);
  const auto* native_layout = reinterpret_cast<const std::uint8_t*>(first);
  for (std::size_t i = sizeof(void*) + sizeof(int); i < 2 * sizeof(void*); ++i)
    assert(native_layout[i] == 0); // CB's upper length half is deterministic.
  assert(std::string_view(first->buffer, first->len) == gsc);
  auto* linked = const_cast<char*>(first->buffer);
  linked[first->len - 1] ^= 1; // Simulate the engine mutating a linked return.
  const auto third = assets.companion(server_companion, error);
  assert(std::string_view(second->buffer, second->len) == gsc);
  assert(std::string_view(third->buffer, third->len) == gsc);
  assert(assets.companion("not_a_companion", error) == nullptr);
  test_asset* concurrent[2]{};
  std::thread a([&] { std::string e; concurrent[0] = assets.companion(client_companion, e); });
  std::thread b([&] { std::string e; concurrent[1] = assets.companion(client_companion, e); });
  a.join(); b.join();
  assert(concurrent[0] && concurrent[1] && concurrent[0] != concurrent[1]);
  // A budget accepts the pinned templates and a small bounded number of clones,
  // then fails rather than freeing or reusing any engine-visible memory.
  schema_asset_store<test_asset> bounded(4096 * sizeof(void*) + gsc.size() + csc.size() + gsc.size() * 3);
  assert(bounded.pin(gsc, csc, error));
  const auto retained = bounded.companion(server_companion, error);
  assert(retained);
  int accepted = 1;
  while (bounded.companion(server_companion, error)) ++accepted;
  assert(accepted < 4 && !error.empty());
  assert(std::string_view(retained->buffer, retained->len) == gsc);
  schema_asset_store<test_asset> invalid;
  assert(!invalid.pin("truncated", csc, error));
  assert(invalid.companion(server_companion, error) == nullptr);
}
