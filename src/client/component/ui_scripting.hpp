#pragma once

#include <atomic>
#include <functional>

#include <game/ui_scripting/script_value.hpp>

namespace ui_scripting {
using namespace game::lua::hks;

extern std::atomic<bool> ui_initialized;

extern std::atomic<bool> unsafe_function_called_message_shown;
extern std::atomic<bool> unsafe_lua_approved_for_session;
void show_unsafe_lua_dialog();

template <class... Args, std::size_t... I>
auto wrap_function(const std::function<void(Args...)> &f,
                   std::index_sequence<I...>) {
  return [f](const function_arguments &args) {
    f(args[I]...);
    return arguments{{}};
  };
}

template <class... Args, std::size_t... I>
auto wrap_function(const std::function<arguments(Args...)> &f,
                   std::index_sequence<I...>) {
  return [f](const function_arguments &args) { return f(args[I]...); };
}

template <typename R, class... Args, std::size_t... I>
auto wrap_function(const std::function<R(Args...)> &f,
                   std::index_sequence<I...>) {
  return
      [f](const function_arguments &args) { return arguments{f(args[I]...)}; };
}

template <typename R, class... Args>
auto wrap_function(const std::function<R(Args...)> &f) {
  return wrap_function(f, std::index_sequence_for<Args...>{});
}

template <class F> auto wrap_function(F f) {
  return wrap_function(std::function(f));
}

template <typename F> cclosure *convert_function(F f);
} // namespace ui_scripting