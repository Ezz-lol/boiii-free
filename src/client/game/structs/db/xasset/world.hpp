#pragma once

#include <cstdint>

#include <game/structs/quake/vec.hpp>
#include <game/structs/gfx/core.hpp>
#include <game/structs/scr/primitives.hpp>

namespace game {
namespace db {
namespace xasset {
namespace world {
struct pathnode_tree_t;
typedef pathnode_tree_t *pathnode_tree_ptr;

struct pathnode_tree_nodes_t {
  int32_t nodeCount;
  uint16_t *nodes;
};

union pathnode_tree_info_t {
  pathnode_tree_ptr child[2];
  pathnode_tree_nodes_t s;
};

struct pathnode_tree_t {
  int32_t axis;
  vec_t dist;
  pathnode_tree_info_t u;
};

struct pathbasenode_t {
  vec3_t vOrigin;
  uint32_t type;
};

enum class nodeType : uint32_t {
  BADNODE = 0x0,
  PATHNODE = 0x1,
  COVER_STAND = 0x2,
  COVER_CROUCH = 0x3,
  COVER_CROUCH_WINDOW = 0x4,
  COVER_PRONE = 0x5,
  COVER_RIGHT = 0x6,
  COVER_LEFT = 0x7,
  COVER_PILLAR = 0x8,
  AMBUSH = 0x9,
  EXPOSED = 0xA,
  CONCEALMENT_STAND = 0xB,
  CONCEALMENT_CROUCH = 0xC,
  CONCEALMENT_PRONE = 0xD,
  REACQUIRE = 0xE,
  BALCONY = 0xF,
  SCRIPTED = 0x10,
  NEGOTIATION_BEGIN = 0x11,
  NEGOTIATION_END = 0x12,
  TURRET = 0x13,
  GUARD = 0x14,
  NUMTYPES = 0x15,
  DONTLINK = 0x15,
};

struct __declspec(align(4)) pathlink_s {
  float fDist;
  uint16_t nodeNum;
  uint8_t disconnectCount;
  uint8_t negotiationLink;
  uint8_t flags;
  uint8_t ubBadPlaceCount[5];
};

typedef pathlink_s pathlink_t;

struct __attribute__((aligned(8))) pathnode_constant_t {
  nodeType type;
  int32_t spawnflags;
  int32_t movementtype_ignore;
  int32_t movementtype_require;
  scr::ScrString_t targetname;
  scr::ScrString_t script_linkName;
  scr::ScrString_t script_noteworthy;
  scr::ScrString_t target;
  scr::ScrString_t animscript;
  scr::scr_funcptr_t animscriptfunc;
  vec3_t vOrigin;
  float fAngle;
  vec2_t forward;
  short wOverlapNode[4];
  uint16_t totalLinkCount;
  pathlink_t *Links;
  int32_t navFaceIndex;
  int32_t navRegionIndex;
  int32_t negotiationLinkedNodeIndex;
  float negotiationCostModifier;
  float width;
};

class SentientHandle {
public:
  uint16_t number;
  uint16_t infoIndex;
  uint32_t pad;
};

struct pathnode_dynamic_t {
  SentientHandle pOwner;
  int32_t iFreeTime;
  int32_t iValidTime[3];
  int32_t dangerousNodeTime[3];
  int32_t inPlayerLOSTime;
  short wLinkCount;
  short wOverlapCount;
  short turretEntNumber;
  short userCount;
  int32_t flags;
};

struct pathnode_t;

struct __attribute__((aligned(8))) pathnode_transient_t {
  int32_t iSearchFrame;
  pathnode_t *pNextOpen;
  pathnode_t *pPrevOpen;
  pathnode_t *pParent;
  float fCost;
  float fHeuristic;
  union {
    float nodeCost;
    int32_t linkIndex;
  };
};

struct pathnode_t {
  pathnode_constant_t constant;
  pathnode_dynamic_t dynamic;
  pathnode_transient_t transient;
};

struct PathData {
  uint32_t nodeCount;
  uint32_t originalNodeCount;
  pathnode_t *nodes;
  pathbasenode_t *basenodes;
  int32_t visBytes;
  uint8_t *pathVis;
  int32_t smoothBytes;
  uint8_t *smoothCache;
  int32_t nodeTreeCount;
  pathnode_tree_t *nodeTree;
};

// Verified
struct GameWorld {
  const char *name;
  PathData path;
};
ASSERT_SIZE(GameWorld, 0x50);

// Verified
struct GfxWorld {
  const char *name;
  const char *baseName;
  int32_t planeCount;
  int32_t nodeCount;
  int32_t surfaceCount;
  gfx::GfxWorldStreamInfo streamInfo;
  gfx::WorldSpawnConfig worldSpawnConfig;
  uint32_t coronaCount;
  gfx::GfxLightCorona *coronas;
  uint32_t sunVolumeCount;
  gfx::GfxSunVolume *sunVolumes;
  uint32_t sunVolumePlanesCount;
  vec4_t *sunVolumePlanes;
  uint32_t outdoorVolumeCount;
  gfx::GfxOutdoorVolume *outdoorVolumes;
  uint32_t outdoorVolumePlanesCount;
  vec4_t *outdoorVolumePlanes;
  uint32_t exposureVolumeCount;
  gfx::GfxExposureVolume *exposureVolumes;
  uint32_t exposureVolumePlaneCount;
  vec4_t *exposureVolumePlanes;
  uint32_t worldFogVolumeCount;
  gfx::GfxWorldFogVolume *worldFogVolumes;
  uint32_t worldFogVolumePlaneCount;
  vec4_t *worldFogVolumePlanes;
  uint32_t worldFogModifierVolumeCount;
  gfx::GfxWorldFogModifierVolume *worldFogModifierVolumes;
  uint32_t worldFogModifierVolumePlaneCount;
  gfx::GfxVolumePlane *worldFogModifierVolumePlanes;
  uint32_t lutVolumeCount;
  gfx::GfxLutVolume *lutVolumes;
  uint32_t lutVolumePlaneCount;
  gfx::GfxVolumePlane *lutVolumePlanes;
  uint32_t weatherGrimeVolumeCount;
  gfx::GfxWeatherGrimeVolume *weatherGrimeVolumes;
  uint32_t weatherGrimeVolumePlaneCount;
  vec4_t *weatherGrimeVolumePlanes;
  uint32_t genericVolumeCount;
  gfx::GfxGenericVolume *genericVolumes;
  uint32_t genericVolumePlaneCount;
  vec4_t *genericVolumePlanes;
  uint32_t attenuationVolumeCount;
  gfx::GfxConfig_LightAttenuationVolume *attenuationVolumes;
  gfx::GfxSkyDynamicIntensity skyDynIntensity;
  gfx::GfxWorldDpvsPlanes dpvsPlanes;
  int32_t cellBitsCount;
  gfx::GfxCell *cells;
  gfx::GfxWorldDraw draw;
  int32_t modelCount;
  gfx::GfxBrushModel *models;
  vec3_t mins;
  vec3_t maxs;
  uint32_t checksum;
  int32_t materialMemoryCount;
  MaterialMemory *materialMemory;
  uint32_t *cellCasterBits;
  gfx::GfxSceneDynModel *sceneDynModel;
  gfx::GfxWorldDpvsStatic dpvs;
  gfx::GfxWorldDpvsDynamic dpvsDyn;
  const Material *coronaMaterial;
  const Material *ropeMaterial;
  const Material *lutMaterial;
  const Material *lutMaterialALT;
  uint32_t numOccluders;
  gfx::Occluder *occluders;
  uint32_t lightingFlags;
  int32_t lightingQuality;
  int32_t umbraNumTomes;
  gfx::GfxUmbraTome *umbraTomes;
  bool umbraTomeIncludesLightsAndFx;
  uint32_t numSiegeAnims;
  gfx::GfxSiegeAnimPtr *siegeAnims;
  uint32_t numSiegeAnimsAutoPlay;
  gfx::SiegeAutoPlay *siegeAnimsAutoPlay;
  uint32_t lensPackCount;
  gfx::GfxCameraLens *lensPackData;
  uint32_t litFogVolumeCount;
  gfx::GfxLitFogVolume *litFogVolumes;
  uint32_t volumeDecalCount;
  gfx::GfxVolumeDecal *volumeDecals;
  gfx::GfxImagePtr volumeDecalRevealTexture;
  gfx::GfxWorldWaterDisk water;
  gfx::GfxVATLASUKKOGPU vukkoLVX[32];
  uint32_t baseGfxVUKKO[4];
  uint32_t nGfxModOverVUKKO;
  gfx::GfxModVUKKO *pGfxModOverVUKKO;
  uint32_t nGfxBoxOverVUKKO;
  gfx::GfxBoxVUKKO *pGfxBoxOverVUKKO;
  uint32_t nGfxBoxBaseVUKKO;
  gfx::GfxBoxVUKKO *pGfxBoxBaseVUKKO;
  gfx::GfxVBLOOMGPU baseGfxVBLOOMGPU[4];
  uint32_t nGfxModOverVBLOOM;
  gfx::GfxModVBLOOM *pGfxModOverVBLOOM;
  uint32_t nGfxBoxOverVBLOOM;
  gfx::GfxBoxVBLOOM *pGfxBoxOverVBLOOM;
  uint32_t nGfxBoxBaseVBLOOM;
  gfx::GfxBoxVBLOOM *pGfxBoxBaseVBLOOM;
  uint32_t baseGfxVLUT[4];
  uint32_t nGfxModOverVLUT;
  gfx::GfxModVLUT *pGfxModOverVLUT;
  uint32_t nGfxBoxOverVLUT;
  gfx::GfxBoxVLUT *pGfxBoxOverVLUT;
  uint32_t nGfxBoxBaseVLUT;
  gfx::GfxBoxVLUT *pGfxBoxBaseVLUT;
  gfx::GfxVCOLORGPU baseGfxVCOLORGPU[4];
  uint32_t nGfxModOverVCOLOR;
  gfx::GfxModVCOLOR *pGfxModOverVCOLOR;
  uint32_t nGfxBoxOverVCOLOR;
  gfx::GfxBoxVCOLOR *pGfxBoxOverVCOLOR;
  uint32_t nGfxBoxBaseVCOLOR;
  gfx::GfxBoxVCOLOR *pGfxBoxBaseVCOLOR;
  uint32_t nGfxBoxOverUNDERWATER;
  gfx::GfxBoxUNDERWATER *pGfxBoxOverUNDERWATER;
  uint32_t nGfxBoxBaseUNDERWATER;
  gfx::GfxBoxUNDERWATER *pGfxBoxBaseUNDERWATER;
};
ASSERT_SIZE(GfxWorld, 0x2040);

typedef uint8_t byte4;

struct ComUmbraVolume {
  vec3_t min;
  vec3_t max;
};

struct ComUmbraGate {
  const char *name;
  int32_t umbraId;
  int32_t gateIndex;
};

struct ComUmbraTome {
  const char *name;
  uint32_t numVolumes;
  ComUmbraVolume *volumes;
  uint32_t indicesSize;
  byte4 *indicesSTQuery;
  byte4 *indicesMTQuery;
  uint32_t numGates;
  ComUmbraGate *gates;
};

struct __attribute__((aligned(4))) ComProbeTriggerData {
  int32_t probeID;
  int32_t volumeID;
  int32_t delayOn;
  int32_t delayOff;
  int32_t fadeIn;
  int32_t fadeOut;
  bool reversed;
};

struct ComProbeExploder {
  int32_t nameHash;
  int32_t triggerCount;
  ComProbeTriggerData *triggerData;
};

struct __attribute__((aligned(2))) ComLightTriggerData {
  int32_t delayOn;
  int32_t delayOff;
  int32_t fadeIn;
  int32_t fadeOut;
  uint16_t primaryLightIndex;
  bool reversed;
};

struct ComLightExploder {
  int32_t nameHash;
  int32_t triggerCount;
  ComLightTriggerData *triggerData;
};

struct ComPrimaryLight;

struct ComWorld {
  const char *name;
  qboolean isInUse;
  uint32_t primaryLightCount;
  ComPrimaryLight *primaryLights;
  uint32_t lightExploderCount;
  ComLightExploder *lightExploders;
  uint32_t sunVolumeCount;
  uint32_t *sunVolumeProbeCount;
  uint32_t probeCount;
  uint32_t *probeGuids;
  uint32_t probeExploderCount;
  ComProbeExploder *probeExploders;
  uint32_t umbraTomeCount;
  ComUmbraTome *umbraTomes;
  uint8_t _unknown68[0x20];
};
ASSERT_SIZE(ComWorld, 0x88); // Correct size

} // namespace world
} // namespace xasset
} // namespace db
} // namespace game