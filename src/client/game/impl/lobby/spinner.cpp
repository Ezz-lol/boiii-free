#include <std_include.hpp>

#include "spinner.hpp"

namespace game {
namespace lobby {
namespace spinner {

void LobbySpinner_Update_Impl(SpinnerSystem system, int32_t state) {
  if (game::is_client()) {
    const ControllerIndex_t primaryController =
        com::Com_ControllerIndexes_GetPrimary();
    const char *root =
        game::ui::UI_CoD_GetRootNameForController(primaryController);

    {
      game::lua::lui::LUIScopedEvent event(*lua::hks::UI_luaVM, root,
                                           "spinner_update");
      if (!event._finished) {
        const char *system_name = GetName_Impl(system);
        lua::Lua_SetTableString("system", system_name, event._vm);
        lua::Lua_SetTableInt("state", state, event._vm);
      }
    }
  }
}
} // namespace spinner
} // namespace lobby
} // namespace game
