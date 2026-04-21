#pragma once

#include <cstdint>
#include "../quake.hpp"

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
} // namespace snd
namespace db {
namespace xasset {
/*
 For future reference, some known-correct asset struct sizes as per
 `DB_GetXAssetTypeSize`:
  - ASSET_TYPE_PHYSPRESET: 120 or 0x78
  - ASSET_TYPE_XMODELMESH: 120 or 0x78
  - ASSET_TYPE_PHYSCONSTRAINTS: 1680 or 0x690
  - ASSET_TYPE_DESTRUCTIBLEDEF: 48 or 0x30
  - ASSET_TYPE_XANIMPARTS: 248 or 0xF8
  - ASSET_TYPE_XMODEL: 392 or 0x188
  - ASSET_TYPE_MATERIAL: 672 or 0x2A0
  - ASSET_TYPE_COMPUTE_SHADER_SET: 24 or 0x18
  - ASSET_TYPE_SOUND_PATCH: 24 or 0x18
  - ASSET_TYPE_TECHNIQUE_SET: 112 or 0x70
  - ASSET_TYPE_IMAGE: 264 or 0x108
  - ASSET_TYPE_SOUND: 139440 or 0x220B0
  - ASSET_TYPE_CLIPMAP: 720 or 0x2D0
  - ASSET_TYPE_COMWORLD: 136 or 0x88
  - ASSET_TYPE_GAMEWORLD: 80 or 0x50
  - ASSET_TYPE_MAP_ENTS: 72 or 0x48
  - ASSET_TYPE_GFXWORLD: 8256 or 0x2040
  - ASSET_TYPE_LIGHT_DEF: 40 or 0x28
  - ASSET_TYPE_LENSFLARE_DEF: 536 or 0x218
*/

enum class XAssetType : int32_t {
  ASSET_TYPE_PHYSPRESET = 0x0,
  ASSET_TYPE_PHYSCONSTRAINTS = 0x1,
  ASSET_TYPE_DESTRUCTIBLEDEF = 0x2,
  ASSET_TYPE_XANIMPARTS = 0x3,
  ASSET_TYPE_XMODEL = 0x4,
  ASSET_TYPE_XMODELMESH = 0x5,
  ASSET_TYPE_MATERIAL = 0x6,
  ASSET_TYPE_COMPUTE_SHADER_SET = 0x7,
  ASSET_TYPE_TECHNIQUE_SET = 0x8,
  ASSET_TYPE_IMAGE = 0x9,
  ASSET_TYPE_SOUND = 0xA,
  ASSET_TYPE_SOUND_PATCH = 0xB,
  ASSET_TYPE_CLIPMAP = 0xC,
  ASSET_TYPE_COMWORLD = 0xD,
  ASSET_TYPE_GAMEWORLD = 0xE,
  ASSET_TYPE_MAP_ENTS = 0xF,
  ASSET_TYPE_GFXWORLD = 0x10,
  ASSET_TYPE_LIGHT_DEF = 0x11,
  ASSET_TYPE_LENSFLARE_DEF = 0x12,
  ASSET_TYPE_UI_MAP = 0x13,
  ASSET_TYPE_FONT = 0x14,
  ASSET_TYPE_FONTICON = 0x15,
  ASSET_TYPE_LOCALIZE_ENTRY = 0x16,
  ASSET_TYPE_WEAPON = 0x17,
  ASSET_TYPE_WEAPONDEF = 0x18,
  ASSET_TYPE_WEAPON_VARIANT = 0x19,
  ASSET_TYPE_WEAPON_FULL = 0x1A,
  ASSET_TYPE_CGMEDIA = 0x1B,
  ASSET_TYPE_PLAYERSOUNDS = 0x1C,
  ASSET_TYPE_PLAYERFX = 0x1D,
  ASSET_TYPE_SHAREDWEAPONSOUNDS = 0x1E,
  ASSET_TYPE_ATTACHMENT = 0x1F,
  ASSET_TYPE_ATTACHMENT_UNIQUE = 0x20,
  ASSET_TYPE_WEAPON_CAMO = 0x21,
  ASSET_TYPE_CUSTOMIZATION_TABLE = 0x22,
  ASSET_TYPE_CUSTOMIZATION_TABLE_FE_IMAGES = 0x23,
  ASSET_TYPE_CUSTOMIZATION_TABLE_COLOR = 0x24,
  ASSET_TYPE_SNDDRIVER_GLOBALS = 0x25,
  ASSET_TYPE_FX = 0x26,
  ASSET_TYPE_TAGFX = 0x27,
  ASSET_TYPE_NEW_LENSFLARE_DEF = 0x28,
  ASSET_TYPE_IMPACT_FX = 0x29,
  ASSET_TYPE_IMPACT_SOUND = 0x2A,
  ASSET_TYPE_PLAYER_CHARACTER = 0x2B,
  ASSET_TYPE_AITYPE = 0x2C,
  ASSET_TYPE_CHARACTER = 0x2D,
  ASSET_TYPE_XMODELALIAS = 0x2E,
  ASSET_TYPE_RAWFILE = 0x2F,
  ASSET_TYPE_STRINGTABLE = 0x30,
  ASSET_TYPE_STRUCTURED_TABLE = 0x31,
  ASSET_TYPE_LEADERBOARD = 0x32,
  ASSET_TYPE_DDL = 0x33,
  ASSET_TYPE_GLASSES = 0x34,
  ASSET_TYPE_TEXTURELIST = 0x35,
  ASSET_TYPE_SCRIPTPARSETREE = 0x36,
  ASSET_TYPE_KEYVALUEPAIRS = 0x37,
  ASSET_TYPE_VEHICLEDEF = 0x38,
  ASSET_TYPE_ADDON_MAP_ENTS = 0x39,
  ASSET_TYPE_TRACER = 0x3A,
  ASSET_TYPE_SLUG = 0x3B,
  ASSET_TYPE_SURFACEFX_TABLE = 0x3C,
  ASSET_TYPE_SURFACESOUNDDEF = 0x3D,
  ASSET_TYPE_FOOTSTEP_TABLE = 0x3E,
  ASSET_TYPE_ENTITYFXIMPACTS = 0x3F,
  ASSET_TYPE_ENTITYSOUNDIMPACTS = 0x40,
  ASSET_TYPE_ZBARRIER = 0x41,
  ASSET_TYPE_VEHICLEFXDEF = 0x42,
  ASSET_TYPE_VEHICLESOUNDDEF = 0x43,
  ASSET_TYPE_TYPEINFO = 0x44,
  ASSET_TYPE_SCRIPTBUNDLE = 0x45,
  ASSET_TYPE_SCRIPTBUNDLELIST = 0x46,
  ASSET_TYPE_RUMBLE = 0x47,
  ASSET_TYPE_BULLETPENETRATION = 0x48,
  ASSET_TYPE_LOCDMGTABLE = 0x49,
  ASSET_TYPE_AIMTABLE = 0x4A,
  ASSET_TYPE_ANIMSELECTORTABLESET = 0x4B,
  ASSET_TYPE_ANIMMAPPINGTABLE = 0x4C,
  ASSET_TYPE_ANIMSTATEMACHINE = 0x4D,
  ASSET_TYPE_BEHAVIORTREE = 0x4E,
  ASSET_TYPE_BEHAVIORSTATEMACHINE = 0x4F,
  ASSET_TYPE_TTF = 0x50,
  ASSET_TYPE_SANIM = 0x51,
  ASSET_TYPE_LIGHT_DESCRIPTION = 0x52,
  ASSET_TYPE_SHELLSHOCK = 0x53,
  ASSET_TYPE_XCAM = 0x54,
  ASSET_TYPE_BG_CACHE = 0x55,
  ASSET_TYPE_TEXTURE_COMBO = 0x56,
  ASSET_TYPE_FLAMETABLE = 0x57,
  ASSET_TYPE_BITFIELD = 0x58,
  ASSET_TYPE_ATTACHMENT_COSMETIC_VARIANT = 0x59,
  ASSET_TYPE_MAPTABLE = 0x5A,
  ASSET_TYPE_MAPTABLE_LOADING_IMAGES = 0x5B,
  ASSET_TYPE_MEDAL = 0x5C,
  ASSET_TYPE_MEDALTABLE = 0x5D,
  ASSET_TYPE_OBJECTIVE = 0x5E,
  ASSET_TYPE_OBJECTIVE_LIST = 0x5F,
  ASSET_TYPE_UMBRA_TOME = 0x60,
  ASSET_TYPE_NAVMESH = 0x61,
  ASSET_TYPE_NAVVOLUME = 0x62,
  ASSET_TYPE_BINARYHTML = 0x63,
  ASSET_TYPE_LASER = 0x64,
  ASSET_TYPE_BEAM = 0x65,
  ASSET_TYPE_STREAMER_HINT = 0x66,
  ASSET_TYPE_COUNT = 0x67,
  ASSET_TYPE_STRING = 0x68,
  ASSET_TYPE_ASSETLIST = 0x69,
  ASSET_TYPE_REPORT = 0x6A,
  ASSET_TYPE_DEPEND = 0x68,
  ASSET_TYPE_FULL_COUNT = 0x6C,
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
  int32_t len;
  uint8_t _padding0C[4];
  const char *buffer;
};
static_assert(sizeof(RawFile) == 0x18, "RawFile size must be 0x18 bytes");
#pragma pack(pop)

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
  // MaterialTechniqueSet *techniqueSet;
  // GfxImage *image;
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
  RawFile *luaFile;
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
  XAssetPool pools[static_cast<int>(XAssetType::ASSET_TYPE_COUNT)];
  TypedXAssetPools typed;
};
static_assert(
    sizeof(XAssetPools) ==
        sizeof(XAssetPool) * static_cast<int>(XAssetType::ASSET_TYPE_COUNT),
    "sizeof(XAssetPools) must be sizeof(XAssetPool) * ASSET_TYPE_COUNT");
static_assert(sizeof(XAssetPools) == sizeof(TypedXAssetPools),
              "sizeof(XAssetPools) must be equal to sizeof(TypedXAssetPools)");
#pragma pack(pop)

} // namespace xasset
} // namespace db
} // namespace game