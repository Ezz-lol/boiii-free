#ifndef GAME_STRUCTS_DB_XASSETS_HPP
#define GAME_STRUCTS_DB_XASSETS_HPP

#include <cstdint>
#include "../core.hpp"
#include "../quake.hpp"
#include "../gfx.hpp"

namespace game {

namespace snd {
/*
  Must #include <structs/structs.hpp> or <structs/snd.hpp> directly
  to use full structures - we need to only use forward declarations here to
  avoid circular dependencies between header files.
*/
struct SndBank;
struct SndPatch;
struct SndDriverGlobals;
typedef uint32_t SndAliasId;
} // namespace snd
namespace db {
namespace xasset {
/*
 For future reference, some known-correct asset struct sizes as per
 `DB_GetXAssetTypeSize`:
  - PHYSPRESET: 120 or 0x78
  - XMODELMESH: 120 or 0x78
  - PHYSCONSTRAINTS: 1680 or 0x690
  - DESTRUCTIBLEDEF: 48 or 0x30
  - XANIMPARTS: 248 or 0xF8
  - XMODEL: 392 or 0x188
  - MATERIAL: 672 or 0x2A0
  - COMPUTE_SHADER_SET: 24 or 0x18
  - SOUND_PATCH: 24 or 0x18
  - TECHNIQUE_SET: 112 or 0x70
  - IMAGE: 264 or 0x108
  - SOUND: 139440 or 0x220B0
  - CLIPMAP: 720 or 0x2D0
  - COMWORLD: 136 or 0x88
  - GAMEWORLD: 80 or 0x50
  - MAP_ENTS: 72 or 0x48
  - GFXWORLD: 8256 or 0x2040
  - LIGHT_DEF: 40 or 0x28
  - LENSFLARE_DEF: 536 or 0x218
*/

enum class XAssetType : int32_t {
  PHYSPRESET = 0x0,
  PHYSCONSTRAINTS = 0x1,
  DESTRUCTIBLEDEF = 0x2,
  XANIMPARTS = 0x3,
  XMODEL = 0x4,
  XMODELMESH = 0x5,
  MATERIAL = 0x6,
  COMPUTE_SHADER_SET = 0x7,
  TECHNIQUE_SET = 0x8,
  IMAGE = 0x9,
  SOUND = 0xA,
  SOUND_PATCH = 0xB,
  CLIPMAP = 0xC,
  COMWORLD = 0xD,
  GAMEWORLD = 0xE,
  MAP_ENTS = 0xF,
  GFXWORLD = 0x10,
  LIGHT_DEF = 0x11,
  LENSFLARE_DEF = 0x12,
  UI_MAP = 0x13,
  FONT = 0x14,
  FONTICON = 0x15,
  LOCALIZE_ENTRY = 0x16,
  WEAPON = 0x17,
  WEAPONDEF = 0x18,
  WEAPON_VARIANT = 0x19,
  WEAPON_FULL = 0x1A,
  CGMEDIA = 0x1B,
  PLAYERSOUNDS = 0x1C,
  PLAYERFX = 0x1D,
  SHAREDWEAPONSOUNDS = 0x1E,
  ATTACHMENT = 0x1F,
  ATTACHMENT_UNIQUE = 0x20,
  WEAPON_CAMO = 0x21,
  CUSTOMIZATION_TABLE = 0x22,
  CUSTOMIZATION_TABLE_FE_IMAGES = 0x23,
  CUSTOMIZATION_TABLE_COLOR = 0x24,
  SNDDRIVER_GLOBALS = 0x25,
  FX = 0x26,
  TAGFX = 0x27,
  NEW_LENSFLARE_DEF = 0x28,
  IMPACT_FX = 0x29,
  IMPACT_SOUND = 0x2A,
  PLAYER_CHARACTER = 0x2B,
  AITYPE = 0x2C,
  CHARACTER = 0x2D,
  XMODELALIAS = 0x2E,
  RAWFILE = 0x2F,
  STRINGTABLE = 0x30,
  STRUCTURED_TABLE = 0x31,
  LEADERBOARD = 0x32,
  DDL = 0x33,
  GLASSES = 0x34,
  TEXTURELIST = 0x35,
  SCRIPTPARSETREE = 0x36,
  KEYVALUEPAIRS = 0x37,
  VEHICLEDEF = 0x38,
  ADDON_MAP_ENTS = 0x39,
  TRACER = 0x3A,
  SLUG = 0x3B,
  SURFACEFX_TABLE = 0x3C,
  SURFACESOUNDDEF = 0x3D,
  FOOTSTEP_TABLE = 0x3E,
  ENTITYFXIMPACTS = 0x3F,
  ENTITYSOUNDIMPACTS = 0x40,
  ZBARRIER = 0x41,
  VEHICLEFXDEF = 0x42,
  VEHICLESOUNDDEF = 0x43,
  TYPEINFO = 0x44,
  SCRIPTBUNDLE = 0x45,
  SCRIPTBUNDLELIST = 0x46,
  RUMBLE = 0x47,
  BULLETPENETRATION = 0x48,
  LOCDMGTABLE = 0x49,
  AIMTABLE = 0x4A,
  ANIMSELECTORTABLESET = 0x4B,
  ANIMMAPPINGTABLE = 0x4C,
  ANIMSTATEMACHINE = 0x4D,
  BEHAVIORTREE = 0x4E,
  BEHAVIORSTATEMACHINE = 0x4F,
  TTF = 0x50,
  SANIM = 0x51,
  LIGHT_DESCRIPTION = 0x52,
  SHELLSHOCK = 0x53,
  XCAM = 0x54,
  BG_CACHE = 0x55,
  TEXTURE_COMBO = 0x56,
  FLAMETABLE = 0x57,
  BITFIELD = 0x58,
  ATTACHMENT_COSMETIC_VARIANT = 0x59,
  MAPTABLE = 0x5A,
  MAPTABLE_LOADING_IMAGES = 0x5B,
  MEDAL = 0x5C,
  MEDALTABLE = 0x5D,
  OBJECTIVE = 0x5E,
  OBJECTIVE_LIST = 0x5F,
  UMBRA_TOME = 0x60,
  NAVMESH = 0x61,
  NAVVOLUME = 0x62,
  BINARYHTML = 0x63,
  LASER = 0x64,
  BEAM = 0x65,
  STREAMER_HINT = 0x66,
  COUNT = 0x67,
  STRING = 0x68,
  ASSETLIST = 0x69,
  REPORT = 0x6A,
  DEPEND = 0x68,
  FULL_COUNT = 0x6C,
};

struct AssetLink;
struct AssetLink {
  AssetLink *next;
};

struct XAssetPool {
  void *pool;
  uint32_t itemSize;
  int32_t itemCount;
  qboolean isSingleton;
  int32_t itemAllocCount;
  AssetLink *freeHead;
};

template <typename T> struct TypedXAssetPool {
  T *pool;
  uint32_t itemSize;
  int32_t itemCount;
  qboolean isSingleton;
  int32_t itemAllocCount;
  AssetLink *freeHead;
};

static_assert(sizeof(XAssetPool) == 0x20, "XAssetPool size must be 0x20 bytes");
#pragma pack(push, 1)
// sizeof=x10
struct ScriptStringList {
  int count;
  uint8_t _padding04[4];
  const char **strings;
};
static_assert(sizeof(ScriptStringList) == 0x10,
              "ScriptStringList size must be 0x10 bytes");
#pragma pack(pop)

/*
  All XAssets union members have first field `const char * name;`
  The engine uses this shared field to use shared utility functions to access
  name of the asset Prior to REing and verifying all XAsset union member
  structures, we can start with using the known-shared field only - name.
  Later we can define each of the XAsset structs with `: NamedXAsset` and
  declare the shared util function symbols using type signature `*NamedXAsset`
  to more succintly type what is being passed to the function.
*/
#pragma pack(push, 1)
struct NamedXAsset {
  const char *name;
};
static_assert(sizeof(NamedXAsset) == 0x8, "NamedXAsset size must be 8 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
struct RawFile : NamedXAsset {
  uint64_t len;
  const char *buffer;
};
static_assert(sizeof(RawFile) == 0x18, "RawFile size must be 0x18 bytes");
#pragma pack(pop)

struct MaterialTechniqueSet;
typedef MaterialTechniqueSet *MaterialTechniqueSetPtr;

// TODO
struct XModel;
typedef XModel *XModelPtr;

typedef const char *XString;

struct Set;
typedef Set *SetPtr;

typedef int32_t FxUniqueHandle;
struct TagFxSetHandles {
  FxUniqueHandle handle[8];
  int32_t count;
  SetPtr tagFX;
};

struct FxFloatRange {
  float base;
  float amplitude;
};

typedef int FxElemDefFlags;
typedef int FxElemDefExtraFlags;

struct FxIntRange {
  int base;
  int amplitude;
};

// sizeof=0x10
struct FxSpawnDefLooping {
  int32_t intervalMsec;
  int32_t count;
  FxIntRange spawnCount;
};
static_assert(sizeof(FxSpawnDefLooping) == 0x10,
              "FxSpawnDefLooping size must be 16 bytes");

// sizeof=0x8
struct FxSpawnDefOneShot {
  FxIntRange count;
};
static_assert(sizeof(FxSpawnDefOneShot) == 0x8,
              "FxSpawnDefOneShot size must be 8 bytes");

// sizeof=0x10
union FxSpawnDef {
  FxSpawnDefLooping looping;
  FxSpawnDefOneShot oneShot;
};
static_assert(sizeof(FxSpawnDef) == 0x10, "FxSpawnDef size must be 16 bytes");

typedef uint32_t contents_t;

struct __attribute__((aligned(8))) MaterialInfo {
  const char *name;
  uint32_t namehash;
  uint32_t gameFlags;
  uint8_t layerSortDecal;
  uint8_t pad2;
  uint8_t textureAtlasRowCount;
  uint8_t textureAtlasColumnCount;
  gfx::GfxSortKey drawSurf;
  uint32_t bindlessMaterialSortIndex;
  int surfaceFlags;
  contents_t contents;
};

struct FxLensFlareRootDef;
struct FxSpotLightDef;
struct FxTrailDef;
union FxElemVisuals;
struct FxElemMarkVisuals;
struct FxElemAttractorStateSample;
struct FxElemVisStateSample;
struct FxElemInheritStateSample;
struct FxElemVelStateSample;
struct FxEffectDef;
struct Material;
struct XModel;

typedef Material *MaterialHandle;

/* 4482 */
typedef int32_t FxElemDefFlags;

/* 4481 */
typedef int32_t FxElemDefExtraFlags;

// sizeof=0x7
struct FxElemAtlas {
  uint8_t behavior;
  uint8_t index;
  uint8_t fps;
  uint8_t loopCount;
  uint8_t colIndexBits;
  uint8_t rowIndexBits;
  uint8_t indexRange;
};

static_assert(sizeof(FxElemAtlas) == 0x7, "FxElemAtlas size must be 7 bytes");

typedef uint8_t FxElemType;

typedef Material FxMaterial;

typedef FxMaterial *FxMaterialHandle;
typedef MaterialHandle MaterialDeferredHandle;

typedef XModel *FxModelHandle;

typedef const FxEffectDef *FxEffectDefHandleRaw;

// sizeof=0x8
union FxEffectDefRef {
  const char *name;
  FxEffectDefHandleRaw handle;
};
static_assert(sizeof(FxEffectDefRef) == 0x8,
              "FxEffectDefRef size must be 8 bytes");

// sizeof=0x10
struct FxElemSound {
  const char *sound;
  snd::SndAliasId soundAlias;
  uint8_t _padding0C[4];
};
static_assert(sizeof(FxElemSound) == 0x10, "FxElemSound size must be 16 bytes");

// sizeof=0x20
struct FxLensFlareVisualDef {
  uint32_t uuidData1;
  uint16_t uuidData2;
  uint16_t uuidData3;
  uint8_t uuidData4[8];
  vec3_t sourceDir;
  float sourceSize;
};
static_assert(sizeof(FxLensFlareVisualDef) == 0x20,
              "FxLensFlareVisualDef size must be 32 bytes");

union FxElemVisuals {
  const void *anonymous;
  FxMaterialHandle material;
  FxModelHandle model;
  FxEffectDefRef effectDef;
  FxElemSound soundName;
  gfx::GfxLightDef *lightDef;
  gfx::GfxLightDescription *lightDescription;
  FxLensFlareVisualDef lensFlareVisualDef;
  char editorEffectName[64];
};

// sizeof=0x40
union FxElemDefVisuals {
  FxElemMarkVisuals *markArray;
  FxElemVisuals *array;
  FxElemVisuals instance;
};
static_assert(sizeof(FxElemDefVisuals) == 0x40,
              "FxElemDefVisuals size must be 64 bytes");

// sizeof=0x8
union FxElemExtendedDefPtr {
  FxTrailDef *localTrailDef;
  FxTrailDef *trailDef;
  FxSpotLightDef *localSpotLightDef;
  FxSpotLightDef *spotLightDef;
  gfx::GfxLightDescription *dynamicLightDef;
  const FxLensFlareRootDef *lensFlareDef;
  void *unknownDef; // verbatim name of title in engine
};
static_assert(sizeof(FxElemExtendedDefPtr) == 0x8,
              "FxElemExtendedDefPtr size must be 8 bytes");

union FxElemDefUnion {
  FxIntRange cloudDensityRange;
};

// sizeof=0x260
struct FxElemDef {
  FxElemDefFlags flags;
  FxElemDefExtraFlags extraFlags;
  FxSpawnDef spawn;
  FxFloatRange spawnRange;
  FxFloatRange fadeInRange;
  FxFloatRange fadeOutRange;
  float spawnFrustumCullRadius;
  FxIntRange spawnDelayMsec;
  FxIntRange lifeSpanMsec;
  FxFloatRange spawnOrigin[3];
  FxFloatRange spawnOffsetRadius;
  FxFloatRange spawnOffsetHeight;
  FxFloatRange spawnAngles[3];
  FxFloatRange angularVelocity[3];
  FxFloatRange initialRotation;
  uint32_t rotationAxis;
  FxFloatRange gravity;
  FxFloatRange reflectionFactor;
  FxElemAtlas atlas;
  uint8_t _paddingBF[1];
  float windInfluence;
  FxElemType elemType;
  uint8_t visualCount;
  uint8_t velIntervalCount;
  uint8_t inhIntervalCount;
  uint8_t attIntervalCount;
  uint8_t visStateIntervalCount;
  uint8_t _paddingCA[6];
  const FxElemVelStateSample *velSamples;
  const FxElemInheritStateSample *inheritSamples;
  const FxElemVisStateSample *visSamples;
  const FxElemAttractorStateSample *attSamples;
  FxElemDefVisuals visuals;
  FxElemDefVisuals computeVisuals;
  vec3_t collMins;
  vec3_t collMaxs;
  vec3_t attractorLocalPosition;
  uint8_t _padding194[4];
  FxEffectDefRef effectOnImpact;
  FxEffectDefRef effectOnDeath;
  FxEffectDefRef effectEmitted;
  FxFloatRange emitDist;
  FxFloatRange emitDistVariance;
  FxFloatRange emitDensity;
  float emitSizeForDensity;
  uint8_t _padding1CC[4];
  FxEffectDefRef effectAttached;
  FxFloatRange attachedDensity;
  float attachedSizeForDensity;
  uint8_t _padding1E4[4];
  FxElemExtendedDefPtr extended;
  uint8_t displacement;
  uint8_t lightingFrac;
  uint8_t depthMultiplier;
  uint8_t _padding1F3[1];
  uint16_t alphaFadeTimeMsec;
  uint16_t maxWindStrength;
  uint16_t spawnIntervalAtMaxWind;
  uint16_t lifespanAtMaxWind;
  FxElemDefUnion u;
  uint8_t _padding204[4];
  FxElemSound spawnSound;
  FxElemSound followSound;
  vec2_t billboardPivot;
  vec_t alphaDissolve;
  vec_t zFeather;
  uint32_t falloffBeginAngle;
  uint32_t falloffEndAngle;
  float hdrScale;
  float levelsInputMin;
  float levelsInputMax;
  float levelsOutputMin;
  float levelsOutputMax;
  float desaturationAmount;
  bool useOldHDRScale;
  uint8_t _padding259[1];
  uint16_t libDefIndex;
  uint16_t libVisualIndex;
  uint8_t _padding25E[2];
};
static_assert(sizeof(FxElemDef) == 0x260, "FxElemDef size must be 608 bytes");

typedef int FxEffectDefFlags;
typedef const FxEffectDef *FxEffectDefHandle;
typedef uint8_t FxNormalsShape;

// sizeof=0x98
struct FxEffectDef {
  const char *name;
  FxEffectDefFlags flags;
  uint8_t efPriority;
  uint8_t _paddingB[1];
  int16_t elemDefCountLooping;
  int16_t elemDefCountOneShot;
  int16_t elemDefCountEmission;
  uint8_t _padding12[2];
  int32_t totalSize;
  int32_t msecLoopingLife;
  int32_t msecNonLoopingLife;
  const FxElemDef *elemDefs;
  vec3_t boundingBoxDim;
  vec3_t boundingBoxCentre;
  float occlusionQueryDepthBias;
  int32_t occlusionQueryFadeIn;
  int32_t occlusionQueryFadeOut;
  FxFloatRange occlusionQueryScaleRange;
  uint8_t _padding54[4];
  FxElemSound loopedSound;
  FxNormalsShape normalsShape;
  uint8_t _padding69[3];
  vec3_t normalsOffset;
  vec3_t normalsNormal;
  int32_t gpuMsecLife;
  uint16_t libDefIndex;
  uint8_t _padding8A[6];
};
static_assert(sizeof(FxEffectDef) == 0x98,
              "FxEffectDef size must be 144 bytes");

struct FxElemDef;

// sizeof=0x30
struct EntityImpactPositionsFx {
  FxEffectDefHandle entityImpactPositions[6];
};
static_assert(sizeof(EntityImpactPositionsFx) == 0x30,
              "EntityImpactPositionsFx size must be 48 bytes");

// sizeof=0x218
struct EntityFxImpacts {
  const char *name;
  EntityImpactPositionsFx entityImpacts[11];
};
static_assert(sizeof(EntityFxImpacts) == 0x218,
              "EntityFxImpacts size must be 536 bytes");

// TODO
struct SurfaceFXTableDef;

typedef SurfaceFXTableDef *SurfaceFXTableDefPtr;

typedef EntityFxImpacts *EntityFxImpactsPtr;

struct FxImpactTable {
  const char *name;
  SurfaceFXTableDefPtr surfaceFX;
  EntityFxImpactsPtr entityFXImpacts;
};

struct FxElemMarkVisuals {
  FxMaterialHandle materials[3];
};

struct FxElemDef;

typedef uint8_t FxNormalsShape;

struct Objective;
typedef Objective *ObjectivePtr;

struct FxImpactTable;
typedef FxImpactTable *FxImpactTablePtr;

struct BeamDef;
typedef BeamDef *BeamDefPtr;

struct FlameTable;
typedef FlameTable *FlameTablePtr;

struct SoundsImpactTable;
typedef SoundsImpactTable *SoundsImpactTablePtr;

struct SharedWeaponSounds;
typedef SharedWeaponSounds *SharedWeaponSoundsPtr;

struct TaxFxSet;
typedef TaxFxSet *TaxFxSetPtr;

struct TracerDef;
typedef TracerDef *TracerDefPtr;

struct LaserDef;
typedef LaserDef *LaserDefPtr;

struct Material;
typedef Material *MaterialHandle;

enum class GfxPixelFormat : int32_t {
  GFX_PF_INVALID = 0x0,
  GFX_PF_UNKNOWN = 0x0,
  GFX_PF_D24S8 = 0x1,
  GFX_PF_D24FS8 = 0x2,
  GFX_PF_A8 = 0x3,
  GFX_PF_R8_UN = 0x4,
  GFX_PF_R8_UI = 0x5,
  GFX_PF_D16 = 0x6,
  GFX_PF_R16_UN = 0x7,
  GFX_PF_R16_UI = 0x8,
  GFX_PF_R16F = 0x9,
  GFX_PF_R8G8_UN = 0xA,
  GFX_PF_R8G8_SNORM = 0xB,
  GFX_PF_B5G6R5 = 0xC,
  GFX_PF_B4G4R4A4 = 0xD,
  GFX_PF_D32F = 0xE,
  GFX_PF_R32F = 0xF,
  GFX_PF_R16G16_UN = 0x10,
  GFX_PF_R16G16_SNORM = 0x11,
  GFX_PF_R16G16F = 0x12,
  GFX_PF_R9G9B9E5 = 0x13,
  GFX_PF_R8G8B8A8 = 0x14,
  GFX_PF_R8G8B8A8_SNORM = 0x15,
  GFX_PF_B8G8R8A8 = 0x16,
  GFX_PF_R8G8B8A8_SRGB = 0x17,
  GFX_PF_R10G10B10A2 = 0x18,
  GFX_PF_B10G10R10A2 = 0x19,
  GFX_PF_R11G11B10F = 0x1A,
  GFX_PF_R32G32F = 0x1B,
  GFX_PF_R16G16B16A16_UN = 0x1C,
  GFX_PF_R16G16B16A16F = 0x1D,
  GFX_PF_R32G32B32A32F = 0x1E,
  GFX_PF_BC1 = 0x1F,
  GFX_PF_BC1_SRGB = 0x20,
  GFX_PF_BC2 = 0x21,
  GFX_PF_BC2_SRGB = 0x22,
  GFX_PF_BC3 = 0x23,
  GFX_PF_BC3_SRGB = 0x24,
  GFX_PF_BC4 = 0x25,
  GFX_PF_BC5 = 0x26,
  GFX_PF_BC6_UH = 0x27,
  GFX_PF_BC6_SH = 0x28,
  GFX_PF_BC7 = 0x29,
  GFX_PF_BC7_SRGB = 0x2A,
  GFX_PF_R32G32B32A32_UN = 0x2B,
  GFX_PF_R32_UI = 0x2C,
  GFX_PF_R32G32_UI = 0x2D,
};

struct XPakEntryInfo {
  uint64_t key;
  int64_t offset;
  uint64_t size;
  uint64_t xpakIndex : 5;
  uint64_t compressed : 1;
  uint64_t valid : 1;
  uint64_t adjacentLeftType : 3;
  uint64_t adjacentRightType : 3;
  uint64_t adjacentLeft : 18;
  uint64_t adjacentRight : 18;
  uint64_t padding : 15;
};

struct GfxStreamedPartInfo {
  XPakEntryInfo xpakEntry;
  uint32_t levelCountAndSize;
  uint16_t width;
  uint16_t height;
};

enum class MapType : uint8_t {
  MAPTYPE_NONE = 0x0,
  MAPTYPE_2D = 0x1,
  MAPTYPE_2D_ARRAY = 0x2,
  MAPTYPE_3D = 0x3,
  MAPTYPE_CUBE = 0x4,
  MAPTYPE_CUBE_ARRAY = 0x5,
  MAPTYPE_COUNT = 0x6,
};

struct ScriptBundle;
typedef ScriptBundle *ScriptBundlePtr;

struct XModel;
typedef XModel *XModelPtr;

struct PlayerFXTable;
typedef PlayerFXTable *PlayerFXTablePtr;

struct XAnimParts;
typedef XAnimParts *XAnimPartsPtr;

struct XCam;
typedef XCam *XCamPtr;

struct PlayerSoundsTable;
typedef PlayerSoundsTable *PlayerSoundsTablePtr;

struct FootstepTableDef;
typedef FootstepTableDef *FootstepTableDefPtr;

struct ScriptBundleKVP;
typedef ScriptBundleKVP *ScriptBundleKVPPtr;

struct TagFxSet;
typedef TagFxSet *TagFxSetPtr;

struct XAnimTree;

union XAssetHeader {
  NamedXAsset *named;
  // PhysPreset *physPreset;
  // PhysConstraints *physConstraints;
  // DestructibleDef *destructibleDef;
  // XAnimParts *parts;
  // XModel *model;
  // XModelMesh *modelMesh;
  // Material *material;
  // MaterialComputeShaderSet *computeShaderSet;
  MaterialTechniqueSet *techniqueSet;
  gfx::GfxImage *image;
  snd::SndBank *sound;
  snd::SndPatch *soundPatch;
  // clipMap_t *clipMap;
  // ComWorld *comWorld;
  // GameWorld *gameWorld;
  // MapEnts *mapEnts;
  // GfxWorld *gfxWorld;
  // GfxLightDef *lightDef;
  // GfxLensFlareDef *lensFlareDef;
  // Font *font;
  // FontIcon *fontIcon;
  // LocalizeEntry *localize;
  // WeaponVariantDef *weapon;
  // WeaponAttachment *attachment;
  // WeaponAttachmentUnique *attachmentUnique;
  // WeaponCamo *weaponCamo;
  // CustomizationTable *customizationTable;
  // CustomizationColorInfo *customizationColorInfo;
  snd::SndDriverGlobals *sndDriverGlobals;
  // FxEffectDefHandleRaw fx;
  // TagFxSet *tagFX;
  // FxLensFlareDefPtr newLensFlareDef;
  // FxImpactTable *impactFx;
  // SoundsImpactTable *impactSounds;
  // CgMediaTable *cgMedia;
  // PlayerSoundsTable *playerSounds;
  // PlayerFXTable *playerFX;
  // SharedWeaponSounds *sharedWeaponSounds;

  // E.g. compiled lua scripts, game CFGs,
  // some DDL files, vision and anim scripts
  RawFile *rawfile;
  // StringTable *stringTable;
  // StructuredTable *structuredTable;
  // LeaderboardDef *leaderboardDef;
  // DDLRoot *ddlRoot;
  // Glasses *glasses;
  // TextureList *textureList;
  // ScriptParseTree *scriptParseTree;
  // KeyValuePairs *keyValuePairs;
  // VehicleDef *vehicleDef;
  // AddonMapEnts *addonMapEnts;
  // TracerDef *tracerDef;
  // Qdb *qdb;
  // Slug *slug;
  // SurfaceFXTableDef *surfaceFXTableDef;
  // SurfaceSoundDef *surfaceSoundDef;
  // FootstepTableDef *footstepTableDef;
  // EntitySoundImpacts *entitySoundImpacts;
  // EntityFxImpacts *entityFxImpacts;
  // ZBarrierDef *zbarrierDef;
  // VehicleFxDef *vehicleFxDef;
  // VehicleSoundDef *vehicleSoundDef;
  // ArchiveTypeInfoArray *typeInfo;
  // ScriptBundle *scriptBundle;
  // ScriptBundleList *scriptBundleList;
  // RumbleInfo *rumble;
  // BulletPenetrationTable *bulletPenetration;
  // LocDmgTable *locDmgTable;
  // AimTable *aimTable;
  // XModelAlias *xModelAlias;
  // Character *character;
  // AIType *aiType;
  // PlayerCharacter *player_character;
  // AnimSelectorTableSet *animSelectorTableSet;
  // AnimMappingTable *animMappingTable;
  // AnimStateMachine *animStateMachine;
  // BehaviorTree *behaviorTree;
  // BehaviorStateMachine *behaviorStateMachine;
  // TTFDef *ttfDef;
  // GfxSiegeAnim *sanim;
  // GfxLightDescription *lightDescription;
  // ShellshockParams *shellshock;
  // XCam *xcam;
  // BGCacheInfo *bgCache;
  // TextureCombo *textureCombo;
  // FlameTable *flameTable;
  // Bitfield *bitfield;
  // AttachmentCosmeticVariant *attachmentCosmeticVariant;
  // MapTable *mapTable;
  // Medal *medal;
  // MedalTable *medalTable;
  // Objective *objective;
  // ObjectiveList *objectiveList;
  // NavMeshData *navMesh;
  // NavVolumeData *navVolume;
  // BinaryHTML *binaryHTML;
  // LaserDef *laserDef;
  // BeamDef *beamDef;
  // StreamerHint *streamerHint;
  void *data;
};
static_assert(sizeof(XAssetHeader) == 0x8,
              "XAssetHeader size must be 0x8 bytes");

// sizeof=0x10
#pragma pack(push, 1)
struct XAsset {
  XAssetType type;
  uint8_t _padding04[4];
  XAssetHeader header;
};
static_assert(sizeof(XAsset) == 0x10, "XAsset size must be 0x10 bytes");
#pragma pack(pop)

using XAssetEnum = void(XAssetHeader, void *);

#pragma pack(push, 1)
// sizeof=0x20
struct XAssetList {
  ScriptStringList stringList;
  int assetCount;
  uint8_t _padding14[4];
  XAsset *assets;
};
static_assert(sizeof(XAssetList) == 0x20, "XAssetList size must be 0x20 bytes");
#pragma pack(pop)

#pragma pack(push, 1)
// sizeof=0x20
struct XAssetEntry {
  XAsset asset;
  uint8_t zoneIndex;
  bool inuse;
  uint8_t _padding12[2];
  uint32_t nextHash;
  uint32_t nextType;
  uint32_t nextOverride;
};
static_assert(sizeof(XAssetEntry) == 0x20, "sizeof(XAssetEntry) must be 0x20");
typedef XAssetEntry *XAssetEntryPtr;
#pragma pack(pop)

union XAssetEntryPoolEntry;
union XAssetEntryPoolEntry {
  XAssetEntry entry;
  XAssetEntryPoolEntry *next;
};
static_assert(sizeof(XAssetEntryPoolEntry) == 0x20,
              "sizeof(XAssetEntryPoolEntry) must be 0x20");
static const std::size_t XASSET_ENTRY_POOL_LENGTH = 156671;

#pragma pack(push, 1)
struct XAssetEntryPool {
  XAssetEntryPoolEntry pool[XASSET_ENTRY_POOL_LENGTH];
};

#pragma pack(pop)

#pragma pack(push, 1)

struct TypedXAssetPools {
  XAssetPool physpreset;
  XAssetPool physconstraints;
  XAssetPool destructibledef;
  XAssetPool xanimparts;
  XAssetPool xmodel;
  XAssetPool xmodelmesh;
  XAssetPool material;
  XAssetPool compute_shader_set;
  XAssetPool technique_set;
  XAssetPool image;
  TypedXAssetPool<snd::SndBank> sound;
  TypedXAssetPool<snd::SndPatch> sound_patch;
  XAssetPool clipmap;
  XAssetPool comworld;
  XAssetPool gameworld;
  XAssetPool map_ents;
  XAssetPool gfxworld;
  XAssetPool light_def;
  XAssetPool lensflare_def;
  XAssetPool ui_map;
  XAssetPool font;
  XAssetPool fonticon;
  XAssetPool localize_entry;
  XAssetPool weapon;
  XAssetPool weapondef;
  XAssetPool weapon_variant;
  XAssetPool weapon_full;
  XAssetPool cgmedia;
  XAssetPool playersounds;
  XAssetPool playerfx;
  XAssetPool sharedweaponsounds;
  XAssetPool attachment;
  XAssetPool attachment_unique;
  XAssetPool weapon_camo;
  XAssetPool customization_table;
  XAssetPool customization_table_fe_images;
  XAssetPool customization_table_color;
  TypedXAssetPool<snd::SndDriverGlobals> snddriver_globals;
  XAssetPool fx;
  XAssetPool tagfx;
  XAssetPool new_lensflare_def;
  XAssetPool impact_fx;
  XAssetPool impact_sound;
  XAssetPool player_character;
  XAssetPool aitype;
  XAssetPool character;
  XAssetPool xmodelalias;
  TypedXAssetPool<RawFile> rawfile;
  XAssetPool stringtable;
  XAssetPool structured_table;
  XAssetPool leaderboard;
  XAssetPool ddl;
  XAssetPool glasses;
  XAssetPool texturelist;
  TypedXAssetPool<RawFile> scriptparsetree;
  XAssetPool keyvaluepairs;
  XAssetPool vehicledef;
  XAssetPool addon_map_ents;
  XAssetPool tracer;
  XAssetPool slug;
  XAssetPool surfacefx_table;
  XAssetPool surfacesounddef;
  XAssetPool footstep_table;
  XAssetPool entityfximpacts;
  XAssetPool entitysoundimpacts;
  XAssetPool zbarrier;
  XAssetPool vehiclefxdef;
  XAssetPool vehiclesounddef;
  XAssetPool typeinfo;
  XAssetPool scriptbundle;
  XAssetPool scriptbundlelist;
  XAssetPool rumble;
  XAssetPool bulletpenetration;
  XAssetPool locdmgtable;
  XAssetPool aimtable;
  XAssetPool animselectortableset;
  XAssetPool animmappingtable;
  XAssetPool animstatemachine;
  XAssetPool behaviortree;
  XAssetPool behaviorstatemachine;
  XAssetPool ttf;
  XAssetPool sanim;
  XAssetPool light_description;
  XAssetPool shellshock;
  XAssetPool xcam;
  XAssetPool bg_cache;
  XAssetPool texture_combo;
  XAssetPool flametable;
  XAssetPool bitfield;
  XAssetPool attachment_cosmetic_variant;
  XAssetPool maptable;
  XAssetPool maptable_loading_images;
  XAssetPool medal;
  XAssetPool medaltable;
  XAssetPool objective;
  XAssetPool objective_list;
  XAssetPool umbra_tome;
  XAssetPool navmesh;
  XAssetPool navvolume;
  XAssetPool binaryhtml;
  XAssetPool laser;
  XAssetPool beam;
  XAssetPool streamer_hint;
};

#pragma pack(pop)

#pragma pack(push, 1)

union XAssetPools {
  XAssetPool pools[static_cast<int>(XAssetType::COUNT)];
  TypedXAssetPools typed;
};
static_assert(sizeof(XAssetPools) ==
                  sizeof(XAssetPool) * static_cast<int>(XAssetType::COUNT),
              "sizeof(XAssetPools) must be sizeof(XAssetPool) * COUNT");
static_assert(sizeof(XAssetPools) == sizeof(TypedXAssetPools),
              "sizeof(XAssetPools) must be equal to sizeof(TypedXAssetPools)");
#pragma pack(pop)

} // namespace xasset
} // namespace db
} // namespace game
#endif