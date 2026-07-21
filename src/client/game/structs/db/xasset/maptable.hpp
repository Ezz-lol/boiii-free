#pragma once

#include "core.hpp"

namespace game {
namespace db {
namespace xasset {
namespace maptable {

enum class MapSize : uint32_t {
  SMALL = 0x0,
  MEDIUM = 0x1,
  LARGE = 0x2,
  COUNT = 0x3,
};

PACKED(struct MapTableEntry {
  XString name;
  MapSize size;
  uint8_t _padding0C[4];
  XString mapName;
  XString mapNameCaps;
  XString rootMapName;
  XString mapDescription;
  XString mapDescriptionShort;
  XString missionName;
  XString mapLocation;
  XString mapDateTime;
  XString presenceString;
  gfx::GfxImage *previewImage;
  XString loadingImage;
  XString loadingImage_axis;
  gfx::GfxImage *compassMap;
  gfx::GfxImage *compassMapCorrupt;
  bool isSafeHouse;
  bool isSubLevel;
  uint8_t _padding82[6];
  XString safeHouse;
  XString introMovie;
  XString outroMovie;
  ObjectiveListPtr objectives;
  XString collectibles;
  ScriptBundleListPtr accoladelist;
  XString mapObjective;
  vec3_t engagementRanges;
  uint8_t _paddingCC[4];
  XString resistanceText;
  gfx::GfxImage *operationsBackground;
  bool isFreeRunMap;
  uint8_t _paddingE1[3];
  uint32_t freerunTrackIndex;
  uint32_t mapVersion;
  uint32_t kvpCount;
  ScriptBundleKVP *kvpItems;
  int32_t uniqueID;
  uint8_t _paddingFD[4];
});

PACKED(struct MapTable {
  XString name;
  uint32_t mapCount;
  uint8_t _padding0C[4];
  MapTableEntry *maps;
  eModes sessionMode;
  CampaignMode campaignMode;
  dlcIndex_t dlcIndex;
  uint8_t _padding24[4];
});
ASSERT_SIZE(MapTable, 0x28);
} // namespace maptable
} // namespace xasset
} // namespace db
} // namespace game