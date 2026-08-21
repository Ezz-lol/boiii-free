#pragma once

#include <game/game.hpp>
#include <utils/concurrency.hpp>

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

} // namespace lua_state