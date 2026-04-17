#pragma once
#ifdef GAME_HPP

#include "../macros.hpp"

namespace game {
namespace ui {

// UI
WEAK
    symbol<void(int localClientNumber, int errorcode, const char *errorMessage)>
        UI_OpenErrorPopupWithMessage{0x14228DEE0};
WEAK symbol<void(bool frontend)> UI_CoD_Init{0x141F29010, 0x1404A0A50};
WEAK symbol<void()> UI_CoD_LobbyUI_Init{0x141F2BD80, 0x1404A1F50};
WEAK symbol<void()> UI_CoD_Shutdown{0x141F32E10, 0x0};
WEAK symbol<void(const char *, const char *, int, lua::hks::lua_State *)>
    UI_AddMenu{0x1427018F0, 0x0};
WEAK symbol<const char *(int)> UI_CoD_GetRootNameForController{0x141F28940,
                                                               0x0};

} // namespace ui
} // namespace game

#endif