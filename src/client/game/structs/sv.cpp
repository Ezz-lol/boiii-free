#include <std_include.hpp>

namespace game {
namespace sv {

namespace {
constexpr size_t DEDICATED_XUID_OFFSET = 0x55D0;
}

XUID client_s::get_xuid() const noexcept {
  if (is_server()) {
    return *reinterpret_cast<const XUID *>(
        reinterpret_cast<const char *>(this) + DEDICATED_XUID_OFFSET);
  }
  return xuid;
}

void client_s::set_xuid(const XUID value) noexcept {
  if (is_server()) {
    *reinterpret_cast<XUID *>(reinterpret_cast<char *>(this) +
                              DEDICATED_XUID_OFFSET) = value;
  } else {
    xuid = value;
  }
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
      reliableAcknowledge, messageAcknowledge, ::serialize(gentity), name,
      clanAbbrev, get_xuid(), doubleXPGroupMask, availableMappackFlags,
      nextReliableTime, lastPacketTime, lastConnectTime, nextSnapshotTime,
      lastSnapshotTime, timeoutCount, guid, scriptId,
      ::serialize(isSplitscreenClient), ::serialize(isSecondScreenClient),
      ::serialize(bIsTestClient), serverId);
}

} // namespace sv
} // namespace game
