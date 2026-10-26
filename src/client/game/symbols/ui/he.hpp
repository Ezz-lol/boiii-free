#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {
namespace he {
WEAK symbol<void()> HudElem_DestroyAll{0x141990DF0, 0x141990DD0, 0x1402A1260};
WEAK symbol<void(volatile game_hudelem_t *hud)> Scr_FreeHudElem{
    0x141B20380, 0x0, 0x1402F4E20};
WEAK symbol<volatile HudElementPool<game_hudelem_t>> g_hudelems{
    0x14A00C560, 0x14A08B560, 0x1470D7D70};
} // namespace he
} // namespace ui
} // namespace game
