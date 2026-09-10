#include <std_include.hpp>

#include <game/game.hpp>

#include <loader/component_loader.hpp>
#include <component/lua/lua_state.hpp>
#include <component/path.hpp>

#include <utils/io.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace json {
using namespace game::lua::hks;
using namespace game::lua;

template <typename T> std::optional<T> try_parse(std::string_view str) {
  T value{};
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);

  if (ec == std::errc{} && ptr == (str.data() + str.size())) {
    return value;
  }

  return std::nullopt;
}

luaReturnCount_e read(hks::lua_State *luaVM) {
  try {
    if (lua_gettop(luaVM) > 3) {
      const char *file_str = lua_tostring(luaVM, 1);
      if (file_str) {
        const std::filesystem::path file = path::normalize(file_str);
        if (std::filesystem::is_regular_file(file)) {

          HksObject *default_value;
          std::string key;
          std::string_view type;

          // Get arguments
          if (lua_gettop(luaVM) == 4) {
            const char *key_raw = lua_tostring(luaVM, 2);
            const char *type_raw = lua_tostring(luaVM, 3);
            default_value = hks::getObjectForIndex(luaVM, 4);

            if (type_raw && key_raw && default_value) {
              if (key_raw[0] != '/') {
                key += '/';
              }
              key += key_raw;
              type = type_raw;
            } else {
              return luaReturnCount_e::NONE;
            }

          } else {
            const char *root_key_raw = lua_tostring(luaVM, 2);
            const char *sub_key_raw = lua_tostring(luaVM, 3);
            const char *type_raw = lua_tostring(luaVM, 4);
            default_value = hks::getObjectForIndex(luaVM, 5);

            if (type_raw && root_key_raw && sub_key_raw && default_value) {
              key += '/';
              key += root_key_raw;
              const std::string sub_key = sub_key_raw;
              if (sub_key.size()) {
                if (key.size() > 1) {
                  key += '/';
                  key += sub_key;
                } else {
                  key += sub_key;
                }
              }
              type = type_raw;
            } else {
              return luaReturnCount_e::NONE;
            }
          }

          std::ifstream ifs(file);
          if (ifs.is_open()) {
            rapidjson::IStreamWrapper isw(ifs);

            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            if (!doc.HasParseError()) {

              rapidjson::Value *val = rapidjson::Pointer(key.c_str()).Get(doc);
              if (val) {

                switch (val->GetType()) {
                // TODO
                case rapidjson::Type::kArrayType:
                // TODO
                case rapidjson::Type::kObjectType:
                case rapidjson::Type::kNullType:
                  lua_push(luaVM, default_value);
                  break;
                case rapidjson::Type::kFalseType: {
                  if (type == "string") {
                    lua_pushstring(luaVM, "0");
                  } else if (type == "number" || type == "float" ||
                             type == "double") {
                    lua_pushnumber(luaVM, 0.0f);
                  } else if (type == "integer" || type == "int" ||
                             type == "unsigned" || type == "uint") {
                    lua_pushinteger(luaVM, 0);
                  } else if (type == "bool" || type == "boolean") {
                    lua_pushboolean(luaVM, hfalse);
                  } else {
                    lua_push(luaVM, default_value);
                  }
                  break;
                }
                case rapidjson::Type::kTrueType: {
                  if (type == "string") {
                    lua_pushstring(luaVM, "1");
                  } else if (type == "number" || type == "float" ||
                             type == "double") {
                    lua_pushnumber(luaVM, 1.0f);
                  } else if (type == "integer" || type == "int" ||
                             type == "unsigned" || type == "uint") {
                    lua_pushinteger(luaVM, 1);
                  } else if (type == "bool" || type == "boolean") {
                    lua_pushboolean(luaVM, htrue);
                  } else {
                    lua_push(luaVM, default_value);
                  }
                  break;
                }
                case rapidjson::Type::kNumberType: {
                  if (type == "string") {
                    lua_pushstring(luaVM,
                                   utils::string::va("%0.8g", val->GetFloat()));
                  } else if (type == "number" || type == "float" ||
                             type == "double") {
                    lua_pushnumber(luaVM, val->GetFloat());
                  } else if (type == "integer" || type == "int" ||
                             type == "unsigned" || type == "uint") {
                    lua_pushinteger(luaVM,
                                    static_cast<uint32_t>(val->GetFloat()));
                  } else if (type == "bool" || type == "boolean") {
                    lua_pushboolean(luaVM,
                                    qboolean::from(val->GetFloat() != 0.0f));
                  } else {
                    lua_push(luaVM, default_value);
                  }
                  break;
                }
                case rapidjson::Type::kStringType: {
                  const char *val_str = val->GetString();
                  if (val_str) {
                    if (type == "string") {
                      lua_pushstring(luaVM, val_str);
                      break;

                    } else if (type == "number" || type == "float" ||
                               type == "double") {
                      const std::optional<float> parsed =
                          try_parse<float>(val_str);
                      if (parsed.has_value()) {
                        lua_pushnumber(luaVM, parsed.value());
                        break;
                      }
                    } else if (type == "integer" || type == "int" ||
                               type == "unsigned" || type == "uint") {
                      // int64_t can handle both the `-` prefix and the full
                      // range of possible positive values in an unsigned 32-bit
                      // integer.
                      const std::optional<int64_t> parsed =
                          try_parse<int64_t>(val_str);
                      if (parsed.has_value()) {
                        lua_pushinteger(luaVM,
                                        static_cast<uint32_t>(parsed.value()));
                        break;
                      }
                    } else if (type == "bool" || type == "boolean") {
                      lua_pushboolean(
                          luaVM,
                          val_str &&
                              ((val_str[0] == '1' && val_str[1] == '\0') ||
                               ((val_str[0] == 't' || val_str[0] == 'T') &&
                                (val_str[1] == 'r' || val_str[1] == 'R') &&
                                (val_str[2] == 'u' || val_str[2] == 'U') &&
                                (val_str[3] == 'e' || val_str[3] == 'E') &&
                                val_str[4] == '\0')));
                      break;
                    }
                  }
                  lua_push(luaVM, default_value);
                  break;
                }
                }
              } else {
                lua_push(luaVM, default_value);
              }
              return luaReturnCount_e::ONE;
            }
          }
        }
      }
    }
  } catch (...) {
  }
  return luaReturnCount_e::NONE;
}

luaReturnCount_e write(hks::lua_State *luaVM) {
  try {
    if (lua_gettop(luaVM) > 3) {
      const char *file_str = lua_tostring(luaVM, 1);
      if (file_str) {
        const std::filesystem::path file = path::normalize(file_str);
        if (!std::filesystem::is_regular_file(file)) {
          std::filesystem::create_directories(file.parent_path());
          utils::io::write_file(file, "{}", false);
        }

        int32_t value_idx;
        std::string key;
        std::string_view type;

        // Get arguments
        if (lua_gettop(luaVM) == 4) {
          const char *key_raw = lua_tostring(luaVM, 2);
          const char *type_raw = lua_tostring(luaVM, 3);
          value_idx = 4;

          if (type_raw && key_raw) {
            if (key_raw[0] != '/') {
              key += '/';
            }
            key += key_raw;
            type = type_raw;
          } else {
            return luaReturnCount_e::NONE;
          }

        } else {
          const char *root_key_raw = lua_tostring(luaVM, 2);
          const char *sub_key_raw = lua_tostring(luaVM, 3);
          const char *type_raw = lua_tostring(luaVM, 4);
          value_idx = 5;

          if (type_raw && root_key_raw && sub_key_raw) {
            key += '/';
            key += root_key_raw;
            const std::string sub_key = sub_key_raw;
            if (sub_key.size()) {
              if (key.size() > 1) {
                key += '/';
                key += sub_key;
              } else {
                key += sub_key;
              }
            }
            type = type_raw;
          } else {
            return luaReturnCount_e::NONE;
          }
        }

        std::ifstream ifs(file);
        if (ifs.is_open()) {
          rapidjson::IStreamWrapper isw(ifs);

          rapidjson::Document doc;
          doc.ParseStream(isw);
          ifs.close();

          if (!doc.HasParseError()) {
            // `Create` ensures the path exists if it does not already, but does
            // not overwrite an existing path.
            rapidjson::Value &val = rapidjson::Pointer(key.c_str()).Create(doc);
            if (type == "string") {
              size_t len;
              const char *value_str = lua_tostring(luaVM, value_idx, &len);
              val.SetString(value_str, len);
            } else if (type == "number" || type == "float" ||
                       type == "double") {
              val.SetFloat(lua_tonumber(luaVM, value_idx));
            } else if (type == "integer" || type == "int" ||
                       type == "unsigned" || type == "uint") {
              val.SetInt(lua_tointeger<int32_t>(luaVM, value_idx));
            } else if (type == "bool" || type == "boolean") {
              val.SetBool(lua_toboolean(luaVM, value_idx));
              // Fallback: match lua types to JSON types where possible
            } else {
              const std::optional<HksObjectType> top_type =
                  lua_gettype(luaVM, value_idx);

              if (top_type.has_value()) {
                switch (top_type.value()) {
                case HksObjectType::TSTRING: {
                  size_t len;
                  const char *value_str = lua_tostring(luaVM, value_idx, &len);
                  val.SetString(value_str, len);
                  break;
                }
                case HksObjectType::TBOOLEAN: {
                  val.SetBool(lua_toboolean(luaVM, value_idx));
                  break;
                }
                case HksObjectType::TNIL:
                case HksObjectType::TNONE:
                  val.SetNull();
                  break;
                case HksObjectType::TNUMBER:
                  val.SetFloat(lua_tonumber(luaVM, value_idx));
                  break;
                default:
                  break;
                }
              } else {
                val.SetNull();
              }
            }

            std::ofstream ofs(file, std::ios::trunc);
            if (ofs.is_open()) {
              rapidjson::OStreamWrapper osw(ofs);
              rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
              doc.Accept(writer);
            }
          }
        }
      }
    }
  } catch (...) {
  }
  return luaReturnCount_e::NONE;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    static constexpr const luaL_Reg JSONLibrary[] = {
        lua_state::luaL_LoggedReg<"JSON", "ReadJson",
                                  lua_state::unsafe_function<read>>(),
        lua_state::luaL_LoggedReg<"JSON", "WriteJson",
                                  lua_state::unsafe_function<write>>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("JSON", JSONLibrary);
  }
};
} // namespace json

REGISTER_COMPONENT(json::component)