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
WEAK symbol<UIModelIndex(live::settings::ProfileSetting setting,
                         ControllerIndex_t controllerIndex)>
    UI_Model_GetProfileModelForController{0x14164E990};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreateModelFromPath{0x142018FC0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreatePersistentModelFromPath{0x142019080};
WEAK symbol<bool(const UIModelIndex nodeIndex)> UI_Model_ForceNotify{
    0x142019140};
WEAK symbol<UIModelIndex()> UI_Model_GetGlobalModel{0x142019490};
WEAK symbol<UIModelIndex(const ControllerIndex_t controllerIndex)>
    UI_Model_GetModelForController{0x142019660};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_GetModelFromPath{0x142019670};
WEAK symbol<void(UIModelIndex model)> UI_Model_NotifySubscriptions{0x142019950};
WEAK symbol<bool(const UIModelIndex nodeIndex, bool newValue)> UI_Model_SetBool{
    0x142019C60};
WEAK symbol<bool(const UIModelIndex nodeIndex, int32_t newValue)>
    UI_Model_SetInt{0x142019D10};
WEAK symbol<bool(const UIModelIndex nodeIndex, const char *newValue)>
    UI_Model_SetString{0x142019DB0};
WEAK symbol<bool(const UIModelIndex nodeIndex, uint64_t newValue)>
    UI_Model_SetUInt64{0x142019EB0};

} // namespace ui
} // namespace game

#endif