#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace lui {
namespace quad {
WEAK symbol<int32_t> g_LUIAllocatedQuadCaches{0x159BF7DF0, 0x0, 0x0};
WEAK symbol<size_t> g_LUIAllocatedQuadCachesMemory{0x159BF7DF8, 0x0, 0x0};
} // namespace quad
} // namespace lui
} // namespace lua
} // namespace game
