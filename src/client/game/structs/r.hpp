#pragma once

#include "game/structs/quake/core.hpp"
namespace game {
namespace r {

constexpr float DEFAULT_UI_VIEW_WIDTH = 1920.0f;
constexpr float DEFAULT_UI_VIEW_HEIGHT = 1080.0f;
constexpr float DEFAULT_UI_VIEW_ASPECT_RATIO =
    DEFAULT_UI_VIEW_WIDTH / DEFAULT_UI_VIEW_HEIGHT;

/*
   Known to be very incorrect/out of date as of initial definition here.
   Definition is left here both to use its type in a function signature, and
   to help give ideas for what is stored in this struct for future reverse
   engineering.
*/
struct GfxWindowParms {
  bool isWideScreen;
  int32_t sceneWidth;
  int32_t sceneHeight;
  int32_t displayWidth;
  int32_t displayHeight;
};

struct vidConfig_t {
  uint32_t sceneWidth;
  uint32_t sceneHeight;
  float sceneAspectRatio;
  int32_t displayWidth;
  int32_t displayHeight;
  float displayAspectRatio;
  qboolean isFullscreen;
  qboolean isWideScreen;
  float viewAspectRatioPx; // Real aspect ratio of displayed UI view
  uint32_t viewWidth;
  uint32_t viewHeight;
  float viewAspectRatio; // UI aspect ratio, scaled up/down for rendering to
                         // viewAspectRatioPX
  float viewScalePx;     // Scale of viewAspectRatio / viewAspectRatioPx
  uint32_t maxTextureSize;

#ifndef NDEBUG
  template <const size_t N>
  inline str<N> &serialize(str<N> &buf) const noexcept {
    snprintf(buf, N,
             "vidConfig_t { sceneWidth: %u, sceneHeight: %u, sceneAspectRatio: "
             "%f, displayWidth: %u, displayHeight: %u, displayAspectRatio: %f, "
             "isFullScreen: %s, isWideScreen: %s, viewAspectRatioPX: %f, "
             "viewWidth: %u, viewHeight: %u, viewAspectRatio: %f, viewScalePx: "
             "%f, maxTextureSize: %u }",
             sceneWidth, sceneHeight, sceneAspectRatio, displayWidth,
             displayHeight, displayAspectRatio, isFullscreen.serialize(),
             isWideScreen.serialize(), viewAspectRatioPx, viewWidth, viewHeight,
             viewAspectRatio, viewScalePx, maxTextureSize);
    return buf;
  }
#endif
};
ASSERT_SIZE(vidConfig_t, 0x38);

} // namespace r
} // namespace game