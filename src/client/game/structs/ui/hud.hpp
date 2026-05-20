#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../quake.hpp"

namespace game::ui::hud {
union hudelem_color_t {
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };
  int32_t rgba;
};
ASSERT_SIZE(hudelem_color_t, 0x4);

// sizeof=0x7C
#pragma pack(push, 1)
struct hudelem_s {
  float x;
  float y;
  float z;
  float fontScale;
  float fromFontScale;
  int32_t fontScaleStartTime;
  hudelem_color_t color;
  hudelem_color_t fromColor;
  int32_t fadeStartTime;
  int32_t scaleStartTime;
  float fromX;
  float fromY;
  int32_t moveStartTime;
  int32_t time;
  int32_t duration;
  float value;
  float sort;
  hudelem_color_t glowColor;
  int32_t fxBirthTime;
  uint32_t flags;
  short targetEntNum;
  short fontScaleTime;
  short fadeTime;
  short label;
  short width;
  short height;
  short fromWidth;
  short fromHeight;
  short scaleTime;
  short moveTime;
  short text;
  uint16_t fxLetterTime;
  uint16_t fxDecayStartTime;
  uint16_t fxDecayDuration;
  uint16_t fxRedactDecayStartTime;
  uint16_t fxRedactDecayDuration;
  uint8_t type;
  uint8_t font;
  uint8_t alignOrg;
  uint8_t alignScreen;
  uint16_t materialIndex;
  uint16_t offscreenMaterialIdx;
  uint8_t fromAlignOrg;
  uint8_t fromAlignScreen;
  uint8_t soundID;
  uint8_t _padding[1];
};
ASSERT_SIZE(hudelem_s, 0x7C);
#pragma pack(pop)

typedef hudelem_s hudelem_t;

struct game_hudelem_s {
  hudelem_t elem;
  ClientNum_t clientNum;
  team_t team;
  qboolean archived;
};
ASSERT_SIZE(game_hudelem_s, 0x88);
typedef game_hudelem_s game_hudelem_t;

#pragma pack(push, 1)
struct serverHudMenu_t {
  UIModelData modelData[16];
  int32_t menuNameIndex;
  int32_t modelNames[16];
  bool persistent;
  uint8_t _padding145[11];
};
ASSERT_SIZE(serverHudMenu_t, 0x150);
#pragma pack(pop)
} // namespace game::ui::hud