#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "core.hpp"

namespace game {
namespace db {

namespace xzone {

inline constexpr size_t ZONE_POOL_LENGTH = 64;

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
  int32_t flags;
  char name[64];
};
static_assert(sizeof(XZoneInfoInternal) == 0x44,
              "XZoneInfoInternal size must be 0x44 bytes");

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