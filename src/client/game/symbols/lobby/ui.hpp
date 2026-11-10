#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace ui {

WEAK symbol<LobbyUIScreen> s_uiScreen{0x156111F18, 0x148FC40C0};
WEAK symbol<LobbyLeaderActivity> s_uiLeaderActivity{0x156111F1C, 0x148FC40C4};

WEAK symbol<void(LobbyLeaderActivity activity)> LobbyUI_SetLeaderActivity{
    0x141EF6500, 0x1404AFA10};
WEAK symbol<void(LobbyType lobbyType, LobbyUIScreen uiScreen)>
    LobbyUI_SetUIScreenFromState{0x141EF66D0, 0x1404AFAB0};
} // namespace ui
} // namespace lobby
} // namespace game
