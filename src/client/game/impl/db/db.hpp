#pragma once
#include <cstdint>
#include "game/game.hpp"
#include "game/structs/structs.hpp"

namespace game {
namespace db {
namespace xasset {
void reallocate_asset_pool(const XAssetType type, const unsigned int new_size);

// The engine always inlines this function, so we reimplement it here for use
// elsewhere.
uint32_t DB_HashForName(const char *name, const XAssetType type);
XAssetEntryPoolEntry *DB_GetAssetEntryPoolEntryByName(const char *name,
                                                      const XAssetType type);

} // namespace xasset
} // namespace db
} // namespace game