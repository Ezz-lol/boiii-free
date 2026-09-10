#include <std_include.hpp>

#include <game/game.hpp>

#include <utils/HTTPRequest.hpp>

#include <loader/component_loader.hpp>
#include <component/lua/lua_state.hpp>

namespace axios {
using namespace game::lua::hks;
using namespace game::lua;
luaReturnCount_e get(lua_State *s) {
  if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
    const char *url = lua_tostring(s, 1);

    try {
      http::Request request{url};

      const http::Response response = request.send("GET");

      const std::string responseBody =
          std::string{response.body.begin(), response.body.end()};
      lua_pushstring(s, responseBody.c_str());
    } catch (...) {
      lua_pushstring(s, "");
    }

    return luaReturnCount_e::ONE;
  }

  return luaReturnCount_e::NONE;
}

luaReturnCount_e post(lua_State *s) {
  if (lua_gettop(s) > 1 && lua_isstring(s, 1) && lua_isstring(s, 2)) {
    const char *url = lua_tostring(s, 1);
    const char *body = lua_tostring(s, 2);

    try {
      http::Request request{url};
      const http::Response response =
          request.send("POST", body, {{"Content-Type", "application/json"}});

      const std::string responseBody =
          std::string{response.body.begin(), response.body.end()};
      lua_pushstring(s, responseBody.c_str());
    } catch (...) {
      lua_pushstring(s, "");
    }

    return luaReturnCount_e::ONE;
  }

  return luaReturnCount_e::NONE;
}

/*
  Purposely stubbed.

  T7Recharged's implementation calls the `ping` application with arguments `-c
  1`. These arguments are valid in unix-like operating systems (e.g. BSDs,
  Linux, MacOS), but not in the Windows `ping` application, which should take
  the arguments `-n 1` instead.

  As such, this function is useless in T7Recharged, and is not used by any mods
  accordingly.

  We do not need to implement this.
*/
luaReturnCount_e ping(lua_State *s) {
  lua_pushstring(s, "");

  return luaReturnCount_e::ONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg AxiosLibrary[] = {
        lua_state::luaL_LoggedReg<"Axios", "Get",
                                  lua_state::unsafe_function<get>>(),
        lua_state::luaL_LoggedReg<"Axios", "GetEncrypted",
                                  lua_state::unsafe_function<get>>(),
        lua_state::luaL_LoggedReg<"Axios", "GetRetrieve",
                                  lua_state::unsafe_function<get>>(),
        lua_state::luaL_LoggedReg<"Axios", "GetUpdate",
                                  lua_state::unsafe_function<get>>(),
        lua_state::luaL_LoggedReg<"Axios", "GetUpdateEncrypted",
                                  lua_state::unsafe_function<get>>(),
        lua_state::luaL_LoggedReg<"Axios", "Ping",
                                  lua_state::unsafe_function<ping>>(),
        lua_state::luaL_LoggedReg<"Axios", "Post",
                                  lua_state::unsafe_function<post>>(),
        lua_state::luaL_LoggedReg<"Axios", "PostEncrypted",
                                  lua_state::unsafe_function<post>>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("Axios", AxiosLibrary);
  }
};
} // namespace axios

REGISTER_COMPONENT(axios::component)