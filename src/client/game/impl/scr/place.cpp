#include <std_include.hpp>

#include "place.hpp"

#include <game/utils.hpp>

namespace game {
namespace scr {
namespace place {
void ScrPlace_Init_Impl() {
  *safeArea_horizontal = game::register_dvar_float(
      "safeArea_horizontal", 1.0, 0.85000002, 1.0, 0,
      "Horizontal safe area as a fraction of the screen width");
  *safeArea_vertical = game::register_dvar_float(
      "safeArea_vertical", 1.0, 0.85000002, 1.0, 0,
      "Vertical safe area as a fraction of the screen height");
  if (game::is_client()) {
    *ui_safeArea = game::register_dvar_bool("ui_safearea", false, 0,
                                            "Sets the safe area to show/hide");
  }
  *game::cg::cg_hudSplitscreenScale = 1.0;
}

} // namespace place
} // namespace scr
} // namespace game