#pragma once
#include <cstdint>
#include "game/game.hpp"
#include "game/structs/structs.hpp"

namespace game {
namespace db {
namespace xasset {
void reallocate_asset_pool(const XAssetType type, const unsigned int new_size);

// Always inlined in engine, reimplemented here for use
uint32_t DB_HashForName(const char *name, const XAssetType type);
XAssetEntryPoolEntry *DB_GetAssetEntryPoolEntryByName(const char *name,
                                                      const XAssetType type);

} // namespace xasset
} // namespace db
} // namespace game