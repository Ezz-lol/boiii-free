#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace pmem {
#ifndef PMem_Alloc
#define PMem_Alloc(size, alignment, pool, stack, asyncCommit, memTrack)        \
  _PMem_Alloc(size, alignment, pool, stack, asyncCommit, memTrack, __FILE__,   \
              __LINE__)
#endif

WEAK symbol<void(const char *name, int allocDir)> PMem_Free{0x1420C48B0,
                                                            0x1405D8B30};
WEAK symbol<void(const char *name, PMemStack stack, EMemTrack memTrack)>
    PMem_BeginAlloc{0x1420C47C0, 0x0};
WEAK symbol<void(const char *name, PMemStack stack)> PMem_EndAlloc{0x1420C4850,
                                                                   0x0};
WEAK symbol<void *(size_t size, size_t alignment, PMemPool pool,
                   PMemStack stack, bool asyncCommit, EMemTrack memTrack,
                   const char *file, int lineNum)>
    _PMem_Alloc{0x1420C5820, 0x0};

WEAK symbol<const char *> PerLocalClientMemoryName{0x1432A7EE0, 0x0};
} // namespace pmem
} // namespace game
