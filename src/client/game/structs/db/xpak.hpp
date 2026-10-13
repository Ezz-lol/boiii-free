#pragma once

#include "../stream.hpp"
#include <cstdint>

namespace game {
namespace db {
namespace xpak {
struct XPakLoadedPackFileRef {
  uint64_t size; // offset 0

  int32_t refCount;         // offset 9
  stream::stream_fileid fh; // offset 13

  inline std::string serialize() const noexcept {
    return std::format("{{ \"size\": {}, \"refCount\": {}, \"fh\": 0x{:08X} }}",
                       size, refCount, fh);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"key\": 0x{:016X}, \"offset\": {}, \"size\": {}, \"xpakIndex\": "
        "{}, "
        "\"compressed\": {}, \"valid\": {}, \"adjacentLeftType\": {}, "
        "\"adjacentRightType\": {}, "
        "\"adjacentLeft\": {}, \"adjacentRight\": {}, \"padding\": {} }}",
        key, offset, size, static_cast<uint64_t>(xpakIndex),
        static_cast<bool>(compressed), static_cast<bool>(valid),
        static_cast<uint64_t>(adjacentLeftType),
        static_cast<uint64_t>(adjacentRightType),
        static_cast<uint64_t>(adjacentLeft),
        static_cast<uint64_t>(adjacentRight), static_cast<uint64_t>(padding));
  }
};

enum class StreamWrappedType : uint8_t {
  PROBE_ARRAY = 0x0,
  PROBE_VOLUME = 0x1,
  SST = 0x2,
  UNCOMP_SIEGE = 0x3,
  SKY_BOX = 0x4,
};

inline constexpr const char *serialize(const StreamWrappedType type) {
  switch (type) {
  case StreamWrappedType::PROBE_ARRAY:
    return "PROBE_ARRAY";
  case StreamWrappedType::PROBE_VOLUME:
    return "PROBE_VOLUME";
  case StreamWrappedType::SST:
    return "SST";
  case StreamWrappedType::UNCOMP_SIEGE:
    return "UNCOMP_SIEGE";
  case StreamWrappedType::SKY_BOX:
    return "SKY_BOX";
  default:
    return "STREAM_WRAPPED_TYPE_INVALID";
  }
}

enum class StreamWrappedMode : uint8_t {
  RESIDENT = 0x0,
  STREAMED = 0x1,
};

inline constexpr const char *serialize(const StreamWrappedMode mode) {
  switch (mode) {
  case StreamWrappedMode::RESIDENT:
    return "RESIDENT";
  case StreamWrappedMode::STREAMED:
    return "STREAMED";
  default:
    return "STREAM_WRAPPED_MODE_INVALID";
  }
}

struct StreamWrappedBuffer {
  StreamWrappedHandle handle;
  XPakEntryInfo xpakEntry;
  const char *name;
  StreamWrappedType type;
  StreamWrappedMode mode;
  uint32_t dataSize;
  uint8_t *data;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"handle\": {}, \"xpakEntry\": {}, \"name\": {}, \"type\": {}, "
        "\"mode\": {}, \"dataSize\": {}, \"data\": {:p} }}",
        handle, xpakEntry.serialize(), ::serialize(name),
        game::db::xpak::serialize(type), game::db::xpak::serialize(mode),
        dataSize, reinterpret_cast<const void *>(data));
  }
};

} // namespace xpak
} // namespace db
} // namespace game
