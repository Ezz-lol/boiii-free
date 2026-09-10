#include <std_include.hpp>

#include <game/game.hpp>

#include <loader/component_loader.hpp>
#include <component/lua/lua_state.hpp>
#include <component/auth.hpp>

namespace mainlua {
using namespace game::lua::hks;
using namespace game::lua;

inline constexpr const uint32_t T7RECHARGED_REVISION =
    std::numeric_limits<uint32_t>::max();
inline constexpr hexArray_t<uint32_t> T7RECHARGED_REVISION_STR_BUF =
    to_hex<uint32_t>(T7RECHARGED_REVISION);
inline constexpr const char *T7RECHARGED_REVISION_STR =
    T7RECHARGED_REVISION_STR_BUF.data();

luaReturnCount_e remove_ui_error(lua_State *s) {
  lua_pushboolean(s, htrue);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e revision(lua_State *s) {
  // Ensure minimum T7Recharged DLL revision checks always pass
  lua_pushinteger(s, T7RECHARGED_REVISION);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e version(lua_State *s) {
  // Ensure minimum T7Recharged DLL version checks always pass
  lua_pushstring(s, T7RECHARGED_REVISION_STR);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e module_loaded(lua_State *s) {
  lua_pushboolean(s, htrue);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e mod_publisher_id(lua_State *s) {
  lua_pushstring(s, game::ugc::active_mod->publisherId);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e unload_mod([[maybe_unused]] lua_State *s) {
  return luaReturnCount_e::NONE;
}

// Not sure what this is meant to return. CPU family, model, stepping? Brand?
// And of the parameters used, how are they formatted or otherwise used to
// generate the final output?
luaReturnCount_e hardware_cpu(lua_State *s) {
  lua_pushstring(s, "generic");
  return luaReturnCount_e::ONE;
}

// Hardware ID could be generated with numerous possible data sources.
// TODO: reverse engineer T7Recharged's `GetHardwareId` implementation to
// re-create its generation logic
luaReturnCount_e hardware_id(lua_State *s) {
  lua_pushstring(s, "generic");
  return luaReturnCount_e::ONE;
}

luaReturnCount_e restart_hud(lua_State *s) {
  lua_pushboolean(s, htrue);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e lui_reload(lua_State *s) {
  lua_pushboolean(s, htrue);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e mod_directory(lua_State *s) {
  lua_pushstring(s, game::ugc::active_mod->absolutePathZoneFiles);
  return luaReturnCount_e::ONE;
}

std::chrono::milliseconds now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());
}

luaReturnCount_e get_time(lua_State *s) {
  lua_pushinteger(s, static_cast<uint32_t>(now_ms().count()));
  return luaReturnCount_e::ONE;
}

static std::chrono::milliseconds TIMER_START = now_ms();

luaReturnCount_e start_timer(lua_State *s) {
  TIMER_START = now_ms();
  return luaReturnCount_e::NONE;
}

luaReturnCount_e stop_timer(lua_State *s) {
  const std::chrono::milliseconds now = now_ms();
  const std::chrono::milliseconds elapsed = now - TIMER_START;
  TIMER_START = now;
  lua_pushinteger(s, static_cast<uint32_t>(elapsed.count()));
  return luaReturnCount_e::ONE;
}

/*
  I strongly dislike this API.

  This should take an optional controller index as argument, and return only the
  XUID as integer or string. It also should not cast the XUID to a signed
  integer, nor should the XUID value be casted/truncated to a 32-bit integer
  before being serialized in the return string.

  This API is poorly designed, and essentially useless.

  However, this matches the T7Recharged API, so must be implemented as-is.
*/
luaReturnCount_e get_xuid(lua_State *luaVM) {
  lua_pushstring(luaVM, utils::string::va("XUID: %i", static_cast<int32_t>(
                                                          auth::get_guid())));
  return luaReturnCount_e::ONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg MainLUALibrary[] = {
        lua_state::luaL_LoggedReg<"MainLUA", "GetHardwareCPU", hardware_cpu>(),
        lua_state::luaL_LoggedReg<"MainLUA", "GetHardwareID", hardware_id>(),
        lua_state::luaL_LoggedReg<"MainLUA", "GetTime", get_time>(),
        lua_state::luaL_LoggedReg<"MainLUA", "GeXuid", get_xuid>(),
        lua_state::luaL_LoggedReg<"MainLUA", "LuiReload", lui_reload>(),
        lua_state::luaL_LoggedReg<"MainLUA", "ModFolder", mod_directory>(),
        lua_state::luaL_LoggedReg<"MainLUA", "ModPublisherID",
                                  mod_publisher_id>(),
        lua_state::luaL_LoggedReg<"MainLUA", "ModuleLoaded", module_loaded>(),
        lua_state::luaL_LoggedReg<"MainLua", "RemoveUIError",
                                  remove_ui_error>(),
        lua_state::luaL_LoggedReg<"MainLUA", "RestartHUD", restart_hud>(),
        lua_state::luaL_LoggedReg<"MainLUA", "Revision", revision>(),
        lua_state::luaL_LoggedReg<"MainLUA", "StartTimer", start_timer>(),
        lua_state::luaL_LoggedReg<"MainLUA", "StopTimer", stop_timer>(),
        lua_state::luaL_LoggedReg<"MainLUA", "UnloadMod", unload_mod>(),
        lua_state::luaL_LoggedReg<"MainLUA", "Version", version>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("MainLUA", MainLUALibrary);
  }
};
} // namespace mainlua

REGISTER_COMPONENT(mainlua::component)