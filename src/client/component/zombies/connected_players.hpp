#pragma once

#include "builtin_snapshot.hpp"

namespace zombies::coop::connected_players_policy {
struct context {
  bool hook_ready{}, dedicated{}, opted_in{}, zombies{}, frontend{}, map_armed{};
  int capacity{};
};

inline bool eligible(const context& current) noexcept {
  return current.hook_ready && current.dedicated && current.opted_in
      && current.zombies && !current.frontend && current.map_armed
      && current.capacity >= 5 && current.capacity <= 8;
}

inline bool default_value(int instance, unsigned index, bool original,
                          const context& current) noexcept {
  return original || (instance == 0 && index == 0 && eligible(current));
}

template <typename Counter>
int expected_floor(int original, const context& current, Counter&& counter) {
  if (!eligible(current)) return original; // Do not access native pools here.
  const int count = counter();
  if (count < 0 || count > current.capacity) return original;
  return original < count ? count : original;
}

template <typename Client, typename State>
int native_connected_count(const context& current, const Client* clients,
                            State connected_state) noexcept {
  if (!eligible(current) || !clients) return 0;
  int result = 0;
  for (int slot = 0; slot < current.capacity; ++slot)
    if (clients[slot].state >= connected_state) ++result;
  return result;
}

inline constexpr std::uintptr_t action_rva = 0x2CB8A0;
inline constexpr std::size_t optional_call_offset = 13;
inline constexpr std::uintptr_t optional_bool_rva = 0x1708E0;
// Exact supported dedicated bytes captured read-only from the pinned image.
// This ends at 0x1402CB8FD after the tail jump, excluding unused padding and
// the adjacent expected-player function.
inline constexpr std::array<std::uint8_t, 93> pristine_body = {
  0x40,0x53,0x48,0x83,0xEC,0x20,0x45,0x33,0xC0,0x33,0xD2,0x33,0xC9,0xE8,0x2E,0x50,
  0xEA,0xFF,0x48,0x8B,0x0D,0xB7,0x35,0x1C,0x09,0x0F,0xB6,0xD8,0xE8,0x5F,0xA3,0x2A,
  0x00,0x33,0xD2,0x4C,0x63,0xC0,0x85,0xC0,0x7E,0x27,0x48,0x8B,0x0D,0xC7,0xD5,0xEA,
  0x09,0x83,0x39,0x05,0x75,0x0F,0x84,0xDB,0x75,0x09,0x83,0xB9,0x60,0xB3,0x0B,0x00,
  0x00,0x75,0x02,0xFF,0xC2,0x48,0x81,0xC1,0x10,0x51,0x0E,0x00,0x49,0xFF,0xC8,0x75,
  0xE0,0x33,0xC9,0x48,0x83,0xC4,0x20,0x5B,0xE9,0x63,0x38,0xEA,0xFF
};

inline bool verified_body(const builtin_snapshot::snapshot& captured) noexcept {
  if (!captured.valid || captured.checksum != builtin_snapshot::dedicated_checksum
      || !builtin_snapshot::range_contains(captured.image_base, captured.image_size,
                                           captured.action_address, pristine_body.size())
      || !builtin_snapshot::range_contains(captured.image_base, captured.image_size,
                                           captured.entry_address, sizeof(builtin_snapshot::builtin_entry))
      || captured.entry_address - captured.image_base !=
          builtin_snapshot::common_table - builtin_snapshot::preferred_base
            + builtin_snapshot::connected_index * sizeof(builtin_snapshot::builtin_entry)
      || captured.action_address < captured.image_base
      || captured.connected.action != captured.action_address
      || captured.action_address - captured.image_base != action_rva
      || captured.code_size < pristine_body.size() || captured.code_size > builtin_snapshot::code_limit
      || captured.connected.canonical != builtin_snapshot::connected_hash
      || captured.connected.minimum || captured.connected.maximum > 1 || captured.connected.type
      || captured.expected.canonical != builtin_snapshot::expected_hash
      || captured.expected.minimum || captured.expected.maximum > 1 || captured.expected.type
      || captured.expected.action != captured.image_base + (builtin_snapshot::expected_action - builtin_snapshot::preferred_base)
      || std::memcmp(captured.code.data(), pristine_body.data(), pristine_body.size()) != 0) return false;
  std::int32_t displacement{};
  std::memcpy(&displacement, captured.code.data() + optional_call_offset + 1, sizeof(displacement));
  return captured.code[optional_call_offset] == 0xE8
      && displacement == static_cast<std::int32_t>(
          static_cast<std::int64_t>(optional_bool_rva) - static_cast<std::int64_t>(action_rva + optional_call_offset + 5));
}
} // namespace zombies::coop::connected_players_policy

namespace zombies::coop {
connected_players_policy::context connected_players_context() noexcept;
}
