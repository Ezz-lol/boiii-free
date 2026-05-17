#pragma once

#include "core.hpp"
#include "ui/core.hpp"
#include <cstdint>

namespace game {
namespace ugc {
enum class ZoneType : uint32_t { OFFICIAL = 0x0, MOD = 0x1, USERMAP = 0x2 };

struct WorkshopData {
  char title[100];
  str32_t folderName;
  str32_t publisherId;
  char description[256];
  char contentPathToZoneFiles[260];
  char absolutePathContentFolder[260];
  char absolutePathZoneFiles[260];
  int32_t unk;  // 1
  int32_t unk2; // 0
  uint32_t publisherIdInteger;
  int32_t unk3;
  uint32_t unk4;
  ZoneType type;
};

#ifdef __cplusplus
static_assert(sizeof(WorkshopData) == 0x4C8,
              "WorkshopData size must be 0x4C8 bytes");
#endif

template <const uint32_t POOL_SIZE> struct WorkshopDataPool {
  uint32_t count;
  uint8_t _padding04[4];
  WorkshopData data[POOL_SIZE];
};

template <const uint32_t POOL_SIZE>
struct ClientWorkshopDataPool : WorkshopDataPool<POOL_SIZE> {
  ui::UIModelIndex lastListEntryIdx;
};

static const uint32_t BUILTIN_WORKSHOP_DATA_POOL_SIZE = 128;
typedef WorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinWorkshopDataPool;

static const uint32_t EXTENDED_WORKSHOP_DATA_POOL_SIZE = 8192;
typedef ClientWorkshopDataPool<EXTENDED_WORKSHOP_DATA_POOL_SIZE>
    ExtendedWorkshopDataPool;
static const uint32_t EXTENDED_WORKSHOP_DATA_POOL_STRUCT_SIZE =
    sizeof(ExtendedWorkshopDataPool);
} // namespace ugc
} // namespace game