#pragma once

#include <game/structs/macros.hpp>

namespace game {
namespace cl {
namespace voice {

PACKED(struct ClientVoicePacket_t {
  uint8_t data[256];
  int32_t dataSize;
});

PACKED(struct voiceCommunication_t {
  ClientVoicePacket_t voicePackets[10];
  int32_t voicePacketCount;
  int32_t voicePacketLastTransmit;
  int32_t voiceTotalBytes;
});
} // namespace voice
} // namespace cl
} // namespace game
