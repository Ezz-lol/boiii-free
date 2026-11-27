#pragma once

// T7 PC raw compiled-object include transform, shared by the opt-in native
// asset bootstrap and structural tests. This is not a bytecode verifier.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace zombies::coop::t7 {
inline constexpr std::size_t object_header_size = 0x48;
inline constexpr std::size_t object_size_limit = 64 * 1024 * 1024;
inline constexpr std::array<std::uint8_t, 8> object_magic{
    0x80, 'G', 'S', 'C', 0x0d, 0x0a, 0, 0x1c};

enum class include_result {
  inserted, already_present, invalid_header, invalid_name,
  invalid_include, self_include, too_many_includes, size_limit
};
struct include_clone {
  include_result result{include_result::invalid_header};
  std::vector<std::uint8_t> bytes;
};

inline std::uint32_t read32(std::span<const std::uint8_t> bytes,
    std::size_t offset) noexcept {
  return static_cast<std::uint32_t>(bytes[offset]) |
      (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
      (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
      (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}
inline void write32(std::span<std::uint8_t> bytes, std::size_t offset,
    std::uint32_t value) noexcept {
  for (std::size_t i = 0; i < 4; ++i)
    bytes[offset + i] = static_cast<std::uint8_t>(value >> (i * 8));
}
inline std::optional<std::string> canonical_script(std::string_view name) {
  if (name.empty() || name.size() > 255) return std::nullopt;
  std::string result;
  result.reserve(name.size());
  for (const char c : name) {
    if (c == '\\') result.push_back('/');
    else if (c >= 'A' && c <= 'Z') result.push_back(static_cast<char>(c + ('a' - 'A')));
    else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
             c == '_' || c == '/' || c == '-' || c == '.') result.push_back(c);
    else return std::nullopt;
  }
  if (result.front() == '/' || result.back() == '/') return std::nullopt;
  std::size_t begin = 0;
  while (begin < result.size()) {
    const auto end = result.find('/', begin);
    const auto part = std::string_view(result).substr(begin,
        end == std::string::npos ? result.size() - begin : end - begin);
    if (part.empty() || part == "." || part == "..") return std::nullopt;
    if (end == std::string::npos) break;
    begin = end + 1;
  }
  if (result.ends_with(".gsc") || result.ends_with(".csc")) result.resize(result.size() - 4);
  return result.empty() ? std::nullopt : std::optional<std::string>(std::move(result));
}
inline std::optional<std::string_view> bounded_name(
    std::span<const std::uint8_t> object, std::uint32_t offset) noexcept {
  if (offset < object_header_size || offset >= object.size()) return std::nullopt;
  const auto limit = (std::min)(object.size() - offset, std::size_t{256});
  for (std::size_t length = 0; length < limit; ++length)
    if (!object[offset + length])
      return std::string_view(reinterpret_cast<const char *>(object.data() + offset), length);
  return std::nullopt;
}

// Append one dependency without moving existing code/import/fixup bytes. Only
// include location/count change. source_crc is an opaque compiler/source
// checksum, not CRC32(object); preserve it exactly. Format is Ezz's GSC_OBJ and
// compiler emitter: include_offset+0x0c, count+0x44, file-relative uint32 strings.
// This is structural validation of the cloned area, not a bytecode verifier.
inline include_clone clone_with_include(std::span<const std::uint8_t> object,
    std::string_view dependency) {
  const auto fail = [](include_result result) { return include_clone{result, {}}; };
  if (object.size() > object_size_limit) return fail(include_result::size_limit);
  if (object.size() < object_header_size ||
      !std::equal(object_magic.begin(), object_magic.end(), object.begin()))
    return fail(include_result::invalid_header);
  for (const std::size_t field : {0x0c, 0x10, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c}) {
    const auto offset = read32(object, field);
    if (offset && (offset < object_header_size || offset > object.size()))
      return fail(include_result::invalid_header);
  }
  const auto code = read32(object, 0x14);
  const auto code_size = read32(object, 0x30);
  if (code < object_header_size || code > object.size() || code_size > object.size() - code)
    return fail(include_result::invalid_header);
  const auto filename = bounded_name(object, read32(object, 0x34));
  const auto name = filename ? canonical_script(*filename) : std::nullopt;
  const auto wanted = canonical_script(dependency);
  if (!name || !wanted) return fail(include_result::invalid_name);
  if (*name == *wanted) return fail(include_result::self_include);
  const auto count = object[0x44];
  const auto table = read32(object, 0x0c);
  const auto table_size = static_cast<std::size_t>(count) * 4;
  if (count && (table < object_header_size || table > object.size() ||
      table_size > object.size() - table)) return fail(include_result::invalid_include);
  bool present = false;
  for (std::size_t i = 0; i < count; ++i) {
    const auto raw = bounded_name(object, read32(object, table + i * 4));
    const auto path = raw ? canonical_script(*raw) : std::nullopt;
    if (!path) return fail(include_result::invalid_include);
    if (*path == *wanted) present = true;
  }
  if (present) return fail(include_result::already_present);
  if (count == 255) return fail(include_result::too_many_includes);
  const auto extra = wanted->size() + 1 + 3 + table_size + 4;
  if (extra > object_size_limit - object.size()) return fail(include_result::size_limit);
  include_clone output{include_result::inserted,
      std::vector<std::uint8_t>(object.begin(), object.end())};
  const auto path_offset = static_cast<std::uint32_t>(output.bytes.size());
  output.bytes.insert(output.bytes.end(), wanted->begin(), wanted->end());
  output.bytes.push_back(0);
  output.bytes.resize((output.bytes.size() + 3) & ~std::size_t{3}, 0);
  const auto new_table = static_cast<std::uint32_t>(output.bytes.size());
  output.bytes.resize(output.bytes.size() + table_size + 4, 0);
  if (table_size) std::copy_n(object.begin() + table, table_size, output.bytes.begin() + new_table);
  write32(output.bytes, new_table + table_size, path_offset);
  write32(output.bytes, 0x0c, new_table);
  output.bytes[0x44] = static_cast<std::uint8_t>(count + 1);
  return output;
}
} // namespace zombies::coop::t7
