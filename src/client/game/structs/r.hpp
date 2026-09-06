#pragma once

#include <structs/str.hpp>
#include "game/structs/quake/core.hpp"

#ifndef NDEBUG
#include <string>
#include <format>
#endif

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
  inline std::string serialize() const noexcept {
    return std::format(
        "vidConfig_t {{ sceneWidth: {}, sceneHeight: {}, sceneAspectRatio: "
        "{}, displayWidth: {}, displayHeight: {}, displayAspectRatio: {}, "
        "isFullScreen: {}, isWideScreen: {}, viewAspectRatioPX: {}, "
        "viewWidth: {}, viewHeight: {}, viewAspectRatio: {}, viewScalePx: "
        "{}, maxTextureSize: {} }}",
        sceneWidth, sceneHeight, sceneAspectRatio, displayWidth, displayHeight,
        displayAspectRatio, isFullscreen.serialize(), isWideScreen.serialize(),
        viewAspectRatioPx, viewWidth, viewHeight, viewAspectRatio, viewScalePx,
        maxTextureSize);
  }

#endif
};
ASSERT_SIZE(vidConfig_t, 0x38);

} // namespace r
} // namespace game