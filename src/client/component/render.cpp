#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>

#include <utils/hook.hpp>

namespace render {
using namespace game::r;
utils::hook::detour R_StoreWindowSettings_hook;

void R_StoreWindowSettings_AllowPositiveViewScale(
    const GfxWindowParms *wndParms) {
  R_StoreWindowSettings_hook.invoke(wndParms);

  if (vidConfig->sceneAspectRatio > DEFAULT_UI_VIEW_ASPECT_RATIO) {
    vidConfig->viewAspectRatio = vidConfig->sceneAspectRatio;
  }
}

class component final : public generic_component {
public:
  void post_unpack() override {
    R_StoreWindowSettings_hook.create(
        game::r::R_StoreWindowSettings,
        R_StoreWindowSettings_AllowPositiveViewScale);
  }
};
} // namespace render
REGISTER_COMPONENT(render::component);