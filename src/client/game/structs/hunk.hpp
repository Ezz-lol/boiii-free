#pragma once

#include <cstdint>
namespace game {
namespace hunk {

typedef uint64_t HUNKUSER_FLAGS;

enum class HU_ALLOCATION_SCHEME : uint32_t {
  HU_SCHEME_DEFAULT = 0x0,
  HU_SCHEME_DEBUG = 0x1,
  HU_SCHEME_FIRSTFIT = 0x2,
  HU_SCHEME_FIXED = 0x3,
  HU_SCHEME_NULL = 0x4,
  HU_SCHEME_DLMALLOC = 0x5,
  HU_SCHEME_MT = 0x6,
  HU_SCHEME_COUNT = 0x7,
};

#pragma pack(push, 1)
struct HunkUser {
  HU_ALLOCATION_SCHEME scheme;
  uint8_t _padding04[4];
  HUNKUSER_FLAGS flags;
  const char *name;
  int32_t type;
  uint8_t _padding1C[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct HunkUserDefault;

struct HunkUserDefault {
  HunkUser hunkUser;
  HunkUserDefault *current;
  HunkUserDefault *next;
  size_t maxSize;
  intptr_t end;
  intptr_t pos;
  intptr_t locked;
  uint8_t buf[1];
  uint8_t _padding51[7];
};
#pragma pack(pop)
} // namespace hunk
} // namespace game