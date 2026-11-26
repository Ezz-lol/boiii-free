#pragma once

// Exact, pre-link game-script metadata bindings.
// This changes import metadata only; every byte of executable code is retained.
#include "compiled_includes.hpp"
#include <map>
#include <set>
#include <tuple>
#include <utility>

namespace zombies::coop::map_import_binding {
using digest = std::array<std::uint8_t, 32>;
using hash_function = digest (*)(std::span<const std::uint8_t>);
inline constexpr std::size_t input_limit = 256 * 1024;
inline constexpr std::size_t output_limit = 512 * 1024;
inline constexpr std::size_t row_limit = 4096;

struct export_descriptor {
  std::uint32_t checksum{}, start{}, end{}, name{}, space{};
  std::uint8_t params{}, flags{};
  bool operator==(const export_descriptor &) const = default;
};
struct import_key {
  std::uint32_t name{}, space{};
  std::uint8_t params{}, flags{};
  auto operator<=>(const import_key &) const = default;
};
struct call_site {
  export_descriptor owner{};
  std::uint32_t reference{};
  std::uint16_t opcode{}; // independently captured mapped opcode, never guessed
  std::uint8_t parameter_byte{};
  std::vector<std::uint8_t> instruction; // complete opcode..aligned operand
  bool function_reference{}; // explicit GetResolveFunction shape, no argc byte
};
struct binding {
  import_key source{}, destination{};
  std::vector<call_site> sites;
};
struct contract {
  bool independently_verified = false;
  std::string logical_name; // exact embedded name, including .gsc
  digest sha256{};
  std::size_t object_size{};
  std::uint32_t source_crc{}, code{}, code_size{};
  std::string companion_include; // canonical extensionless script path
  std::vector<binding> bindings;
  // Some captured compiler objects keep several rows of an identical key.
  // Opt in only with the complete exact observed multiplicity, never a blanket
  // permission. Every individual reference still has one nonoverlapping owner.
  std::map<import_key, std::uint16_t> repeated_import_keys;
  bool verified_function_reference_shape = false;
};
struct result {
  bool changed{};
  std::string reason;
  std::vector<std::uint8_t> bytes;
};

namespace detail {
using namespace zombies::coop::t7;
struct region { std::size_t start{}, size{}; };
struct import_row { import_key key; std::vector<std::uint32_t> refs; };
inline bool bounded(std::span<const std::uint8_t> b, std::size_t at,
                    std::size_t size) noexcept {
  return at <= b.size() && size <= b.size() - at;
}
// All region endpoints are validated against the small input limit first.
inline bool overlap(region a, region b) noexcept {
  return a.size && b.size && a.start < b.start + b.size &&
         b.start < a.start + a.size;
}
inline std::uint16_t read16(std::span<const std::uint8_t> b,
                            std::size_t p) noexcept {
  return static_cast<std::uint16_t>(b[p] | (b[p + 1] << 8));
}
inline void write16(std::span<std::uint8_t> b, std::size_t p,
                    std::uint16_t value) noexcept {
  b[p] = static_cast<std::uint8_t>(value);
  b[p + 1] = static_cast<std::uint8_t>(value >> 8);
}
inline bool same_symbol(import_key a, import_key b) noexcept {
  return a.name == b.name && a.space == b.space;
}
inline bool selected_call_shape(import_key source,
                                import_key destination) noexcept {
  const auto kind = source.flags & 15;
  // Explicit external destination of the same call kind and argument count.
  // Only the compiler's local-namespace routing bit can be removed.
  return source.name && destination.name && destination.space &&
         !same_symbol(source, destination) && kind >= 1 && kind <= 5 &&
         (kind != 1 || source.params == 0) &&
         !(source.flags & 0xd0) &&
         destination.params == source.params &&
         destination.flags == (source.flags & ~std::uint8_t{0x20});
}
inline std::optional<std::size_t> instruction_end(std::uint32_t reference,
                                                 unsigned kind,
                                                 std::size_t code_end) noexcept {
  const auto at = static_cast<std::size_t>(reference);
  // at is bounded before addition, including on a 32-bit size_t platform.
  if ((at & 1) || at > code_end || code_end - at < 2) return {};
  const auto prefix = kind == 1 ? std::size_t{2} : std::size_t{3};
  if (prefix > code_end - at) return {};
  const auto operand = (at + prefix + 7) & ~std::size_t{7};
  if (operand > code_end || code_end - operand < 8) return {};
  return operand + 8;
}
} // namespace detail

inline result transform(std::span<const std::uint8_t> input, const contract &c,
                        hash_function hash) {
  using namespace detail;
  const auto fail = [](const char *reason) { return result{false, reason, {}}; };
  const auto canonical = canonical_script(c.logical_name);
  const auto companion = canonical_script(c.companion_include);
  if (!c.independently_verified || !hash || !canonical || !companion ||
      !c.logical_name.ends_with(".gsc") || *companion != c.companion_include ||
      *canonical == *companion || c.bindings.empty() || c.bindings.size() > 256 ||
      c.repeated_import_keys.size() > row_limit ||
      std::all_of(c.sha256.begin(), c.sha256.end(), [](auto v) { return v == 0; }))
    return fail("unverified_contract");
  if (input.size() < object_header_size || input.size() > input_limit ||
      input.size() != c.object_size ||
      !std::equal(object_magic.begin(), object_magic.end(), input.begin()))
    return fail("raw_header_or_size");
  if (hash(input) != c.sha256) return fail("unknown_asset_sha256");
  const auto code = read32(input, 0x14), size = read32(input, 0x30);
  if (read32(input, 8) != c.source_crc || code != c.code || size != c.code_size ||
      code < object_header_size || !size || !bounded(input, code, size))
    return fail("code_signature");
  const auto code_end = static_cast<std::size_t>(code) + size;
  const auto raw_name_at = read32(input, 0x34);
  const auto raw_name = bounded_name(input, raw_name_at);
  if (!raw_name || *raw_name != c.logical_name) return fail("logical_name");
  for (const std::size_t field : {0x0c, 0x10, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c}) {
    const auto offset = read32(input, field);
    if (offset && (offset < object_header_size || offset > input.size()))
      return fail("section_offset");
  }

  const auto exports_at = read32(input, 0x20);
  const auto export_count = read16(input, 0x3a);
  if (exports_at < object_header_size || !export_count || export_count > row_limit ||
      !bounded(input, exports_at, static_cast<std::size_t>(export_count) * 20))
    return fail("export_bounds");
  std::vector<export_descriptor> owners;
  std::set<std::uint32_t> starts;
  std::set<std::pair<std::uint32_t, std::uint32_t>> owner_symbols;
  for (std::size_t i = 0; i < export_count; ++i) {
    const auto p = static_cast<std::size_t>(exports_at) + i * 20;
    export_descriptor e{read32(input, p), read32(input, p + 4),
                        static_cast<std::uint32_t>(code_end), read32(input, p + 8),
                        read32(input, p + 12), input[p + 16], input[p + 17]};
    if (e.start < code || e.start >= code_end || !starts.insert(e.start).second ||
        !owner_symbols.emplace(e.name, e.space).second)
      return fail("ambiguous_export");
    owners.push_back(e);
  }
  std::sort(owners.begin(), owners.end(), [](const auto &a, const auto &b) {
    return a.start < b.start;
  });
  for (std::size_t i = 0; i + 1 < owners.size(); ++i) owners[i].end = owners[i + 1].start;
  const auto owner_for = [&](std::uint32_t reference) -> const export_descriptor * {
    auto at = std::upper_bound(owners.begin(), owners.end(), reference,
                             [](auto ref, const auto &owner) { return ref < owner.start; });
    return at == owners.begin() ? nullptr : &*--at;
  };

  // Map exact references once. A reference cannot be claimed by two bindings.
  struct selected { const binding *rule; const call_site *site; bool found{}; };
  std::map<std::uint32_t, selected> selected_refs;
  std::map<std::pair<std::uint32_t, std::uint32_t>, import_key> destinations;
  std::set<std::pair<import_key, std::uint32_t>> selected_owner_sources;
  for (const auto &rule : c.bindings) {
    const auto kind = rule.source.flags & 15;
    if (!selected_call_shape(rule.source, rule.destination) || rule.sites.empty() ||
        (kind == 1 && !c.verified_function_reference_shape) ||
        rule.sites.size() > row_limit)
      return fail("unsupported_binding_contract");
    const auto symbol = std::pair{rule.destination.name, rule.destination.space};
    const auto [dest, inserted] = destinations.emplace(symbol, rule.destination);
    if (!inserted && dest->second != rule.destination)
      return fail("destination_contract_collision");
    for (const auto &site : rule.sites) {
      if (selected_refs.size() == row_limit || !site.owner.name ||
          std::find(owners.begin(), owners.end(), site.owner) == owners.end())
        return fail("target_export_signature");
      const auto *owner = owner_for(site.reference);
      const auto end = instruction_end(site.reference, rule.source.flags & 15, code_end);
      if (site.function_reference != (kind == 1) ||
          (site.function_reference && site.parameter_byte != 0) ||
          !owner || *owner != site.owner || !end || *end > owner->end ||
          site.instruction.size() != *end - site.reference ||
          !std::equal(site.instruction.begin(), site.instruction.end(),
                      input.begin() + site.reference) ||
          read16(input, site.reference) != site.opcode ||
          (!site.function_reference && input[site.reference + 2] != site.parameter_byte))
        return fail("site_signature");
      if (!selected_refs.emplace(site.reference, selected{&rule, &site, false}).second)
        return fail("duplicate_binding_reference");
      selected_owner_sources.emplace(rule.source, site.owner.start);
    }
  }

  const auto imports_at = read32(input, 0x24);
  const auto import_count = read16(input, 0x3c);
  if (imports_at < object_header_size || imports_at > input.size() ||
      !import_count || import_count > row_limit) return fail("import_bounds");
  std::size_t position = imports_at;
  std::vector<import_row> rows;
  std::map<import_key, std::size_t> key_counts;
  std::set<std::uint32_t> all_refs;
  std::vector<region> windows;
  for (std::size_t i = 0; i < import_count; ++i) {
    if (!bounded(input, position, 12)) return fail("import_bounds");
    import_row row{{read32(input, position), read32(input, position + 4),
                    input[position + 10], input[position + 11]}, {}};
    const auto count = read16(input, position + 8);
    position += 12;
    if (!count || !bounded(input, position, static_cast<std::size_t>(count) * 4))
      return fail("reference_bounds");
    const auto kind = row.key.flags & 15;
    if (kind < 1 || kind > 5 || (row.key.flags & 0xc0) ||
        (kind == 1 && row.key.params != 0)) return fail("unknown_import_shape");
    if (++key_counts[row.key] > 1 && !c.repeated_import_keys.contains(row.key))
      return fail("duplicate_import_identity");
    if (destinations.contains({row.key.name, row.key.space}))
      return fail("preexisting_destination");
    for (std::size_t j = 0; j < count; ++j) {
      const auto reference = read32(input, position); position += 4;
      const auto end = instruction_end(reference, kind, code_end);
      const auto *owner = owner_for(reference);
      if (reference < code || !end || !owner || *end > owner->end ||
          !all_refs.insert(reference).second) return fail("reference_bounds_or_ownership");
      windows.push_back({reference, *end - reference});
      row.refs.push_back(reference);
      const auto chosen = selected_refs.find(reference);
      if (chosen != selected_refs.end()) {
        if (chosen->second.rule->source != row.key || *owner != chosen->second.site->owner)
          return fail("wrong_import_owner");
        chosen->second.found = true;
      }
      // Once an owner/source pair is selected, its complete reference set is
      // explicit. An omitted sibling call must not silently remain stock.
      if (selected_owner_sources.contains({row.key, owner->start}) &&
          chosen == selected_refs.end()) return fail("incomplete_owner_reference_set");
    }
    rows.push_back(std::move(row));
  }
  for (const auto &[key, count] : c.repeated_import_keys)
    if (count < 2 || count > row_limit || key_counts[key] != count)
      return fail("repeated_import_identity_contract");
  for (const auto &[reference, chosen] : selected_refs)
    if (!chosen.found) return fail("missing_target_reference");
  std::sort(windows.begin(), windows.end(), [](auto a, auto b) { return a.start < b.start; });
  for (std::size_t i = 1; i < windows.size(); ++i)
    if (overlap(windows[i - 1], windows[i])) return fail("overlapping_reference_windows");

  const auto include_count = input[0x44];
  const auto includes_at = read32(input, 0x0c);
  const auto include_bytes = static_cast<std::size_t>(include_count) * 4;
  if (include_count && (includes_at < object_header_size ||
                        !bounded(input, includes_at, include_bytes)))
    return fail("include_bounds");
  std::vector<region> protected_regions{{code, size},
      {exports_at, static_cast<std::size_t>(export_count) * 20},
      {imports_at, position - imports_at}, {raw_name_at, raw_name->size() + 1}};
  if (include_count) protected_regions.push_back({includes_at, include_bytes});
  std::set<std::string> included_names;
  for (std::size_t i = 0; i < include_count; ++i) {
    const auto path_at = read32(input, includes_at + i * 4);
    const auto path = bounded_name(input, path_at);
    const auto normalized = path ? canonical_script(*path) : std::nullopt;
    if (!normalized || *normalized == *canonical || !included_names.insert(*normalized).second)
      return fail("include_name");
    protected_regions.push_back({path_at, path->size() + 1});
  }
  for (std::size_t i = 0; i < protected_regions.size(); ++i)
    for (std::size_t j = i + 1; j < protected_regions.size(); ++j)
      if (overlap(protected_regions[i], protected_regions[j])) return fail("aliased_sections_or_names");

  // Rebuild once for every binding. Original unselected refs keep their order;
  // selected refs are grouped by the complete destination key. Original table
  // storage remains untouched, even if a selected row was shared by callers.
  std::map<import_key, std::vector<std::uint32_t>> redirected;
  std::vector<import_row> rebuilt;
  for (auto &row : rows) {
    std::vector<std::uint32_t> remaining;
    for (const auto reference : row.refs) {
      const auto found = selected_refs.find(reference);
      if (found == selected_refs.end()) remaining.push_back(reference);
      else redirected[found->second.rule->destination].push_back(reference);
    }
    if (!remaining.empty()) rebuilt.push_back({row.key, std::move(remaining)});
  }
  for (auto &[key, refs] : redirected) rebuilt.push_back({key, std::move(refs)});
  if (rebuilt.size() > row_limit) return fail("import_count_limit");
  std::size_t table_bytes = 0;
  for (const auto &row : rebuilt) {
    if (row.refs.empty() || row.refs.size() > 65535 ||
        table_bytes > output_limit - 12 ||
        row.refs.size() > (output_limit - table_bytes - 12) / 4)
      return fail("output_table_limit");
    table_bytes += 12 + row.refs.size() * 4;
  }
  const auto new_table = (input.size() + 3) & ~std::size_t{3};
  if (new_table > output_limit || table_bytes > output_limit - new_table)
    return fail("output_limit");
  result output{true, "map_imports_redirected", {input.begin(), input.end()}};
  output.bytes.resize(new_table + table_bytes, 0);
  auto next = new_table;
  for (const auto &row : rebuilt) {
    write32(output.bytes, next, row.key.name);
    write32(output.bytes, next + 4, row.key.space);
    write16(output.bytes, next + 8, static_cast<std::uint16_t>(row.refs.size()));
    output.bytes[next + 10] = row.key.params;
    output.bytes[next + 11] = row.key.flags;
    next += 12;
    for (const auto reference : row.refs) { write32(output.bytes, next, reference); next += 4; }
  }
  write32(output.bytes, 0x24, static_cast<std::uint32_t>(new_table));
  write16(output.bytes, 0x3c, static_cast<std::uint16_t>(rebuilt.size()));
  auto dependency = clone_with_include(output.bytes, c.companion_include);
  if (dependency.result == include_result::inserted) output.bytes = std::move(dependency.bytes);
  else if (dependency.result != include_result::already_present)
    return fail("companion_include_declined");
  if (output.bytes.size() > output_limit) return fail("output_limit");
  return output;
}
} // namespace zombies::coop::map_import_binding
