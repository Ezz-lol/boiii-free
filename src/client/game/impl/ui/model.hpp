#pragma once

#include <game/game.hpp>

namespace game {
namespace ui {
void UI_Model_InitPool();

UIModelIndex UI_Model_AllocateNode_Impl(const UIModelIndex ancestorIndex,
                                        const char *key, bool persistent);
void UI_Model_FreeModel_Impl(UIModelIndex model);

bool UI_Model_GetBool_Impl(const UIModelIndex nodeIndex);
UIModelDataType UI_Model_GetDataType_Impl(const UIModelIndex model);
int32_t UI_Model_GetFunction_Impl(const UIModelIndex nodeIndex,
                                  lua::hks::lua_State *luaVM);
int32_t UI_Model_GetInt_Impl(const UIModelIndex nodeIndex);
UIModelIndex UI_Model_GetModel_Impl(const UIModelIndex nodeIndex,
                                    const char *key);
float UI_Model_GetReal_Impl(const UIModelIndex nodeIndex);
const char *UI_Model_GetString_Impl(const UIModelIndex nodeIndex);
uint64_t UI_Model_GetUInt64_Impl(const UIModelIndex nodeIndex);

void UI_Model_NotifySubscriptions_Impl(UIModelIndex model);
void UI_Model_ResetModelsAndSubscriptions_Impl();

bool Demo_SetUIModelInternal_Impl(UIModelIndex model, const UIModelData *value);
bool UI_Model_SetBool_Impl(const UIModelIndex nodeIndex, bool newValue);
bool UI_Model_SetFunction_Impl(const UIModelIndex nodeIndex,
                               lua::hks::lua_State *luaVM, int32_t newValue);
bool UI_Model_SetInt_Impl(const UIModelIndex nodeIndex, int32_t newValue);
bool UI_Model_SetReal_Impl(const UIModelIndex nodeIndex, float newValue);
bool UI_Model_SetString_Impl(const UIModelIndex nodeIndex,
                             const char *newValue);
bool UI_Model_SetUInt64_Impl(const UIModelIndex nodeIndex, uint64_t newValue);

UIModelSubscription UI_Model_AddSubscription_Impl(UIModelIndex model,
                                                  int32_t functionRef,
                                                  lua::hks::lua_State *luaVM);
void UI_Model_RemoveSubscription_Impl(UIModelSubscription subscription);
void UI_Model_UnsubscribeAndFreeModel_Impl(UIModelIndex model);
} // namespace ui
} // namespace game
