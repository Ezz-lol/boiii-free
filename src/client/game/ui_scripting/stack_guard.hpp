#pragma once

#include <game/game.hpp>

#include <cstddef>

namespace ui_scripting::detail {
using namespace game::lua::hks;

class api_stack_guard final {
public:
  explicit api_stack_guard(lua_State *state) noexcept
      : state_(state),
        offset_(state->m_apistack.top - state->m_apistack.bottom) {}

  api_stack_guard(const api_stack_guard &) = delete;
  api_stack_guard &operator=(const api_stack_guard &) = delete;

  ~api_stack_guard() { state_->m_apistack.top = saved_top(); }

  [[nodiscard]] HksObject *saved_top() const noexcept {
    return state_->m_apistack.bottom + offset_;
  }

private:
  lua_State *state_;
  std::ptrdiff_t offset_;
};
} // namespace ui_scripting::detail