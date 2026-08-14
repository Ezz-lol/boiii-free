#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <component/scheduler.hpp>

#include <game/impl/scr/place.hpp>

#include <utils/hook.hpp>

namespace render {
using namespace game::r;
utils::hook::detour R_StoreWindowSettings_hook;

void R_StoreWindowSettings_AllowPositiveViewScale(
    const GfxWindowParms *wndParms) {
#ifndef NDEBUG
  const void *callerAddr = _ReturnAddress();
#endif

  R_StoreWindowSettings_hook.invoke(wndParms);

#ifndef NDEBUG
  str1024_t vidConfigSerializationBuf;
  game::trace("R_StoreWindowSettings called at 0x%p with vidConfig: %s",
              game::derelocate(callerAddr),
              vidConfig->serialize<std::size(vidConfigSerializationBuf)>(
                  vidConfigSerializationBuf));
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
  memset(vidConfigSerializationBuf, 0, std::size(vidConfigSerializationBuf));
  game::trace(
      "R_StoreWindowSettings returning from call at 0x%p with vidConfig: %s",
      game::derelocate(callerAddr),
      vidConfig->serialize<std::size(vidConfigSerializationBuf)>(
          vidConfigSerializationBuf));
#endif
}

utils::hook::detour ScrPlace_Init_hook;
class component final : public generic_component {
public:
  void post_unpack() override {
    if (game::ultrawide()) {
      R_StoreWindowSettings_hook.create(
          game::r::R_StoreWindowSettings,
          R_StoreWindowSettings_AllowPositiveViewScale);

      if (game::is_client()) {
        ScrPlace_Init_hook.create(game::scr::place::ScrPlace_Init,
                                  game::scr::place::ScrPlace_Init_Impl);
      } else {
        scheduler::once(game::scr::place::ScrPlace_Init_Impl,
                        scheduler::pipeline::main);
      }
    }
  }
};
} // namespace render
REGISTER_COMPONENT(render::component);