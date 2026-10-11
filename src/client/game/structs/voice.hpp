#pragma once

#include <cstdint>

#include <dsound.h>

#include <game/structs/macros.hpp>
namespace game {
namespace voice {
/*
  This data structure is similar to the one used currently in the
  engine, but is known to be out of date and significantly smaller than the one
  currently used. This data structure was found in the BO2 dedicated server.

  Most of the platform voice-related data structures and architecture are
  nearly identical to that used in BO2, with encoding and decoding changed to
  use Steam's APIs instead of `speex`, and packets sent using the
  `LobbySession` abstraction instead of sent directly via lower level CL NET
  functions.

  This data structure, similarly, is mostly identical, but incrementing a
  pointer to this data structure where the correct data structure is expected
  leads to "p = p[5 + constant]" in decompilations, so is clearly missing some
  relatively large fields.
*/
PACKED(struct dsound_sample_t {
  IDirectSoundCaptureBuffer *DSCB;
  IDirectSoundBuffer *DSB;
  uint32_t dwBufferSize;
  uint32_t dwCaptureOffset;
  uint32_t currentOffset;
  uint32_t lastOffset;
  uint32_t currentBufferLength;
  int32_t stopPosition;
  uint32_t lastPlayPos;
  int32_t bytesBuffered;
  int32_t mode;
  int32_t frequency;
  int32_t volume;
  int32_t pan;
  int32_t channels;
  bool playing;
  uint8_t _padding45[3];
  int32_t channel;
  uint8_t playMode;
  uint8_t _padding4D[3];
});
ASSERT_SIZE(dsound_sample_t, 0x50);
} // namespace voice
} // namespace game
