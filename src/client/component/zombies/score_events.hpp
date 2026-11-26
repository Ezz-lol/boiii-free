#pragma once
// Exact pre-link stock score-event import adapter. The production entry point
// uses a fixed captured-object contract; the generic structural worker is
// testable with synthetic objects. No native pointer or bytecode operand is
// manufactured.
#include "compiled_includes.hpp"
#include <set>
#include <utility>

namespace zombies::coop::score_events {
namespace detail {
using namespace zombies::coop::t7;
using digest = std::array<std::uint8_t, 32>;
using hash_function = digest (*)(std::span<const std::uint8_t>);
constexpr std::size_t input_limit = 256 * 1024, output_limit = 512 * 1024;
constexpr std::uint32_t target_function = 0x4ef6f494;
constexpr std::uint32_t fnv(std::string_view name) {
  std::uint32_t h = 0x4b9ace2f;
  for (const auto c : name)
    h = (h ^ static_cast<std::uint8_t>(c)) * 0x1000193;
  return h * 0x1000193; // terminal NUL
}
static_assert(fnv("score_event_increment_uimodel") == 0xb7cb798e);
static_assert(fnv("eroded_coop_fields") == 0x33eb1daa);
struct export_descriptor {
  std::uint32_t checksum{}, start{}, end{}, name{}, space{};
  std::uint8_t params{}, flags{};
  bool operator==(const export_descriptor &) const = default;
};
struct contract {
  bool independently_verified = false;
  std::string logical_name; // exact raw name; no basename/wildcard matching
  digest sha256{};
  std::uint32_t source_crc{}, code{}, code_size{}, reference{};
  export_descriptor owner{};
  std::uint32_t original_name{}, original_space{}, destination_name{},
      destination_space{};
  std::uint8_t import_params{}, import_flags{};
  std::uint16_t opcode{}; // actual mapped uint16 opcode, NOT enum0x54
  std::uint8_t site_parameter_byte{};    // pinned PRElink byte; retail value0,
                                         // import params1
  std::vector<std::uint8_t> instruction; // exact entire opcode..aligned operand
};
struct result {
  bool changed{};
  std::string reason;
  std::vector<std::uint8_t> bytes;
};
struct import_record {
  std::uint32_t name{}, space{};
  std::uint8_t params{}, flags{};
  std::vector<std::uint32_t> refs;
};
inline std::uint16_t read16(std::span<const std::uint8_t> b, std::size_t p) {
  return static_cast<std::uint16_t>(b[p] | (b[p + 1] << 8));
}
inline void write16(std::span<std::uint8_t> b, std::size_t p, std::uint16_t v) {
  b[p] = static_cast<std::uint8_t>(v);
  b[p + 1] = static_cast<std::uint8_t>(v >> 8);
}
inline bool overlap(std::size_t a, std::size_t n, std::size_t b,
                    std::size_t m) {
  return a < b + m && b < a + n;
}
inline result transform(std::span<const std::uint8_t> input, const contract &c,
                        hash_function hash) {
  const auto fail = [](std::string reason) {
    return result{false, std::move(reason), {}};
  };
  if (!c.independently_verified || !hash || c.logical_name.empty() ||
      std::all_of(c.sha256.begin(), c.sha256.end(),
                  [](auto v) { return v == 0; }))
    return fail("unverified_contract");
  if (input.size() < object_header_size || input.size() > input_limit ||
      !std::equal(object_magic.begin(), object_magic.end(), input.begin()))
    return fail("raw_header");
  if (hash(input) != c.sha256)
    return fail("unknown_asset_sha256");
  const auto code = read32(input, 0x14), size = read32(input, 0x30);
  if (read32(input, 8) != c.source_crc || code != c.code ||
      size != c.code_size || code < object_header_size || !size ||
      code > input.size() || size > input.size() - code)
    return fail("code_signature");
  const auto name = bounded_name(input, read32(input, 0x34));
  if (!name || *name != c.logical_name)
    return fail("logical_name");
  const auto exports = read32(input, 0x20);
  const auto export_count = read16(input, 0x3a);
  if (exports < object_header_size || !export_count || export_count > 4096 ||
      exports > input.size() || export_count > (input.size() - exports) / 20 ||
      overlap(code, size, exports, export_count * 20u))
    return fail("export_bounds");
  std::vector<export_descriptor> owners;
  std::set<std::uint32_t> starts;
  for (std::size_t i = 0; i < export_count; ++i) {
    const auto p = exports + i * 20;
    export_descriptor e{read32(input, p),      read32(input, p + 4),
                        code + size,           read32(input, p + 8),
                        read32(input, p + 12), input[p + 16],
                        input[p + 17]};
    if (e.start < code || e.start >= code + size ||
        !starts.insert(e.start).second)
      return fail("ambiguous_export");
    owners.push_back(e);
  }
  std::sort(owners.begin(), owners.end(),
            [](const auto &a, const auto &b) { return a.start < b.start; });
  for (std::size_t i = 0; i + 1 < owners.size(); ++i)
    owners[i].end = owners[i + 1].start;
  const auto matches =
      std::count_if(owners.begin(), owners.end(), [&](const auto &e) {
        return e.name == target_function && e.space == c.owner.space;
      });
  if (matches != 1 || c.owner.name != target_function ||
      std::none_of(owners.begin(), owners.end(),
                   [&](const auto &e) { return e == c.owner; }))
    return fail("target_export_signature");
  const auto ref = static_cast<std::size_t>(c.reference);
  const auto operand = (ref + 3 + 7) & ~std::size_t{7};
  const auto end = operand + 8;
  if ((ref & 1) || ref < c.owner.start || ref > c.owner.end ||
      end > c.owner.end || c.instruction.size() != end - ref ||
      c.instruction.size() < 11 ||
      !std::equal(c.instruction.begin(), c.instruction.end(),
                  input.begin() + ref) ||
      read16(input, ref) != c.opcode || input[ref + 2] != c.site_parameter_byte)
    return fail("site_signature");
  const auto table = read32(input, 0x24);
  const auto count = read16(input, 0x3c);
  if (table < object_header_size || table > input.size() || !count ||
      count > 4096)
    return fail("import_bounds");
  std::size_t position = table;
  std::vector<import_record> imports;
  std::set<std::uint32_t> seen_refs;
  std::vector<std::pair<std::size_t, std::size_t>> windows;
  std::size_t chosen = 0, selected_position = 0, selected_refs = 0,
              owned_candidates = 0;
  for (std::size_t i = 0; i < count; ++i) {
    if (position > input.size() || input.size() - position < 12)
      return fail("import_bounds");
    const auto row_position = position;
    const auto refs = read16(input, position + 8);
    import_record imp{read32(input, position),
                      read32(input, position + 4),
                      input[position + 10],
                      input[position + 11],
                      {}};
    position += 12;
    if (!refs || refs > (input.size() - position) / 4)
      return fail("reference_bounds");
    const auto kind = imp.flags & 15;
    if (kind < 1 || kind > 5 || (imp.flags & 0xc0) ||
        (kind == 1 && imp.params != 0))
      return fail("unknown_import_shape");
    const bool match =
        imp.name == c.original_name && imp.space == c.original_space &&
        imp.params == c.import_params && imp.flags == c.import_flags;
    if (imp.name == c.destination_name && imp.space == c.destination_space)
      return fail("preexisting_destination");
    for (std::size_t j = 0; j < refs; ++j) {
      const auto address = read32(input, position);
      position += 4;
      const auto last =
          ((static_cast<std::size_t>(address) + (kind == 1 ? 2 : 3) + 7) &
           ~std::size_t{7}) +
          8;
      if ((address & 1) || address < code || last > code + size ||
          !seen_refs.insert(address).second)
        return fail("reference_bounds_or_duplicate");
      auto owner = std::upper_bound(
          owners.begin(), owners.end(), address,
          [](std::uint32_t at, const auto &e) { return at < e.start; });
      if (owner == owners.begin())
        return fail("reference_export_ownership");
      --owner;
      if (last > owner->end)
        return fail("reference_export_ownership");
      windows.emplace_back(address, last - address);
      imp.refs.push_back(address);
      if (match && address >= c.owner.start && address < c.owner.end)
        ++owned_candidates;
      if (address == c.reference) {
        if (!match)
          return fail("wrong_import_owner");
        chosen = i;
        selected_position = row_position;
        ++selected_refs;
      }
    }
    imports.push_back(std::move(imp));
  }
  std::sort(windows.begin(), windows.end());
  for (std::size_t i = 1; i < windows.size(); ++i)
    if (overlap(windows[i - 1].first, windows[i - 1].second, windows[i].first,
                windows[i].second))
      return fail("overlapping_reference_windows");
  if (overlap(table, position - table, code, size) ||
      overlap(table, position - table, exports, export_count * 20u))
    return fail("overlapping_import_table");
  const auto includes = input[0x44];
  const auto include_table = read32(input, 0x0c);
  if (includes &&
      (include_table < object_header_size || include_table > input.size() ||
       static_cast<std::size_t>(includes) * 4 > input.size() - include_table ||
       overlap(include_table, static_cast<std::size_t>(includes) * 4, code,
               size) ||
       overlap(include_table, static_cast<std::size_t>(includes) * 4, table,
               position - table) ||
       overlap(include_table, static_cast<std::size_t>(includes) * 4, exports,
               export_count * 20u)))
    return fail("overlapping_include_table");
  const auto name_at = read32(input, 0x34);
  if (overlap(name_at, name->size() + 1, code, size) ||
      overlap(name_at, name->size() + 1, table, position - table) ||
      overlap(name_at, name->size() + 1, exports, export_count * 20u) ||
      (includes && overlap(name_at, name->size() + 1, include_table,
                           static_cast<std::size_t>(includes) * 4)))
    return fail("overlapping_logical_name");
  if (selected_refs != 1 || owned_candidates != 1)
    return fail("ambiguous_or_missing_target_import");
  // This exact import row owns only the selected method call. Preserve every
  // other import, all opcode/operand bytes, the source CRC and export offsets.
  if (imports[chosen].refs.size() != 1)
    return fail("shared_target_import");
  result output{true, "score_import_redirected", {input.begin(), input.end()}};
  write32(output.bytes, selected_position, c.destination_name);
  write32(output.bytes, selected_position + 4, c.destination_space);
  auto included =
      clone_with_include(output.bytes, "scripts/zm/eroded_coop_fields");
  if (included.result == include_result::inserted)
    output.bytes = std::move(included.bytes);
  else if (included.result != include_result::already_present)
    return fail("companion_include_declined");
  if (output.bytes.size() > output_limit)
    return fail("output_limit");
  return output;
}
inline contract captured_contract() {
  contract c{};
  c.independently_verified = true;
  c.logical_name = "scripts/zm/_zm_score.gsc";
  c.sha256 = {0x4d, 0x9e, 0x55, 0xc9, 0xcc, 0xd8, 0x86, 0x9f, 0x1f, 0x52, 0xc6,
              0x9e, 0xcb, 0xc9, 0x5c, 0xda, 0x29, 0x37, 0xb8, 0xbb, 0x3f, 0xad,
              0x05, 0x08, 0x09, 0x75, 0x51, 0xcc, 0x2b, 0x57, 0xc3, 0xd9};
  c.source_crc = 0x5faa19f9;
  c.code = 0x6cc;
  c.code_size = 0x185a;
  c.owner = {0xb5f8f17c, 0xbd8, 0xc40, 0x4ef6f494, 0xdc726775, 2, 0};
  c.reference = 0xc22;
  c.opcode = 0x1d3e;
  c.site_parameter_byte = 0;
  c.instruction = {0x3e, 0x1d, 0x00, 0x65, 0x61, 0x2b, 0x34,
                   0x0a, 0xf5, 0x4c, 0x00, 0x00, 0x00, 0x00};
  c.original_name = 0x4cf50a34;
  c.original_space = 0x71e9cb84;
  c.destination_name = 0xb7cb798e;
  c.destination_space = 0x33eb1daa;
  c.import_params = 1;
  c.import_flags = 4; // actual ScriptMethodCall, viewer receiver
  return c;
}
} // namespace detail
using digest = detail::digest;
using hash_function = detail::hash_function;
using result = detail::result;
inline constexpr auto input_limit = detail::input_limit;
inline constexpr std::size_t captured_size = 10998;
inline constexpr std::string_view asset_name = "scripts/zm/_zm_score.gsc";
inline bool is_asset(std::string_view name) noexcept {
  if (name.size() != asset_name.size())
    return false;
  for (std::size_t i = 0; i < name.size(); ++i) {
    auto c = name[i];
    if (c == '\\')
      c = '/';
    if (c >= 'A' && c <= 'Z')
      c = static_cast<char>(c + ('a' - 'A'));
    if (c != asset_name[i])
      return false;
  }
  return true;
}
inline result rewrite(std::span<const std::uint8_t> input, hash_function hash) {
  if (input.size() != captured_size)
    return {false, "unknown_asset_size", {}};
  return detail::transform(input, detail::captured_contract(), hash);
}
} // namespace zombies::coop::score_events
