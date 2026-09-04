#include <filesystem>
#include <std_include.hpp>

#include <game/game.hpp>

#include "component/lua_state.hpp"
#include "component/path.hpp"
#include <loader/component_loader.hpp>
#include <utils/io.hpp>

namespace fileio {
using namespace game::lua::hks;
using namespace game::lua;

bool SetClipboardText(const std::string_view &text) {
  if (!OpenClipboard(nullptr)) {
    return false;
  }

  if (!EmptyClipboard()) {
    CloseClipboard();
    return false;
  }

  size_t size = text.length() + 1;
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
  if (!hMem) {
    CloseClipboard();
    return false;
  }

  char *pMem = static_cast<char *>(GlobalLock(hMem));
  if (pMem) {
    memcpy(pMem, text.data(), size);
    GlobalUnlock(hMem);
  } else {
    GlobalFree(hMem);
    CloseClipboard();
    return false;
  }

  if (!SetClipboardData(CF_TEXT, hMem)) {
    GlobalFree(hMem);
    CloseClipboard();
    return false;
  }

  CloseClipboard();
  return true;
}

std::wstring GetClipboardText() {
  if (!OpenClipboard(nullptr)) {
    return L"";
  }

  if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) {
    CloseClipboard();
    return L"";
  }

  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
  if (hData == nullptr) {
    CloseClipboard();
    return L"";
  }

  const wchar_t *pszText = static_cast<const wchar_t *>(GlobalLock(hData));
  if (pszText == nullptr) {
    CloseClipboard();
    return L"";
  }

  std::wstring result(pszText);

  GlobalUnlock(hData);
  CloseClipboard();

  return result;
}

std::string WideToNarrow(const std::wstring_view &wstr) {
  // Set the locale to the system's default to handle multibyte characters
  // properly
  std::setlocale(LC_CTYPE, "");

  std::mbstate_t state = std::mbstate_t();
  const wchar_t *src = wstr.data();

  // Get the required size of the destination narrow string buffer
  size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
  if (len == static_cast<size_t>(-1)) {
    return ""; // Conversion failed (invalid wide character encountered)
  }

  // Allocate space and perform the actual conversion
  std::vector<char> buffer(len + 1);
  src = wstr.data(); // Reset source pointer
  std::wcsrtombs(buffer.data(), &src, buffer.size(), &state);

  return std::string(buffer.data());
}

luaReturnCount_e clipboard_get(lua_State *s) {
  std::string contents = "";
  try {
    const std::wstring wide_contents = GetClipboardText();
    contents = WideToNarrow(wide_contents);
  } catch (...) {
  }
  lua_pushstring(s, contents.c_str());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e clipboard_set(lua_State *s) {
  try {
    if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
      SetClipboardText(lua_tostring(s, 1));
    }
  } catch (...) {
  }
  return luaReturnCount_e::NONE;
}

luaReturnCount_e copy(lua_State *s) {
  try {
    if (lua_gettop(s) > 1 && lua_isstring(s, 1) && lua_isstring(s, 2)) {
      const char *src_path_arg = lua_tostring(s, 1);
      const char *dest_path_arg = lua_tostring(s, 2);
      // Third arg is boolean, and can take boolean-like string ("true"). Not
      // sure what it is for.
      if (src_path_arg && dest_path_arg) {
        const std::filesystem::path src_path =
            path::normalize_path(src_path_arg);
        const std::filesystem::path dest_path =
            path::normalize_path(dest_path_arg);
        if (std::filesystem::is_regular_file(src_path) &&
            std::filesystem::is_directory(dest_path.parent_path())) {
          std::filesystem::copy_file(src_path, dest_path);
        }
      }
    }
  } catch (...) {
  }
  return luaReturnCount_e::NONE;
}

luaReturnCount_e mkdir(lua_State *s) {
  try {
    if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
      const char *arg_path = lua_tostring(s, 1);
      if (arg_path) {
        const std::filesystem::path path = path::normalize_path(arg_path);
        if (!std::filesystem::exists(path)) {
          utils::io::create_directory(path);
        }
      }
    }
  } catch (...) {
  }
  return luaReturnCount_e::NONE;
}

luaReturnCount_e directory_exists(lua_State *s) {
  if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
    const char *arg_path = lua_tostring(s, 1);
    lua_pushboolean(
        s, hksBool::from(arg_path && std::filesystem::is_directory(
                                         path::normalize_path(arg_path))));
  } else {
    lua_pushboolean(s, hfalse);
  }
  return luaReturnCount_e::ONE;
}

luaReturnCount_e file_exists(lua_State *s) {
  if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
    const char *arg_path = lua_tostring(s, 1);
    lua_pushboolean(
        s, hksBool::from(arg_path && std::filesystem::exists(
                                         path::normalize_path(arg_path))));
  } else {
    lua_pushboolean(s, hfalse);
  }
  return luaReturnCount_e::ONE;
}

luaReturnCount_e file_size(lua_State *s) {
  size_t result = 0;
  if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
    const char *arg_path = lua_tostring(s, 1);
    if (arg_path) {
      const std::filesystem::path path = path::normalize_path(arg_path);
      if (std::filesystem::is_regular_file(path)) {
        result = std::filesystem::file_size(path);
      }
    }
  }
  lua_pushinteger(s, result);
  return luaReturnCount_e::ONE;
}

luaReturnCount_e read_file(lua_State *s) {
  std::string result = "";
  try {
    if (lua_gettop(s) > 0 && lua_isstring(s, 1)) {
      const char *arg_path = lua_tostring(s, 1);
      if (arg_path) {
        const std::filesystem::path path = path::normalize_path(arg_path);
        if (std::filesystem::is_regular_file(path)) {
          result = utils::io::read_file(path);
        }
      }
    }
  } catch (...) {
  }
  lua_pushstring(s, result.c_str());
  return luaReturnCount_e::ONE;
}

luaReturnCount_e write_file(lua_State *s) {
  std::string result = "";
  try {
    if (lua_gettop(s) > 1 && lua_isstring(s, 1) && lua_isstring(s, 2)) {
      const char *arg_path = lua_tostring(s, 1);
      const char *data = lua_tostring(s, 2);
      /*
        Not sure what arg3 and arg4 are.
        One of arg3 or arg4 is probably "Append" based on common file write API
        function arguments and flags, but I am uncertain what the other would
        be. "Create" is the default behaviour for this function, so it is
        probably not "Create".

        These arguments are unused (always `0`) in AAE's lua code, which is
        where this function was encountered.
      */
      [[maybe_unused]] const bool arg3 =
          lua_gettop(s) > 2 && lua_isboolean(s, 3) ? lua_toboolean(s, 3)
                                                   : false;
      [[maybe_unused]] const bool arg4 =
          lua_gettop(s) > 3 && lua_isboolean(s, 4) ? lua_toboolean(s, 4)
                                                   : false;
      if (arg_path && data) {
        const std::filesystem::path path = path::normalize_path(arg_path);
        if (std::filesystem::is_directory(path.parent_path()) &&
            !std::filesystem::is_directory(path)) {
          utils::io::write_file(path, data);
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
    static constexpr const luaL_Reg FileIOLibrary[] = {
        lua_state::luaL_LoggedReg<"FileIO", "ClipboardGet",
                                  lua_state::unsafe_function<clipboard_get>>(),
        lua_state::luaL_LoggedReg<"FileIO", "ClipboardSet",
                                  lua_state::unsafe_function<clipboard_set>>(),
        lua_state::luaL_LoggedReg<"FileIO", "Copy",
                                  lua_state::unsafe_function<copy>>(),
        lua_state::luaL_LoggedReg<"FileIO", "CreateDirectory",
                                  lua_state::unsafe_function<mkdir>>(),
        lua_state::luaL_LoggedReg<
            "FileIO", "DirectoryExists",
            lua_state::unsafe_function<directory_exists>>(),
        lua_state::luaL_LoggedReg<"FileIO", "FileExists",
                                  lua_state::unsafe_function<file_exists>>(),
        lua_state::luaL_LoggedReg<"FileIO", "FileSize",
                                  lua_state::unsafe_function<file_size>>(),
        lua_state::luaL_LoggedReg<"FileIO", "ReadFile",
                                  lua_state::unsafe_function<read_file>>(),
        lua_state::luaL_LoggedReg<"FileIO", "WriteFile",
                                  lua_state::unsafe_function<write_file>>(),
        {nullptr, nullptr},
    };
    lua_state::register_library("FileIO", FileIOLibrary);
  }
};
} // namespace fileio

REGISTER_COMPONENT(fileio::component)