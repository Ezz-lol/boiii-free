#include "../src/client/component/zombies/compiled_includes.hpp"
#include <cassert>
#include <limits>
using namespace zombies::coop::t7;

// A structural test object; these bytes are not claimed to be executable GSC.
std::vector<std::uint8_t> fixture(std::uint8_t includes = 1) {
  std::vector<std::uint8_t> data(object_header_size, 0);
  std::copy(object_magic.begin(), object_magic.end(), data.begin());
  const auto name = static_cast<std::uint32_t>(data.size());
  const std::string script = "scripts/zm/_zm_score.csc";
  data.insert(data.end(), script.begin(), script.end()); data.push_back(0);
  const auto dependency = static_cast<std::uint32_t>(data.size());
  const std::string path = "scripts/shared/system_shared";
  data.insert(data.end(), path.begin(), path.end()); data.push_back(0);
  data.resize((data.size() + 3) & ~std::size_t{3}, 0);
  const auto table = static_cast<std::uint32_t>(data.size());
  data.resize(data.size() + static_cast<std::size_t>(includes) * 4, 0);
  for (std::size_t i = 0; i < includes; ++i) write32(data, table + i * 4, dependency);
  const auto code = static_cast<std::uint32_t>(data.size());
  data.push_back(0x11); data.push_back(0x22);
  write32(data, 0x0c, table); write32(data, 0x14, code);
  write32(data, 0x30, 2); write32(data, 0x34, name); data[0x44] = includes;
  write32(data, 8, 0x12345678); // Declared source checksum need not hash object bytes.
  return data;
}
int main() {
  constexpr std::string_view companion = "scripts/zm/eroded_coop_fields";
  const auto input = fixture(); const auto original = input;
  const auto output = clone_with_include(input, companion);
  assert(output.result == include_result::inserted && input == original);
  assert(output.bytes[0x44] == 2);
  const auto table = read32(output.bytes, 0x0c);
  assert(read32(output.bytes, table) == read32(input, read32(input, 0x0c)));
  assert(bounded_name(output.bytes, read32(output.bytes, table + 4)) == companion);
  for (std::size_t i = 0; i < input.size(); ++i)
    if (!((i >= 0x0c && i < 0x10) || i == 0x44)) assert(output.bytes[i] == input[i]);
  assert(read32(output.bytes, 8) == read32(input, 8));
  assert(clone_with_include(output.bytes, companion).result == include_result::already_present);
  assert(clone_with_include(input, "SCRIPTS\\SHARED\\SYSTEM_SHARED.csc").result == include_result::already_present);
  assert(clone_with_include(input, "scripts/zm/_zm_score").result == include_result::self_include);
  assert(clone_with_include(fixture(0), companion).result == include_result::inserted);
  assert(clone_with_include(fixture(255), companion).result == include_result::too_many_includes);
  auto duplicate_then_bad = output.bytes;
  write32(duplicate_then_bad, read32(duplicate_then_bad, 0x0c),
      (std::numeric_limits<std::uint32_t>::max)());
  assert(clone_with_include(duplicate_then_bad, companion).result == include_result::invalid_include);
  for (const auto name : {"", "../escape", "/absolute", "a//b", "a/./b", "a/../b", "C:/file", "a\n"})
    assert(clone_with_include(input, name).result == include_result::invalid_name);
  for (std::size_t size = 0; size < object_header_size; ++size)
    assert(clone_with_include(std::span<const std::uint8_t>(input).first(size), companion).result == include_result::invalid_header);
  auto bad = input; bad[7] = 0x1b;
  assert(clone_with_include(bad, companion).result == include_result::invalid_header);
  bad = input; write32(bad, 0x0c, static_cast<std::uint32_t>(bad.size() - 1));
  assert(clone_with_include(bad, companion).result == include_result::invalid_include);
  bad = input; write32(bad, read32(bad, 0x0c), (std::numeric_limits<std::uint32_t>::max)());
  assert(clone_with_include(bad, companion).result == include_result::invalid_include);
  bad = input; write32(bad, 0x30, (std::numeric_limits<std::uint32_t>::max)());
  assert(clone_with_include(bad, companion).result == include_result::invalid_header);
  bad = input; write32(bad, 0x34, static_cast<std::uint32_t>(bad.size()));
  assert(clone_with_include(bad, companion).result == include_result::invalid_name);
}
