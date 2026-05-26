#include "sd.hpp"
#include <mutex>
#include <unordered_map>

namespace game {
namespace snd {
namespace sd {
void *aligned_alloc(size_t alignment, size_t size) {
  void *original = malloc(size + alignment + sizeof(void *));
  if (original) {

    // Calculate aligned address
    uintptr_t raw_addr = (uintptr_t)original + sizeof(void *);
    void *aligned = (void *)((raw_addr + (alignment - 1)) & ~(alignment - 1));

    // Store original pointer before aligned address
    *(void **)((uintptr_t)aligned - sizeof(void *)) = original;

    return aligned;
  }

  return nullptr;
}

void aligned_free(void *ptr) {
  if (ptr) {
    // Retrieve original pointer and free it
    free(*(void **)((uintptr_t)ptr - sizeof(void *)));
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
  for (std::unordered_map<std::string, sd_byte *>::iterator it =
           sd_allocations.begin();
       it != sd_allocations.end();) {
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