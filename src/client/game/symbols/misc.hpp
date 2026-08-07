#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
// misc globals
WEAK symbol<clientplatform_t> clientplatform{0x0, 0x14A63D4E8};
WEAK symbol<qboolean(void *ent)> StuckInClient{0x1415A8360, 0x14023BFE0};
} // namespace game