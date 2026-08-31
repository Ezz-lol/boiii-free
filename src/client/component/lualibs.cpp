#include <std_include.hpp>

#include <game/game.hpp>

#include "component/lua_state.hpp"
#include <loader/component_loader.hpp>

// Adds stubbed variations of miscellaneous lua library API functions seen in
// T7Recharged. These would be useless in boiii, so can be safely stubbed.
namespace lualibs {
using namespace game::lua::hks;
using namespace game::lua;
luaReturnCount_e lua_stub_func([[maybe_unused]] lua_State *s) {

  return luaReturnCount_e::NONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg HotReloadLibrary[] = {
        {"Start", lua_stub_func},
        {nullptr, nullptr},
    };
    lua_state::register_lib("HotReload", HotReloadLibrary);

    static constexpr const luaL_Reg UIErrorHashLibrary[] = {
        {"Remove", lua_stub_func},
        {nullptr, nullptr},
    };
    lua_state::register_lib("UIErrorHash", UIErrorHashLibrary);
  }
};
} // namespace lualibs

REGISTER_COMPONENT(lualibs::component)