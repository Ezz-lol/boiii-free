#pragma once

#include "game/structs/quake/vec.hpp"

#include "hks.hpp"

namespace game {
namespace db {
namespace xasset {
namespace ttf {
struct TTFDef;
}
} // namespace xasset
} // namespace db
namespace gfx {
struct GfxImage;
union GfxColor;
} // namespace gfx

namespace lua {
namespace lui {

struct UIQuadCache {
  vec3_t *xyz;
  vec2_t *st;
  gfx::GfxColor *color;
  hks::LUA_MATERIAL_DATATYPE material;
  const gfx::GfxImage *image;
  int16_t numQuads;
  bool dirty;
  uint8_t fontCacheVersion;
  int next;
};

enum class LUIAlignment : uint32_t {
  NONE = 0x0,
  LEFT = 0x1,
  CENTER = 0x2,
  RIGHT = 0x4,
  TOP = 0x8,
  MIDDLE = 0x10,
  BOTTOM = 0x20,
  SCREEN_SPACE = 0x40,
  HORIZONTAL_MASK = 0x7,
  VERTICAL_MASK = 0x38,
};

struct __declspec(align(8)) LUIAnimationState {
  hks::LUA_MATERIAL_DATATYPE material;
  db::xasset::ttf::TTFDef *font;
  uint64_t flags;
  int luaRef;
  unsigned __int8 tweenFlags;
  int ui3DWindow;
  float leftPct;
  float topPct;
  float rightPct;
  float bottomPct;
  float leftPx;
  float topPx;
  float rightPx;
  float bottomPx;
  float globalLeft;
  float globalTop;
  float globalRight;
  float globalBottom;
  float zoom;
  float xRot;
  float yRot;
  float zRot;
  float scale;
  float red;
  float green;
  float blue;
  float alpha;
  vec4_t shaderVector0;
  vec4_t shaderVector1;
  vec4_t shaderVector2;
  vec4_t shaderVector3;
  vec4_t shaderVector4;
  union {
    struct {
      union {
        uint64_t xuid;
        uint64_t fileID;
        uint64_t groupID;
        uint64_t controllerIndex;
        struct {
          float letterSpacing;
          float lineSpacing;
        };
      };
      union {
        hks::LUA_IMAGE_DATATYPE replacementImage;
        hks::LUA_MATERIAL_DATATYPE renderFromTextureMaterial;
      };
    };
    vec4_t shaderVector5;
  };
  union {
    float userData;
    ui::UIModelIndex userDataModel;
    int userDataInt;
    short userDataShorts[2];
    byte userDataBytes[4];
    char userDataChars[4];
    float spacing;
  };
  LUIAlignment alignment;
  unsigned __int32 useGameTime : 1;
};

// TODO
typedef void *UnknownFunc;
typedef UnknownFunc LUIElementLayoutFunction;
typedef UnknownFunc LUIElementRenderFunction;
typedef UnknownFunc LUIElementMouseFunction;
typedef UnknownFunc LUIElementCloseFunction;

struct __declspec(align(8)) LUIElement {
  LUIAnimationState currentAnimationState;
  LUIElement *parent;
  LUIElement *prevSibling;
  LUIElement *nextSibling;
  LUIElement *firstChild;
  LUIElement *lastChild;
  LUIElementLayoutFunction layoutFunction;
  LUIElementRenderFunction renderFunction;
  LUIElementMouseFunction mouseFunction;
  LUIElementCloseFunction closeFunction;
  LUIAnimationState *prevAnimationState;
  LUIAnimationState *nextAnimationState;
  int cacheRef;
  UIQuadCache *cache;
  int priority;
  int textRef;
  int animationTimeLeft;
  int animationDuration;
  int animationNameLuaRef;
  int strongLuaReference;
  float left;
  float top;
  float right;
  float bottom;
  union {
    float textDimLeft;
    float xOffset;
  };
  union {
    float textDimTop;
    float yOffset;
  };
  union {
    float textDimRight;
    float zOffset;
  };
  float textDimBottom;
  union {
    float textLeft;
    float scrollHandlePosition;
  };
  union {
    float textTop;
    float scrollHandleSize;
  };
  ui::UIModelIndex model;
};

/*
   Mostly unverified. `memset` on LUIElementPool in engine uses size 0x7EF400,
   which would nearly match, but does not include the fields up to and including
   m_firstFree. However, this could be purposefully resetting only data in the
   pool.

  This requires verification in general, but is likely similar to the correct
  implementation.
*/
struct __declspec(align(4)) LUIElementPool {
  LUIElement m_pool[20000];
  size_t m_poolData[20000];
  uint16_t m_firstFree;
  uint16_t m_iteration;
  uint16_t m_allocatedCount;
};

ASSERT_SIZE(LUIElementPool, 0x7EF408);

} // namespace lui
} // namespace lua
} // namespace game
