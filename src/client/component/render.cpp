#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <component/scheduler.hpp>
#include <game/game.hpp>

#include <game/impl/scr/place.hpp>

#include <utils/hook.hpp>

namespace render {
using namespace game::r;
utils::hook::detour R_StoreWindowSettings_hook;
utils::hook::detour ScrPlace_Init_hook;

void ScrPlace_Init_FlaggedHook() {
  if (game::ultrawide()) {
    game::scr::place::ScrPlace_Init_Impl();
  } else {
    ScrPlace_Init_hook.invoke<void>();
  }
}

void R_StoreWindowSettings_AllowPositiveViewScale(
    const GfxWindowParms *wndParms) {
#ifndef NDEBUG
  const void *callerAddr = _ReturnAddress();
#endif

  R_StoreWindowSettings_hook.invoke(wndParms);
  if (game::ultrawide()) {
#ifndef NDEBUG
    game::trace("R_StoreWindowSettings called at {:p} with vidConfig: {}",
                game::derelocate(callerAddr), vidConfig->serialize());
#endif

    if (vidConfig->sceneAspectRatio > DEFAULT_UI_VIEW_ASPECT_RATIO &&
        vidConfig->sceneAspectRatio !=
            static_cast<uint32_t>(vidConfig->viewAspectRatio)) {
      vidConfig->viewHeight = vidConfig->sceneHeight;
      vidConfig->viewWidth = vidConfig->sceneWidth;
      vidConfig->displayWidth = vidConfig->viewWidth;
      vidConfig->displayHeight = vidConfig->viewHeight;
      vidConfig->displayAspectRatio = vidConfig->sceneAspectRatio;
      vidConfig->viewScalePx = 1.0;
      vidConfig->viewAspectRatio = vidConfig->sceneAspectRatio;
      vidConfig->isWideScreen = qtrue;
      *game::cl::cls_vidConfig = *vidConfig;
    }

#ifndef NDEBUG
    game::trace(
        "R_StoreWindowSettings returning from call at {:p} with vidConfig: {}",
        game::derelocate(callerAddr), vidConfig->serialize());
#endif
  }
}

class component final : public generic_component {
#ifndef NDEBUG
  std::string name() override { return "render"; }
#endif

public:
  void post_unpack() override {
    R_StoreWindowSettings_hook.create(
        game::r::R_StoreWindowSettings,
        R_StoreWindowSettings_AllowPositiveViewScale);

    if (game::is_client()) {
      ScrPlace_Init_hook.create(game::scr::place::ScrPlace_Init,
                                ScrPlace_Init_FlaggedHook);
    } else if (game::ultrawide()) {
      scheduler::once(game::scr::place::ScrPlace_Init_Impl,
                      scheduler::pipeline::main);
    }
  }
};
} // namespace render
REGISTER_COMPONENT(render::component);
