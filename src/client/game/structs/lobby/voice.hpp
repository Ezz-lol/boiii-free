#pragma once

#include "game/structs/lobby/core.hpp"
#include <cstdint>

#include <game/structs/macros.hpp>

namespace game {
namespace lobby {
namespace voice {
enum class eVoipStatus : int32_t {
  HIDDEN = 0,
  TALKING = 1,
  NOTTALKING = 2,
  MUTED = 3,
  ENEMY_TALKING = 4,
};

PACKED(struct Msg_VoicePacket {
  LobbyType lobbyType;
  uint8_t talkerIndex;
  uint8_t _padding05[3];
  int32_t relayBits;
  uint16_t sizeofVoiceData;
  uint8_t numPacketsInData;
  uint8_t voiceData[1198];
  uint8_t _padding4BD[3];
});
ASSERT_SIZE(Msg_VoicePacket, 0x4C0);

PACKED(struct SharedVoiceSessionData {
  uint32_t voiceOutPort;
  uint32_t microphonePort;
  uint32_t headsetPort;
  bool microphoneAttached;
  uint8_t _padding0D[3];
  uint32_t lastCaughtTalking;
});
ASSERT_SIZE(SharedVoiceSessionData, 0x14);

} // namespace voice
} // namespace lobby
} // namespace game
