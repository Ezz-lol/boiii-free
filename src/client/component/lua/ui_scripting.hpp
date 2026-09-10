#pragma once

#include <atomic>
#include <functional>

#include <game/ui_scripting/script_value.hpp>

#include <frozen/unordered_set.h>
#include <frozen/string.h>

namespace ui_scripting {
using namespace game::lua::hks;

extern std::atomic<bool> ui_initialized;

extern std::atomic<bool> unsafe_function_called_message_shown;
extern std::atomic<bool> unsafe_lua_approved_for_session;
bool show_unsafe_lua_dialog();

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

struct CaseInsensitiveHash {
  constexpr djb2Hash64_t operator()(frozen::string key) const {
    return djb264(key.data());
  }

  constexpr fnv1aHash64_t operator()(frozen::string key,
                                     fnv1aHash64_t seed) const {
    return fnv1a64(seed, key.data());
  }
};

// Custom case-insensitive equality functor
struct CaseInsensitiveEqual {
  constexpr bool operator()(frozen::string a, frozen::string b) const {
    if (a.size() != b.size())
      return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
      if (std::tolower(a.data()[i]) != std::tolower(b.data()[i]))
        return false;
    }
    return true;
  }
};

template <const size_t Size>
using frozen_case_insensitive_unordered_set =
    frozen::unordered_set<frozen::string, Size, CaseInsensitiveHash,
                          CaseInsensitiveEqual>;

template <const size_t Size>
inline constexpr frozen_case_insensitive_unordered_set<Size>
make_frozen_case_insensitive_unordered_set(frozen::string const (&keys)[Size]) {
  return frozen::make_unordered_set<frozen::string, Size, CaseInsensitiveHash,
                                    CaseInsensitiveEqual>(
      keys, CaseInsensitiveHash{}, CaseInsensitiveEqual{});
}

} // namespace ui_scripting