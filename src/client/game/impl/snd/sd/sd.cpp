#include "sd.hpp"
#include <mutex>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <cstdlib>

namespace game {
namespace snd {
namespace sd {
struct AllocationHeader {
  void *original_ptr;
};

void *aligned_alloc(size_t alignment, size_t size) {
  void *original = malloc(size + alignment + sizeof(AllocationHeader));
  if (original) {

    uintptr_t raw_addr =
        reinterpret_cast<uintptr_t>(original) + sizeof(AllocationHeader);
    uintptr_t aligned_addr = (raw_addr + (alignment - 1)) & ~(alignment - 1);
    void *aligned = reinterpret_cast<void *>(aligned_addr);

    AllocationHeader *header = reinterpret_cast<AllocationHeader *>(
        aligned_addr - sizeof(AllocationHeader));
    header->original_ptr = original;

    return aligned;
  }

  return nullptr;
}

void aligned_free(void *ptr) {
  if (ptr) {
    AllocationHeader *header = reinterpret_cast<AllocationHeader *>(
        reinterpret_cast<uintptr_t>(ptr) - sizeof(AllocationHeader));
    free(header->original_ptr);
  }
}

static std::mutex sd_allocations_mutex;
static std::unordered_map<std::string, sd_byte *> sd_allocations;

sd_byte *SD_HeapAlloc(const char *name, uint32_t size, uint32_t align) {
  std::lock_guard<std::mutex> lock(sd_allocations_mutex);
  sd_byte *allocation = reinterpret_cast<sd_byte *>(
      aligned_alloc(static_cast<size_t>(align), static_cast<size_t>(size)));
  if (allocation) {
    sd_allocations[name] = allocation;
  } else {
    com::Com_Printf(game::CON_LABEL_LOBBYHOST, 28,
                    "SOUND ERROR: unable to allocate %u bytes for %s\n", size,
                    name);
    sd_allocations[name] = nullptr;
  }

  return allocation;
}

void SD_HeapFree(sd_byte *ptr) {
  std::lock_guard<std::mutex> lock(sd_allocations_mutex);
  for (auto it = sd_allocations.begin(); it != sd_allocations.end();) {
    if (it->second == ptr) {
      aligned_free(ptr);
      sd_allocations.erase(it);
      return;
    }
    ++it;
  }
}
} // namespace sd
} // namespace snd
} // namespace game