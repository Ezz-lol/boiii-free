#pragma once

#include <structs/func.hpp>

#include "game/structs/quake/vec.hpp"

#include "core.hpp"
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
typedef vec4<uint8_t> GfxColor;
} // namespace gfx

namespace lua {
namespace lui {
typedef vec4<vec3_t> QuadVert;

struct UIQuadCache {
  QuadVert *xyz;
  vec4<vec2_t> *st;
  gfx::GfxColor *color;
  hks::LUA_MATERIAL_DATATYPE material;
  const gfx::GfxImage *image;
  int16_t numQuads;
  bool dirty;
  uint8_t fontCacheVersion;
  int32_t next;

  IMPL_SLICE_METHODS(xyz, numQuads);
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
IMPL_ENUM_OPERATORS(LUIAlignment);

struct __declspec(align(8)) LUIAnimationState {
  hks::LUA_MATERIAL_DATATYPE material;
  db::xasset::ttf::TTFDef *font;
  uint64_t flags;
  int32_t luaRef;
  uint8_t tweenFlags;
  int32_t ui3DWindow;
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
    int32_t userDataInt;
    short userDataShorts[2];
    byte userDataBytes[4];
    char userDataChars[4];
    float spacing;
  };
  LUIAlignment alignment;
  uint32_t useGameTime : 1;
};

typedef fastcall_t<void(const LocalClientNum_t localClientNum, LUIElement *,
                        LUIElement *, int, int, hks::lua_State *luaVM)>
    LUIElementLayoutFunction;
typedef fastcall_t<void(const LocalClientNum_t localClientNum, LUIElement *,
                        LUIElement *, float, float, float, float,
                        hks::lua_State *luaVM)>
    LUIElementRenderFunction;
typedef fastcall_t<void(const LocalClientNum_t localClientNum, LUIElement *,
                        LUIElement *, float, float, hks::lua_State *luaVM)>
    LUIElementMouseFunction;
typedef fastcall_t<void(LUIElement *element)> LUIElementCloseFunction;

PACKED(struct LUIElement {
  LUIAnimationState currentAnimationState;
  LUIElement *parent;
  LUIElement *prevSibling;
  LUIElement *nextSibling;
  LUIElement *firstChild;
  LUIElement *lastChild;
  LUIElementLayoutFunction *layoutFunction;
  LUIElementRenderFunction *renderFunction;
  LUIElementMouseFunction *mouseFunction;
  LUIElementCloseFunction *closeFunction;
  LUIAnimationState *prevAnimationState;
  LUIAnimationState *nextAnimationState;
  int32_t cacheRef;
  DEF_PAD(144, 4);
  UIQuadCache *cache;
  int32_t priority;
  int32_t textRef;
  int32_t animationTimeLeft;
  int32_t animationDuration;
  int32_t animationNameLuaRef;
  int32_t strongLuaReference;
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
  DEF_PAD(192, 6);
});
ASSERT_SIZE(LUIElement, 0x198);

/*
   Mostly unverified. `memset` on LUIElementPool in engine uses size 0x7EF400,
   which would nearly match, but does not include the fields after and including
   m_firstFree. However, this could be purposefully resetting only data in the
   pool.

  This requires verification in general, but is likely similar to the correct
  implementation.
*/
PACKED(struct LUIElementPool {
  static constexpr auto POOL_SIZE = 20000;

  LUIElement m_pool[POOL_SIZE];
  size_t m_poolData[POOL_SIZE];
  uint16_t m_firstFree;
  uint16_t m_iteration;
  uint16_t m_allocatedCount;
  DEF_PAD(7EF407, 2);
});

ASSERT_SIZE(LUIElementPool, 0x7EF408);

struct LUIScopedEvent : public LuaScopedEventBase {
  bool _initializedWithElement;
  DEF_PAD(59, 7);

  struct syms {
    static inline constexpr symbol<void(LUIScopedEvent *)> destructor{
        0x142685850, 0x0};

    static inline constexpr symbol<void(LUIScopedEvent *, hks::lua_State *luaVM,
                                        const char *rootName,
                                        const char *eventName)>
        LUIScopedEvent{0x142685620, 0x0};
  };

  LUIScopedEvent(hks::lua_State *luaVM, const char *rootName,
                 const char *eventName) {
    syms::LUIScopedEvent(this, luaVM, rootName, eventName);
  }

  ~LUIScopedEvent() { syms::destructor(this); }
};
ASSERT_SIZE(LUIScopedEvent, 0x60);

} // namespace lui
} // namespace lua
} // namespace game
