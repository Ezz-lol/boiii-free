#include <std_include.hpp>



namespace game {
namespace sv {

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
      clanAbbrev, xuid, doubleXPGroupMask, availableMappackFlags,
      nextReliableTime, lastPacketTime, lastConnectTime, nextSnapshotTime,
      lastSnapshotTime, timeoutCount, guid, scriptId,
      ::serialize(isSplitscreenClient), ::serialize(isSecondScreenClient),
      ::serialize(bIsTestClient), serverId);
}

} // namespace sv
} // namespace game
