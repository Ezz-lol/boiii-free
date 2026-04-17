#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "core.hpp"

namespace game {
namespace db {

namespace xzone {

static inline const size_t ZONE_POOL_LENGTH = 64;
// 64 slots allocated, but the engine will error if 64 are used
static inline const size_t MAX_ZONE_COUNT = 63;

template <typename T> using ZonePool = std::array<T, ZONE_POOL_LENGTH>;
enum class XZoneState : int32_t {
  XZONE_UNLOADING = -1,
  XZONE_EMPTY = 0x0,
  XZONE_LOADING = 0x1,
  XZONE_LOADED = 0x2,
  XZONE_COMPLETE = 0x3,
  XZONE_FAILED = 0x4,
};

// sizeof=0x10
#pragma pack(push, 1)
struct XZoneBuffer {
  const void *data;
  size_t dataSize;
};
static_assert(sizeof(XZoneBuffer) == 0x10,
              "XZoneBuffer size must be 0x10 bytes");
#pragma pack(pop)

struct XZoneInfoInternal {
  char name[64];           // Correct
  int allocFlags;          // Correct
  int freeFlags;           // Correct
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
static_assert(sizeof(XZoneInfoInternal) == 0x58,
              "XZoneInfoInternal size must be 0x58 bytes");

/*
   Correct. allocFlags and freeFlags are verified, and
   allocSlot-freeSlot are used as in earlier engines with size of allocSlot -
  end of XZoneBuffer confirmed as 0x18 (2 * 4 + sizeof(XZoneBuffer) == 0x18)
*/
#pragma pack(push, 1)
struct XZoneInfo {
  const char *name;
  int32_t allocFlags;
  int32_t freeFlags;
  int32_t allocSlot;
  int32_t freeSlot;
  XZoneBuffer fileBuffer;
};
static_assert(sizeof(XZoneInfo) == 0x28, "XZoneInfo size must be 0x28 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
struct XZoneName // Size must be 96 == 0x60
{
  char name[64];
  int32_t flags;
  int32_t slot;
  // Definitely value being set at _unknown[0]
  // It might be related to whether it's loaded?
  // No immediate indication of what this is for, and we do not currently need
  // to know, so leaving it as unknown for now.
  uint8_t _unknown[8];
  uint32_t fastfileChecksum;
  uint8_t _gap54[4]; // probably padding
  XZoneState state;
  bool streamPreloaded;
  char __padding[3];
};
static_assert(sizeof(XZoneName) == 0x60, "XZoneName size must be 0x60 bytes");
#pragma pack(pop)

// Verified correct
#pragma pack(push, 1)
struct XZone {
  uint16_t index;
  uint8_t _padding02[2];
  int flags;
  int slot;
  uint8_t _padding0C[4];
  db::XBlock blocks[10];
};
static_assert(sizeof(XZone) == 0xB0, "XZone size must be 0xB0 bytes");
#pragma pack(pop)

} // namespace xzone
} // namespace db
} // namespace game