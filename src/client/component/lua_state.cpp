#include <std_include.hpp>

#include <loader/component_loader.hpp>
#include <game/game.hpp>

#include "lua_state.hpp"

#include <utils/hook.hpp>

namespace lua_state {
utils::concurrency::container<std::vector<lua_state_cb>> state_init_tasks;
utils::concurrency::container<std::vector<lua_state_cb>> state_openlibs_tasks;
utils::concurrency::container<libmap_t> custom_libs;
namespace {

utils::hook::detour hks_newstate_hook;
lua_State *hks_newstate_exec_callbacks(HksStateSettings *settings) {
  lua_State *s = hks_newstate_hook.invoke<lua_State *>(settings);
  state_init_tasks.access([s](const std::vector<lua_state_cb> &tasks) {
    for (const lua_state_cb func : tasks) {
      func(s);
    }
  });
  return s;
}

utils::hook::detour luaL_openlibs_hook;
void luaL_openlibs_exec_callbacks(lua_State *s) {
  luaL_openlibs_hook.invoke(s);
  custom_libs.access([s](libmap_t &map) {
    for (auto [name, lib] : map) {
      hksI_openlib(s, name, lib, 0, qtrue);
    }
  });
  state_openlibs_tasks.access([s](const std::vector<lua_state_cb> &tasks) {
    for (const lua_state_cb func : tasks) {
      func(s);
    }
  });
}
} // namespace

void on_state_init(const lua_state_cb callback) {
  state_init_tasks.access([callback](std::vector<lua_state_cb> &tasks) {
    tasks.emplace_back(callback);
  });
}

void on_state_openlibs(const lua_state_cb callback) {
  state_openlibs_tasks.access([callback](std::vector<lua_state_cb> &tasks) {
    tasks.emplace_back(callback);
  });
}

class component final : public generic_component {
public:
  void post_unpack() override {
    hks_newstate_hook.create(hks_newstate, hks_newstate_exec_callbacks);
    luaL_openlibs_hook.create(luaL_openlibs, luaL_openlibs_exec_callbacks);
  }
};
} // namespace lua_state

REGISTER_COMPONENT(lua_state::component)