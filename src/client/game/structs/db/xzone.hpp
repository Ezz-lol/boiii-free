#pragma once

#include <cstddef>
#include <cstdint>

#include <game/structs/core.hpp>
#include <game/structs/db/core.hpp>

#include <structs/str.hpp>

namespace game {
namespace db {

namespace xzone {

static inline const size_t ZONE_POOL_LENGTH = 64;
// 64 slots allocated, but the engine will error if 64 are used
static inline const size_t MAX_ZONE_COUNT = 63;

template <typename T> using ZonePool = array<T, ZONE_POOL_LENGTH>;
enum class XZoneState : int32_t {
  XZONE_UNLOADING = -1,
  XZONE_EMPTY = 0x0,
  XZONE_LOADING = 0x1,
  XZONE_LOADED = 0x2,
  XZONE_COMPLETE = 0x3,
  XZONE_FAILED = 0x4,
};

// sizeof=0x10
PACKED(struct XZoneBuffer {
  const void *data;
  size_t dataSize;

  inline std::span<const uint8_t> buffer() const noexcept {
    return std::span(reinterpret_cast<const uint8_t *>(data), dataSize);
  }
});
ASSERT_SIZE(XZoneBuffer, 0x10);

struct XZoneInfoInternal {
  str64_t name;            // Correct
  int32_t allocFlags;      // Correct
  int32_t freeFlags;       // Correct
  XZoneInfoInternal *next; // Most likely correct
  /*
    Almost certainly a pointer, unsure on what _to_.
    My guess was the previous node if this is a doubly linked list, but
    otherwise this could maybe be a list of dependency xzones, or maybe an
    XZoneBuffer.

    TODO: investigate and confirm actual pointee type and update
  */
  XZoneInfoInternal *prevMaybe;
};
// Correct size is 0x58
ASSERT_SIZE(XZoneInfoInternal, 0x58);

/*
  Verified. allocFlags and freeFlags are verified, and
  allocSlot-freeSlot are used as in earlier engines with size of allocSlot -
  end of XZoneBuffer confirmed as 0x18 (2 * 4 + sizeof(XZoneBuffer) == 0x18)
*/
PACKED(struct XZoneInfo {
  const char *name;
  int32_t allocFlags;
  int32_t freeFlags;
  int32_t allocSlot;
  int32_t freeSlot;
  XZoneBuffer fileBuffer;
});
ASSERT_SIZE(XZoneInfo, 0x28);

PACKED(struct XZoneName // Size must be 96 == 0x60
       {
         char name[64];
         int32_t flags;
         int32_t slot;
         ZoneType zoneType;
         uint8_t _padding4C[4];
         size_t size;
         XZoneState state;
         bool streamPreloaded;
         uint8_t _padding5d[3];
       });
ASSERT_SIZE(XZoneName, 0x60);

// Verified
PACKED(struct XZone {
  uint16_t index;
  uint8_t _padding02[2];
  int32_t flags;
  int32_t slot;
  uint8_t _padding0C[4];
  db::XBlock blocks[10];
});
ASSERT_SIZE(XZone, 0xB0);

} // namespace xzone
} // namespace db
} // namespace game
