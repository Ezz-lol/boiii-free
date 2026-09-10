#include <std_include.hpp>

#include <game/game.hpp>

#include <component/lua/lua_state.hpp>
#include <loader/component_loader.hpp>

namespace net {
using namespace game::lua::hks;
using namespace game::lua;

static std::atomic_bool demonware_enabled = true;
static std::atomic_bool steam_enabled = true;

luaReturnCount_e enable_demonware(lua_State *luaVM) {
  demonware_enabled.store(true, std::memory_order_release);
  return luaReturnCount_e::NONE;
}

luaReturnCount_e enable_steam(lua_State *luaVM) {
  steam_enabled.store(true, std::memory_order_release);
  return luaReturnCount_e::NONE;
}

luaReturnCount_e disable_demonware(lua_State *luaVM) {
  demonware_enabled.store(false, std::memory_order_release);
  return luaReturnCount_e::NONE;
}

luaReturnCount_e disable_steam(lua_State *luaVM) {
  steam_enabled.store(false, std::memory_order_release);
  return luaReturnCount_e::NONE;
}

luaReturnCount_e demonware_disabled(lua_State *luaVM) {
  lua_pushboolean(luaVM, !demonware_enabled.load(std::memory_order_acquire));
  return luaReturnCount_e::ONE;
}

luaReturnCount_e steam_disabled(lua_State *luaVM) {
  lua_pushboolean(luaVM, !steam_enabled.load(std::memory_order_acquire));
  return luaReturnCount_e::ONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {

    static constexpr const luaL_Reg NETLibrary[] = {
        lua_state::luaL_LoggedReg<
            "NET", "DisableDemonware",
            lua_state::unsafe_function<disable_demonware>>(),
        lua_state::luaL_LoggedReg<"NET", "DisableSteam",
                                  lua_state::unsafe_function<disable_steam>>(),
        lua_state::luaL_LoggedReg<
            "NET", "EnableDemonware",
            lua_state::unsafe_function<enable_demonware>>(),
        lua_state::luaL_LoggedReg<"NET", "EnableSteam",
                                  lua_state::unsafe_function<enable_steam>>(),
        lua_state::luaL_LoggedReg<
            "NET", "IsDemonwareDisabled",
            lua_state::unsafe_function<demonware_disabled>>(),
        lua_state::luaL_LoggedReg<"NET", "IsSteamDisabled",
                                  lua_state::unsafe_function<steam_disabled>>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("NET", NETLibrary);
  }
};
} // namespace net

REGISTER_COMPONENT(net::component)