#include "ptr.hpp"

#include <vector>
#include <windows.h>

#include <utils/nt.hpp>

bool valid_heap_ptr(uintptr_t ptr) {
  if (ptr) {
    // Get the number of heaps belonging to the current process
    uint32_t num_heaps = GetProcessHeaps(0, nullptr);
    if (num_heaps > 0) {
      // Allocate a buffer to store the heap handles
      std::vector<HANDLE> heaps(num_heaps);
      GetProcessHeaps(num_heaps, heaps.data());

      // Iterate over every heap
      for (uint32_t i = 0; i < num_heaps; ++i) {
        HANDLE hHeap = heaps[i];

        if (HeapLock(hHeap)) {
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

          HeapUnlock(hHeap);

          if (found) {
            return true;
          }
        }
      }
    }
  }

  return false;
}

constexpr DWORD READ_FLAGS = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                             PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                             PAGE_EXECUTE_WRITECOPY;
constexpr DWORD PROTECT_FLAGS = PAGE_GUARD | PAGE_NOACCESS;
/// NOTE: DO NOT USE ON WINDOWS. Wine performs this check very quickly, but
/// using this on Windows _throttles_ performance.
/// @brief Rapidly checks if a memory address is committed and readable.
/// Safe to use in high-frequency loops.
bool readable_ptr(uintptr_t ptr) {
  if (ptr) {

    // `VirtualQuery` is extremely slow on Windows. Use only in wine, and
    // otherwise use an ugly try-catch block.
    if (utils::nt::is_wine()) {
      MEMORY_BASIC_INFORMATION mbi;
      return VirtualQuery(reinterpret_cast<LPCVOID>(ptr), &mbi, sizeof(mbi)) &&
             mbi.State == MEM_COMMIT && (mbi.Protect & PROTECT_FLAGS) == 0 &&
             (mbi.Protect & READ_FLAGS) != 0;
    }
  } else {
    __try {
      [[maybe_unused]] const char _c = *reinterpret_cast<const char *>(ptr);
      return true;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                    ? EXCEPTION_EXECUTE_HANDLER
                    : EXCEPTION_CONTINUE_SEARCH) {
    }
  }

  return false;
}
