#pragma once

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace mem {
PACKED(struct MemoryFile {
  uint8_t *buffer;
  int bufferSize;
  int bytesUsed;
  uint8_t *careerStatsBuffer;
  int cacheBufferUsed;
  int segmentIndex;
  int segmentStart;
  int nonZeroCount;
  int zeroCount;
  bool errorOnOverflow;
  bool memoryOverflow;
  bool compress_enabled;
  bool rle_enabled;
  bool is_writing;
  uint8_t _padding31[3];
  int cacheBufferAvail;
  uint8_t cacheBuffer[32760];
});

} // namespace mem
} // namespace game