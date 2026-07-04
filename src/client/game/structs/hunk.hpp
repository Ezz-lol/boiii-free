#pragma once

#include <cstdint>
#include "func.hpp"
#include "macros.hpp"

namespace game {
namespace hunk {

typedef uint32_t HUNKUSER_FLAGS;

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
  HUNKUSER_FLAGS flags;
  const char *name;
  int32_t type;
  uint8_t _padding14[4];
};
ASSERT_SIZE(HunkUser, 0x18);
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
ASSERT_SIZE(HunkUserDefault, 0x50);
#pragma pack(pop)

struct HunkUserNull {
  HunkUser hunkUser;
  int32_t size;
  int32_t alignment;
};
ASSERT_SIZE(HunkUserNull, 0x20);

struct ALLOCATION_SCHEME_FUNCTIONS {
  fastcall_t<HunkUser *(HunkUserDefault *defaultUser, uint64_t size,
                        HU_ALLOCATION_SCHEME scheme, HUNKUSER_FLAGS flags,
                        void *scheme_specific_data, const char *name,
                        int32_t type)>
      Init;
  fastcall_t<void(HunkUser *user)> Reset;
  fastcall_t<void(HunkUser *user)> Destroy;
  fastcall_t<void *(HunkUser *user, uint64_t size, int32_t alignment,
                    const char *name)>
      Alloc;
  fastcall_t<void(HunkUser *user, void *alloc)> Free;
};

union HunkUserAllocationSchemeMap {
  struct {
    ALLOCATION_SCHEME_FUNCTIONS _default;
    ALLOCATION_SCHEME_FUNCTIONS debug;
    ALLOCATION_SCHEME_FUNCTIONS firstFit;
    ALLOCATION_SCHEME_FUNCTIONS fixed;
    ALLOCATION_SCHEME_FUNCTIONS null;
    ALLOCATION_SCHEME_FUNCTIONS dlMalloc;
    ALLOCATION_SCHEME_FUNCTIONS mt;
  };
  ALLOCATION_SCHEME_FUNCTIONS
  functions[static_cast<size_t>(HU_ALLOCATION_SCHEME::HU_SCHEME_COUNT)];
};
ASSERT_SIZE(HunkUserAllocationSchemeMap, 0x118);
} // namespace hunk
} // namespace game