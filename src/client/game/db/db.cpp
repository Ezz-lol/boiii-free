#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "db.hpp"

namespace game {
namespace db {
namespace xasset {
void reallocate_asset_pool(const XAssetType type, const unsigned int new_size) {
  if (static_cast<int>(type) < 0 || type >= XAssetType::ASSET_TYPE_COUNT) {
    printf("[AssetLimits] Invalid asset type %d\n", static_cast<int>(type));
    return;
  }

  const int32_t entry_size = DB_GetXAssetTypeSize(type);
  if (entry_size <= 0) {
    printf("[AssetLimits] Invalid entry size for type %d\n",
           static_cast<int>(type));
    return;
  }

  XAssetPool *pool = reinterpret_cast<XAssetPool *>(
      &s_assetPools->pools[static_cast<int>(type)]);

  // Skip if pool already meets or exceeds requested size
  if (pool->itemAllocCount >= static_cast<int>(new_size)) {
    return;
  }

  XAssetPool *new_pool =
      reinterpret_cast<XAssetPool *>(calloc(new_size, entry_size));
  if (!new_pool) {
    printf("Failed to allocate asset pool for type %d (size: %u)\n",
           static_cast<int>(type), new_size);
    return;
  }

  if (pool->pool != nullptr && pool->itemAllocCount > 0) {
    // Copy existing entries
    memcpy(new_pool, pool->pool,
           pool->itemAllocCount * static_cast<size_t>(entry_size));
  }

  // Rebuild free list for new entries
  pool->freeHead = reinterpret_cast<AssetLink *>(
      reinterpret_cast<char *>(new_pool) +
      static_cast<size_t>(entry_size) * pool->itemAllocCount);

  for (auto i = pool->itemAllocCount; i < static_cast<int>(new_size) - 1; i++) {
    auto *current =
        reinterpret_cast<AssetLink *>(reinterpret_cast<char *>(new_pool) +
                                      static_cast<size_t>(entry_size) * i);
    current->next = reinterpret_cast<AssetLink *>(
        reinterpret_cast<char *>(new_pool) +
        static_cast<size_t>(entry_size) * (i + 1));
  }

  // Last entry points to null
  auto *last = reinterpret_cast<AssetLink *>(
      reinterpret_cast<char *>(new_pool) +
      static_cast<size_t>(entry_size) * (new_size - 1));
  last->next = nullptr;

  pool->pool = new_pool;
  pool->itemAllocCount = static_cast<int>(new_size);

  printf("Reallocated asset pool type %d: %d -> %u entries\n",
         static_cast<int>(type), pool->itemCount, new_size);
}

uint32_t DB_HashForName(const char *name, const XAssetType type) {
  // The engine always inlines this function, so we reimplement it here for use
  uint32_t hash = static_cast<uint32_t>(type);
  while (*name) {
    char c = *name++;
    if (c == '\\') {
      c = '/';
    }
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    hash = (hash << 16) + (hash << 6) + c - hash;
  }
  return hash;
}

XAssetEntryPoolEntry *DB_GetAssetEntryPoolEntryByName(const char *name,
                                                      const XAssetType type) {
  uint32_t hash = DB_HashForName(name, type);
  uint32_t index = hash % xasset::XASSET_ENTRY_POOL_LENGTH;
  xasset::XAssetEntryPoolEntry *entry =
      reinterpret_cast<xasset::XAssetEntryPoolEntry *>(
          &xasset::g_assetEntryPool->pool[index]);
  // Find match in collision list
  while (entry != nullptr) {
    if (entry->entry.asset.type == type &&
        strcmp(entry->entry.asset.header.named->name, name) == 0) {
      break;
    }

    if (entry->next == nullptr) {
      break;
    }

    entry = entry->next;
  }

  return entry;
}
} // namespace xasset
} // namespace db
} // namespace game