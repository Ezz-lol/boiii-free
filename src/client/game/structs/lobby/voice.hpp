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
