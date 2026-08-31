#include <std_include.hpp>

#include <game/game.hpp>

#include "component/lua_state.hpp"
#include <loader/component_loader.hpp>

namespace mainlua {
using namespace game::lua::hks;
using namespace game::lua;

luaReturnCount_e revision(lua_State *s) {
#ifndef NDEBUG
  game::trace("MainLUA.Revision called with argc: %d", lua_gettop(s));
#endif

  // Ensure minimum T7Recharged DLL revision checks always pass
  lua_pushnumber(s, std::numeric_limits<float>::max());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e module_loaded(lua_State *s) {
#ifndef NDEBUG
  game::trace("MainLUA.ModuleLoaded called with argc: %d", lua_gettop(s));
#endif
  lua_pushboolean(s, htrue);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e unload_mod([[maybe_unused]] lua_State *s) {
#ifndef NDEBUG
  game::trace("MainLUA.UnloadMod called with argc: %d", lua_gettop(s));
#endif
  return luaReturnCount_e::NONE;
}

// Not sure what this is meant to return. CPU family, model, stepping? Brand?
// And of the parameters used, how are they formatted or otherwise used to
// generate the final output?
luaReturnCount_e hardware_cpu(lua_State *s) {
#ifndef NDEBUG
  game::trace("MainLUA.HardwareCPU called with argc: %d", lua_gettop(s));
#endif

  lua_pushstring(s, "generic");
  return luaReturnCount_e::ONE;
}

// Hardware ID could be generated with numerous possible data sources.
// TODO: reverse engineer T7Recharged's `GetHardwareId` implementation to
// re-create its generation logic
luaReturnCount_e hardware_id(lua_State *s) {
#ifndef NDEBUG
  game::trace("MainLUA.HardwareID called with argc: %d", lua_gettop(s));
#endif

  lua_pushstring(s, "generic");
  return luaReturnCount_e::ONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg MainLUALibrary[] = {
        {"GetHardwareCPU", lua_state::unsafe_function<hardware_cpu>},
        {"GetHardwareID", lua_state::unsafe_function<hardware_id>},
        {"ModuleLoaded", lua_state::unsafe_function<module_loaded>},
        {"Revision", revision},
        {"UnloadMod", unload_mod},
        {nullptr, nullptr},
    };
    lua_state::register_lib("MainLUA", MainLUALibrary);
  }
};
} // namespace mainlua

REGISTER_COMPONENT(mainlua::component)