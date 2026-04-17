#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/ui_scripting/execution.hpp"
#include "toast.hpp"
#include "scheduler.hpp"

#include <utils/string.hpp>

namespace toast {
namespace {
std::string escape_lua_string(const std::string &s) {
  std::string result;
  result.reserve(s.size() + 8);
  for (const auto c : s) {
    switch (c) {
    case '\\':
      result += "\\\\";
      break;
    case '"':
      result += "\\\"";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      break;
    default:
      result += c;
      break;
    }
  }
  return result;
}

bool execute_lua(const std::string &code) {
  const auto state = *game::ui::lua::hks::lua_state;
  if (!state)
    return false;

  try {
    const auto globals = state->globals.v.table;
    const ui_scripting::table lua{globals};

    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_ON;
    const auto load_results = lua["loadstring"](code, "toast");
    state->m_global->m_bytecodeSharingMode =
        game::ui::lua::hks::HKS_BYTECODE_SHARING_SECURE;

    if (load_results[0].is<ui_scripting::function>()) {
      const auto results = lua["pcall"](load_results);
      if (!results[0].as<bool>()) {
        const auto err = results[1].as<std::string>();
        printf("[Toast] Lua error: %s\n", err.c_str());
        return false;
      }
      return true;
    } else if (load_results[1].is<std::string>()) {
      const auto err = load_results[1].as<std::string>();
      printf("[Toast] Lua compile error: %s\n", err.c_str());
    }
  } catch (const std::exception &ex) {
    printf("[Toast] Exception: %s\n", ex.what());
  }

  return false;
}
} // namespace

void show(const std::string &title, const std::string &description,
          const std::string &icon) {
  if (game::is_server())
    return;
  if (game::com::Com_IsInGame())
    return;

  const auto escaped_title = escape_lua_string(title);
  const auto escaped_desc = escape_lua_string(description);
  const auto escaped_icon = escape_lua_string(icon);

  const std::string code = utils::string::va(
      "pcall(function()\n"
      "  local ctrl = "
      "Engine.GetModelForController(Engine.GetPrimaryController())\n"
      "  local toast = Engine.CreateModel(ctrl, \"FrontendToast\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"state\"), "
      "\"DefaultState\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"kicker\"), \"%s\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"description\"), "
      "\"%s\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"contentIcon\"), "
      "\"%s\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"functionIcon\"), "
      "\"blacktransparent\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"backgroundId\"), "
      "\"blacktransparent\")\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"emblemDecal\"), 0)\n"
      "  Engine.SetModelValue(Engine.CreateModel(toast, \"notify\"), true)\n"
      "end)\n",
      escaped_title.c_str(), escaped_desc.c_str(), escaped_icon.c_str());

  // Must run on main thread where Lua state is available
  scheduler::once([code] { execute_lua(code); }, scheduler::main);
}

void success(const std::string &title, const std::string &description) {
  show(title, description, "t7_icon_save_overlays");
}

void warn(const std::string &title, const std::string &description) {
  show(title, description, "t7_icon_notice_overlays_bkg");
}

void error(const std::string &title, const std::string &description) {
  show(title, description, "t7_icon_error_overlays");
}

void info(const std::string &title, const std::string &description) {
  show(title, description, "t7_icon_info_overlays_bkg");
}
} // namespace toast
