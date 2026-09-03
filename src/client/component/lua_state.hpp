#pragma once

#include <game/game.hpp>
#include <utils/concurrency.hpp>
#include "ui_scripting.hpp"

namespace lua_state {
using namespace game::lua;
using namespace game::lua::hks;
using lua_state_cb = fastcallPtr_t<void(lua_State *luaVM)>;

void on_state_init(const lua_state_cb callback);
void on_state_openlibs(const lua_state_cb callback);

typedef std::unordered_map<const char *, const luaL_Reg *> libmap_t;
extern utils::concurrency::container<libmap_t> custom_libs;

template <const IntegralLike<size_t> auto N>
inline void register_lib(const char *name, const array<luaL_Reg, N> &lib) {
  custom_libs.access([name, &lib](libmap_t &libs) { libs[name] = &lib; });
}

template <const IntegralLike<size_t> auto N>
inline void register_lib(const char *name, const std::array<luaL_Reg, N> &lib) {
  custom_libs.access([name, &lib](libmap_t &libs) { libs[name] = lib.data(); });
}

inline void register_lib(const char *name,
                         const std::span<const luaL_Reg> &lib) {
  custom_libs.access([name, &lib](libmap_t &libs) { libs[name] = lib.data(); });
}

inline void register_lib(const char *name, const luaL_Reg *lib) {
  custom_libs.access([name, &lib](libmap_t &libs) { libs[name] = lib; });
}

template <lua_CFunction &func>
luaReturnCount_e unsafe_function(lua_State *luaVM) {
  if (game::is_server() || ui_scripting::unsafe_lua_approved_for_session.load(
                               std::memory_order_acquire)) {
    return func(luaVM);
  }

  ui_scripting::show_unsafe_lua_dialog();
  return luaReturnCount_e::NONE;
}

template <const char *Library, const char *Name, lua_CFunction *func>
luaReturnCount_e log_call(lua_State *luaVM) {
#ifndef NDEBUG
  game::trace("%s.%s called with argc: %d", Library, Name, lua_gettop(luaVM));
#endif
  return func(luaVM);
}

template <ConstString Library, ConstString Name, lua_CFunction *Function>
struct luaL_LoggedReg : public luaL_Reg {
#ifdef NDEBUG
  constexpr luaL_LoggedReg() {
    this->name = Name;
    this->function = Function;
  }
#else
  constexpr luaL_LoggedReg() {
    this->name = Name;
    this->function = log_call<Library, Name, Function>;
  }
#endif
};

} // namespace lua_state