#include <std_include.hpp>

#include "in.hpp"

namespace game {
namespace in {

bool GPad_IsDurangoGamepad_Impl(const LocalClientNum_t localClientNum) {
  ControllerIndex_t controllerIndex =
      com::Com_LocalClient_GetControllerIndex(localClientNum);
  // A mouse and keyboard player has no gamepad, which picked PlayStation button
  // icons even with an Xbox gamepad mapped to the other player
  if (!GamePad_Enabled(controllerIndex)) {
    for (ControllerIndex_t other = CONTROLLER_INDEX_FIRST;
         other < CONTROLLER_INDEX_COUNT; ++other) {
      if (GamePad_Enabled(other)) {
        controllerIndex = other;
        break;
      }
    }
  }
  return GPad_GetGamepadType(controllerIndex) == GAMEPAD_TYPE_DURANGO;
}

} // namespace in
} // namespace game
