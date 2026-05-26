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

void SND_EnqueueLoadedAssets_Impl(SndBankLoad *load) {
  SND_ErrorIfSoundGlobalsTrashed();

  load->loadedAssetCount = 0;
  uint32_t entryCount = load->loadAssetBank.header.entryCount;
  load->loadedEntryCount = entryCount;

  size_t loadedDataSize = 0;
  uint64_t lowestEntryOffset = 0;

  if (entryCount > 0 && load->loadedEntries != nullptr) {
    // Initialize with the first entry to avoid continuous 0-checks in the loop
    lowestEntryOffset = load->loadedEntries[0].offset;

    for (uint32_t i = 0; i < entryCount; ++i) {
      const SndAssetBankEntry *entry = &load->loadedEntries[i];

      // Track the lowest starting offset
      if (entry->offset < lowestEntryOffset) {
        lowestEntryOffset = entry->offset;
      }

      // Track the maximum boundary of the data (offset + size)
      size_t currentEntryEnd = entry->offset + entry->size;
      if (currentEntryEnd > loadedDataSize) {
        loadedDataSize = currentEntryEnd;
      }
    }
  }

  load->loadedDataSize = loadedDataSize;
  if (entryCount) {
    Sys_EnterCriticalSection(CriticalSection::CRITSECT_SOUND_BANK);

    uint32_t allocationSize =
        sizeof(SndAssetBankEntry) * load->loadedEntryCount;
    const char *loading_sd_alloc_name =
        utils::string::va("loads   %s", load->loadAssetBank.filename);
    load->loadedEntries = (SndAssetBankEntry *)SD_Alloc_BasicImpl(
        loading_sd_alloc_name, allocationSize, 0x100u);
    const char *loaded_sd_alloc_name =
        utils::string::va("loaded  %s", load->loadAssetBank.filename);
    sd_byte *allocation =
        SD_Alloc_BasicImpl(loaded_sd_alloc_name, load->loadedDataSize, 0x1000u);
    load->loadedData = allocation;
    Com_Printf(CON_LABEL_LOBBYCLIENT, 28, "SOUND loaded alloc %s %p %p\n",
               load->loadAssetBank.filename, load->loadedEntries, allocation);
    Sys_LeaveCriticalSection(CriticalSection::CRITSECT_SOUND_BANK);
  } else {
    load->loadedEntries = 0;
    load->loadedData = 0;
  }
  memmove(load->loadedEntries, load->loadAssetBank.entries,
          sizeof(SndAssetBankEntry) * load->loadedEntryCount);
  if (load->loadedEntryCount) {
    for (uint32_t i = 0; i < load->loadedEntryCount; i++) {
      load->loadedEntries[i].offset -= lowestEntryOffset;
    }
  }
  load->loadedAssetTotal = load->loadedEntryCount;
  if (loadedDataSize)
    SND_StreamRead(load, load->loadAssetBank.fileHandle, lowestEntryOffset,
                   loadedDataSize, reinterpret_cast<void *>(load->loadedData));

  qsort(load->loadedEntries, load->loadedEntryCount, sizeof(SndAssetBankEntry),
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
      const char *entryName =
          utils::string::va("entries %s", assetBank->filename);
      sd_byte *allocation = SD_Alloc_BasicImpl(entryName, allocSize, 0x800u);
      assetBank->entryCount = entryCount;
      assetBank->entries = (SndAssetBankEntry *)allocation;
      if (allocation) {
        Com_Printf(CON_LABEL_LOBBYCLIENT, 28, "SOUND entry alloc %s %d\n",
                   assetBank->filename, entryCount);
        int64_t entryOffset = assetBank->header.entryOffset;
        stream::stream_fileid fileHandle = assetBank->fileHandle;
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

uint32_t SND_AssetBankGetFrameRate_Impl(const SndAssetBankEntry *entry) {
  uint32_t result = 0;

  switch (entry->frameRateIndex) {
  case 0u:
    result = 8000;
    break;
  case 1u:
    result = 12000;
    break;
  case 2u:
    result = 16000;
    break;
  case 3u:
    result = 24000;
    break;
  case 4u:
    result = 32000;
    break;
  case 5u:
    result = 44100;
    break;
  case 7u:
    result = 96000;
    break;
  case 8u:
    result = 192000;
    break;
  default:
    result = 48000;
    break;
  }
  return result;
}

inline uint32_t SND_AssetBankGetLengthMs_Impl(const SndAssetBankEntry *entry) {
  return 1000 * static_cast<uint64_t>(entry->frameCount) /
         SND_AssetBankGetFrameRate_Impl(entry);
}

/*
  A reimplementation of the `SND_HashName` function seen in Black Ops 3.

  Output values are identical to those defined with types `SndAliasId` and
  `SndStringHash` in the game. The hasher implements a variation of the sdbm
  hash algorithm using the constant 0x1003f with a djb2 initial seed of 0x1505.
  It is case-insensitive and features a specific edge-case where a resulting
  hash of 0 is promoted to 1.

  - Constant: 0x1003f,
  - Initial Seed: 0x1505
*/

constexpr SndStringHash SND_HASH_EMPTY_STRING = 0;
constexpr SndStringHash SND_HASH_DJB2_INITIAL_SEED = 0x1505;
constexpr SndStringHash SND_HASH_DJB2_CONSTANT = 0x1003f;
SndStringHash SND_HashName_Impl(const char *name) {
  if (!name || !*name)
    return SND_HASH_EMPTY_STRING;

  SndStringHash hash = SND_HASH_DJB2_INITIAL_SEED;
  for (const char *c = name; *c; c++) {
    hash = static_cast<SndStringHash>(
               std::tolower(static_cast<unsigned char>(*c))) +
           hash * SND_HASH_DJB2_CONSTANT;
  }
  return hash ? hash : 1;
}

constexpr int32_t PLAYBACK_TIME_NOT_FOUND = -1;
constexpr uint32_t SOUND_ALIAS_LIST_MAX_COUNT = 0x40;

int32_t SND_GetPlaybackTimeById(SndAliasId id) {
  int32_t playbackTime = PLAYBACK_TIME_NOT_FOUND;
  if (id) {
    SndAliasList *list = game::snd::SND_BankAliasLookup(id);
    if (list) {
      uint32_t count = (std::min)(SOUND_ALIAS_LIST_MAX_COUNT, list->count);
      for (uint32_t i = 0; i < count; i++) {
        SndAlias *alias = &list->head[i];
        if (alias) {
          SndStringHash assetId = alias->assetId;
          SndAssetBankEntry *entry = nullptr;
          sd_byte *data = nullptr;
          if (assetId) {
            stream::stream_fileid fid = 0;
            if (SND_AssetBankFindEntry(assetId, &entry, &fid, 1)) {
              playbackTime =
                  (std::max)(static_cast<int32_t>(
                                 SND_AssetBankGetLengthMs_Impl(entry)) +
                                 500,
                             playbackTime);
            }
          } else if (SND_AssetBankFindLoaded(
                         alias->assetId, &entry,
                         reinterpret_cast<void **>(&data))) {
            playbackTime =
                (std::max)(static_cast<int32_t>(
                               SND_AssetBankGetLengthMs_Impl(entry)) +
                               250,
                           playbackTime);
          }
        } else {
          return 0;
        }
      }
    }
  }

  return playbackTime;
}
int32_t SND_GetPlaybackTime_Impl(const char *name) {
  SndStringHash id = SND_HashName_Impl(name);
  return SND_GetPlaybackTimeById(static_cast<SndAliasId>(id));
}

} // namespace snd
} // namespace game