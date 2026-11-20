#include <std_include.hpp>

namespace game {
namespace sv {

namespace {
constexpr size_t DEDICATED_FIELD_SHIFT = 8;

template <typename T> T *field_at(client_s *client, const size_t offset) {
  return reinterpret_cast<T *>(reinterpret_cast<char *>(client) + offset +
                               (is_server() ? DEDICATED_FIELD_SHIFT : 0));
}

template <typename T>
const T *field_at(const client_s *client, const size_t offset) {
  return field_at<T>(const_cast<client_s *>(client), offset);
}
} // namespace

level::gentity_t *client_s::get_gentity() const noexcept {
  return *field_at<level::gentity_t *>(this, offsetof(client_s, gentity));
}

char *client_s::get_name() noexcept {
  return field_at<char>(this, offsetof(client_s, name));
}

const char *client_s::get_name() const noexcept {
  return field_at<char>(this, offsetof(client_s, name));
}

char *client_s::get_clan_abbrev() noexcept {
  return field_at<char>(this, offsetof(client_s, clanAbbrev));
}

const char *client_s::get_clan_abbrev() const noexcept {
  return field_at<char>(this, offsetof(client_s, clanAbbrev));
}

XUID client_s::get_xuid() const noexcept {
  return *field_at<XUID>(this, offsetof(client_s, xuid));
}

void client_s::set_xuid(const XUID value) noexcept {
  *field_at<XUID>(this, offsetof(client_s, xuid)) = value;
}

std::string client_s::serialize() const noexcept {
  return std::format(
      "{{ \"state\": {}, \"address\": {}, "
      "\"reliableSequence\": {}, \"reliableAcknowledge\": {}, "
      "\"messageAcknowledge\": {}, \"gentity\": {}, \"name\": \"{}\", "
      "\"clanAbbrev\": \"{}\", \"xuid\": 0x{:016X}, \"doubleXPGroupMask\": "
      "{:08X}, \"availableMappackFlags\": 0x{:08X}, \"nextReliableTime\": "
      "{}, "
      "\"lastPacketTime\": {}, \"lastConnectTime\": {}, "
      "\"nextSnapshotTime\": {}, \"lastSnapshotTime\": {}, \"timeoutCount\": "
      "{}, "
      "\"guid\": 0x{:08X}, \"scriptId\": 0x{:08X}, \"isSplitscreenClient\": "
      "{}, "
      "\"isSecondScreenClient\": {}, \"bIsTestClient\": {}, \"serverId\": "
      "{:08X} }}",
      net::serialize(state), address.serialize(), reliableSequence,
      reliableAcknowledge, messageAcknowledge, ::serialize(get_gentity()),
      get_name(), get_clan_abbrev(), get_xuid(), doubleXPGroupMask,
      availableMappackFlags, nextReliableTime, lastPacketTime, lastConnectTime,
      nextSnapshotTime, lastSnapshotTime, timeoutCount, guid, scriptId,
      ::serialize(isSplitscreenClient), ::serialize(isSecondScreenClient),
      ::serialize(bIsTestClient), serverId);
}

} // namespace sv
} // namespace game
