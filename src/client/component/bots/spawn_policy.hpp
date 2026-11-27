#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace bots::policy {
inline constexpr int maximum_slots = 18;

// Missing argument means one; an explicit empty/malformed argument is rejected.
inline std::optional<int>
parse_count(std::optional<std::string_view> argument) noexcept {
  if (!argument)
    return 1;
  if (*argument == "all")
    return maximum_slots;
  if (argument->empty())
    return std::nullopt;
  int count = 0;
  for (const char digit : *argument) {
    if (digit < '0' || digit > '9')
      return std::nullopt;
    const int value = digit - '0';
    if (count > (maximum_slots - value) / 10)
      return std::nullopt;
    count = count * 10 + value;
  }
  return count >= 1 && count <= maximum_slots ? std::optional<int>{count}
                                              : std::nullopt;
}

// BO3 playerName_t/name_t are 32 bytes, clanAbbrev_t is eight, including NUL.
// Keep delimiters/control bytes out of the locally generated connect userinfo.
inline constexpr std::size_t player_name_bytes = 32;
inline constexpr std::size_t clan_abbrev_bytes = 8;
inline std::string userinfo_component(std::string_view text,
                                      std::size_t field_bytes) {
  if (field_bytes < 2)
    return {};
  std::string result;
  result.reserve(std::min(text.size(), field_bytes - 1));
  for (const auto byte : text) {
    if (result.size() >= field_bytes - 1)
      break;
    const auto value = static_cast<unsigned char>(byte);
    result +=
        value < 32 || value == 127 || byte == '\\' || byte == '"' || byte == ';'
            ? '_'
            : byte;
  }
  return result;
}

struct session_snapshot {
  std::uint64_t epoch = 0;
  int mode = 0;
  int capacity = 0;
  int occupied = 0;
  bool running = false;
  bool zombies = false;
  bool schema_ready = false;
  int schema_capacity = 0;
  std::string map;
};

inline bool valid(const session_snapshot &value) noexcept {
  if (!value.running || value.epoch == 0 || value.map.empty() ||
      value.map == "core_frontend" || value.capacity < 1 ||
      value.capacity > maximum_slots || value.occupied < 0 ||
      value.occupied > value.capacity)
    return false;
  return !value.zombies || value.capacity <= 4 ||
         (value.capacity <= 8 && value.schema_ready &&
          value.schema_capacity == value.capacity);
}

inline bool same_session(const session_snapshot &requested,
                         const session_snapshot &current) noexcept {
  return valid(requested) && valid(current) &&
         requested.epoch == current.epoch && requested.mode == current.mode &&
         requested.zombies == current.zombies &&
         requested.capacity == current.capacity && requested.map == current.map;
}

inline std::optional<std::uint64_t>
parse_unsigned(std::string_view text, std::uint64_t maximum) noexcept {
  if (text.empty())
    return std::nullopt;
  std::uint64_t value = 0;
  for (const char digit : text) {
    if (digit < '0' || digit > '9')
      return std::nullopt;
    const auto next = static_cast<std::uint64_t>(digit - '0');
    if (next > maximum || value > (maximum - next) / 10)
      return std::nullopt;
    value = value * 10 + next;
  }
  return value;
}

inline bool may_drop_test_client(const session_snapshot &requested,
                                 const session_snapshot &current,
                                 std::uint64_t generation, int slot,
                                 bool connected,
                                 bool native_is_test_client) noexcept {
  return generation != 0 && generation == requested.epoch &&
         same_session(requested, current) && slot >= 0 &&
         slot < current.capacity && connected && native_is_test_client;
}

// Runs on the native server pipeline. Observe current occupancy and lifecycle
// before every allocation; pending humans/bots consume slots just like active
// ones.
template <typename ReadSession, typename AddBot>
int spawn_batch(const session_snapshot &requested, int count,
                ReadSession &&read_session, AddBot &&add_bot) {
  if (!valid(requested) || count < 1 || count > maximum_slots)
    return 0;
  int spawned = 0;
  for (; spawned < count; ++spawned) {
    const auto current = read_session();
    if (!current || !same_session(requested, *current) ||
        current->occupied >= current->capacity)
      break;
    if (!add_bot())
      break;
  }
  return spawned;
}
} // namespace bots::policy
