#include <std_include.hpp>

#include "ptr.hpp" // IWYU pragma: keep

namespace game {

bool valid_heap_ptr(uintptr_t ptr) {
  if (!ptr) {
    return false;
  }

  // Get the number of heaps belonging to the current process
  uint32_t num_heaps = GetProcessHeaps(0, nullptr);
  if (num_heaps > 0) {
    // Allocate a buffer to store the heap handles
    std::vector<HANDLE> heaps(num_heaps);
    GetProcessHeaps(num_heaps, heaps.data());

    // Iterate over every heap
    for (uint32_t i = 0; i < num_heaps; ++i) {
      HANDLE hHeap = heaps[i];

      // Lock the heap to safely walk it.
      // This prevents crashes if the game engine allocates/frees memory on
      // another thread while we walk.
      if (!HeapLock(hHeap)) {
        continue;
      }

      PROCESS_HEAP_ENTRY entry;
      entry.lpData = nullptr;
      bool found = false;

      // Walk through all blocks in this specific heap
      while (HeapWalk(hHeap, &entry)) {
        // Check if the current block is actively allocated (not freed)
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) {
          uintptr_t block_start = reinterpret_cast<uintptr_t>(entry.lpData);
          uintptr_t block_end = block_start + entry.cbData;

          // Check if our pointer falls within this block's boundaries
          if (ptr >= block_start && ptr < block_end) {
            found = true;
            break;
          }
        }
      }

      // Always unlock the heap when done
      HeapUnlock(hHeap);

      if (found) {
        return true;
      }
    }
  }

  return false;
}

/// NOTE: DO NOT USE ON WINDOWS. Wine performs this check very quickly, but
/// using this on Windows _throttles_ performance.
/// @brief Rapidly checks if a memory address is committed and readable.
/// Safe to use in high-frequency loops.
bool readable_ptr(uintptr_t ptr) {
  if (ptr) {

    if (valid_engine_ptr(ptr)) {
      return true;
    }

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(reinterpret_cast<LPCVOID>(ptr), &mbi, sizeof(mbi))) {
      // Check if the memory page is committed (actually exists in physical
      // memory/pagefile)
      if (mbi.State != MEM_COMMIT) {
        return false;
      }

      // Check if the page has guard or no-access protections
      if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) {
        return false;
      }

      // Check if we have some form of read access
      const DWORD read_flags = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                               PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                               PAGE_EXECUTE_WRITECOPY;

      return (mbi.Protect & read_flags) != 0;
    }
  }

  return false;
}
} // namespace game