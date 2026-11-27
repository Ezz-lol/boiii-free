#include "map_import_binding.hpp"
#include "schema_asset_store.hpp"
#include <iostream>
#include <stdexcept>

namespace m = zombies::coop::map_import_binding;
namespace d = zombies::coop::map_import_binding::detail;
namespace t7 = zombies::coop::t7;
static unsigned checks = 0;
static void check(bool value, const char *label = "check") {
  ++checks;
  if (!value) throw std::runtime_error(label);
}
// A deterministic synthetic witness, deliberately not represented as SHA-256.
// Runtime integrations must provide their real SHA-256 implementation.
static m::digest witness(std::span<const std::uint8_t> b) {
  m::digest h{}; std::uint32_t v = 0x811c9dc5;
  for (std::size_t i = 0; i < b.size(); ++i) {
    v = (v ^ b[i]) * 0x1000193;
    h[i % 32] ^= static_cast<std::uint8_t>(v >> ((i % 4) * 8));
  }
  return h;
}
static const m::import_key source_a{0x11111111, 0x22222222, 1, 0x22};
static const m::import_key source_b{0x33333333, 0x22222222, 2, 0x25};
static const m::import_key unrelated{0x55555555, 0x66666666, 0, 2};
static const m::import_key destination_a{0x77777777, 0x99999999, 1, 2};
static const m::import_key destination_b{0x88888888, 0x99999999, 1, 2};
static const m::import_key destination_c{0xaaaaaaaa, 0x99999999, 2, 5};
static std::size_t row(std::vector<std::uint8_t> &b, std::size_t at,
                       m::import_key key, const std::vector<std::uint32_t> &refs) {
  t7::write32(b, at, key.name); t7::write32(b, at + 4, key.space);
  d::write16(b, at + 8, static_cast<std::uint16_t>(refs.size()));
  b[at + 10] = key.params; b[at + 11] = key.flags;
  at += 12;
  for (auto ref : refs) { t7::write32(b, at, ref); at += 4; }
  return at;
}
static std::vector<std::uint8_t> fixture() {
  std::vector<std::uint8_t> b(0x800);
  std::copy(t7::object_magic.begin(), t7::object_magic.end(), b.begin());
  t7::write32(b, 8, 0xabcdef01); t7::write32(b, 0x14, 0x180);
  t7::write32(b, 0x30, 0x300); t7::write32(b, 0x34, 0x60);
  const std::string name = "scripts/zm/__map_binding_fixture.gsc";
  std::copy(name.begin(), name.end(), b.begin() + 0x60);
  t7::write32(b, 0x20, 0x500); d::write16(b, 0x3a, 3);
  for (unsigned i = 0; i < 3; ++i) {
    const auto p = 0x500 + i * 20;
    t7::write32(b, p, 0x12340000 + i); t7::write32(b, p + 4, 0x180 + i * 0x100);
    t7::write32(b, p + 8, 0x56780000 + i); t7::write32(b, p + 12, 0x11110000);
    b[p + 16] = static_cast<std::uint8_t>(i);
  }
  t7::write32(b, 0x24, 0x600); d::write16(b, 0x3c, 3);
  auto next = row(b, 0x600, source_a, {0x1a0, 0x1c0, 0x2a0, 0x3e0});
  next = row(b, next, source_b, {0x3a0}); row(b, next, unrelated, {0x3c0});
  for (auto ref : {0x1a0, 0x1c0, 0x2a0, 0x3a0, 0x3c0, 0x3e0}) {
    d::write16(b, static_cast<std::size_t>(ref), 0x1234);
    b[static_cast<std::size_t>(ref) + 2] = 0;
  }
  return b;
}
static m::contract contract_for(const std::vector<std::uint8_t> &b) {
  m::contract c{}; c.independently_verified = true;
  c.logical_name = "scripts/zm/__map_binding_fixture.gsc";
  c.sha256 = witness(b); c.object_size = b.size(); c.source_crc = 0xabcdef01;
  c.code = 0x180; c.code_size = 0x300; c.companion_include = "scripts/zm/eroded_coop_map_quests";
  const auto site = [&](std::uint32_t ref, unsigned owner) {
    m::call_site s{};
    s.owner = {0x12340000 + owner, 0x180 + owner * 0x100, 0x280 + owner * 0x100,
               0x56780000 + owner, 0x11110000, static_cast<std::uint8_t>(owner), 0};
    s.reference = ref; s.opcode = 0x1234; s.parameter_byte = 0;
    s.instruction.assign(b.begin() + ref, b.begin() + ref + 16);
    return s;
  };
  c.bindings = {{source_a, destination_a, {site(0x1a0, 0), site(0x1c0, 0)}},
                {source_a, destination_b, {site(0x2a0, 1)}},
                {source_b, destination_c, {site(0x3a0, 2)}}};
  return c;
}
static std::vector<d::import_row> imports(const std::vector<std::uint8_t> &b) {
  std::vector<d::import_row> result;
  std::size_t p = t7::read32(b, 0x24);
  for (unsigned i = 0; i < d::read16(b, 0x3c); ++i) {
    d::import_row r{{t7::read32(b, p), t7::read32(b, p + 4), b[p + 10], b[p + 11]}, {}};
    const auto count = d::read16(b, p + 8); p += 12;
    for (unsigned j = 0; j < count; ++j) { r.refs.push_back(t7::read32(b, p)); p += 4; }
    result.push_back(std::move(r));
  }
  return result;
}
static void preservation(const std::vector<std::uint8_t> &original,
                         const std::vector<std::uint8_t> &output) {
  check(output.size() > original.size());
  for (std::size_t i = 0; i < original.size(); ++i) {
    const bool header_change = (i >= 0x0c && i < 0x10) || (i >= 0x24 && i < 0x28) ||
                               (i >= 0x3c && i < 0x3e) || i == 0x44;
    if (!header_change) check(original[i] == output[i], "original-byte preservation");
  }
  check(t7::read32(original, 8) == t7::read32(output, 8));
  check(t7::read32(output, 0x24) >= original.size());
}
static void synthetic_tests() {
  const auto b = fixture(); const auto c = contract_for(b);
  const auto r = m::transform(b, c, witness);
  check(r.changed && r.reason == "map_imports_redirected", "valid transform");
  preservation(b, r.bytes); check(b == fixture());
  const auto rows = imports(r.bytes); check(rows.size() == 5);
  const auto refs_for = [&](m::import_key key) {
    const auto at = std::find_if(rows.begin(), rows.end(), [&](const auto &v) { return v.key == key; });
    check(at != rows.end()); return at->refs;
  };
  check(refs_for(source_a) == std::vector<std::uint32_t>{0x3e0}, "shared original refs preserved");
  check(refs_for(unrelated) == std::vector<std::uint32_t>{0x3c0});
  check(refs_for(destination_a) == std::vector<std::uint32_t>({0x1a0, 0x1c0}));
  check(refs_for(destination_b) == std::vector<std::uint32_t>{0x2a0});
  check(refs_for(destination_c) == std::vector<std::uint32_t>{0x3a0});
  check(r.bytes[0x44] == 1);
  const auto dep = t7::bounded_name(r.bytes, t7::read32(r.bytes, t7::read32(r.bytes, 0x0c)));
  check(dep && *dep == c.companion_include);
  check(!m::transform(r.bytes, c, witness).changed, "raw repeat fails digest/size");
  const auto bad_contract = [&](auto mutate) {
    auto changed = c; mutate(changed); const auto q = m::transform(b, changed, witness);
    check(!q.changed && q.bytes.empty(), q.reason.c_str()); check(b == fixture());
  };
  bad_contract([](auto &x) { x.independently_verified = false; });
  bad_contract([](auto &x) { x.sha256 = {}; });
  bad_contract([](auto &x) { x.object_size++; });
  bad_contract([](auto &x) { x.source_crc++; });
  bad_contract([](auto &x) { x.logical_name = "../module.gsc"; });
  bad_contract([](auto &x) { x.companion_include += ".gsc"; });
  bad_contract([](auto &x) { x.bindings.clear(); });
  bad_contract([](auto &x) { x.bindings[0].sites[0].owner.checksum++; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].owner.end--; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].reference = 0xffffffff; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].reference++; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].opcode++; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].parameter_byte++; });
  bad_contract([](auto &x) { x.bindings[0].sites[0].instruction.pop_back(); });
  bad_contract([](auto &x) { x.bindings[0].sites.pop_back(); });
  bad_contract([](auto &x) { x.bindings[0].sites.push_back(x.bindings[0].sites.front()); });
  bad_contract([](auto &x) { x.bindings[0].destination.params++; });
  bad_contract([](auto &x) { x.bindings[0].destination.flags |= 0x20; });
  bad_contract([](auto &x) { x.bindings[0].source.flags |= 0x10; });
  bad_contract([](auto &x) { x.bindings[0].source.flags = 1; });
  bad_contract([](auto &x) { x.bindings[0].destination = unrelated; });
  bad_contract([](auto &x) { x.bindings[0].destination = x.bindings[0].source; });
  bad_contract([](auto &x) { x.bindings[0].destination = x.bindings[2].destination;
                            x.bindings[0].destination.params = 1; x.bindings[0].destination.flags = 2; });
  check(!m::transform(b, c, nullptr).changed);
  const auto bad_object = [&](auto mutate) {
    auto object = b; mutate(object); auto contract = c;
    contract.sha256 = witness(object); contract.object_size = object.size();
    const auto original = object;
    const auto q = m::transform(object, contract, witness);
    check(!q.changed && q.bytes.empty(), q.reason.c_str()); check(object == original);
  };
  bad_object([](auto &x) { x[0] ^= 1; });
  bad_object([](auto &x) { t7::write32(x, 0x14, 0xffffffff); });
  bad_object([](auto &x) { t7::write32(x, 0x30, 0xffffffff); });
  bad_object([](auto &x) { t7::write32(x, 0x34, 0xffffffff); });
  bad_object([](auto &x) { t7::write32(x, 0x10, 0xffffffff); });
  bad_object([](auto &x) { t7::write32(x, 0x20, 0x180); });
  bad_object([](auto &x) { d::write16(x, 0x3a, 4097); });
  bad_object([](auto &x) { t7::write32(x, 0x518, 0x180); });
  bad_object([](auto &x) { t7::write32(x, 0x24, 0xffffffff); });
  bad_object([](auto &x) { d::write16(x, 0x3c, 4097); });
  bad_object([](auto &x) { d::write16(x, 0x608, 0); });
  bad_object([](auto &x) { d::write16(x, 0x608, 65535); });
  bad_object([](auto &x) { x[0x60b] = 0xc2; });
  bad_object([](auto &x) { t7::write32(x, 0x60c, 0xffffffff); });
  bad_object([](auto &x) { t7::write32(x, 0x610, 0x1a0); });
  bad_object([](auto &x) { t7::write32(x, 0x610, 0x1a4); });
  bad_object([](auto &x) { t7::write32(x, 0x618, 0x47e); });
  bad_object([](auto &x) { x[0x44] = 1; t7::write32(x, 0x0c, 0x500); });
  bad_object([](auto &x) { x[0x44] = 1; t7::write32(x, 0x0c, 0x740);
                          t7::write32(x, 0x740, 0x180); });
  bad_object([](auto &x) { row(x, 0x61c, source_a, {0x3a0}); });
  // Existing destination collisions are rejected even if that symbol has a
  // different arity or call-kind row, rather than silently merging it.
  bad_object([](auto &x) { t7::write32(x, 0x62c, destination_a.name);
                          t7::write32(x, 0x630, destination_a.space); });
  for (std::size_t size = 0; size < b.size(); ++size) {
    auto short_b = std::vector<std::uint8_t>(b.begin(), b.begin() + size);
    check(!m::transform(short_b, c, witness).changed, "unknown exact size");
    auto short_c = c; short_c.object_size = size; short_c.sha256 = witness(short_b);
    const auto q = m::transform(short_b, short_c, witness);
    // Padding beyond the last declared table is not a format requirement. A
    // deliberately separate synthetic exact contract can omit only that tail.
    check(q.changed == (size >= 0x63c), "truncated declared structure");
    if (q.changed) preservation(short_b, q.bytes);
  }
  // Equal destinations consolidate multiple named-owner bindings into one row.
  auto merged_c = c; merged_c.bindings[1].destination = destination_a;
  auto merged = m::transform(b, merged_c, witness); check(merged.changed);
  const auto merged_rows = imports(merged.bytes); check(merged_rows.size() == 4);
  const auto found = std::find_if(merged_rows.begin(), merged_rows.end(), [](const auto &v) { return v.key == destination_a; });
  check(found != merged_rows.end() && found->refs == std::vector<std::uint32_t>({0x1a0, 0x1c0, 0x2a0}));
  // Canonical external calls need no routing-bit change.
  auto external_b = b; external_b[0x60b] = 2; auto external_c = c;
  external_c.bindings[0].source.flags = 2; external_c.bindings[1].source.flags = 2;
  external_c.sha256 = witness(external_b); check(m::transform(external_b, external_c, witness).changed);
  auto included = t7::clone_with_include(b, c.companion_include);
  check(included.result == t7::include_result::inserted);
  auto included_c = c; included_c.object_size = included.bytes.size(); included_c.sha256 = witness(included.bytes);
  const auto included_r = m::transform(included.bytes, included_c, witness);
  check(included_r.changed && included_r.bytes[0x44] == 1, "existing include retained once");
  preservation(included.bytes, included_r.bytes);
  // The actual Temple compiler keeps repeated equal keys in disjoint rows.
  // Require the precise multiplicity and keep the per-row reference ownership.
  auto repeated_b = b;
  auto next = row(repeated_b, 0x600, source_a, {0x1a0, 0x1c0});
  next = row(repeated_b, next, source_a, {0x2a0, 0x3e0});
  next = row(repeated_b, next, source_b, {0x3a0});
  row(repeated_b, next, unrelated, {0x3c0}); d::write16(repeated_b, 0x3c, 4);
  auto repeated_c = c; repeated_c.sha256 = witness(repeated_b);
  check(!m::transform(repeated_b, repeated_c, witness).changed, "repeated keys require explicit contract");
  repeated_c.repeated_import_keys.emplace(source_a, std::uint16_t{2});
  const auto repeated_r = m::transform(repeated_b, repeated_c, witness);
  check(repeated_r.changed, "exact repeated key multiplicity accepted");
  preservation(repeated_b, repeated_r.bytes);
  const auto repeated_rows = imports(repeated_r.bytes);
  const auto remaining = std::find_if(repeated_rows.begin(), repeated_rows.end(), [](const auto &v) { return v.key == source_a; });
  check(remaining != repeated_rows.end() && remaining->refs == std::vector<std::uint32_t>{0x3e0}, "repeated-key unselected ref kept");
  repeated_c.repeated_import_keys[source_a] = 3;
  check(!m::transform(repeated_b, repeated_c, witness).changed, "wrong repeated multiplicity declines");
  repeated_c.repeated_import_keys[source_a] = 2;
  repeated_c.repeated_import_keys.emplace(unrelated, std::uint16_t{2});
  check(!m::transform(repeated_b, repeated_c, witness).changed, "spurious repeated identity declines");
  auto pointer_b = b;
  pointer_b[0x60a] = 0; pointer_b[0x60b] = 0x21;
  t7::write32(pointer_b, 0x60c, 0x1a6);
  d::write16(pointer_b, 0x1a6, 0x1234); pointer_b[0x1a8] = 0x7f;
  auto pointer_c = c; pointer_c.sha256 = witness(pointer_b);
  for (unsigned i = 0; i < 2; ++i) {
    auto &rule = pointer_c.bindings[i]; rule.source.params = 0; rule.source.flags = 0x21;
    rule.destination.params = 0; rule.destination.flags = 1;
    for (auto &site : rule.sites) site.function_reference = true;
  }
  auto &first_pointer = pointer_c.bindings[0].sites[0]; first_pointer.reference = 0x1a6;
  first_pointer.instruction.assign(pointer_b.begin() + 0x1a6, pointer_b.begin() + 0x1b0);
  check(!m::transform(pointer_b, pointer_c, witness).changed, "kind1 requires independent fixture proof");
  pointer_c.verified_function_reference_shape = true;
  const auto pointer_r = m::transform(pointer_b, pointer_c, witness);
  check(pointer_r.changed, "kind1 explicit two-byte prefix accepted"); preservation(pointer_b, pointer_r.bytes);
  pointer_c.bindings[0].sites[0].function_reference = false;
  check(!m::transform(pointer_b, pointer_c, witness).changed, "kind1 cannot use call prefix");
  pointer_c.bindings[0].sites[0].function_reference = true;
  pointer_c.bindings[0].sites[0].parameter_byte = 1;
  check(!m::transform(pointer_b, pointer_c, witness).changed, "kind1 has no parameter byte contract");
}

struct asset { const char *name{}; std::size_t len{}; const char *buffer{}; };
static void generation_tests() {
  // The production integration must have one producer per logical cache name.
  // These tests exercise the exact existing store API and immutable template.
  t7::schema_asset_store<asset> store;
  const auto b = fixture(); const auto c = contract_for(b);
  const auto lease = store.capture_generation(c.logical_name);
  const auto changed = m::transform(b, c, witness); check(changed.changed);
  std::string error;
  auto *first = store.retain_if(c.logical_name, changed.bytes, error, lease,
                                std::optional{b}, [] { return true; });
  check(first != nullptr); check(store.cached(c.logical_name, lease) == first);
  // Same-generation requests reuse the stable identity, even after VM writes.
  auto *mutable_bytes = const_cast<char *>(first->buffer); mutable_bytes[0x1a0] ^= 1;
  check(store.cached(c.logical_name, lease) == first);
  auto pristine = store.template_for(first); check(pristine && *pristine == b);
  check(store.begin_generation(false)); const auto next = store.capture_generation(c.logical_name);
  check(next.generation != lease.generation); check(!store.cached(c.logical_name, lease));
  bool stale_action = false; check(!store.if_current(lease, [&] { stale_action = true; }));
  check(!stale_action);
  check(!store.retain_if(c.logical_name, changed.bytes, error, lease,
                         std::optional{b}, [] { return true; }));
  const auto fresh = m::transform(*pristine, c, witness); check(fresh.changed);
  auto *second = store.retain_if(c.logical_name, fresh.bytes, error, next,
                                 pristine, [] { return true; });
  check(second && second != first && second->buffer != first->buffer);
  check(store.template_for(second) == std::optional{b});
  check(static_cast<std::uint8_t>(first->buffer[0x1a0]) != fresh.bytes[0x1a0], "retired VM backing stays live and separate");
  check(store.begin_generation(true)); // client generation does not revoke server
  check(store.cached(c.logical_name, next) == second);
  check(store.begin_generation(false)); const auto last = store.capture_generation(c.logical_name);
  check(!store.retain_if(c.logical_name, changed.bytes, error, last,
                         std::optional{b}, [] { return false; }));
  check(!store.cached(c.logical_name, last));
}
int main() {
  try {
    synthetic_tests(); generation_tests();
    std::cout << "checks=" << checks << " scope=synthetic-object-and-generation-store-only\n";
  } catch (const std::exception &error) {
    std::cerr << "failed check=" << checks << " reason=" << error.what() << '\n';
    return 1;
  }
}
