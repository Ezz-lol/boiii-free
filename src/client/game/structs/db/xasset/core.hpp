#pragma once

#include <cstdint>
#include <format>
#include <game/ptr.hpp>
#include <game/structs/gfx/gfx.hpp>
#include <string>

namespace game {

namespace weapon {
struct WeaponVariantDef;
}

namespace vehicle {
struct VehicleDef;
struct VehicleFxDef;
struct VehicleSoundDef;
} // namespace vehicle

namespace ddl {
struct DDLDef;
struct StringTable;
} // namespace ddl

namespace bg {
namespace cache {
struct BGCacheInfo;
}
} // namespace bg

namespace scr {
typedef uint32_t ScrString_t;
struct ScriptParseTree;
} // namespace scr

namespace snd {
struct SndBank;
struct SndPatch;
struct SndDriverGlobals;

typedef uint32_t SndAliasId;

} // namespace snd

namespace cm {
struct clipMap_t;
}

namespace db {
namespace xasset {

namespace ttf {
struct TTFDef;
}

namespace destructible {
struct DestructibleDef;
typedef DestructibleDef *DestructibleDefPtr;
} // namespace destructible

namespace xmodel {
struct XModel;
typedef XModel *XModelPtr;

struct XModelMesh;
typedef XModelMesh *XModelMeshPtr;

struct XModelAlias;
typedef XModelAlias *XModelAliasPtr;

} // namespace xmodel

namespace font {
struct FontIcon;
struct Font_s;
typedef Font_s Font;
} // namespace font

namespace phys {
struct PhysPreset;
struct PhysGeomList;
struct PhysConstraints;
} // namespace phys

namespace world {
struct GameWorld;
struct GfxWorld;
struct ComWorld;
} // namespace world

struct RumbleInfo;
typedef RumbleInfo *RumbleInfoPtr;

struct ShellshockParams;
typedef ShellshockParams *ShellshockParamsPtr;

struct StreamerHint;
typedef StreamerHint *StreamerHintPtr;

namespace maptable {
struct MapTable;
}

typedef array<int32_t, 12> XPartBits;

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
  // Probably `WeaponDef`
  WEAPONDEF = 0x18,
  // Probably `WeaponVariantDef`, but unsure, as `WeaponVariantDef` is
  // verified to be the type used for `Weapon` `XAsset` types.
  WEAPON_VARIANT = 0x19,
  // Probably `WeaponFullDef`
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
IMPL_ENUM_OPERATORS(XAssetType);

inline constexpr const char *serialize(const XAssetType type) {
  switch (type) {
  case XAssetType::PHYSPRESET:
    return "ASSET_PHYSPRESET";
  case XAssetType::PHYSCONSTRAINTS:
    return "ASSET_PHYSCONSTRAINTS";
  case XAssetType::DESTRUCTIBLEDEF:
    return "ASSET_DESTRUCTIBLEDEF";
  case XAssetType::XANIMPARTS:
    return "ASSET_XANIMPARTS";
  case XAssetType::XMODEL:
    return "ASSET_XMODEL";
  case XAssetType::XMODELMESH:
    return "ASSET_XMODELMESH";
  case XAssetType::MATERIAL:
    return "ASSET_MATERIAL";
  case XAssetType::COMPUTE_SHADER_SET:
    return "ASSET_COMPUTE_SHADER_SET";
  case XAssetType::TECHNIQUE_SET:
    return "ASSET_TECHNIQUE_SET";
  case XAssetType::IMAGE:
    return "ASSET_IMAGE";
  case XAssetType::SOUND:
    return "ASSET_SOUND";
  case XAssetType::SOUND_PATCH:
    return "ASSET_SOUND_PATCH";
  case XAssetType::CLIPMAP:
    return "ASSET_CLIPMAP";
  case XAssetType::COMWORLD:
    return "ASSET_COMWORLD";
  case XAssetType::GAMEWORLD:
    return "ASSET_GAMEWORLD";
  case XAssetType::MAP_ENTS:
    return "ASSET_MAP_ENTS";
  case XAssetType::GFXWORLD:
    return "ASSET_GFXWORLD";
  case XAssetType::LIGHT_DEF:
    return "ASSET_LIGHT_DEF";
  case XAssetType::LENSFLARE_DEF:
    return "ASSET_LENSFLARE_DEF";
  case XAssetType::UI_MAP:
    return "ASSET_UI_MAP";
  case XAssetType::FONT:
    return "ASSET_FONT";
  case XAssetType::FONTICON:
    return "ASSET_FONTICON";
  case XAssetType::LOCALIZE_ENTRY:
    return "ASSET_LOCALIZE_ENTRY";
  case XAssetType::WEAPON:
    return "ASSET_WEAPON";
  case XAssetType::WEAPONDEF:
    return "ASSET_WEAPONDEF";
  case XAssetType::WEAPON_VARIANT:
    return "ASSET_WEAPON_VARIANT";
  case XAssetType::WEAPON_FULL:
    return "ASSET_WEAPON_FULL";
  case XAssetType::CGMEDIA:
    return "ASSET_CGMEDIA";
  case XAssetType::PLAYERSOUNDS:
    return "ASSET_PLAYERSOUNDS";
  case XAssetType::PLAYERFX:
    return "ASSET_PLAYERFX";
  case XAssetType::SHAREDWEAPONSOUNDS:
    return "ASSET_SHAREDWEAPONSOUNDS";
  case XAssetType::ATTACHMENT:
    return "ASSET_ATTACHMENT";
  case XAssetType::ATTACHMENT_UNIQUE:
    return "ASSET_ATTACHMENT_UNIQUE";
  case XAssetType::WEAPON_CAMO:
    return "ASSET_WEAPON_CAMO";
  case XAssetType::CUSTOMIZATION_TABLE:
    return "ASSET_CUSTOMIZATION_TABLE";
  case XAssetType::CUSTOMIZATION_TABLE_FE_IMAGES:
    return "ASSET_CUSTOMIZATION_TABLE_FE_IMAGES";
  case XAssetType::CUSTOMIZATION_TABLE_COLOR:
    return "ASSET_CUSTOMIZATION_TABLE_COLOR";
  case XAssetType::SNDDRIVER_GLOBALS:
    return "ASSET_SNDDRIVER_GLOBALS";
  case XAssetType::FX:
    return "ASSET_FX";
  case XAssetType::TAGFX:
    return "ASSET_TAGFX";
  case XAssetType::NEW_LENSFLARE_DEF:
    return "ASSET_NEW_LENSFLARE_DEF";
  case XAssetType::IMPACT_FX:
    return "ASSET_IMPACT_FX";
  case XAssetType::IMPACT_SOUND:
    return "ASSET_IMPACT_SOUND";
  case XAssetType::PLAYER_CHARACTER:
    return "ASSET_PLAYER_CHARACTER";
  case XAssetType::AITYPE:
    return "ASSET_AITYPE";
  case XAssetType::CHARACTER:
    return "ASSET_CHARACTER";
  case XAssetType::XMODELALIAS:
    return "ASSET_XMODELALIAS";
  case XAssetType::RAWFILE:
    return "ASSET_RAWFILE";
  case XAssetType::STRINGTABLE:
    return "ASSET_STRINGTABLE";
  case XAssetType::STRUCTURED_TABLE:
    return "ASSET_STRUCTURED_TABLE";
  case XAssetType::LEADERBOARD:
    return "ASSET_LEADERBOARD";
  case XAssetType::DDL:
    return "ASSET_DDL";
  case XAssetType::GLASSES:
    return "ASSET_GLASSES";
  case XAssetType::TEXTURELIST:
    return "ASSET_TEXTURELIST";
  case XAssetType::SCRIPTPARSETREE:
    return "ASSET_SCRIPTPARSETREE";
  case XAssetType::KEYVALUEPAIRS:
    return "ASSET_KEYVALUEPAIRS";
  case XAssetType::VEHICLEDEF:
    return "ASSET_VEHICLEDEF";
  case XAssetType::ADDON_MAP_ENTS:
    return "ASSET_ADDON_MAP_ENTS";
  case XAssetType::TRACER:
    return "ASSET_TRACER";
  case XAssetType::SLUG:
    return "ASSET_SLUG";
  case XAssetType::SURFACEFX_TABLE:
    return "ASSET_SURFACEFX_TABLE";
  case XAssetType::SURFACESOUNDDEF:
    return "ASSET_SURFACESOUNDDEF";
  case XAssetType::FOOTSTEP_TABLE:
    return "ASSET_FOOTSTEP_TABLE";
  case XAssetType::ENTITYFXIMPACTS:
    return "ASSET_ENTITYFXIMPACTS";
  case XAssetType::ENTITYSOUNDIMPACTS:
    return "ASSET_ENTITYSOUNDIMPACTS";
  case XAssetType::ZBARRIER:
    return "ASSET_ZBARRIER";
  case XAssetType::VEHICLEFXDEF:
    return "ASSET_VEHICLEFXDEF";
  case XAssetType::VEHICLESOUNDDEF:
    return "ASSET_VEHICLESOUNDDEF";
  case XAssetType::TYPEINFO:
    return "ASSET_TYPEINFO";
  case XAssetType::SCRIPTBUNDLE:
    return "ASSET_SCRIPTBUNDLE";
  case XAssetType::SCRIPTBUNDLELIST:
    return "ASSET_SCRIPTBUNDLELIST";
  case XAssetType::RUMBLE:
    return "ASSET_RUMBLE";
  case XAssetType::BULLETPENETRATION:
    return "ASSET_BULLETPENETRATION";
  case XAssetType::LOCDMGTABLE:
    return "ASSET_LOCDMGTABLE";
  case XAssetType::AIMTABLE:
    return "ASSET_AIMTABLE";
  case XAssetType::ANIMSELECTORTABLESET:
    return "ASSET_ANIMSELECTORTABLESET";
  case XAssetType::ANIMMAPPINGTABLE:
    return "ASSET_ANIMMAPPINGTABLE";
  case XAssetType::ANIMSTATEMACHINE:
    return "ASSET_ANIMSTATEMACHINE";
  case XAssetType::BEHAVIORTREE:
    return "ASSET_BEHAVIORTREE";
  case XAssetType::BEHAVIORSTATEMACHINE:
    return "ASSET_BEHAVIORSTATEMACHINE";
  case XAssetType::TTF:
    return "ASSET_TTF";
  case XAssetType::SANIM:
    return "ASSET_SANIM";
  case XAssetType::LIGHT_DESCRIPTION:
    return "ASSET_LIGHT_DESCRIPTION";
  case XAssetType::SHELLSHOCK:
    return "ASSET_SHELLSHOCK";
  case XAssetType::XCAM:
    return "ASSET_XCAM";
  case XAssetType::BG_CACHE:
    return "ASSET_BG_CACHE";
  case XAssetType::TEXTURE_COMBO:
    return "ASSET_TEXTURE_COMBO";
  case XAssetType::FLAMETABLE:
    return "ASSET_FLAMETABLE";
  case XAssetType::BITFIELD:
    return "ASSET_BITFIELD";
  case XAssetType::ATTACHMENT_COSMETIC_VARIANT:
    return "ASSET_ATTACHMENT_COSMETIC_VARIANT";
  case XAssetType::MAPTABLE:
    return "ASSET_MAPTABLE";
  case XAssetType::MAPTABLE_LOADING_IMAGES:
    return "ASSET_MAPTABLE_LOADING_IMAGES";
  case XAssetType::MEDAL:
    return "ASSET_MEDAL";
  case XAssetType::MEDALTABLE:
    return "ASSET_MEDALTABLE";
  case XAssetType::OBJECTIVE:
    return "ASSET_OBJECTIVE";
  case XAssetType::OBJECTIVE_LIST:
    return "ASSET_OBJECTIVE_LIST";
  case XAssetType::UMBRA_TOME:
    return "ASSET_UMBRA_TOME";
  case XAssetType::NAVMESH:
    return "ASSET_NAVMESH";
  case XAssetType::NAVVOLUME:
    return "ASSET_NAVVOLUME";
  case XAssetType::BINARYHTML:
    return "ASSET_BINARYHTML";
  case XAssetType::LASER:
    return "ASSET_LASER";
  case XAssetType::BEAM:
    return "ASSET_BEAM";
  case XAssetType::STREAMER_HINT:
    return "ASSET_STREAMER_HINT";
  case XAssetType::STRING:
    return "ASSET_STRING";
  case XAssetType::ASSETLIST:
    return "ASSET_ASSETLIST";
  case XAssetType::REPORT:
    return "ASSET_REPORT";
  default:
    return "ASSET_INVALID";
  }
}

template <IntegralLike<std::underlying_type_t<XAssetType>> Value>
inline constexpr bool valid_xassettype(Value val_arg) {
  const std::underlying_type_t<XAssetType> val =
      static_cast<std::underlying_type_t<XAssetType>>(val_arg);
  return val >= +XAssetType::PHYSPRESET && val < +XAssetType::COUNT;
}

/*
  All XAssets union members have first field `const char * name;`
  The engine uses this shared field to use shared utility functions to access
  name of the asset. Prior to REing and verifying all XAsset union member
  structures, we can start with using the known-shared field only - name.
  Later we can define each of the XAsset structs with `: NamedXAsset` and
  declare the shared util function symbols using type signature `NamedXAsset *`
  to more succintly type what is being passed to the function.
*/
PACKED(struct NamedXAsset {
  const char *name;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {} }}", ::serialize(name));
  }
});
ASSERT_SIZE(NamedXAsset, 0x8);

PACKED(struct RawFile : NamedXAsset {
  size_t len;
  uint8_t *buffer;

  inline constexpr std::string serialize() const noexcept {
    return std::format(
        " {{ \"name\": \"{}\", \"len\": 0x{}, \"buffer\": {:p} }}",
        ::serialize(name), len, static_cast<const void *>(buffer));
  }
});
ASSERT_SIZE(RawFile, 0x18);

struct MaterialTechniqueSet;
typedef MaterialTechniqueSet *MaterialTechniqueSetPtr;

typedef const char *XString;

struct Set;
typedef Set *SetPtr;

typedef int32_t FxUniqueHandle;
struct TagFxSetHandles {
  FxUniqueHandle handle[8];
  int32_t count;
  SetPtr tagFX;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"handle\": {}, \"count\": {}, \"tagFX\": {:p} }}",
                       ::serialize(handle), count,
                       reinterpret_cast<const void *>(tagFX));
  }
};

template <typename T> struct FxRange {
  T base;
  T amplitude;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"base\": {}, \"amplitude\": {} }}", base,
                       amplitude);
  }
};

typedef FxRange<float> FxFloatRange;
typedef FxRange<float> FxIntRange;

typedef int32_t FxElemDefFlags;
typedef int32_t FxElemDefExtraFlags;

// sizeof=0x10
struct FxSpawnDefLooping {
  int32_t intervalMsec;
  int32_t count;
  FxIntRange spawnCount;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"intervalMsec\": {}, \"count\": {}, \"spawnCount\": {} }}",
        intervalMsec, count, spawnCount.serialize());
  }
};
ASSERT_SIZE(FxSpawnDefLooping, 0x10);

// sizeof=0x8
struct FxSpawnDefOneShot {
  FxIntRange count;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"count\": {} }}", count.serialize());
  }
};
ASSERT_SIZE(FxSpawnDefOneShot, 0x8);

// sizeof=0x10
union FxSpawnDef {
  FxSpawnDefLooping looping;
  FxSpawnDefOneShot oneShot;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"looping\": {} }}", looping.serialize());
  }
};
ASSERT_SIZE(FxSpawnDef, 0x10);

typedef uint32_t contents_t;

struct __attribute__((aligned(8))) MaterialInfo : NamedXAsset {
  uint32_t namehash;
  uint32_t gameFlags;
  uint8_t layerSortDecal;
  uint8_t pad2;
  uint8_t textureAtlasRowCount;
  uint8_t textureAtlasColumnCount;
  gfx::GfxSortKey drawSurf;
  uint32_t bindlessMaterialSortIndex;
  int32_t surfaceFlags;
  contents_t contents;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"namehash\": 0x{:08X}, \"gameFlags\": 0x{:08X}, "
        "\"layerSortDecal\": {}, \"pad2\": {}, \"textureAtlasRowCount\": {}, "
        "\"textureAtlasColumnCount\": {}, \"drawSurf\": {}, "
        "\"bindlessMaterialSortIndex\": {}, "
        "\"surfaceFlags\": 0x{:08X}, \"contents\": 0x{:08X} }}",
        ::serialize(name), namehash, gameFlags, layerSortDecal, pad2,
        textureAtlasRowCount, textureAtlasColumnCount, drawSurf.serialize(),
        bindlessMaterialSortIndex, surfaceFlags, contents);
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"behavior\": {}, \"index\": {}, \"fps\": {}, \"loopCount\": {}, "
        "\"colIndexBits\": {}, \"rowIndexBits\": {}, \"indexRange\": {} }}",
        behavior, index, fps, loopCount, colIndexBits, rowIndexBits,
        indexRange);
  }
};

ASSERT_SIZE(FxElemAtlas, 0x7);

typedef uint8_t FxElemType;

typedef Material FxMaterial;

typedef FxMaterial *FxMaterialHandle;
typedef MaterialHandle MaterialDeferredHandle;

typedef xmodel::XModel *FxModelHandle;

typedef const FxEffectDef *FxEffectDefHandleRaw;

// sizeof=0x8
union FxEffectDefRef {
  const char *name;
  FxEffectDefHandleRaw handle;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {} }}", ::serialize(name));
  }
};
ASSERT_SIZE(FxEffectDefRef, 0x8);

// sizeof=0x10
struct FxElemSound {
  const char *sound;
  snd::SndAliasId soundAlias;
  uint8_t _padding0C[4];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"sound\": {}, \"soundAlias\": 0x{:08X} }}",
                       ::serialize(sound), soundAlias);
  }
};
ASSERT_SIZE(FxElemSound, 0x10);

// sizeof=0x20
struct FxLensFlareVisualDef {
  uint32_t uuidData1;
  uint16_t uuidData2;
  uint16_t uuidData3;
  uint8_t uuidData4[8];
  vec3_t sourceDir;
  float sourceSize;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"uuidData1\": 0x{:08X}, \"uuidData2\": 0x{:04X}, \"uuidData3\": "
        "{:04X}, "
        "\"uuidData4\": {}, \"sourceDir\": {}, \"sourceSize\": {} }}",
        uuidData1, uuidData2, uuidData3, ::serialize(uuidData4),
        sourceDir.serialize(), sourceSize);
  }
};
ASSERT_SIZE(FxLensFlareVisualDef, 0x20);

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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"anonymous\": {:p} }}", anonymous);
  }
};

// sizeof=0x40
union FxElemDefVisuals {
  FxElemMarkVisuals *markArray;
  FxElemVisuals *array;
  FxElemVisuals instance;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"instance\": {} }}", instance.serialize());
  }
};
ASSERT_SIZE(FxElemDefVisuals, 0x40);

// sizeof=0x8
union FxElemExtendedDefPtr {
  FxTrailDef *localTrailDef;
  FxTrailDef *trailDef;
  FxSpotLightDef *localSpotLightDef;
  FxSpotLightDef *spotLightDef;
  gfx::GfxLightDescription *dynamicLightDef;
  const FxLensFlareRootDef *lensFlareDef;
  void *unknownDef; // verbatim name of title in engine

  inline std::string serialize() const noexcept {
    return std::format("{{ \"unknownDef\": {:p} }}", unknownDef);
  }
};
ASSERT_SIZE(FxElemExtendedDefPtr, 0x8);

union FxElemDefUnion {
  FxIntRange cloudDensityRange;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"cloudDensityRange\": {} }}",
                       cloudDensityRange.serialize());
  }
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"flags\": 0x{:08X}, \"extraFlags\": 0x{:08X}, \"spawn\": {}, "
        "\"spawnRange\": {}, "
        "\"fadeInRange\": {}, \"fadeOutRange\": {}, "
        "\"spawnFrustumCullRadius\": {}, \"spawnDelayMsec\": {}, "
        "\"lifeSpanMsec\": {}, \"spawnOrigin\": {}, \"spawnOffsetRadius\": {}, "
        "\"spawnOffsetHeight\": {}, "
        "\"spawnAngles\": {}, \"angularVelocity\": {}, \"initialRotation\": "
        "{}, \"rotationAxis\": 0x{:08X}, "
        "\"gravity\": {}, \"reflectionFactor\": {}, \"atlas\": {}, "
        "\"windInfluence\": {}, "
        "\"elemType\": {}, \"visualCount\": {}, \"velSamples\": {:p}, "
        "\"inheritSamples\": {:p}, "
        "\"visSamples\": {:p}, \"attSamples\": {:p}, \"visuals\": {}, "
        "\"computeVisuals\": {}, "
        "\"collMins\": {}, \"collMaxs\": {}, \"attractorLocalPosition\": {}, "
        "\"effectOnImpact\": {}, "
        "\"effectOnDeath\": {}, \"effectEmitted\": {}, \"emitDist\": {}, "
        "\"emitDistVariance\": {}, "
        "\"emitDensity\": {}, \"emitSizeForDensity\": {}, \"effectAttached\": "
        "{}, \"attachedDensity\": {}, "
        "\"attachedSizeForDensity\": {}, \"extended\": {}, \"displacement\": "
        "{}, \"lightingFrac\": {}, "
        "\"depthMultiplier\": {}, \"alphaFadeTimeMsec\": {}, "
        "\"maxWindStrength\": {}, \"spawnIntervalAtMaxWind\": {}, "
        "\"lifespanAtMaxWind\": {}, \"u\": {}, \"spawnSound\": {}, "
        "\"followSound\": {}, "
        "\"billboardPivot\": {}, \"alphaDissolve\": {}, \"zFeather\": {}, "
        "\"falloffBeginAngle\": 0x{:08X}, "
        "\"falloffEndAngle\": 0x{:08X}, \"hdrScale\": {}, \"levelsInputMin\": "
        "{}, \"levelsInputMax\": {}, "
        "\"levelsOutputMin\": {}, \"levelsOutputMax\": {}, "
        "\"desaturationAmount\": {}, \"useOldHDRScale\": {}, "
        "\"libDefIndex\": {}, \"libVisualIndex\": {} }}",
        flags, extraFlags, spawn.serialize(), spawnRange.serialize(),
        fadeInRange.serialize(), fadeOutRange.serialize(),
        spawnFrustumCullRadius, spawnDelayMsec.serialize(),
        lifeSpanMsec.serialize(), ::serialize(spawnOrigin),
        spawnOffsetRadius.serialize(), spawnOffsetHeight.serialize(),
        ::serialize(spawnAngles), ::serialize(angularVelocity),
        initialRotation.serialize(), rotationAxis, gravity.serialize(),
        reflectionFactor.serialize(), atlas.serialize(), windInfluence,
        elemType, visualCount, reinterpret_cast<const void *>(velSamples),
        reinterpret_cast<const void *>(inheritSamples),
        reinterpret_cast<const void *>(visSamples),
        reinterpret_cast<const void *>(attSamples), visuals.serialize(),
        computeVisuals.serialize(), collMins.serialize(), collMaxs.serialize(),
        attractorLocalPosition.serialize(), effectOnImpact.serialize(),
        effectOnDeath.serialize(), effectEmitted.serialize(),
        emitDist.serialize(), emitDistVariance.serialize(),
        emitDensity.serialize(), emitSizeForDensity, effectAttached.serialize(),
        attachedDensity.serialize(), attachedSizeForDensity,
        extended.serialize(), displacement, lightingFrac, depthMultiplier,
        alphaFadeTimeMsec, maxWindStrength, spawnIntervalAtMaxWind,
        lifespanAtMaxWind, u.serialize(), spawnSound.serialize(),
        followSound.serialize(), billboardPivot.serialize(), alphaDissolve,
        zFeather, falloffBeginAngle, falloffEndAngle, hdrScale, levelsInputMin,
        levelsInputMax, levelsOutputMin, levelsOutputMax, desaturationAmount,
        ::serialize(useOldHDRScale), libDefIndex, libVisualIndex);
  }
};
ASSERT_SIZE(FxElemDef, 0x260);

typedef int32_t FxEffectDefFlags;
typedef const FxEffectDef *FxEffectDefHandle;
typedef uint8_t FxNormalsShape;

// sizeof=0x98
struct FxEffectDef : NamedXAsset {
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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"flags\": 0x{:08X}, \"efPriority\": {}, "
        "\"elemDefCountLooping\": {}, "
        "\"elemDefCountOneShot\": {}, \"elemDefCountEmission\": {}, "
        "\"totalSize\": {}, \"msecLoopingLife\": {}, "
        "\"msecNonLoopingLife\": {}, \"elemDefs\": {:p}, \"boundingBoxDim\": "
        "{}, \"boundingBoxCentre\": {}, "
        "\"occlusionQueryDepthBias\": {}, \"occlusionQueryFadeIn\": {}, "
        "\"occlusionQueryFadeOut\": {}, \"occlusionQueryScaleRange\": {}, "
        "\"loopedSound\": {}, \"normalsShape\": {}, \"normalsOffset\": {}, "
        "\"normalsNormal\": {}, "
        "\"gpuMsecLife\": {}, \"libDefIndex\": {} }}",
        ::serialize(name), flags, efPriority, elemDefCountLooping,
        elemDefCountOneShot, elemDefCountEmission, totalSize, msecLoopingLife,
        msecNonLoopingLife, reinterpret_cast<const void *>(elemDefs),
        boundingBoxDim.serialize(), boundingBoxCentre.serialize(),
        occlusionQueryDepthBias, occlusionQueryFadeIn, occlusionQueryFadeOut,
        occlusionQueryScaleRange.serialize(), loopedSound.serialize(),
        normalsShape, normalsOffset.serialize(), normalsNormal.serialize(),
        gpuMsecLife, libDefIndex);
  }
};
ASSERT_SIZE(FxEffectDef, 0x98);

struct FxElemDef;

// sizeof=0x30
struct EntityImpactPositionsFx {
  FxEffectDefHandle entityImpactPositions[6];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"entityImpactPositions\": {} }}",
                       ::serialize(entityImpactPositions));
  }
};
ASSERT_SIZE(EntityImpactPositionsFx, 0x30);

// sizeof=0x218
struct EntityFxImpacts : public NamedXAsset {
  EntityImpactPositionsFx entityImpacts[11];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"name\": {}, \"entityImpacts\": {} }}",
                       ::serialize(name), ::serialize(entityImpacts));
  }
};
ASSERT_SIZE(EntityFxImpacts, 0x218);

// TODO
struct SurfaceFXTableDef;

typedef SurfaceFXTableDef *SurfaceFXTableDefPtr;

typedef EntityFxImpacts *EntityFxImpactsPtr;

struct FxImpactTable : public NamedXAsset {
  SurfaceFXTableDefPtr surfaceFX;
  EntityFxImpactsPtr entityFXImpacts;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"surfaceFX\": {:p}, \"entityFXImpacts\": {} }}",
        ::serialize(name), reinterpret_cast<const void *>(surfaceFX),
        ::serialize(entityFXImpacts));
  }
};

struct FxElemMarkVisuals {
  FxMaterialHandle materials[3];

  inline std::string serialize() const noexcept {
    return std::format("{{ \"materials\": {:p} }}",
                       reinterpret_cast<const void *>(materials));
  }
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

inline constexpr const char *serialize(const GfxPixelFormat format) {
  switch (format) {
  case GfxPixelFormat::GFX_PF_INVALID:
    return "PF_INVALID";
  case GfxPixelFormat::GFX_PF_D24S8:
    return "PF_D24S8";
  case GfxPixelFormat::GFX_PF_D24FS8:
    return "PF_D24FS8";
  case GfxPixelFormat::GFX_PF_A8:
    return "PF_A8";
  case GfxPixelFormat::GFX_PF_R8_UN:
    return "PF_R8_UN";
  case GfxPixelFormat::GFX_PF_R8_UI:
    return "PF_R8_UI";
  case GfxPixelFormat::GFX_PF_D16:
    return "PF_D16";
  case GfxPixelFormat::GFX_PF_R16_UN:
    return "PF_R16_UN";
  case GfxPixelFormat::GFX_PF_R16_UI:
    return "PF_R16_UI";
  case GfxPixelFormat::GFX_PF_R16F:
    return "PF_R16F";
  case GfxPixelFormat::GFX_PF_R8G8_UN:
    return "PF_R8G8_UN";
  case GfxPixelFormat::GFX_PF_R8G8_SNORM:
    return "PF_R8G8_SNORM";
  case GfxPixelFormat::GFX_PF_B5G6R5:
    return "PF_B5G6R5";
  case GfxPixelFormat::GFX_PF_B4G4R4A4:
    return "PF_B4G4R4A4";
  case GfxPixelFormat::GFX_PF_D32F:
    return "PF_D32F";
  case GfxPixelFormat::GFX_PF_R32F:
    return "PF_R32F";
  case GfxPixelFormat::GFX_PF_R16G16_UN:
    return "PF_R16G16_UN";
  case GfxPixelFormat::GFX_PF_R16G16_SNORM:
    return "PF_R16G16_SNORM";
  case GfxPixelFormat::GFX_PF_R16G16F:
    return "PF_R16G16F";
  case GfxPixelFormat::GFX_PF_R9G9B9E5:
    return "PF_R9G9B9E5";
  case GfxPixelFormat::GFX_PF_R8G8B8A8:
    return "PF_R8G8B8A8";
  case GfxPixelFormat::GFX_PF_R8G8B8A8_SNORM:
    return "PF_R8G8B8A8_SNORM";
  case GfxPixelFormat::GFX_PF_B8G8R8A8:
    return "PF_B8G8R8A8";
  case GfxPixelFormat::GFX_PF_R8G8B8A8_SRGB:
    return "PF_R8G8B8A8_SRGB";
  case GfxPixelFormat::GFX_PF_R10G10B10A2:
    return "PF_R10G10B10A2";
  case GfxPixelFormat::GFX_PF_B10G10R10A2:
    return "PF_B10G10R10A2";
  case GfxPixelFormat::GFX_PF_R11G11B10F:
    return "PF_R11G11B10F";
  case GfxPixelFormat::GFX_PF_R32G32F:
    return "PF_R32G32F";
  case GfxPixelFormat::GFX_PF_R16G16B16A16_UN:
    return "PF_R16G16B16A16_UN";
  case GfxPixelFormat::GFX_PF_R16G16B16A16F:
    return "PF_R16G16B16A16F";
  case GfxPixelFormat::GFX_PF_R32G32B32A32F:
    return "PF_R32G32B32A32F";
  case GfxPixelFormat::GFX_PF_BC1:
    return "PF_BC1";
  case GfxPixelFormat::GFX_PF_BC1_SRGB:
    return "PF_BC1_SRGB";
  case GfxPixelFormat::GFX_PF_BC2:
    return "PF_BC2";
  case GfxPixelFormat::GFX_PF_BC2_SRGB:
    return "PF_BC2_SRGB";
  case GfxPixelFormat::GFX_PF_BC3:
    return "PF_BC3";
  case GfxPixelFormat::GFX_PF_BC3_SRGB:
    return "PF_BC3_SRGB";
  case GfxPixelFormat::GFX_PF_BC4:
    return "PF_BC4";
  case GfxPixelFormat::GFX_PF_BC5:
    return "PF_BC5";
  case GfxPixelFormat::GFX_PF_BC6_UH:
    return "PF_BC6_UH";
  case GfxPixelFormat::GFX_PF_BC6_SH:
    return "PF_BC6_SH";
  case GfxPixelFormat::GFX_PF_BC7:
    return "PF_BC7";
  case GfxPixelFormat::GFX_PF_BC7_SRGB:
    return "PF_BC7_SRGB";
  case GfxPixelFormat::GFX_PF_R32G32B32A32_UN:
    return "PF_R32G32B32A32_UN";
  case GfxPixelFormat::GFX_PF_R32_UI:
    return "PF_R32_UI";
  case GfxPixelFormat::GFX_PF_R32G32_UI:
    return "PF_R32G32_UI";
  default:
    return "PF_UNKNOWN_VAL";
  }
}

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

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"key\": 0x{:016X}, \"offset\": {}, \"size\": {}, \"xpakIndex\": "
        "{}, "
        "\"compressed\": {}, \"valid\": {}, \"adjacentLeftType\": {}, "
        "\"adjacentRightType\": {}, \"adjacentLeft\": {}, \"adjacentRight\": "
        "{} }}",
        key, offset, size, xpakIndex,
        ::serialize(static_cast<bool>(compressed)),
        ::serialize(static_cast<bool>(valid)), adjacentLeftType,
        adjacentRightType, adjacentLeft, adjacentRight);
  }
};

struct GfxStreamedPartInfo {
  XPakEntryInfo xpakEntry;
  uint32_t levelCountAndSize;
  uint16_t width;
  uint16_t height;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"xpakEntry\": {}, \"levelCountAndSize\": 0x{:08X}, "
                       "\"width\": {}, \"height\": {} }}",
                       xpakEntry.serialize(), levelCountAndSize, width, height);
  }
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

inline constexpr const char *serialize(const MapType type) {
  switch (type) {
  case MapType::MAPTYPE_NONE:
    return "MAPTYPE_NONE";
  case MapType::MAPTYPE_2D:
    return "MAPTYPE_2D";
  case MapType::MAPTYPE_2D_ARRAY:
    return "MAPTYPE_2D_ARRAY";
  case MapType::MAPTYPE_3D:
    return "MAPTYPE_3D";
  case MapType::MAPTYPE_CUBE:
    return "MAPTYPE_CUBE";
  case MapType::MAPTYPE_CUBE_ARRAY:
    return "MAPTYPE_CUBE_ARRAY";
  default:
    return "MAPTYPE_INVALID";
  }
}

struct ScriptBundle;
typedef ScriptBundle *ScriptBundlePtr;

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

struct ScriptBundleList;
typedef ScriptBundleList *ScriptBundleListPtr;

struct ObjectiveList;
typedef ObjectiveList *ObjectiveListPtr;

struct ScriptBundleKVP;
typedef ScriptBundleKVP *ScriptBundleKVPPtr;

struct TagFxSet;
typedef TagFxSet *TagFxSetPtr;

struct XAnimTree;

struct LocalizeEntry;

// TODO
struct PlayerRoleTemplate;
typedef PlayerRoleTemplate *PlayerRoleTemplatePtr;
PACKED(struct PlayerRoleLevels {
  bool enabled;
  uint8_t _padding01[3];
  uint32_t numLevels;
  PlayerRoleTemplatePtr *levels;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"enabled\": {}, \"numLevels\": {}, \"levels\": {:p} }}",
        ::serialize(enabled), numLevels,
        reinterpret_cast<const void *>(levels));
  }
});

// TODO
struct CharacterHead;

typedef PlayerRoleLevels *PlayerRoleLevelsPtr;
// Verified
PACKED(struct CustomizationTable : NamedXAsset {
  uint32_t numPlayerRoles;
  uint8_t _padding0C[4];
  PlayerRoleLevelsPtr *playerRoles;
  uint32_t numHeads;
  uint8_t _padding1C[4];
  CharacterHead *heads;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"numPlayerRoles\": {}, \"playerRoles\": {}, "
        "\"numHeads\": {}, \"heads\": {:p} }}",
        ::serialize(name), numPlayerRoles,
        ::serialize(playerRoles, numPlayerRoles), numHeads,
        reinterpret_cast<const void *>(heads));
  }
});
ASSERT_SIZE(CustomizationTable, 0x28);

struct CamoBaseMaterial {
  MaterialHandle material;
  gfx::GfxImage *mask;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"material\": {:p}, \"mask\": {:p} }}",
                       reinterpret_cast<const void *>(material),
                       reinterpret_cast<const void *>(mask));
  }
};

struct CamoMaterial {
  uint16_t replaceFlags;
  uint16_t numBaseMaterials;
  CamoBaseMaterial *baseMaterials;
  MaterialHandle camoMaterial;
  float translationX;
  float translationY;
  float scaleX;
  float scaleY;
  float rotation;
  float normalBlend;
  float glossBlend;
  gfx::GfxColor albedoTint;
  vec2_t glossRange;
  vec_t specColor;
  vec_t specOffset;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"replaceFlags\": 0x{:04X}, \"numBaseMaterials\": {}, "
        "\"baseMaterials\": {:p}, "
        "\"camoMaterial\": {:p}, \"translationX\": {}, "
        "\"translationY\": {}, \"scaleX\": {}, "
        "\"scaleY\": {}, \"rotation\": {}, \"normalBlend\": {}, "
        "\"glossBlend\": {}, "
        "\"albedoTint\": {}, \"glossRange\": {}, \"specColor\": "
        "{}, \"specOffset\": {} }}",
        replaceFlags, numBaseMaterials,
        reinterpret_cast<const void *>(baseMaterials),
        reinterpret_cast<const void *>(camoMaterial), translationX,
        translationY, scaleX, scaleY, rotation, normalBlend, glossBlend,
        albedoTint.serialize(), glossRange.serialize(), specColor, specOffset);
  }
};

struct CharacterBodyType_FrontendImages {
  gfx::GfxImageHandle background;
  gfx::GfxImageHandle backgroundWithCharacter;
  gfx::GfxImageHandle lockedImage;
  gfx::GfxImageHandle personalizeRender;
  gfx::GfxImageHandle frozenMomentRender;
  gfx::GfxImageHandle frozenMomentOverlay;
  gfx::GfxImage *equippedLoadoutIcons[2];
  gfx::GfxImage *unequippedLoadoutIcons[2];
  gfx::GfxImageHandle cardBackIcon;
  gfx::GfxImageHandle weaponCardBackIcon;
  gfx::GfxImageHandle weaponCardBackSubIcon;
  gfx::GfxImageHandle abilityCardBackIcon;
  gfx::GfxImageHandle abilityCardBackSubIcon;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"background\": {}, \"backgroundWithCharacter\": {}, "
        "\"lockedImage\": {}, "
        "\"personalizeRender\": {}, \"frozenMomentRender\": {}, "
        "\"frozenMomentOverlay\": {}, "
        "\"equippedLoadoutIcons\": {}, \"unequippedLoadoutIcons\": {}, "
        "\"cardBackIcon\": {}, "
        "\"weaponCardBackIcon\": {}, \"weaponCardBackSubIcon\": {}, "
        "\"abilityCardBackIcon\": {}, "
        "\"abilityCardBackSubIcon\": {} }}",
        ::serialize(background), ::serialize(backgroundWithCharacter),
        ::serialize(lockedImage), ::serialize(personalizeRender),
        ::serialize(frozenMomentRender), ::serialize(frozenMomentOverlay),
        ::serialize(equippedLoadoutIcons), ::serialize(unequippedLoadoutIcons),
        ::serialize(cardBackIcon), ::serialize(weaponCardBackIcon),
        ::serialize(weaponCardBackSubIcon), ::serialize(abilityCardBackIcon),
        ::serialize(abilityCardBackSubIcon));
  }
};

// Verified
struct CustomizationTable_FEImages : NamedXAsset {
  uint32_t numBodyTypes;
  CharacterBodyType_FrontendImages *bodyTypes;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"numBodyTypes\": {}, \"bodyTypes\": {} }}",
        ::serialize(name), numBodyTypes, ::serialize(bodyTypes, numBodyTypes),
        numBodyTypes);
  }
};
ASSERT_SIZE(CustomizationTable_FEImages, 0x18);

// Verified
struct CustomizationColorInfo : NamedXAsset {
  XString displayName;
  gfx::GfxColor uiColor;
  gfx::GfxImage *icon;
  uint32_t numCamoMaterials;
  CamoMaterial *camoMaterials;

  IMPL_SLICE_METHODS(camoMaterials, numCamoMaterials, CamoMaterials);

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"displayName\": {}, \"uiColor\": {}, \"icon\": {}, "
        "\"numCamoMaterials\": {}, \"camoMaterials\": {} }}",
        ::serialize(name), ::serialize(displayName), uiColor.serialize(),
        ::serialize(icon), numCamoMaterials,
        ::serialize(camoMaterials, numCamoMaterials));
  }
};
ASSERT_SIZE(CustomizationColorInfo, 0x30);

struct KeyValuePair {
  int32_t keyHash;
  const char *value;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"keyHash\": 0x{:08X}, \"value\": {} }}", keyHash,
                       ::serialize(value));
  }
};

// Verified
struct KeyValuePairs : NamedXAsset {
  int32_t numVariables;
  KeyValuePair *keyValuePairs;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"numVariables\": {}, \"keyValuePairs\": {} }}",
        ::serialize(name), numVariables,
        ::serialize(keyValuePairs, numVariables));
  }
};
ASSERT_SIZE(KeyValuePairs, 0x18);

union XAssetHeader {
  NamedXAsset *named;
  phys::PhysPreset *physPreset;
  phys::PhysConstraints *physConstraints;
  destructible::DestructibleDef *destructibleDef;
  // XAnimParts *parts;
  xmodel::XModel *model;
  xmodel::XModelMesh *modelMesh;
  // Material *material;
  // MaterialComputeShaderSet *computeShaderSet;
  MaterialTechniqueSet *techniqueSet;
  gfx::GfxImage *image;
  snd::SndBank *sound;
  snd::SndPatch *soundPatch;
  cm::clipMap_t *clipMap;
  world::ComWorld *comWorld;
  world::GameWorld *gameWorld;
  // MapEnts *mapEnts;
  world::GfxWorld *gfxWorld;
  gfx::GfxLightDef *lightDef;
  // gfx::GfxLensFlareDef *lensFlareDef;
  font::Font *font;
  font::FontIcon *fontIcon;
  LocalizeEntry *localize;
  weapon::WeaponVariantDef *weapon;
  // WeaponAttachment *attachment;
  // WeaponAttachmentUnique *attachmentUnique;
  // WeaponCamo *weaponCamo;
  CustomizationTable *customizationTable;
  CustomizationTable_FEImages *customizationTable_feimages;
  CustomizationColorInfo *customizationColorInfo;
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
  ddl::StringTable *stringTable;
  // StructuredTable *structuredTable;
  // LeaderboardDef *leaderboardDef;
  ddl::DDLDef *ddl;
  // Glasses *glasses;
  // TextureList *textureList;
  scr::ScriptParseTree *scriptParseTree;
  KeyValuePairs *keyValuePairs;
  vehicle::VehicleDef *vehicleDef;
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
  vehicle::VehicleFxDef *vehicleFxDef;
  vehicle::VehicleSoundDef *vehicleSoundDef;
  // ArchiveTypeInfoArray *typeInfo;
  // ScriptBundle *scriptBundle;
  // ScriptBundleList *scriptBundleList;
  // RumbleInfo *rumble;
  // BulletPenetrationTable *bulletPenetration;
  // LocDmgTable *locDmgTable;
  // AimTable *aimTable;
  xmodel::XModelAlias *xModelAlias;
  // Character *character;
  // AIType *aiType;
  // PlayerCharacter *player_character;
  // AnimSelectorTableSet *animSelectorTableSet;
  // AnimMappingTable *animMappingTable;
  // AnimStateMachine *animStateMachine;
  // BehaviorTree *behaviorTree;
  // BehaviorStateMachine *behaviorStateMachine;
  ttf::TTFDef *ttfDef;
  // GfxSiegeAnim *sanim;
  // GfxLightDescription *lightDescription;
  // ShellshockParams *shellshock;
  // XCam *xcam;
  bg::cache::BGCacheInfo *bgCache;
  // TextureCombo *textureCombo;
  // FlameTable *flameTable;
  // Bitfield *bitfield;
  // AttachmentCosmeticVariant *attachmentCosmeticVariant;
  maptable::MapTable *mapTable;
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

  inline std::string serialize() const noexcept {
    return std::format("{{ \"named\": {} }}", ::serialize(named));
  }
};
ASSERT_SIZE(XAssetHeader, sizeof(void *));

// sizeof=0x10
#pragma pack(push, 1)
struct XAsset {
  XAssetType type;
  uint8_t _padding04[4];
  XAssetHeader header;

  // TODO: serialize corresponding header union member for given type
  inline std::string serialize() const noexcept {
    return std::format("{{ \"type\": {}, \"header\": {} }}",
                       game::db::xasset::serialize(type), header.serialize());
  }
};
ASSERT_SIZE(XAsset, 0x10);
#pragma pack(pop)

using XAssetEnum = void(XAssetHeader, void *);

#pragma pack(push, 1)

PACKED(struct ScriptStringList {
  int32_t count;
  uint8_t _padding04[4];
  const char **strings;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"count\": {}, \"strings\": {} }}", count,
                       ::serialize(strings, count));
  }
});
ASSERT_SIZE(ScriptStringList, 0x10);

struct XAssetList {
  ScriptStringList stringList;
  int32_t assetCount;
  uint8_t _padding14[4];
  XAsset *assets;

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"stringList\": {}, \"assetCount\": {}, \"assets\": {} }}",
        stringList.serialize(), assetCount, ::serialize(assets, assetCount));
  }
};
ASSERT_SIZE(XAssetList, 0x20);
#pragma pack(pop)

PACKED(struct XAssetEntry {
  XAsset asset;
  uint8_t zoneIndex;
  bool inuse;
  uint8_t _padding12[2];
  uint32_t nextHash;
  uint32_t nextType;
  uint32_t nextOverride;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"asset\": {}, \"zoneIndex\": {}, \"inuse\": {}, "
                       "\"nextHash\": 0x{:08X}, \"nextType\": 0x{:08X}, "
                       "\"nextOverride\": 0x{:08X} }}",
                       asset.serialize(), zoneIndex, ::serialize(inuse),
                       nextHash, nextType, nextOverride);
  }
});
ASSERT_SIZE(XAssetEntry, 0x20);
typedef XAssetEntry *XAssetEntryPtr;

struct ManagedNotetrack_t {
  scr::ScrString_t elemType;
  scr::ScrString_t param1;
  scr::ScrString_t param2;
  int32_t startTime;
  union {
    db::xasset::FxUniqueHandle fxHandle;
    snd::SndAliasId alias;
  };

  inline std::string serialize() const noexcept {
    return std::format("{{ \"elemType\": 0x{:08X}, \"param1\": 0x{:08X}, "
                       "\"param2\": 0x{:08X}, "
                       "\"startTime\": {} }}",
                       elemType, param1, param2, startTime);
  }
};

struct ManagedNoteTrackList {
  ManagedNotetrack_t notetracks[10];
  db::xasset::TagFxSetHandles effectSetHandles;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"notetracks\": {}, \"effectSetHandles\": {} }}",
                       ::serialize(notetracks), effectSetHandles.serialize());
  }
};

PACKED(struct LocalizeEntry {
  const char *value;
  const char *name;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"value\": {}, \"name\": {} }}", ::serialize(value),
                       ::serialize(name));
  }
});
ASSERT_SIZE(LocalizeEntry, 0x10);

struct accoladeCache {
  uint8_t accoladeIndex;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"accoladeIndex\": {} }}", accoladeIndex);
  }
};

PACKED(struct mapTableEntryCache {
  accoladeCache accoladesWithUnlocks[32];
  const char *rootMapName;
  uint8_t numAccoladesWithUnlocks;
  uint8_t _padding29[7];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"accoladesWithUnlocks\": {}, \"rootMapName\": {}, "
        "\"numAccoladesWithUnlocks\": {} }}",
        ::serialize(accoladesWithUnlocks, numAccoladesWithUnlocks),
        ::serialize(rootMapName), numAccoladesWithUnlocks);
  }
});
ASSERT_SIZE(mapTableEntryCache, 0x30);

PACKED(struct mapTableCache {
  mapTableEntryCache missionsWithAccolades[128];
  uint8_t numMissionsWithAccolades;
  uint8_t _padding1801[7];

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"missionsWithAccolades\": {}, \"numMissionsWithAccolades\": {} }}",
        ::serialize(missionsWithAccolades, numMissionsWithAccolades),
        numMissionsWithAccolades);
  }
});
ASSERT_SIZE(mapTableCache, 0x1808);

struct __declspec(align(8)) Collmap {
  phys::PhysGeomList *geomList;
  BoneIndex boneIndex;

  inline std::string serialize() const noexcept {
    return std::format("{{ \"geomList\": {:p}, \"boneIndex\": 0x{:04X} }}",
                       reinterpret_cast<const void *>(geomList), boneIndex);
  }
};

struct ColorOptionSet {
  uint32_t numOptions;
  CustomizationColorInfo **colors;

  IMPL_SLICE_METHODS(colors, numOptions, Colors);
};

struct CharacterItemInfo {
  const char *displayName;
  gfx::GfxImage *icon;
  xmodel::XModelPtr models[11];
  uint32_t accentColorCount;
  ColorOptionSet *colorOptions;
  int impactType[2];
  ScriptBundlePtr gibDef;

  IMPL_SLICE_METHODS(colorOptions, accentColorCount, ColorOptions);
};

enum class CharacterGenderTypes : uint32_t {
  MALE = 0x0,
  MP = 0x0,
  FIRST = 0x0,
  FEMALE = 0x1,
  COUNT = 0x2,
  INVALID = 0x2,
  ANY = 0x3,
};

inline constexpr const char *serialize(const CharacterGenderTypes gender) {
  switch (gender) {
  case CharacterGenderTypes::MALE:
    return "GENDER_MALE";
  case CharacterGenderTypes::FEMALE:
    return "GENDER_FEMALE";
  default:
    return "GENDER_INVALID";
  }
}

struct CharacterBodyType : NamedXAsset {
  const char *displayName;
  const char *description;
  const char *heroWeapon;
  const char *heroAbility;
  XString bodySoundContext;
  ScriptBundlePtr mpDialog;
  XString chrName;
  gfx::GfxImageHandle rewardIcon;
  XString background;
  XString backgroundWithCharacter;
  XString lockedImage;
  XString personalizeRender;
  XString frozenMomentRender;
  XString frozenMomentOverlay;
  gfx::GfxImageHandle defaultHeroRender;
  gfx::GfxImageHandle defaultHeroRenderAbility;
  gfx::GfxImageHandle positionDraftPortrait;
  gfx::GfxImageHandle positionDraftIcon;
  XString equippedLoadoutIcons[2];
  XString unequippedLoadoutIcons[2];
  gfx::GfxImageHandle zombiePlayerIcon;
  uint32_t numBodyInfos;
  CharacterItemInfo *bodyInfo;
  uint32_t numHelmetInfos;
  CharacterItemInfo *helmetInfo;
  CharacterGenderTypes gender;
  bool disabled;
  XString frontendVignetteStruct;
  XCamPtr frontendVignetteXCam;
  XAnimPartsPtr frontendVignetteXAnim;
  xmodel::XModelPtr frontendVignetteWeaponModel;
  uint32_t kvpCount;
  ScriptBundleKVP *kvpItems;
  PlayerSoundsTablePtr characterMovementSounds;
  PlayerFXTablePtr characterMovementFx;
  FootstepTableDefPtr characterFootsteps;
  FootstepTableDefPtr characterFootstepsQuiet;
  FootstepTableDefPtr characterFootstepsNPC;
  FootstepTableDefPtr characterFootstepsNPCLoud;
  FootstepTableDefPtr characterFootstepsNPCQuiet;
  xmodel::XModelPtr dogtagFriendly;
  xmodel::XModelPtr dogTagEnemy;
  XString cardBackIcon;
  XString realName;
  int32_t age;
  XString genderString;
  XString bio;
  XString weaponCardBackIcon;
  XString weaponCardBackSubIcon;
  XString weaponCardBackDesc;
  XString weaponSubItemDesc;
  XString weaponSchema;
  XString abilityCardBackIcon;
  XString abilityCardBackSubIcon;
  XString abilityCardBackDesc;
  XString abilitySubItemDesc;
  XString abilitySchema;

  IMPL_SLICE_METHODS(bodyInfo, numBodyInfos, BodyInfo);
  IMPL_SLICE_METHODS(helmetInfo, numHelmetInfos, HelmetInfo);

  inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"name\": {}, \"displayName\": {}, \"description\": {}, "
        "\"heroWeapon\": {}, "
        "\"heroAbility\": {}, \"bodySoundContext\": {}, \"mpDialog\": {:p}, "
        "\"chrName\": {}, "
        "\"rewardIcon\": {:p}, \"background\": {}, "
        "\"backgroundWithCharacter\": {}, \"lockedImage\": {}, "
        "\"personalizeRender\": {}, \"frozenMomentRender\": {}, "
        "\"frozenMomentOverlay\": {}, \"defaultHeroRender\": {:p}, "
        "\"defaultHeroRenderAbility\": {:p}, \"positionDraftPortrait\": {:p}, "
        "\"positionDraftIcon\": {:p}, \"equippedLoadoutIcons\": {}, "
        "\"unequippedLoadoutIcons\": {}, \"zombiePlayerIcon\": {:p}, "
        "\"numBodyInfos\": {}, \"bodyInfo\": {:p}, "
        "\"numHelmetInfos\": {}, \"helmetInfo\": {:p}, \"gender\": {}, "
        "\"disabled\": {}, "
        "\"frontendVignetteStruct\": {}, \"frontendVignetteXCam\": {:p}, "
        "\"frontendVignetteXAnim\": {:p}, \"frontendVignetteWeaponModel\": "
        "{:p}, "
        "\"kvpCount\": {}, \"kvpItems\": {:p}, \"characterMovementSounds\": "
        "{:p}, \"characterMovementFx\": {:p}, "
        "\"characterFootsteps\": {:p}, \"characterFootstepsQuiet\": {:p}, "
        "\"characterFootstepsNPC\": {:p}, \"characterFootstepsNPCLoud\": {:p}, "
        "\"characterFootstepsNPCQuiet\": {:p}, \"dogtagFriendly\": {:p}, "
        "\"dogTagEnemy\": {:p}, \"cardBackIcon\": {}, "
        "\"realName\": {}, \"age\": {}, \"genderString\": {}, \"bio\": {}, "
        "\"weaponCardBackIcon\": {}, \"weaponCardBackSubIcon\": {}, "
        "\"weaponCardBackDesc\": {}, \"weaponSubItemDesc\": {}, "
        "\"weaponSchema\": {}, \"abilityCardBackIcon\": {}, "
        "\"abilityCardBackSubIcon\": {}, \"abilityCardBackDesc\": {}, "
        "\"abilitySubItemDesc\": {}, \"abilitySchema\": {} }}",
        ::serialize(name), ::serialize(displayName), ::serialize(description),
        ::serialize(heroWeapon), ::serialize(heroAbility),
        ::serialize(bodySoundContext), reinterpret_cast<const void *>(mpDialog),
        ::serialize(chrName), reinterpret_cast<const void *>(rewardIcon),
        ::serialize(background), ::serialize(backgroundWithCharacter),
        ::serialize(lockedImage), ::serialize(personalizeRender),
        ::serialize(frozenMomentRender), ::serialize(frozenMomentOverlay),
        reinterpret_cast<const void *>(defaultHeroRender),
        reinterpret_cast<const void *>(defaultHeroRenderAbility),
        reinterpret_cast<const void *>(positionDraftPortrait),
        reinterpret_cast<const void *>(positionDraftIcon),
        ::serialize(equippedLoadoutIcons), ::serialize(unequippedLoadoutIcons),
        reinterpret_cast<const void *>(zombiePlayerIcon), numBodyInfos,
        reinterpret_cast<const void *>(bodyInfo), numHelmetInfos,
        reinterpret_cast<const void *>(helmetInfo),
        game::db::xasset::serialize(gender), ::serialize(disabled),
        ::serialize(frontendVignetteStruct),
        reinterpret_cast<const void *>(frontendVignetteXCam),
        reinterpret_cast<const void *>(frontendVignetteXAnim),
        reinterpret_cast<const void *>(frontendVignetteWeaponModel), kvpCount,
        reinterpret_cast<const void *>(kvpItems),
        reinterpret_cast<const void *>(characterMovementSounds),
        reinterpret_cast<const void *>(characterMovementFx),
        reinterpret_cast<const void *>(characterFootsteps),
        reinterpret_cast<const void *>(characterFootstepsQuiet),
        reinterpret_cast<const void *>(characterFootstepsNPC),
        reinterpret_cast<const void *>(characterFootstepsNPCLoud),
        reinterpret_cast<const void *>(characterFootstepsNPCQuiet),
        reinterpret_cast<const void *>(dogtagFriendly),
        reinterpret_cast<const void *>(dogTagEnemy), ::serialize(cardBackIcon),
        ::serialize(realName), age, ::serialize(genderString), ::serialize(bio),
        ::serialize(weaponCardBackIcon), ::serialize(weaponCardBackSubIcon),
        ::serialize(weaponCardBackDesc), ::serialize(weaponSubItemDesc),
        ::serialize(weaponSchema), ::serialize(abilityCardBackIcon),
        ::serialize(abilityCardBackSubIcon), ::serialize(abilityCardBackDesc),
        ::serialize(abilitySubItemDesc), ::serialize(abilitySchema));
  }
};

} // namespace xasset
} // namespace db
} // namespace game
