#pragma once

#include "core.hpp"
#include "steam.hpp"
#include "ui/core.hpp"
#include <cstdint>

namespace game {
namespace ugc {

typedef uint32_t UGCHash;
using UGCPath = str<260>;

struct WorkshopData {
  // Field is called "name" in lua table
  char title[100];
  name_t internalName;
  // Field is called "ugcName" in lua table
  str32_t publisherId;
  str256_t description;
  UGCPath contentPathToZoneFiles;
  UGCPath absolutePathContentDirectory;
  UGCPath absolutePathZoneFiles;
  /*
    Also used in engine to check if mod is installed
    by checking if version == 1.
    Unused logically in engine otherwise, but name of field is "ugcVersion" in
    corresponding lua table, so this must be the UGC version.
  */
  uint32_t version;
  uint8_t _padding4B4[4];
  steam::PublishedFileId_t publisherIdInteger;
  UGCHash publisherIdHash;
  ZoneType type;

  friend inline constexpr void clear(WorkshopData *self) noexcept {
    memset(static_cast<void *>(self), 0, sizeof(WorkshopData));
  }
};

ASSERT_SIZE(WorkshopData, 0x4C8);

enum class ModLoadState : uint32_t {
  IDLE = 0x0,
  LOADING = 0x1,
  COMPLETE = 0x2,
};
IMPL_ENUM_OPERATORS(ModLoadState);

struct ActiveMod : WorkshopData {
  ModLoadState loadState;
  uint8_t _padding4CC[4];
};
ASSERT_SIZE(ActiveMod, 0x4D0);

template <const uint32_t POOL_SIZE> struct WorkshopDataPool {
  uint32_t count;
  uint8_t _padding04[4];
  WorkshopData data[POOL_SIZE];

  friend inline constexpr void
  clear(WorkshopDataPool<POOL_SIZE> *self) noexcept {
    memset(static_cast<void *>(self), 0, sizeof(WorkshopDataPool<POOL_SIZE>));
  }
};

template <const uint32_t POOL_SIZE>
struct ClientWorkshopDataPool : WorkshopDataPool<POOL_SIZE> {
  LocalClientPool<ui::UIModelIndex> listModels;
  uint8_t _padding[4];
};

constexpr uint32_t BUILTIN_WORKSHOP_DATA_POOL_SIZE = 128;
typedef WorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinWorkshopDataPool;
ASSERT_SIZE(BuiltinWorkshopDataPool, 0x26408);
typedef ClientWorkshopDataPool<BUILTIN_WORKSHOP_DATA_POOL_SIZE>
    BuiltinClientWorkshopDataPool;
ASSERT_SIZE(BuiltinClientWorkshopDataPool, 0x26410);

constexpr uint32_t EXTENDED_WORKSHOP_DATA_POOL_SIZE = 8192;
typedef ClientWorkshopDataPool<EXTENDED_WORKSHOP_DATA_POOL_SIZE>
    ExtendedWorkshopDataPool;
constexpr uint32_t EXTENDED_WORKSHOP_DATA_POOL_STRUCT_SIZE =
    sizeof(ExtendedWorkshopDataPool);

typedef WorkshopData *SteamUGCRequestDetailsResult_t;

} // namespace ugc
} // namespace game