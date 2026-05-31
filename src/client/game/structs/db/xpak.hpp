#pragma once

#include <cstdint>
#include "../stream.hpp"

namespace game {
namespace db {
namespace xpak {
struct XPakLoadedPackFileRef {
  uint64_t size; // offset 0

  int32_t refCount;         // offset 9
  stream::stream_fileid fh; // offset 13
};

typedef int32_t StreamWrappedHandle;
struct XPakEntryInfo {
  uint64_t key;
  int64_t offset;
  uint64_t size;
  uint64_t xpakIndex : 5;
  uint64_t compressed : 1;
  uint64_t valid : 1;
  uint64_t adjacentLeftType : 3;
  uint64_t adjacentRightType : 3;
  uint64_t adjacentLeft : 18;
  uint64_t adjacentRight : 18;
  uint64_t padding : 15;
};

enum class StreamWrappedType : uint8_t {
  PROBE_ARRAY = 0x0,
  PROBE_VOLUME = 0x1,
  SST = 0x2,
  UNCOMP_SIEGE = 0x3,
  SKY_BOX = 0x4,
};

enum class StreamWrappedMode : uint8_t {
  RESIDENT = 0x0,
  STREAMED = 0x1,
};

struct StreamWrappedBuffer {
  StreamWrappedHandle handle;
  XPakEntryInfo xpakEntry;
  const char *name;
  StreamWrappedType type;
  StreamWrappedMode mode;
  uint32_t dataSize;
  uint8_t *data;
};

} // namespace xpak
} // namespace db
} // namespace game