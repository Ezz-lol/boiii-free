#pragma once

#include "hks.hpp"

namespace game {
namespace ui {
namespace lua {
namespace plus {

class LuaState // ?
{
private:
  [[maybe_unused]] uint8_t _gap0;
};

class LuaObject {
public:
  LuaState *m_state;
  hks::HksObject m_object;
  LuaObject *m_next;
  LuaObject *m_prev;
};

} // namespace plus
} // namespace lua
} // namespace ui
} // namespace game