#pragma once

#include <cstdint>
#include <game/game.hpp>

namespace game {
namespace db {
namespace xasset {

// The engine always inlines this function, so we reimplement it here for use
// elsewhere.
template <const XAssetType TYPE>
alwaysinline constexpr sdbmHash_t DB_HashForName(const char *name) {
  return sdbm_path<+TYPE>(name);
}

template <const XAssetType TYPE>
alwaysinline pool::XAssetEntryPoolEntry *
DB_GetAssetEntryPoolEntryByName(const std::string_view &name) {
  uint32_t hash = DB_HashForName<TYPE>(name);
  uint32_t index = hash % pool::XASSET_ENTRY_POOL_LENGTH;
  pool::XAssetEntryPoolEntry *entry =
      reinterpret_cast<pool::XAssetEntryPoolEntry *>(
          &pool::g_assetEntryPool->pool[index]);

  // Find match in collision list
  for (; entry != nullptr && (entry->entry.asset.type != TYPE ||
                              entry->entry.asset.header.named->name != name);
       entry = entry->next) {
  }

  return entry;
}

int32_t DB_GetXAssetTypeSize_Impl(XAssetType type);
inline void Gfx_TexturePool_ReleaseSingle(gfx::GfxTexture &texture) {
  if (texture.basemap != nullptr) {
    if (game::is_client()) {
      gfx::Gfx_TexturePool_ReleaseRef(texture, 0);
    }
    texture.release();
    texture.basemap = nullptr;
  }
}
void Gfx_ReleaseImageTextures_Impl(XAssetHeader header);

namespace pool {
void reallocate_asset_pool(XAssetType type, uint32_t new_size);
pool::XAssetEntryPoolEntry *DB_GetAssetEntryPoolEntryByName(const char *name,
                                                            XAssetType type);
void DB_InitBSPGlobals_Impl();
void *DB_AssetPoolAlloc_Impl(XAssetType type);
} // namespace pool
} // namespace xasset
} // namespace db
} // namespace game
