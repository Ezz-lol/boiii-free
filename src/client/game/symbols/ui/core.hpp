#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {

// UI
WEAK symbol<void(LocalClientNum_t localClientNumber, errorCode errorcode,
                 const char *errorMessage)>
    UI_OpenErrorPopupWithMessage{0x1422313C0, 0x14228DEE0, 0x0};
WEAK symbol<void(bool frontend)> UI_CoD_Init{0x141F1C890, 0x141F29010,
                                             0x1404A0A50};
WEAK symbol<void()> UI_CoD_LobbyUI_Init{0x141F1F600, 0x141F2BD80, 0x1404A1F50};
WEAK symbol<void()> UI_CoD_Shutdown{0x141F26690, 0x141F32E10, 0x0};
WEAK symbol<void(const char *, const char *, int32_t, lua::hks::lua_State *)>
    UI_AddMenu{0x142688780, 0x1427018F0, 0x0};
WEAK symbol<const char *(ControllerIndex_t controller)>
    UI_CoD_GetRootNameForController{0x141F1C1C0, 0x141F28940, 0x0};
WEAK symbol<UIModelIndex(live::settings::ProfileSetting setting,
                         ControllerIndex_t controllerIndex)>
    UI_Model_GetProfileModelForController{0x14164E9B0, 0x14164E990, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreateModelFromPath{0x14200C840, 0x142018FC0, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreatePersistentModelFromPath{0x14200C900, 0x142019080, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex)> UI_Model_ForceNotify{
    0x14200C9C0, 0x142019140, 0x0};
WEAK symbol<UIModelIndex()> UI_Model_GetGlobalModel{0x14200CD10, 0x142019490,
                                                    0x0};
WEAK symbol<UIModelIndex(const ControllerIndex_t controllerIndex)>
    UI_Model_GetModelForController{0x14200CEE0, 0x142019660, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_GetModelFromPath{0x14200CEF0, 0x142019670, 0x0};
WEAK symbol<void(UIModelIndex model)> UI_Model_NotifySubscriptions{
    0x14200D1D0, 0x142019950, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, bool newValue)> UI_Model_SetBool{
    0x14200D4E0, 0x142019C60, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, int32_t newValue)>
    UI_Model_SetInt{0x14200D590, 0x142019D10, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, const char *newValue)>
    UI_Model_SetString{0x14200D630, 0x142019DB0, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, uint64_t newValue)>
    UI_Model_SetUInt64{0x14200D730, 0x142019EB0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CloseAllMenus{
    0x14222E5C0, 0x14228B0E0, 0x0};
WEAK symbol<void()> UI_Shutdown{0x142694C90, 0x14270DE00, 0x0};

WEAK symbol<void(const char *rootName)> UI_CoD_ProcessEvents{0x141F21620,
                                                             0x141F2DDA0, 0x0};
WEAK symbol<void(bool frontend)> UI_CoD_ShutdownAndInit{0x141F26800,
                                                        0x141F32F80, 0x0};
WEAK symbol<void(lua::hks::lua_Alloc allocFunction, void *outOfMemoryFunction)>
    UI_Init{0x14268BE80, 0x142704FF0, 0x0};

WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CoD_Layout{
    0x141F1D770, 0x141F29EF0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CoD_Render{
    0x141F219C0, 0x141F2E140, 0x0};
} // namespace ui
} // namespace game
