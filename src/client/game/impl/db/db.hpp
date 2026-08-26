#pragma once
#include <cstdint>
#include <game/game.hpp>

namespace game {
namespace db {
namespace xasset {
int32_t DB_GetXAssetTypeSize_Impl(XAssetType type);
void reallocate_asset_pool(const XAssetType type, const uint32_t new_size);

// The engine always inlines this function, so we reimplement it here for use
// elsewhere.
uint32_t DB_HashForName(const char *name, const XAssetType type);
pool::XAssetEntryPoolEntry *
DB_GetAssetEntryPoolEntryByName(const char *name, const XAssetType type);

} // namespace xasset
} // namespace db
} // namespace game