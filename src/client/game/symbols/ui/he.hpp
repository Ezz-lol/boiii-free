#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {
namespace he {
WEAK symbol<void()> HudElem_DestroyAll{0x141990DD0, 0x1402A1260};
WEAK symbol<HudElementPool<game_hudelem_t>> g_hudelems{0x14A08B560,
                                                       0x1470D7D70};
} // namespace he
} // namespace ui
} // namespace game