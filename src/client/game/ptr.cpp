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
} // namespace game