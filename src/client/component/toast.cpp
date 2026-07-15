#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"
#include "game/ui_scripting/execution.hpp"
#include "toast.hpp"
#include "scheduler.hpp"

#include <utils/string.hpp>

namespace toast {
using namespace game::ui::lua::hks;
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
  lua_State *state = *primary_luaVM;
  if (!state)
    return false;

  try {
    HashTable *globals = state->globals.v.table;
    const ui_scripting::table lua{globals};

    state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;
    const auto load_results = lua["loadstring"](code, "toast");
    state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::SECURE;

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

const char *zombie_toast_patch = R"lua(
if not CoD.isZombie then return end
if CoD.__toastPatched then return end
CoD.__toastPatched = true

local function ensureToast(hud)
	if hud == nil or hud.toastNotification ~= nil or CoD.ToastNotification == nil then
		return
	end
	local controller = hud.controller or Engine.GetPrimaryController()
	local toast = CoD.ToastNotification.new(hud, controller)
	toast:setState("DefaultState")
	toast:setPriority(9999)
	hud.toastNotification = toast
	local parent = hud:getParent()
	if parent then
		parent:addElement(toast)
	end
end

if HUD_FirstSnapshot_Zombie ~= nil then
	local oldZombieFirstSnapshot = HUD_FirstSnapshot_Zombie
	HUD_FirstSnapshot_Zombie = function(hud, event)
		oldZombieFirstSnapshot(hud, event)
		ensureToast(hud)
	end
end

if LUI ~= nil and LUI.roots ~= nil then
	for _, root in pairs(LUI.roots) do
		if type(root) == "table" and root.getFirstChild ~= nil then
			local child = root:getFirstChild()
			while child ~= nil do
				if child.id == "Menu.HUD" then
					ensureToast(child)
				end
				child = child:getNextSibling()
			end
		end
	end
end
)lua";

} // namespace

void patch_hud() {
  static bool patched = false;
  if (patched)
    return;
  if (execute_lua(zombie_toast_patch))
    patched = true;
}

void precache_icon(const std::string &material) {
  const auto escaped = escape_lua_string(material);
  const std::string code = utils::string::va(
      "pcall(function()\n"
      "  if Engine.PrecacheImage then\n"
      "    Engine.PrecacheImage(\"%s\")\n"
      "  end\n"
      "end)\n",
      escaped.c_str());
  scheduler::once([code] { execute_lua(code); }, scheduler::main);
}
 
void show(const std::string &title, const std::string &description,
          const std::string &icon) {
  if (game::is_server())
    return;

  patch_hud();

  const auto escaped_title = escape_lua_string(title);
  const auto escaped_description = escape_lua_string(description);
  const auto escaped_icon = escape_lua_string(icon);

  const std::string code = utils::string::va(
    "pcall(function()\n"
    "  if CoD and CoD.OverlayUtility and CoD.OverlayUtility.ShowToast then\n"
    "    CoD.OverlayUtility.ShowToast(\"Notice\", \"%s\", \"%s\", \"%s\")\n"
    "  end\n"
    "end)\n",
    escaped_title.c_str(),
    escaped_description.c_str(),
    escaped_icon.c_str());

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