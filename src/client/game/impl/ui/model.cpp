#include <std_include.hpp>

#include "model.hpp"

#include <component/error_help.hpp>

namespace game {
namespace ui {
namespace {
UIModelNodePool<UIModelNode> s_modelNodePool{};
UIModelIndex s_firstFreeModelNode = 0;

constexpr size_t MAX_MODEL_KEY_LEN = 0x40;

inline UIModelNode &node(const UIModelIndex index) noexcept {
  return s_modelNodePool[index];
}

inline UIModelSubscriptionEntry &
subscription(const UIModelSubscription index) noexcept {
  return (*s_modelSubscriptions)[index];
}

inline sys::ScopedCriticalSection lock_models() {
  return {sys::CriticalSection::UI,
          sys::ScopedCriticalSection::ScopedCriticalSectionType::NORMAL};
}
} // namespace

void UI_Model_InitPool() {
  for (UIModelIndex model = UIModelNodePool<UIModelNode>::SIZE - 1; model != 0;
       --model) {
    node(model).index = model;
    node(model).nextSibling = s_firstFreeModelNode;
    s_firstFreeModelNode = model;
  }
}

UIModelIndex UI_Model_AllocateNode_Impl(const UIModelIndex ancestorIndex,
                                        const char *key, bool persistent) {
  if (key == nullptr) {
    return 0;
  }

  const sys::ScopedCriticalSection critSect = lock_models();

  const UIModelIndex model = s_firstFreeModelNode;
  if (model == 0 || std::strlen(key) >= MAX_MODEL_KEY_LEN) {
    if (lui_oom->get_int() == 1) {
      error_help::Com_Error_LuiOutOfMemory("", 0, errorParm::FATAL,
                                           "EXE_LUI_OUT_OF_MEMORY");
    } else if (lui_oom->get_int() == 2) {
      error_help::Sys_Error_LuiOutOfMemory("EXE_LUI_OUT_OF_MEMORY");
    }
    return 0;
  }

  UIModelNode &allocated = node(model);
  s_firstFreeModelNode = allocated.nextSibling;
  allocated.ancestor = ancestorIndex;
  allocated.nextSibling = 0;
  allocated.setKey(key);
  allocated.persistent = persistent;

  if (ancestorIndex != 0) {
    allocated.nextSibling = node(ancestorIndex).firstChild;
    node(ancestorIndex).firstChild = model;
  }

  return model;
}

void UI_Model_FreeModel_Impl(UIModelIndex model) {
  if (model == 0) {
    return;
  }

  const sys::ScopedCriticalSection critSect = lock_models();

  UIModelNode &freed = node(model);
  // Every allocated node has a key. Freeing a node that's already on the free
  // list would link it in twice and hand it out to two models
  if (freed.keyHash == 0 || freed.firstSubscription != 0) {
    return;
  }

  for (UIModelIndex child = freed.firstChild; child != 0;) {
    const UIModelIndex nextSibling = node(child).nextSibling;
    if (child != model) {
      UI_Model_FreeModel_Impl(child);
    }
    child = nextSibling;
  }

  if (freed.data.dataType == UIModelDataType::STRING) {
    hunk::Hunk_UserFree(*s_modelStringHunk,
                        const_cast<char *>(freed.data.string));
    freed.data.uint64 = 0;
  }

  if (freed.data.dataType == UIModelDataType::FUNCTION) {
    lua::luaL_unref(*lua::hks::UI_luaVM, lua::LUA_REGISTRYINDEX,
                    freed.data.functionRef);
    freed.data.functionRef = -2;
  }

  if (freed.ancestor != 0) {
    UIModelNode &ancestor = node(freed.ancestor);
    if (ancestor.firstChild == model) {
      ancestor.firstChild = freed.nextSibling;
    } else {
      for (UIModelIndex sibling = ancestor.firstChild; sibling != 0;
           sibling = node(sibling).nextSibling) {
        if (node(sibling).nextSibling == model) {
          node(sibling).nextSibling = freed.nextSibling;
          break;
        }
      }
    }
  }

  freed = {};
  freed.index = model;
  freed.nextSibling = s_firstFreeModelNode;
  s_firstFreeModelNode = model;
}

bool UI_Model_GetBool_Impl(const UIModelIndex nodeIndex) {
  return nodeIndex != 0 &&
         node(nodeIndex).data.dataType == UIModelDataType::BOOL &&
         node(nodeIndex).data.boolean;
}

UIModelDataType UI_Model_GetDataType_Impl(const UIModelIndex model) {
  return model != 0 ? node(model).data.dataType : UIModelDataType::INVALID;
}

int32_t UI_Model_GetFunction_Impl(const UIModelIndex nodeIndex,
                                  lua::hks::lua_State *luaVM) {
  if (nodeIndex == 0) {
    return 0;
  }

  const UIModelData &data = node(nodeIndex).data;
  return data.dataType == UIModelDataType::FUNCTION &&
                 luaVM == *lua::hks::UI_luaVM
             ? data.functionRef
             : -1;
}

int32_t UI_Model_GetInt_Impl(const UIModelIndex nodeIndex) {
  return nodeIndex != 0 && node(nodeIndex).data.dataType == UIModelDataType::INT
             ? node(nodeIndex).data.integer
             : 0;
}

UIModelIndex UI_Model_GetModel_Impl(const UIModelIndex nodeIndex,
                                    const char *key) {
  if (nodeIndex == 0) {
    return 0;
  }

  const sys::ScopedCriticalSection critSect = lock_models();

  const djb2Hash_t keyHash = key != nullptr ? djb2(key) : 0;
  size_t searched = 0;
  for (UIModelIndex child = node(nodeIndex).firstChild;
       child != 0 && searched < UIModelNodePool<UIModelNode>::SIZE;
       child = node(child).nextSibling, ++searched) {
    if (node(child).keyHash == keyHash) {
      return child;
    }
  }

  return 0;
}

float UI_Model_GetReal_Impl(const UIModelIndex nodeIndex) {
  return nodeIndex != 0 &&
                 node(nodeIndex).data.dataType == UIModelDataType::REAL
             ? node(nodeIndex).data.real
             : 0.0f;
}

const char *UI_Model_GetString_Impl(const UIModelIndex nodeIndex) {
  return nodeIndex != 0 &&
                 node(nodeIndex).data.dataType == UIModelDataType::STRING
             ? node(nodeIndex).data.string
             : "";
}

uint64_t UI_Model_GetUInt64_Impl(const UIModelIndex nodeIndex) {
  return nodeIndex != 0 &&
                 node(nodeIndex).data.dataType == UIModelDataType::UINT64
             ? node(nodeIndex).data.uint64
             : 0;
}

void UI_Model_NotifySubscriptions_Impl(UIModelIndex model) {
  if (model == 0 || node(model).firstSubscription == 0) {
    return;
  }

  const sys::ScopedCriticalSection critSect = lock_models();
  r::R_LockRemoteScreenUpdate(true);

  for (UIModelSubscription current = node(model).firstSubscription;
       current != 0 && subscription(current).model == model;) {
    const UIModelSubscriptionEntry &entry = subscription(current);
    lua::hks::lua_State *luaVM = entry.luaVM;

    lua::lua_rawgeti(luaVM, lua::LUA_REGISTRYINDEX,
                     UI_GetModelSubscriptionTableRef());
    lua::lua_rawgeti(luaVM, -1, entry.functionRef);

    // The callback replaces the subscription table on the stack, and the model
    // is its only argument
    lua::hks::HksObject *top = luaVM->m_apistack.top;
    top[-2] = top[-1];
    top[-1].v.lightUserData = lua::hks::LightUserData{.raw = model};
    top[-1].t = lua::hks::HksObjectType::TLIGHTUSERDATA;

    current = entry.next;
    if (lua::hks::hksi_lua_pcall(luaVM, 1, 0, 0) != 0 &&
        !ui_execdemo->get_bool()) {
      lua::cod::Lua_CoD_LuaStateManager_Error(
          "Failed to notify model subscription", luaVM);
    }
  }

  r::R_LockRemoteScreenUpdate(false);
}

void UI_Model_ResetModelsAndSubscriptions_Impl() {
  for (size_t index = 0; index < UI_MODEL_SUBSCRIPTION_COUNT; ++index) {
    subscription(static_cast<UIModelSubscription>(index)) = {
        .luaVM = nullptr,
        .functionRef = -2,
        .model = 0,
        .next = static_cast<UIModelSubscription>(index + 1)};
  }
  subscription(
      static_cast<UIModelSubscription>(UI_MODEL_SUBSCRIPTION_COUNT - 1))
      .next = 0;
  *s_firstFreeSubscription = 1;

  for (UIModelNode &model : s_modelNodePool.nodes) {
    model.firstSubscription = 0;
  }

  for (UIModelIndex model = 0; model < UIModelNodePool<UIModelNode>::SIZE;
       ++model) {
    if (node(model).keyHash != 0 && !node(model).persistent) {
      UI_Model_FreeModel_Impl(model);
    }
  }
}

bool Demo_SetUIModelInternal_Impl(UIModelIndex model,
                                  const UIModelData *value) {
  if (model == 0) {
    return false;
  }

  UIModelData &data = node(model).data;
  if (data.dataType != UIModelDataType::INVALID &&
      data.uint64 == value->uint64) {
    return false;
  }

  data = *value;
  UI_Model_NotifySubscriptions_Impl(model);
  return true;
}

bool UI_Model_SetBool_Impl(const UIModelIndex nodeIndex, bool newValue) {
  if (nodeIndex == 0) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::BOOL && data.boolean == newValue) {
    return false;
  }

  data.dataType = UIModelDataType::BOOL;
  data.boolean = newValue;
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

bool UI_Model_SetFunction_Impl(const UIModelIndex nodeIndex,
                               lua::hks::lua_State *luaVM, int32_t newValue) {
  if (nodeIndex == 0 || luaVM != *lua::hks::UI_luaVM) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::FUNCTION &&
      data.functionRef == newValue) {
    return false;
  }

  data.dataType = UIModelDataType::FUNCTION;
  data.functionRef = newValue;
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

bool UI_Model_SetInt_Impl(const UIModelIndex nodeIndex, int32_t newValue) {
  if (nodeIndex == 0) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::INT && data.integer == newValue) {
    return false;
  }

  data.dataType = UIModelDataType::INT;
  data.integer = newValue;
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

bool UI_Model_SetReal_Impl(const UIModelIndex nodeIndex, float newValue) {
  if (nodeIndex == 0) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::REAL && data.real == newValue) {
    return false;
  }

  data.real = newValue;
  data.dataType = UIModelDataType::REAL;
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

bool UI_Model_SetString_Impl(const UIModelIndex nodeIndex,
                             const char *newValue) {
  if (nodeIndex == 0 || newValue == nullptr) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::STRING && data.string != nullptr) {
    if (I_strcmp(data.string, newValue) == 0) {
      return false;
    }
    hunk::Hunk_UserFree(*s_modelStringHunk, const_cast<char *>(data.string));
  }

  const size_t size = std::strlen(newValue) + 1;
  void *copy = hunk::Hunk_UserAlloc(*s_modelStringHunk, size, 8, "");
  std::memmove(copy, newValue, size);

  data.dataType = UIModelDataType::STRING;
  data.string = static_cast<const char *>(copy);
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

bool UI_Model_SetUInt64_Impl(const UIModelIndex nodeIndex, uint64_t newValue) {
  if (nodeIndex == 0) {
    return false;
  }

  UIModelData &data = node(nodeIndex).data;
  if (data.dataType == UIModelDataType::UINT64 && data.uint64 == newValue) {
    return false;
  }

  data.dataType = UIModelDataType::UINT64;
  data.uint64 = newValue;
  UI_Model_NotifySubscriptions_Impl(nodeIndex);
  return true;
}

UIModelSubscription UI_Model_AddSubscription_Impl(UIModelIndex model,
                                                  int32_t functionRef,
                                                  lua::hks::lua_State *luaVM) {
  if (model == 0 || luaVM == nullptr) {
    return 0;
  }

  const UIModelSubscription added = *s_firstFreeSubscription;
  if (added == 0) {
    return 0;
  }

  UIModelSubscriptionEntry &entry = subscription(added);
  *s_firstFreeSubscription = entry.next;
  entry.model = model;
  entry.luaVM = luaVM;
  entry.functionRef = functionRef;
  entry.next = node(model).firstSubscription;
  node(model).firstSubscription = added;
  return added;
}

void UI_Model_RemoveSubscription_Impl(UIModelSubscription removed) {
  if (removed == 0) {
    return;
  }

  UIModelSubscriptionEntry &entry = subscription(removed);
  if (entry.model == 0) {
    return;
  }

  UIModelSubscription &first = node(entry.model).firstSubscription;
  if (first == removed) {
    first = entry.next;
  } else {
    for (UIModelSubscription current = first; current != 0;
         current = subscription(current).next) {
      if (subscription(current).next == removed) {
        subscription(current).next = entry.next;
        break;
      }
    }
  }

  entry.model = 0;
  entry.next = 0;
}

void UI_Model_UnsubscribeAndFreeModel_Impl(UIModelIndex model) {
  const sys::ScopedCriticalSection critSect = lock_models();

  for (UIModelSubscription current = node(model).firstSubscription;
       current != 0;) {
    const UIModelSubscription next = subscription(current).next;
    UI_Model_RemoveSubscription_Impl(current);
    current = next;
  }

  for (UIModelIndex child = node(model).firstChild; child != 0;) {
    const UIModelIndex nextSibling = node(child).nextSibling;
    UI_Model_UnsubscribeAndFreeModel_Impl(child);
    child = nextSibling;
  }

  UI_Model_FreeModel_Impl(model);
}
} // namespace ui
} // namespace game
