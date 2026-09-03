#pragma once

#include <cstdint>
#include <game/game.hpp>

namespace game {
namespace db {
namespace xasset {
int32_t DB_GetXAssetTypeSize_Impl(XAssetType type);

// The engine always inlines this function, so we reimplement it here for use
// elsewhere.
uint32_t DB_HashForName(const char *name, const XAssetType type);

namespace pool {
void reallocate_asset_pool(const XAssetType type, const uint32_t new_size);
pool::XAssetEntryPoolEntry *
DB_GetAssetEntryPoolEntryByName(const char *name, const XAssetType type);
void DB_InitBSPGlobals_Impl();
void *DB_AssetPoolAlloc_Impl(XAssetType type);
} // namespace pool
} // namespace xasset
} // namespace db
} // namespace game