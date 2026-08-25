#include <std_include.hpp>

#include <game/game.hpp>

#include <utils/HTTPRequest.hpp>

#include "component/lua_state.hpp"
#include <loader/component_loader.hpp>

namespace axios {
using namespace game::lua::hks;
using namespace game::lua;
luaReturnCount_e get(lua_State *s) {
  const char *url = lua_tostring(s, 1);

  http::Request request{url};
  const http::Response response = request.send("GET");

  const std::string responseBody =
      std::string{response.body.begin(), response.body.end()};
  lua_pushstring(s, responseBody.c_str());

  return luaReturnCount_e::ONE;
}

luaReturnCount_e post(lua_State *s) {
  const char *url = lua_tostring(s, 1);
  const char *body = lua_tostring(s, 2);

  http::Request request{url};
  const http::Response response =
      request.send("POST", body, {{"Content-Type", "application/json"}});

  const std::string responseBody =
      std::string{response.body.begin(), response.body.end()};
  lua_pushstring(s, responseBody.c_str());

  return luaReturnCount_e::ONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg AxiosLibrary[3] = {
        {"Get", lua_state::unsafe_function<get>},
        {"Post", lua_state::unsafe_function<post>},
        {nullptr, nullptr},
    };
    lua_state::register_lib("Axios", AxiosLibrary);
  }
};
} // namespace axios

REGISTER_COMPONENT(axios::component)