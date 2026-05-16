#pragma once

#include <cstdint>

namespace game {
namespace ugc {
enum class WorkshopType : uint32_t {
  WORKSHOP_MOD = 0x1,
  WORKSHOP_USERMAP = 0x2
};

struct WorkshopData {
  char title[100];
  char folderName[32];
  char publisherId[32];
  char description[256];
  char contentPathToZoneFiles[260];
  char absolutePathContentFolder[260];
  char absolutePathZoneFiles[260];
  int32_t unk;  // 1
  int32_t unk2; // 0
  uint32_t publisherIdInteger;
  int32_t unk3;
  uint32_t unk4;
  WorkshopType type;
};

#ifdef __cplusplus
static_assert(sizeof(WorkshopData) == 0x4C8,
              "WorkshopData size must be 0x4C8 bytes");
#endif

template <const uint32_t POOL_SIZE> struct WorkshopDataPool {
  uint32_t count;
  WorkshopData data[POOL_SIZE];
};

static const uint32_t BUILTIN_WORKSHOP_DATA_POOL_SIZE = 128;
typedef WorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinWorkshopDataPool;

static const uint32_t EXTENDED_WORKSHOP_DATA_POOL_SIZE = 8192;
typedef WorkshopDataPool<EXTENDED_WORKSHOP_DATA_POOL_SIZE>
    ExtendedWorkshopDataPool;
} // namespace ugc
} // namespace game