#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace db {
namespace xasset {
namespace world {
WEAK symbol<GfxWorld *> s_world{0x14AE59060, 0x147C80850};
WEAK symbol<array<GfxWorld, 2>> s_GfxWorldPool{0x1493A9870, 0x14689A960};

WEAK symbol<ComWorld *> comWorld{0x1568CE6C8, 0x14946FF90};
WEAK symbol<array<ComWorld, 2>> s_ComWorldPool{0x1493A96B0, 0x142D4C300};

WEAK symbol<GameWorld *> gameWorld{0x14A08ABB0, 0x1470D73C0};
WEAK symbol<GameWorld *> gameWorldCurrent{0x1580DD7B0, 0x14A8A7428};
WEAK symbol<array<GameWorld, 2>> s_GameWorldPool{0x1493A97C0, 0x142D4C410};
} // namespace world

} // namespace xasset
} // namespace db
} // namespace game