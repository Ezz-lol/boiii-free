#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace zombies::coop::builtin_snapshot {
// Source: Ezz game/symbols/scr/builtin/table/core.hpp and its 32-byte
// CommonFunctionTable definitions. These coordinates are dedicated-only.
constexpr std::uintptr_t preferred_base = 0x140000000;
constexpr std::uintptr_t common_table = 0x1410D59D0;
constexpr std::size_t expected_index = 217;
constexpr std::size_t connected_index = 218;
constexpr std::uint32_t dedicated_checksum = 0x014C28B4;
constexpr std::uint32_t expected_hash = 0x4441FF8D;
constexpr std::uint32_t connected_hash = 0x6C6E7B10;
constexpr std::uintptr_t expected_action = 0x1402CB900;
constexpr std::size_t code_limit = 256;
constexpr std::size_t json_limit = 2048;

struct builtin_entry {
  std::uint32_t canonical{}, minimum{}, maximum{}, padding0{};
  std::uint64_t action{};
  std::uint32_t type{}, padding1{};
};
static_assert(sizeof(builtin_entry) == 32);
static_assert(offsetof(builtin_entry, action) == 16);
static_assert(connected_index == expected_index + 1);

struct memory_region {
  std::uintptr_t base{}, size{}, allocation_base{};
  bool committed{}, readable{}, executable{}, guarded{};
};

inline bool range_contains(std::uintptr_t base, std::uintptr_t size,
                           std::uintptr_t address, std::size_t length) noexcept {
  return length && size && base <= (std::numeric_limits<std::uintptr_t>::max)() - size
         && address >= base && length <= size && address - base <= size - length;
}

// Each span is checked before a guarded copy. The platform adapter uses
// ReadProcessMemory on this process, so an intervening unmap fails the read.
template <typename Reader>
bool read_checked(Reader& reader, std::uintptr_t base, std::uintptr_t size,
                  std::uintptr_t address, void* destination, std::size_t length,
                  bool require_executable) noexcept {
  if (!destination || !range_contains(base, size, address, length)) return false;
  auto cursor = address;
  auto remaining = length;
  for (unsigned region_count = 0; remaining && region_count < 8; ++region_count) {
    memory_region region{};
    if (!reader.query(cursor, region) || !region.committed || !region.readable
        || region.guarded || (require_executable && !region.executable)
        || region.allocation_base != base
        || !range_contains(region.base, region.size, cursor, 1)) return false;
    const auto available = region.size - (cursor - region.base);
    const auto amount = remaining < available ? remaining : available;
    cursor += amount;
    remaining -= amount;
  }
  return !remaining && reader.copy(address, destination, length);
}

struct snapshot {
  bool valid{};
  const char* reason = "header_unreadable";
  std::uintptr_t image_base{}, image_size{}, entry_address{}, action_address{};
  std::uint32_t checksum{}, section_index{}, code_size{};
  builtin_entry expected{}, connected{};
  std::array<std::uint8_t, code_limit> code{};
};

template <typename Value, std::size_t Size>
Value value_at(const std::array<std::uint8_t, Size>& bytes, std::size_t offset) noexcept {
  Value result{};
  if (offset <= Size && sizeof(Value) <= Size - offset)
    std::memcpy(&result, bytes.data() + offset, sizeof(result));
  return result;
}

template <typename Reader>
snapshot capture(Reader& reader, std::uintptr_t base) noexcept {
  snapshot result{};
  result.image_base = base;
  // PE/COFF header offsets are SDK-defined, not inferred engine structures.
  std::array<std::uint8_t, 64> dos{};
  if (!base || !read_checked(reader, base, 4096, base, dos.data(), dos.size(), false)) return result;
  const auto nt_offset = value_at<std::uint32_t>(dos, 60);
  std::array<std::uint8_t, 264> nt{};
  if (value_at<std::uint16_t>(dos, 0) != 0x5A4D || nt_offset < dos.size()
      || nt_offset > 4096 - nt.size()) {
    result.reason = "invalid_dos_header";
    return result;
  }
  if (!read_checked(reader, base, 4096, base + nt_offset, nt.data(), nt.size(), false)) return result;
  const auto section_count = value_at<std::uint16_t>(nt, 6);
  const auto optional_size = value_at<std::uint16_t>(nt, 20);
  result.image_size = value_at<std::uint32_t>(nt, 80);
  result.checksum = value_at<std::uint32_t>(nt, 88);
  if (value_at<std::uint32_t>(nt, 0) != 0x00004550
      || value_at<std::uint16_t>(nt, 4) != 0x8664
      || value_at<std::uint16_t>(nt, 24) != 0x20B || optional_size < 240
      || value_at<std::uint64_t>(nt, 48) != preferred_base
      || !section_count || section_count > 96
      || nt_offset + 24u + optional_size > 4096
      || section_count > (4096 - nt_offset - 24u - optional_size) / 40u
      || !range_contains(base, result.image_size, base, 4096)) {
    result.reason = "invalid_pe_header";
    return result;
  }
  if (result.checksum != dedicated_checksum) {
    result.reason = "unsupported_checksum";
    return result;
  }
  constexpr auto table_offset = common_table - preferred_base + expected_index * sizeof(builtin_entry);
  if (table_offset > result.image_size || 2 * sizeof(builtin_entry) > result.image_size - table_offset) {
    result.reason = "table_outside_image";
    return result;
  }
  std::array<builtin_entry, 2> entries{};
  if (!read_checked(reader, base, result.image_size, base + table_offset,
                    entries.data(), sizeof(entries), false)) {
    result.reason = "table_unreadable";
    return result;
  }
  result.expected = entries[0];
  result.connected = entries[1];
  result.entry_address = base + table_offset + sizeof(builtin_entry);
  if (result.expected.canonical != expected_hash || result.connected.canonical != connected_hash
      || result.expected.minimum || result.expected.maximum || result.expected.type
      || result.connected.minimum || result.connected.maximum || result.connected.type
      || result.expected.action != base + (expected_action - preferred_base)) {
    result.reason = "table_identity_mismatch";
    return result;
  }
  result.action_address = static_cast<std::uintptr_t>(result.connected.action);
  if (!range_contains(base, result.image_size, result.action_address, code_limit)) {
    result.reason = "action_outside_image";
    return result;
  }
  bool executable_section = false;
  for (std::uint16_t i = 0; i < section_count; ++i) {
    std::array<std::uint8_t, 40> section{};
    const auto address = base + nt_offset + 24u + optional_size + i * section.size();
    if (!read_checked(reader, base, 4096, address, section.data(), section.size(), false)) {
      result.reason = "section_unreadable";
      return result;
    }
    const auto size = value_at<std::uint32_t>(section, 8);
    const auto rva = value_at<std::uint32_t>(section, 12);
    if (rva > result.image_size || size > result.image_size - rva) continue;
    if ((value_at<std::uint32_t>(section, 36) & 0x20000000u)
        && range_contains(base + rva, size, result.action_address, code_limit)) {
      result.section_index = i;
      executable_section = true;
      break;
    }
  }
  if (!executable_section) {
    result.reason = "action_not_executable_section";
    return result;
  }
  if (!read_checked(reader, base, result.image_size, result.action_address,
                    result.code.data(), result.code.size(), true)) {
    result.reason = "action_unreadable";
    return result;
  }
  result.valid = true;
  result.code_size = static_cast<std::uint32_t>(result.code.size());
  result.reason = "ok";
  return result;
}

inline int format(const snapshot& value, const char* family, char* destination,
                  std::size_t capacity) noexcept {
  if (!destination || !capacity || capacity > json_limit || !family
      || (std::strcmp(family, "cb") != 0 && std::strcmp(family, "ezz") != 0)
      || value.code_size > code_limit) return -1;
  std::array<char, code_limit * 2 + 1> hex{};
  constexpr char digits[] = "0123456789abcdef";
  for (std::size_t i = 0; i < value.code_size; ++i) {
    hex[2 * i] = digits[value.code[i] >> 4];
    hex[2 * i + 1] = digits[value.code[i] & 15];
  }
  const auto relative = value.action_address >= value.image_base
      ? value.action_address - value.image_base : 0;
  const auto written = std::snprintf(destination, capacity,
      "{\"version\":1,\"family\":\"%s\",\"role\":\"dedicated\",\"captured\":%s,\"reason\":\"%s\","
      "\"image_base\":\"0x%llx\",\"image_size\":%llu,\"checksum\":%u,"
      "\"entry_address\":\"0x%llx\",\"canonical_hash\":%u,\"min_args\":%u,\"max_args\":%u,\"type\":%u,"
      "\"expected_canonical_hash\":%u,\"expected_action\":\"0x%llx\","
      "\"action_address\":\"0x%llx\",\"action_rva\":\"0x%llx\",\"section_index\":%u,"
      "\"code_bytes\":%u,\"code_hex\":\"%s\",\"scope\":\"read_only_no_invocation\"}\n",
      family, value.valid ? "true" : "false", value.reason,
      static_cast<unsigned long long>(value.image_base), static_cast<unsigned long long>(value.image_size), value.checksum,
      static_cast<unsigned long long>(value.entry_address), value.connected.canonical,
      value.connected.minimum, value.connected.maximum, value.connected.type, value.expected.canonical,
      static_cast<unsigned long long>(value.expected.action), static_cast<unsigned long long>(value.action_address),
      static_cast<unsigned long long>(relative), value.section_index, value.code_size, hex.data());
  return written >= 0 && static_cast<std::size_t>(written) < capacity ? written : -1;
}

class attempt_budget {
  std::atomic_flag busy = ATOMIC_FLAG_INIT;
  unsigned attempts{};
  bool succeeded{};
public:
  unsigned claim() noexcept {
    if (busy.test_and_set(std::memory_order_acquire)) return 0;
    if (succeeded || attempts >= 2) {
      busy.clear(std::memory_order_release);
      return 0;
    }
    return ++attempts;
  }
  void finish(bool success) noexcept {
    succeeded = success;
    busy.clear(std::memory_order_release);
  }
};
} // namespace zombies::coop::builtin_snapshot
