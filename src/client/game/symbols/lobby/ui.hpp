#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace ui {
WEAK symbol<void(LobbyLeaderActivity activity)> LobbyUI_SetLeaderActivity{
    0x141EF6500, 0x0, 0x1404AFA10};
} // namespace ui
} // namespace lobby
} // namespace game
