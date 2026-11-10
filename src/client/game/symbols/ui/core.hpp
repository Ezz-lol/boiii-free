#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ui {

// UI
WEAK symbol<void(LocalClientNum_t localClientNumber, errorCode errorcode,
                 const char *errorMessage)>
    UI_OpenErrorPopupWithMessage{0x1422313C0, 0x0};
WEAK symbol<void(bool frontend)> UI_CoD_Init{0x141F1C890, 0x1404A0A50};
WEAK symbol<void()> UI_CoD_LobbyUI_Init{0x141F1F600, 0x1404A1F50};
WEAK symbol<void()> UI_CoD_Shutdown{0x141F26690, 0x0};
WEAK symbol<void(const char *, const char *, int32_t, lua::hks::lua_State *)>
    UI_AddMenu{0x142688780, 0x0};
WEAK symbol<const char *(ControllerIndex_t controller)>
    UI_CoD_GetRootNameForController{0x141F1C1C0, 0x0};
WEAK symbol<UIModelIndex(live::settings::ProfileSetting setting,
                         ControllerIndex_t controllerIndex)>
    UI_Model_GetProfileModelForController{0x14164E9B0, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreateModelFromPath{0x14200C840, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_CreatePersistentModelFromPath{0x14200C900, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex)> UI_Model_ForceNotify{
    0x14200C9C0, 0x0};
WEAK symbol<UIModelIndex()> UI_Model_GetGlobalModel{0x14200CD10, 0x0};
WEAK symbol<UIModelIndex(const ControllerIndex_t controllerIndex)>
    UI_Model_GetModelForController{0x14200CEE0, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex parentNodeIndex, const char *path)>
    UI_Model_GetModelFromPath{0x14200CEF0, 0x0};
WEAK symbol<void(UIModelIndex model)> UI_Model_NotifySubscriptions{0x14200D1D0,
                                                                   0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, bool newValue)> UI_Model_SetBool{
    0x14200D4E0, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, int32_t newValue)>
    UI_Model_SetInt{0x14200D590, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, const char *newValue)>
    UI_Model_SetString{0x14200D630, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, uint64_t newValue)>
    UI_Model_SetUInt64{0x14200D730, 0x0};
WEAK symbol<UIModelIndex(const UIModelIndex ancestorIndex, const char *key,
                         bool persistent)>
    UI_Model_AllocateNode{0x14200C640, 0x0};
WEAK symbol<void(UIModelIndex model)> UI_Model_FreeModel{0x14200C9E0, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex)> UI_Model_GetBool{0x14200CC90,
                                                                 0x0};
WEAK symbol<UIModelDataType(const UIModelIndex model)> UI_Model_GetDataType{
    0x14200CCC0, 0x0};
WEAK symbol<int32_t(const UIModelIndex nodeIndex, lua::hks::lua_State *luaVM)>
    UI_Model_GetFunction{0x14200CCE0, 0x0};
WEAK symbol<int32_t(const UIModelIndex nodeIndex)> UI_Model_GetInt{0x14200CD20,
                                                                   0x0};
WEAK symbol<UIModelIndex(const UIModelIndex nodeIndex, const char *key)>
    UI_Model_GetModel{0x14200CD50, 0x0};
WEAK symbol<float(const UIModelIndex nodeIndex)> UI_Model_GetReal{0x14200CF90,
                                                                  0x0};
WEAK symbol<const char *(const UIModelIndex nodeIndex)> UI_Model_GetString{
    0x14200CFC0, 0x0};
WEAK symbol<uint64_t(const UIModelIndex nodeIndex)> UI_Model_GetUInt64{
    0x14200CFF0, 0x0};
WEAK symbol<void()> UI_Model_ResetModelsAndSubscriptions{0x14200D3D0, 0x0};
WEAK symbol<bool(UIModelIndex model, const UIModelData *value)>
    Demo_SetUIModelInternal{0x14200D480, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, lua::hks::lua_State *luaVM,
                 int32_t newValue)>
    UI_Model_SetFunction{0x14200D530, 0x0};
WEAK symbol<bool(const UIModelIndex nodeIndex, float newValue)>
    UI_Model_SetReal{0x14200D5E0, 0x0};
WEAK symbol<UIModelSubscription(UIModelIndex model, int32_t functionRef,
                                lua::hks::lua_State *luaVM)>
    UI_Model_AddSubscription{0x14200D780, 0x0};
WEAK symbol<void(UIModelSubscription subscription)> UI_Model_RemoveSubscription{
    0x14200D820, 0x0};
WEAK symbol<void(UIModelIndex model)> UI_Model_UnsubscribeAndFreeModel{
    0x14200D8E0, 0x0};
WEAK symbol<int32_t()> UI_GetModelSubscriptionTableRef{0x14268BE70, 0x0};

WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CloseAllMenus{0x14222E5C0,
                                                                    0x0};
WEAK symbol<void()> UI_Shutdown{0x142694C90, 0x0};

WEAK symbol<void(const char *rootName)> UI_CoD_ProcessEvents{0x141F21620, 0x0};
WEAK symbol<void(bool frontend)> UI_CoD_ShutdownAndInit{0x141F26800, 0x0};
WEAK symbol<void(lua::hks::lua_Alloc allocFunction, void *outOfMemoryFunction)>
    UI_Init{0x14268BE80, 0x0};

WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CoD_Layout{0x141F1D770,
                                                                 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum)> UI_CoD_Render{0x141F219C0,
                                                                 0x0};
WEAK symbol<float(LocalClientNum_t localClientNum, const char *text,
                  db::xasset::ttf::TTFDef *font, float textHeight,
                  float letterSpacing)>
    UI_Interface_GetTextWidth{0x141C9F0A0, 0x0};

WEAK symbol<void(float *x, float *y, float *z)> UI_ApplyTransformsToPoint{
    0x1427A3EA0, 0x0};

WEAK symbol<bool()> UI_IsRenderingImmediately{0x14268C5A0, 0x0};

WEAK symbol<bool(const LocalClientNum_t localClientNum, const char **textPtr,
                 uint32_t *letter, const vec4_t *originalColor,
                 gfx::GfxColor *color, char **buttonName,
                 gfx::GfxImage **iconImage, uint32_t *cursor)>
    UI_CoD_ReadCharFromString{0x141F216A0, 0x0};
WEAK symbol<void(bool renderImmediately)> UI_SetRenderImmediately{0x142694C80,
                                                                  0x0};

WEAK symbol<int32_t> s_currentStencilValue{0x156262E28, 0x0};
WEAK symbol<hunk::HunkUser *> s_modelStringHunk{0x15626C030, 0x0};
WEAK symbol<UIModelSubscription> s_firstFreeSubscription{0x15626C044, 0x0};
WEAK symbol<UIModelSubscriptionEntry[UI_MODEL_SUBSCRIPTION_COUNT]>
    s_modelSubscriptions{0x15626C050, 0x0};
} // namespace ui
} // namespace game
