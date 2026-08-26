#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace db {
namespace xasset {
namespace pool {
WEAK symbol<XAssetPools> s_assetPools{0x1494073F0, 0x1468F84E0};
WEAK symbol<XAssetEntryPool> g_assetEntryPool{0x1494C7B80, 0x1469BB270};
} // namespace pool
} // namespace xasset
} // namespace db
} // namespace game