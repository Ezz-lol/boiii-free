#pragma once

#include <game/symbols/sym_include.hpp>


namespace game {
namespace pmem {
#define PMem_Alloc(size, alignment, pool, stack, asyncCommit, memTrack)        \
  _PMem_Alloc(size, alignment, pool, stack, asyncCommit, memTrack, __FILE__,   \
              __LINE__)

WEAK symbol<void(const char *name, int allocDir)> PMem_Free{0x141439190,
                                                            0x1405D8B30};
WEAK symbol<void(const char *name, PMemStack stack, EMemTrack memTrack)>
    PMem_BeginAlloc{0x1420D0F40};
WEAK symbol<void(const char *name, PMemStack stack)> PMem_EndAlloc{0x1420D0FD0};
WEAK symbol<void *(size_t size, size_t alignment, PMemPool pool,
                   PMemStack stack, bool asyncCommit, EMemTrack memTrack,
                   const char *file, int lineNum)>
    _PMem_Alloc{0x1420D1FA0};

WEAK symbol<const char *> PerLocalClientMemoryName{0x143326EE0};
} // namespace pmem
} // namespace game

