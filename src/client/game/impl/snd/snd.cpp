#include <std_include.hpp>

#include "snd.hpp"
#include "stdlib.h"
#include "stdint.h"
#include <cstring>
#include <mutex>
#include <string>
#include <utils/string.hpp>
#include <cstdint>
#include <unordered_map>

using namespace game::com;
using namespace game::sys;

namespace game {
namespace snd {

void *aligned_alloc(size_t alignment, size_t size) {
  void *original = malloc(size + alignment + sizeof(void *));
  if (!original)
    return NULL;

  // Calculate aligned address
  uintptr_t raw_addr = (uintptr_t)original + sizeof(void *);
  void *aligned = (void *)((raw_addr + (alignment - 1)) & ~(alignment - 1));

  // Store original pointer before aligned address
  *(void **)((uintptr_t)aligned - sizeof(void *)) = original;

  return aligned;
}

void aligned_free(void *ptr) {
  if (ptr) {
    // Retrieve original pointer and free it
    free(*(void **)((uintptr_t)ptr - sizeof(void *)));
  }
}

static std::mutex sd_allocations_mutex;
static std::unordered_map<std::string, sd_byte *> sd_allocations;
sd_byte *SD_Alloc_BasicImpl(const char *name, uint32_t size, uint32_t align) {
  std::lock_guard<std::mutex> lock(sd_allocations_mutex);
  sd_byte *allocation = reinterpret_cast<sd_byte *>(
      aligned_alloc(static_cast<size_t>(align), static_cast<size_t>(size)));
  if (allocation) {
    sd_allocations[name] = allocation;
  } else {
    Com_Printf(game::CON_LABEL_LOBBYHOST, 28,
               "SOUND ERROR: unable to allocate %u bytes for %s\n", size, name);
    sd_allocations[name] = nullptr;
  }

  return allocation;
}

sd_byte *SD_Alloc(const char *name, uint32_t size, uint32_t align) {
  if (game::is_client()) {
    return sd::SD_Alloc(name, size, align);
  } else {
    // Does not exist on server
    return SD_Alloc_BasicImpl(name, size, align);
  }
}

void SD_Free_Impl(sd_byte *ptr) {
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

// Hidden C++ exception states: #wind=1
void SND_EnqueueLoadedAssets_Impl(SndBankLoad *load) {
  SND_ErrorIfSoundGlobalsTrashed();

  load->loadedAssetCount = 0;
  uint32_t entryCount = load->loadAssetBank.header.entryCount;
  load->loadedEntryCount = entryCount;
  load->loadedDataSize = 0;
  size_t loadedDataSize = 0;
  uint64_t lowestEntryOffset = 0;
  uint64_t *entryOffsetCursor = nullptr;
  uint64_t entryOffset = 0;
  if (entryCount) {
    entryOffsetCursor = &load->loadAssetBank.entries->offset;
    /*
       The loop iterates through each entry in the
       asset bank, and calculates the total size of the loaded data by finding
       the lowest entry offset and the highest entry offset + entry size, and
       subtracting the lowest entry offset from the highest entry offset + entry
       size.
    */
    for (uint32_t i = 0; i < entryCount; i++) {
      if (lowestEntryOffset) {
        if (lowestEntryOffset > *entryOffsetCursor) {
          entryOffset = *entryOffsetCursor;
          lowestEntryOffset = *entryOffsetCursor;
        }
      } else {
        lowestEntryOffset = *entryOffsetCursor;
      }

      size_t entrySize = *((uint32_t *)entryOffsetCursor - 3) + entryOffset;
      if (loadedDataSize < entrySize + *entryOffsetCursor) {
        loadedDataSize = entrySize + *entryOffsetCursor;
      }
      entryOffsetCursor = (uint64_t *)((char *)entryOffsetCursor + 36);
    }
  }
  load->loadedDataSize = loadedDataSize;
  if (entryCount) {
    Sys_EnterCriticalSection(CriticalSection::CRITSECT_SOUND_BANK);

    uint32_t allocationSize = 36 * load->loadedEntryCount;
    std::string loading_sd_alloc_name =
        utils::string::va("loads   %s", load->loadAssetBank.filename);
    load->loadedEntries = (SndAssetBankEntry *)SD_Alloc(
        loading_sd_alloc_name.c_str(), allocationSize, 0x100u);
    std::string loaded_sd_alloc_name =
        utils::string::va("loaded  %s", load->loadAssetBank.filename);
    sd_byte *allocation =
        SD_Alloc(loaded_sd_alloc_name.c_str(), load->loadedDataSize, 0x1000u);
    load->loadedData = allocation;
    Com_Printf(CON_LABEL_LOBBYCLIENT, 28, "SOUND loaded alloc %s %p %p\n",
               load->loadAssetBank.filename, load->loadedEntries, allocation);
    Sys_LeaveCriticalSection(CriticalSection::CRITSECT_SOUND_BANK);
  } else {
    load->loadedEntries = 0;
    load->loadedData = 0;
  }
  memmove(load->loadedEntries, load->loadAssetBank.entries,
          36LL * load->loadedEntryCount);
  if (load->loadedEntryCount) {
    for (uint32_t i = 0; i < load->loadedEntryCount; i++) {
      load->loadedEntries[i].offset -= lowestEntryOffset;
    }
  }
  load->loadedAssetTotal = load->loadedEntryCount;
  if (loadedDataSize)
    SND_StreamRead(load, load->loadAssetBank.fileHandle, lowestEntryOffset,
                   loadedDataSize, reinterpret_cast<void *>(load->loadedData));

  qsort(load->loadedEntries, load->loadedEntryCount, 0x24u,
        static_cast<_CoreCrtNonSecureSearchSortCompareFunction>(
            SND_CompareAssetLoads.get()));

  SND_ErrorIfSoundGlobalsTrashed();
}

bool SND_StartTocRead_Impl(SndBankLoad *load, SndAssetBankLoad *assetBank,
                           bool streamed) {
  SND_ErrorIfSoundGlobalsTrashed();

  uint32_t entryCount = 0;
  assetBank->entries = 0;
  assetBank->entryCount = 0;
  if (assetBank->filename[0]) {
    entryCount = assetBank->header.entryCount;
    if (entryCount) {
      uint32_t allocSize =
          (-(entryCount * assetBank->header.entrySize) & 0x7FF) +
          entryCount * assetBank->header.entrySize;
      std::string entryName =
          utils::string::va("entries %s", assetBank->filename);
      sd_byte *allocation =
          SD_Alloc_BasicImpl(entryName.c_str(), allocSize, 0x800u);
      assetBank->entryCount = entryCount;
      assetBank->entries = (SndAssetBankEntry *)allocation;
      if (allocation) {
        Com_Printf(CON_LABEL_LOBBYCLIENT, 28, "SOUND entry alloc %s %d\n",
                   assetBank->filename, entryCount);
        int64_t entryOffset = assetBank->header.entryOffset;
        io::stream_fileid fileHandle = assetBank->fileHandle;
        SndAssetBankEntry *data = assetBank->entries;
        assetBank->indicesAllocated = 1;
        SND_StreamRead(load, fileHandle, entryOffset, allocSize,
                       reinterpret_cast<void *>(data));

        SND_ErrorIfSoundGlobalsTrashed();

        entryCount = 1;
      } else {
        SND_BankLoadError(load);
        Com_Printf(
            CON_LABEL_LOBBYHOST, 28,
            "SOUND ERROR: unable to allocate sound bank entries for %s\n",
            assetBank->filename);
        entryCount = 0;
      }
    }
  }

  return entryCount;
}

} // namespace snd
} // namespace game