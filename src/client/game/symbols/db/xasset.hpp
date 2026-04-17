#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace db {
namespace xasset {

WEAK symbol<XAssetPools> s_assetPools{0x1494093F0, 0x1468F84E0};
WEAK symbol<XAssetEntryPool> g_assetEntryPool{0x1494C7B80, 0x1469BB270};

WEAK symbol<void(XAssetType type, XAssetEnum *func, void *inData,
                 bool includeOverride)>
    DB_EnumXAssets{0x141420970, 0x1401D5A50};
WEAK symbol<XAssetHeader(XAssetType type, const char *name, bool errorIfMissing,
                         int32_t waitTime)>
    DB_FindXAssetHeader{0x141420ED0, 0x1401D5FB0};
WEAK symbol<const char *(const XAsset *asset)> DB_GetXAssetName{0x1413E9DA0,
                                                                0x14019F080};
WEAK symbol<int(XAssetType type)> DB_GetXAssetTypeSize{0x1413E9DD0,
                                                       0x14019F0B0};

} // namespace xasset
} // namespace db
} // namespace game
#endif