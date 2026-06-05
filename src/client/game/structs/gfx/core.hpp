#pragma once

#include <cstdint>
#include "../core.hpp"
#include "../quake.hpp"
#include "../db/xpak.hpp"
#include "HLSL.hpp"
namespace game {

namespace scr {
typedef uint32_t ScrString_t;
}

namespace db {
namespace xasset {
struct Material;
struct XModel;
typedef XModel *XModelPtr;
} // namespace xasset
} // namespace db

namespace gfx {

enum class GfxCameraRegion : int8_t {
  CAMERA_REGION_GBUFFER_DEPTH_HACK = 0x0,
  CAMERA_REGION_GBUFFER_DEPTH_HACK_DECAL = 0x1,
  CAMERA_REGION_GBUFFER = 0x2,
  CAMERA_REGION_GBUFFER_DECAL = 0x3,
  CAMERA_REGION_GBUFFER_NO_VOLUME_DECAL = 0x4,
  CAMERA_REGION_LIT_OPAQUE = 0x5,
  CAMERA_REGION_LIT_TRANS = 0x6,
  CAMERA_REGION_LIT_TRANS_POST_RESOLVE = 0x7,
  CAMERA_REGION_LIT_POST_RESOLVE = 0x8,
  CAMERA_REGION_WATER = 0x9,
  CAMERA_REGION_EMISSIVE_FX = 0xA,
  CAMERA_REGION_DEPTH_HACK_SSS = 0xB,
  CAMERA_REGION_FORWARD_SSS = 0xC,
  CAMERA_REGION_DEPTH_HACK = 0xD,
  CAMERA_REGION_DEPTH_HACK_TRANS = 0xE,
  CAMERA_REGION_DEPTH_HACK_POST_BLUR = 0xF,
  CAMERA_REGION_SONAR = 0x10,
  CAMERA_REGION_OIT = 0x11,
  CAMERA_REGION_DEPTH_PRIME = 0x12,
  CAMERA_REGION_UNDERWATER = 0x13,
  CAMERA_REGION_COUNT = 0x14,
  CAMERA_REGION_NONE = 0x14,
};

struct GfxDrawSurfFields {
  uint64_t useSiegeTextures : 1;
  uint64_t objectId : 18;
  uint64_t lateDepthSort : 5;
  uint64_t materialSortedIndex : 16;
  uint64_t techniqueIndexPlusOne : 11;
  uint64_t decalSurfSort : 6;
  uint64_t earlyDepthSort : 7;
};

union GfxSortKey {
  GfxDrawSurfFields fields;
  uint64_t packed;
};

#pragma pack(push, 1)
// sizeof=0x20
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
ASSERT_SIZE(XPakEntryInfo, 0x20);
#pragma pack(pop)

#pragma pack(push, 1)
struct GfxStreamedPartInfo {
  uint32_t levelCountAndSize;
  uint16_t width;
  uint16_t height;
  XPakEntryInfo xpakEntry;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
};
#pragma pack(pop)
typedef struct _GUID GUID;
typedef GUID IID;

struct IUnknown;

#pragma pack(push, 8)
struct IUnknownVtbl {
  stdcall_t<int32_t, IUnknown *, const IID *const, void **> QueryInterface;
  stdcall_t<uint32_t, IUnknown *> AddRef;
  stdcall_t<uint32_t, IUnknown *> Release;
};
ASSERT_SIZE(IUnknownVtbl, 0x18);
#pragma pack(pop)

#pragma pack(push, 8)
struct IUnknown {
  IUnknownVtbl *lpVtbl;
};
#pragma pack(pop)

struct ID3D11DeviceChild : IUnknown {};

struct ID3D11View : ID3D11DeviceChild {};

struct ID3D11ShaderResourceView : ID3D11View {};

// Unverified
#pragma pack(push, 1)
struct GfxImageLoadDef {
  char levelCount;
  char numElements;
  char pad[2];
  int32_t flags;
  int32_t format;
  int32_t resourceSize;
  char data[1];
  uint8_t _padding011[3];
};
ASSERT_SIZE(GfxImageLoadDef, 0x14);
#pragma pack(pop)

union GfxTexture {
  ID3D11ShaderResourceView *basemap;
  GfxImageLoadDef *loadDef;
};

enum class GfxImageSemantic : uint8_t {
  UNKNOWN = 0x0,
  TWO_DIMENSIONAL = 0x1,
  DIFFUSE_MAP = 0x2,
  EFFECT_MAP = 0x3,
  NORMAL_MAP = 0x4,
  SPECULAR_MASK = 0x5,
  SPECULAR_MAP = 0x6,
  GLOSS_MAP = 0x7,
  OCCLUSION_MAP = 0x8,
  REVEAL_MAP = 0x9,
  MULTIPLE_MASK = 0xA,
  THICKNESS_MAP = 0xB,
  CAMO_MAP = 0xC,
  ONE_CHANNEL = 0xD,
  TWO_CHANNEL = 0xE,
  EMBLEM = 0xF,
  CUSTOM = 0x10,
  LUT_TPAGE = 0x11,
  LIGHT_COOKIE = 0x12,
  HDR = 0x13,
  EYE_CAUSTIC = 0x14,
  COUNT = 0x15,
};

enum class GfxPixelFormat : uint32_t {
  INVALID = 0x0,
  UNKNOWN = 0x0,
  D24S8 = 0x1,
  D24FS8 = 0x2,
  A8 = 0x3,
  R8_UN = 0x4,
  R8_UI = 0x5,
  D16 = 0x6,
  R16_UN = 0x7,
  R16_UI = 0x8,
  R16F = 0x9,
  R8G8_UN = 0xA,
  R8G8_SNORM = 0xB,
  B5G6R5 = 0xC,
  B4G4R4A4 = 0xD,
  D32F = 0xE,
  R32F = 0xF,
  R16G16_UN = 0x10,
  R16G16_SNORM = 0x11,
  R16G16F = 0x12,
  R9G9B9E5 = 0x13,
  R8G8B8A8 = 0x14,
  R8G8B8A8_SNORM = 0x15,
  B8G8R8A8 = 0x16,
  R8G8B8A8_SRGB = 0x17,
  R10G10B10A2 = 0x18,
  B10G10R10A2 = 0x19,
  R11G11B10F = 0x1A,
  R32G32F = 0x1B,
  R16G16B16A16_UN = 0x1C,
  R16G16B16A16F = 0x1D,
  R32G32B32A32F = 0x1E,
  BC1 = 0x1F,
  BC1_SRGB = 0x20,
  BC2 = 0x21,
  BC2_SRGB = 0x22,
  BC3 = 0x23,
  BC3_SRGB = 0x24,
  BC4 = 0x25,
  BC5 = 0x26,
  BC6_UH = 0x27,
  BC6_SH = 0x28,
  BC7 = 0x29,
  BC7_SRGB = 0x2A,
  R32G32B32A32_UN = 0x2B,
  R32_UI = 0x2C,
  R32G32_UI = 0x2D,
};

typedef uint8_t MapType;
typedef uint8_t GfxImageCategory;
#pragma pack(push, 1)
struct GfxImage {
  GfxStreamedPartInfo streamedParts[4];
  uint8_t streamedPartCount;
  GfxImageSemantic semantic;
  MapType mapType;
  GfxImageCategory category;
  qboolean useFallback;
  GfxTexture texture;
  GfxTexture fallbackTexture;
  uint32_t flags;
  bool delayLoadPixels;
  uint8_t _paddingBD[1];
  uint16_t width;
  uint16_t height;
  uint16_t depth;
  int32_t alignment;
  uint8_t levelCount;
  uint8_t fallbackLevel;
  bool streaming;
  uint8_t _paddingCB[5];
  uint8_t *pixels;
  uint8_t *fallbackPixels;
  uint32_t totalSize;
  uint32_t fallbackSize;
  GfxPixelFormat format;
  uint8_t _paddingEC[4];
  const char *name;
  uint32_t hash;
  uint8_t _paddingFC[4];
  // This might not be here, but the struct has size
  // 0x100 otherwise - correct size is known to be 0x108.
  uint8_t _unknown100[8];
};
ASSERT_SIZE(GfxImage, 0x108);
#pragma pack(pop)

typedef GfxImage *GfxImageHandle;
typedef GfxImage *GfxImagePtr;

typedef uint8_t GfxImageCategory;

union GfxColor {
  uint32_t packed;
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  } rgba;
  uint8_t array[4];
};

// TODO
struct GfxLightDescription;
struct GfxLightDef;

struct GfxSkinCacheEntry {
  uint32_t frameCount;
  int32_t skinnedCachedOffset;
  uint16_t numSkinnedVerts;
  uint16_t ageCount;
};

struct GpuShaderConstantSet {
  vec4_t scriptVector0;
  vec4_t scriptVector1;
  vec4_t scriptVector2;
  vec4_t scriptVector3;
  vec4_t scriptVector4;
  vec4_t scriptVector5;
  vec4_t scriptVector6;
  vec4_t scriptVector7;
  vec4_t weaponParam0;
  vec4_t weaponParam1;
  vec4_t weaponParam2;
  vec4_t weaponParam3;
  vec4_t weaponParam4;
  vec4_t weaponParam5;
  vec4_t weaponParam6;
  vec4_t weaponParam7;
  vec4_t characterParam0;
  vec4_t characterParam1;
  vec4_t characterParam2;
  vec4_t characterParam3;
  vec4_t characterParam4;
  vec4_t characterParam5;
  vec4_t characterParam6;
  vec4_t characterParam7;
};
ASSERT_SIZE(GpuShaderConstantSet, 0x180);

struct ShaderConstantSet {
  GpuShaderConstantSet values;
  uint32_t used;
};
ASSERT_SIZE(ShaderConstantSet, 0x184);

union GfxSimpleBuffer {
  struct {
    uint64_t m_data : 36;
    uint64_t m_size : 28;
  };
  uint64_t m_packed;
};

typedef GfxSimpleBuffer GfxIndexBuffer;
typedef GfxSimpleBuffer GfxVertexBuffer;

class ID3D11Resource : ID3D11DeviceChild {};

class ID3D11Buffer : ID3D11Resource {};

struct GfxBuffer {
  ID3D11Buffer ro;
  ID3D11Buffer rw;
};

#pragma pack(push, 1)
struct BlendTargetRemap {
  scr::ScrString_t name;
  uint16_t index;
  uint8_t _padding06[2];
};
#pragma pack(pop)

struct GfxExtraCamActiveState {
  bool anyActive;
  bool active[4];
  const GfxImage *renderTarget[4];
};

enum class eRenderMethod : uint32_t {
  OFF = 0x0,
  DEFAULT_MATERIAL = 0x1,
  THERMAL_MATERIAL = 0x2,
  CUSTOM_MATERIAL = 0x3,
  FORCE_CUSTOM_MATERIAL = 0x4,
  COUNT = 0x5,
};

enum class eRenderCulling : uint32_t {
  CULL = 0x0,
  NO_CULL = 0x1,
  COUNT = 0x2,
};

enum class PostfxAnimation : uint32_t {
  hold = 0x0,
  linear = 0x1,
  step = 0x2,
  ease_in = 0x3,
  ease_out = 0x4,
  ease_inout = 0x5,
  linear_repeat = 0x6,
  linear_mirror = 0x7,
  sin = 0x8,
  COUNT = 0x9,
};

enum class PostfxAngle : uint32_t {
  off = 0x0,
  sin = 0x1,
  cos = 0x2,
  tan = 0x3,
};

struct PostfxConstant {
  PostfxAnimation anm[4];
  int32_t channels;
  vec4_t delay;
  vec4_t end_clr;
  vec4_t end;
  int32_t scriptVectorIndex;
  vec4_t start_clr;
  vec4_t start;
  PostfxAngle angle[4];
  const char *scriptVectorName[4];
};

struct PostfxStage {
  float length;
  db::xasset::Material *material;
  eRenderMethod method;
  eRenderCulling cull;
  int32_t numConsts;
  int32_t screenCaptureStage;
  int32_t spriteFilter;
  int32_t thermal;
  PostfxConstant *constant;
};

struct PostfxHeader {
  int32_t enterStage;
  int32_t exitStage;
  int32_t finishLoopOnExit;
  int32_t firstPersonOnly;
  int32_t looping;
  int32_t numStages;
  int32_t screenCapture;
  PostfxStage *stage;
};

struct GfxDuplicateRenderOption;
struct GfxDuplicateRenderOption {
  struct Target {
    PostfxHeader *header;
    float time;
    int32_t stage;
    db::xasset::Material *customMaterial;
    eRenderMethod renderMethod;
    eRenderCulling renderCulling;
  };

  enum class Type : int8_t {
    GLOBAL = 0x0,
    SUBMODEL = 0x1,
    ANY = 0x2,
  };

  Target targets[3];
  volatile int32_t inUse;
  Type type;
  uint8_t priority;
  uint16_t index;
  scr::ScrString_t submodelBoneName;
  GfxDuplicateRenderOption *nextDupOpt;
};

class GfxConfig_Fog {
public:
  HLSL::float3 _fogcolor;
  float fogopacity;
  float fogintensity;
  float basedist;
  float halfdist;
  float baseheight;
  float halfheight;
  float skyhalfheightoffset;
  HLSL::float3 _sunfogcolor;
  float sunfogopacity;
  float sunfogintensity;
  float sunfoginner;
  float sunfogouter;
  float sunPitchOffset;
  float sunYawOffset;
  HLSL::float3 _atmospherefogcolor;
  float atmospherefogdensity;
  HLSL::float3 _atmospherehazecolor;
  float atmospherehazebasedist;
  float atmospherehazefadedist;
  float atmospherehazedensity;
  float atmospherehazespread;
  float atmosphereinscatterstrength;
  float atmosphereextinctionstrength;
  float atmospheresunstrength;
  float atmospherepbramount;
  float worldfogskysize;
};

struct GfxVBLOOMGPU {
  vec4_t vRGB1;
  vec4_t vLUM1;
  vec4_t vRGB2;
  vec4_t vLUM2;
  vec4_t vRGB3;
  vec4_t vLUM3;
  vec4_t vRGB4;
  vec4_t vLUM4;
  vec4_t vRGB5;
  vec4_t vLUM5;
  vec4_t vLIB;
  vec4_t vLIW;
  vec4_t vLIG;
  vec4_t vLOB;
  vec4_t vLOW;
  vec4_t vMXR;
  vec4_t vMXG;
  vec4_t vMXB;
  char nLVI[32];
};
struct GfxProbeVolumeTextures {
  GfxImagePtr X;
  GfxImagePtr Y;
  GfxImagePtr Z;
  db::xpak::StreamWrappedBuffer *pixelData;
};

struct GfxSkyBoxImage {
  GfxImagePtr image;
  db::xpak::StreamWrappedBuffer pixelData;
};

struct GfxSkyBox {
  db::xasset::XModelPtr model;
  float rotation;
  float size;
  bool dynamicCookie;
  GfxSkyBoxImage *image;
};

struct __attribute__((aligned(8))) GfxExposureProbeBlend {
  vec3_t pos;
  vec3_t size;
  vec3_t blendMins;
  vec3_t blendMaxs;
  vec3_t angles;
  uint32_t numFaces;
  vec4_t *faces;
  bool negative;
};

struct __attribute__((aligned(8))) GfxExposureProbe {
  vec3_t pos;
  vec3_t size;
  vec3_t angles;
  float value;
  uint16_t dX;
  uint16_t dY;
  uint16_t dZ;
  uint16_t *pData;
  uint16_t numBlends;
  GfxExposureProbeBlend *blends;
  bool isOverride;
};

struct GfxExposureGrid {
  uint32_t nProbe;
  float baseValue;
  GfxExposureProbe *pEprobe;
};

struct GfxANIMTRACK {
  uint32_t cFLAG;
  float fLength;
  float fLoopStart;
  float fLoopEnd;
  vec4_t vTrack1_Mix[32];
  vec4_t vTrack1_Lrp[32];
  vec4_t vTrack2_Mix[32];
  vec4_t vTrack2_Lrp[32];
};

struct GfxCOLORDATA {
  vec4_t vTrack1_RangeSHAD_SPEC_MULADD;
  vec4_t vTrack1_RangeMIDT_MULADD;
  vec4_t vTrack1_RangeHIGH_MULADD;
  vec4_t vTrack1_MtrxSHAD_R;
  vec4_t vTrack1_MtrxSHAD_G;
  vec4_t vTrack1_MtrxSHAD_B;
  vec4_t vTrack1_MtrxMIDT_R;
  vec4_t vTrack1_MtrxMIDT_G;
  vec4_t vTrack1_MtrxMIDT_B;
  vec4_t vTrack1_MtrxHIGH_R;
  vec4_t vTrack1_MtrxHIGH_G;
  vec4_t vTrack1_MtrxHIGH_B;
  vec4_t vTrack1_MtrxSPEC_R;
  vec4_t vTrack1_MtrxSPEC_G;
  vec4_t vTrack1_MtrxSPEC_B;
  vec4_t vTrack1_ColorMUL;
  vec4_t vTrack1_ColorADD;
  vec4_t vTrack1_ColorPOW;
  vec4_t vTrack1_MtrxFINAL_R;
  vec4_t vTrack1_MtrxFINAL_G;
  vec4_t vTrack1_MtrxFINAL_B;
  vec4_t vTrack2_RangeSHAD_SPEC_MULADD;
  vec4_t vTrack2_RangeMIDT_MULADD;
  vec4_t vTrack2_RangeHIGH_MULADD;
  vec4_t vTrack2_MtrxSHAD_R;
  vec4_t vTrack2_MtrxSHAD_G;
  vec4_t vTrack2_MtrxSHAD_B;
  vec4_t vTrack2_MtrxMIDT_R;
  vec4_t vTrack2_MtrxMIDT_G;
  vec4_t vTrack2_MtrxMIDT_B;
  vec4_t vTrack2_MtrxHIGH_R;
  vec4_t vTrack2_MtrxHIGH_G;
  vec4_t vTrack2_MtrxHIGH_B;
  vec4_t vTrack2_MtrxSPEC_R;
  vec4_t vTrack2_MtrxSPEC_G;
  vec4_t vTrack2_MtrxSPEC_B;
  vec4_t vTrack2_ColorMUL;
  vec4_t vTrack2_ColorADD;
  vec4_t vTrack2_ColorPOW;
  vec4_t vTrack2_MtrxFINAL_R;
  vec4_t vTrack2_MtrxFINAL_G;
  vec4_t vTrack2_MtrxFINAL_B;
  GfxANIMTRACK animDATA;
  name_t fileNAME;
};

typedef vec3_t vec3x3[3];

struct __attribute__((aligned(4))) GfxConfig_ProbeBlend {
  HLSL::float3 origin;
  HLSL::float3x3 rotation;
  HLSL::float3 size_min;
  HLSL::float3 size_max;
  HLSL::float3 blend_mins;
  HLSL::float3 blend_maxs;
  uint32_t faceCount;
  vec4_t faces[24];
  float blends[24];
  HLSL::float3 cullOrigin;
  float cullRadius;
  float evcomp;
  bool negative;
};

struct GfxConfig_Probe {
  HLSL::float3 origin;
  HLSL::float3x3 rotation;
  HLSL::float3 size_min;
  HLSL::float3 size_max;
  HLSL::float3 cullOrigin;
  float cullRadius;
  float resolution;
  uint32_t grid_density;
  HLSL::uint3 gridDim;
  float evcomp;
  float brightnessAdjust;
  float ao_range;
  float ao_strength;
  float ao_strength_double_sided;
  float ao_power;
  bool inpaint_occluded_rays;
  uint32_t adaptQuality;
  uint32_t planeCount;
  vec4_t wldClipPlanes[6];
  uint32_t guid;
  bool isDynamic;
  bool isVolumetric;
  bool isOverride;
  uint32_t debug_render_index_x;
  uint32_t debug_render_index_y;
  uint32_t debug_render_index_z;
  uint32_t debug_render_phase;
  uint32_t debugLineIndex;
};

struct GfxReflectionProbe {
  float exposure;
  vec3_t avgCubeColor;
  vec3_t volumeCoordMul;
  vec3_t volumeCoordAdd;
  uint16_t firstBlend;
  uint16_t numBlends;
  GfxConfig_Probe config;
  float exploderFade[4];
  bool exploderDisabled[4];
  int umbraId;
};

struct GfxReflectionProbeArray {
  db::xpak::StreamWrappedBuffer *pixelData;
  GfxImagePtr probeImages;
  GfxReflectionProbe *configs;
  GfxConfig_ProbeBlend *blends;
  uint32_t localReflectionProbeCount;
  uint32_t probeBlendsCount;
};
typedef GfxBuffer GfxStructuredBuffer;

struct __attribute__((aligned(8))) GfxMinMaxTree {
  int mip0Width;
  int mip0Height;
  HLSL::float2 sstTexScale;
  uint32_t sstMinMaxCount;
  uint16_t *sstMaxCPU;
  GfxImagePtr sstMaxGPU;
  float treeMax;
};

struct GfxShadowTreeBuffer {
  uint8_t *elements;
  uint32_t elementStrideBytes;
  uint32_t elementCount;
  GfxStructuredBuffer buffer;
};

struct GfxStreamedShadowTreeBuffer {
  db::xpak::StreamWrappedBuffer elements;
  uint32_t *mipByteOffsets;
  uint32_t *mipElementCounts;
  uint32_t totalElementCount;
  uint32_t mipCount;
  uint32_t mipDropTarget;
  uint32_t mipDropCurrent;
  GfxStructuredBuffer buffer;
  GfxMinMaxTree *maxTree;
  GfxShadowTreeBuffer *fallback;
  GfxMinMaxTree *fallbackMaxTree;
};

typedef GfxStreamedShadowTreeBuffer *GfxStreamedShadowTreeBufferPtr;

struct GfxConfig_SST {
  vec4_t wldToPinTransform[4];
  HLSL::float2 dimensionInTiles;
  float inchesPerTexel;
  float spanInInches;
  float yaw;
  float pitch;
};

struct GfxConfig_SSTMultiRes {
  uint32_t rootOffsets[2];
  uint32_t mipScales[2];
  GfxConfig_SST uniformMip;
};

struct GfxMultiResSST {
  GfxConfig_SSTMultiRes config;
  GfxStreamedShadowTreeBufferPtr shadowTree;
};

struct GfxGlobalAOSettings {
  float strength;
  float low;
  float mid;
  float high;
};

struct GfxGlobalLightSettings {
  float pitch;
  float yaw;
  HLSL::float3 color;
  float intensity;
  float specScale;
  float penumbra_inches;
  char skyboxmodel[64];
  bool shadow;
  bool dynamicShadow;
  float evcmp;
  float evmin;
  float evmax;
  uint32_t bounceCount;
  float sunCookieIntensity;
  char sunCookieLightDefName[64];
  float sunCookieScale;
  float sunCookieAngle;
  float sunCookieRotation;
  HLSL::float2 sunCookieOffset;
  HLSL::float2 sunCookieScroll;
  uint32_t sunCookieIndex;
  float sunVolumetricCookieIntensity;
  uint32_t lensFlareUuidData1;
  uint16_t lensFlareUuidData2;
  uint16_t lensFlareUuidData3;
  uint8_t lensFlareUuidData4[8];
  float lensFlarePitchOffset;
  float lensFlareYawOffset;
  float meterToUnitScale;
  float unitToMeterScale;
  float radiusGround;
  float radiusAtmosphere;
  float groundLevelAdjust;
  float aerialPerspectiveFadeStartR;
  float aerialPerspectiveFadeDistanceR;
  float aerialPerspectiveFadeStartM;
  float aerialPerspectiveFadeDistanceM;
  HLSL::float3 betaR;
  float heightScaleR;
  float densityR;
  HLSL::float3 betaM;
  float heightScaleM;
  float densityM;
  float scatterG;
  vec4_t betaRspectral[7];
  float groundFogBaseHeight;
  float groundFogHalfDistance;
  float groundFogHalfHeight;
  float groundFogDensityScale;
  float groundFogHorizonFadeMUL;
  float groundFogHorizonFadeADD;
  float groundFogNearRGBMaxBlend;
  float groundFogHorizonMaxBlend;
  HLSL::float3 groundFogColor;
  float pbrFogFroxelZnear;
  float pbrFogFroxelZfar;
  vec3_t pbrFogWorldNoiseA_pos;
  vec3_t pbrFogWorldNoiseA_dir;
  vec3_t pbrFogWorldNoiseB_pos;
  vec3_t pbrFogWorldNoiseB_dir;
  GfxGlobalAOSettings aoSettings;
};

struct GfxConfig_Sun {
  float shadowSplitDistance;
  float shadowBiasScale;
  float transitionTime;
  bool streamLighting;
  uint32_t lightStateMask;
  HLSL::float3 globalProbeCenter;
  HLSL::float3 globalProbeFill;
  uint32_t maxGridDimension;
  HLSL::uint3 gridDimensions;
  GfxGlobalLightSettings settings[4];
};

struct __attribute__((aligned(8))) GfxSunVolume {
  int32_t planeStart;
  int32_t planeCount;
  uint32_t lightExportFlags;
  uint32_t lightingStateMask;
  GfxConfig_Sun sun;
  GfxMultiResSST multiResSST[4];
  uint32_t lightCount;
  uint16_t *lights;
  GfxReflectionProbeArray reflectionProbes[4];
  GfxProbeVolumeTextures probeVolumes[4];
  GfxSkyBox skyboxes[4];
  GfxExposureGrid exposureGrid[4];
  uint16_t idxLUT;
  uint16_t idxSKY;
  GfxVBLOOMGPU vbloomSUN[4];
  GfxCOLORDATA sunCOLORDATA[4];
};

enum class GfxLodBias : uint8_t {
  DEFAULT = 0x0,
  POSITIVE = 0x1,
  NEGATIVE = 0x2,
  COUNT = 0x3,
};

struct GfxCameraDataFields {
  uint32_t activeLensID : 8;
  uint32_t activeFilterID : 8;
  uint32_t dofMode : 2;
  uint32_t autoMB : 1;
  uint32_t padding : 13;
};

union GfxCameraDataControl {
  GfxCameraDataFields fields;
  uint32_t packed;
};

struct GfxCameraData {
  GfxCameraDataControl cFlags;
  float focalLength;
  float focalDistance;
  float fstop;
  float shutter;
  float iso;
  float EV;
  float EVcomp;
  float filmCropFactor;
  float filmISO_min;
  float filmISO_max;
  float filmISO_noise;
  float fAdsBlur;
  float fAdsScopeBlurStart;
  float fAdsScopeBlurAmount;
  float mbStrength;
};

struct GfxKeylineFx {
  bool enabled;
  int32_t type;
  float desatLevel;
  float solidLevel;
  float outlineLevel;
  float lineWidth;
  float radius;
  float falloff;
};
struct GfxCatsEyeFx {
  bool enabled;
  float textureLevel;
  float noise;
};
struct __attribute__((aligned(4))) GfxRadialPulse {
  vec4_t color_active;
  vec4_t origin_radius;
  float startTime;
  float maxTime;
  bool enabled;
};
struct GfxMotionPulseFx {
  bool enabled;
  float desatLevel;
  float solidLevel;
  float outlineLevel;
  float lineWidth;
  float radius;
  float falloff;
  float fadeOut;
  float pad;
  float startTime;
  float maxTime;
  float startTimeViewModel;
  float maxTimeViewModel;
};
struct __attribute__((aligned(4))) GfxSpeedBlur {
  float blurAmount;
  float xOffset;
  float yOffset;
  float innerRadius;
  float outerRadius;
  bool enabled;
};
struct __attribute__((aligned(4))) GfxCleanse {
  float falloff;
  float maxTime;
  float startTime;
  bool enabled;
};
struct GfxCleansePulse {
  bool enabled;
};
struct GfxEVFx {
  bool enabled;
  float evGeometryRange;
  float evTargetRange;
};
struct GfxCommonFx {
  bool enabled;
  float var0;
  float var1;
  float var2;
  float var3;
};
struct GfxHotZoneFx {
  bool enabled;
  int32_t number;
  vec4_t positionAngle[24];
};
struct GfxHitZoneFx {
  bool enabled;
  vec4_t positionRadius[8];
  vec4_t color;
  float radius;
  float spinRate;
  float pulseRate;
};
struct GfxRandomFadeFx {
  bool enabled;
  float randomLevel;
};
struct __attribute__((aligned(4))) GfxWeakPointFx {
  bool enabled;
  float type;
  bool changed;
};
struct __attribute__((aligned(4))) GfxTmodeFx {
  bool enabled;
  float type;
  bool empActive;
};
struct GfxFsblurFx {
  bool enabled;
  float level;
};
struct GfxEACPathFx {
  bool enabled;
  int32_t posnIndex;
  vec4_t posn[1024];
};
struct GfxMisdirectionFx {
  bool enabled;
};
struct GfxRadioActive {
  bool enabled;
  float blur;
  float speed;
  float intensity;
};
struct GfxBlurAndTint {
  bool enabled;
  float level;
};
struct GfxDragonStrike {
  bool enabled;
  vec4_t position;
  vec4_t color;
  float radius;
  float spinRate;
  float pulseRate;
  int32_t entnum;
};
struct GfxEvScanlines {
  bool enabled;
  int32_t type;
};
struct __attribute__((aligned(4))) GfxPostFx {
  GfxKeylineFx keyline[2];
  GfxCatsEyeFx catsEye;
  GfxRadialPulse radialPulse;
  GfxMotionPulseFx motionPulse;
  GfxSpeedBlur speedBlur;
  GfxCleanse cleanse;
  GfxCleansePulse cleansePulse;
  GfxEVFx ev;
  GfxCommonFx common;
  GfxHotZoneFx hotZone;
  GfxHitZoneFx hitZone;
  GfxRandomFadeFx randomFade;
  GfxWeakPointFx weakPoint;
  GfxTmodeFx tmode;
  GfxTmodeFx zombieTmode;
  GfxFsblurFx fsblur;
  GfxEACPathFx eacPath;
  GfxMisdirectionFx misdirection;
  GfxKeylineFx keylineFrontend;
  GfxKeylineFx keylineFrontendLocked;
  GfxKeylineFx keylineFrontendTokenLocked;
  GfxKeylineFx keylineDepth;
  GfxRadioActive radioActive;
  GfxBlurAndTint blurAndTint;
  GfxKeylineFx playerKeyline;
  GfxDragonStrike dragonStrike[4];
  GfxEvScanlines evScanlines;
  bool IGCActive;
  bool disablePostFxForFreeCamera;
};

struct __attribute__((aligned(8))) GfxGenericFilterPass {
  bool enabled;
  const db::xasset::Material *material;
  const GfxImage *codeTextures[2];
  float params[32];
  int32_t numQuads;
};

struct GfxGenericFilter {
  bool enabled;
  bool firstPersonOnly;
  bool inHDR;
  uint32_t flags;
  GfxGenericFilterPass passes[4];
  PostfxHeader *header;
  const char *bundleName;
  float time;
  int stage;
};

struct GfxGenericFilters {
  GfxGenericFilter filters[11];
};

enum class LightType : uint32_t {
  NONE = 0x0,
  DIR = 0x1,
  BOX = 0x2,
  BALL = 0x4,
  ALL_MASK = 0x7,
};

enum class ShadowQuality : uint32_t {
  NONE = 0x1,
  SST_ONLY = 0x2,
  HARD = 0x3,
  PCSS = 0x4,
  INVALID = 0x0,
};

enum class ShadowUpdateType : uint32_t {
  ALWAYS_NOCACHE = 0x0,
  ALWAYS = 0x1,
  NEVER = 0x2,
};

enum class ShadowResolutionScaleType : uint32_t {
  SHADOW_DS_4_0X = 0x0,
  SHADOW_DS_2_0X = 0x1,
  SHADOW_DS_1_0X = 0x2,
  SHADOW_DS_0_5X = 0x3,
  SHADOW_DS_COUNT = 0x4,
};

struct __attribute__((aligned(4))) ComCookieInfo {
  uint32_t textureIndex;
  bool wrap;
};
struct GfxSceneLightStats {
  struct Stats {
    uint32_t triCount;
    uint32_t viewmodelTriCount;
    float addSurfsMs;
    uint64_t lastFrameUpdate;
  };

  Stats curr;
  Stats prev;
};

struct GfxConfig_Light {
  vec4_t nonSunShadowTransform[4];
  LightType type;
  uint32_t guid;
  HLSL::float3 origin;
  HLSL::float3 wldDir;
  HLSL::float3 rvec;
  HLSL::float3 uvec;
  HLSL::float3 _color;
  float specScale;
  bool probeOnly;
  float cosHalfFov;
  float fov_feather;
  float cut_on;
  float radius;
  float feather;
  float areaOffset;
  HLSL::float2 bulbArea;
  HLSL::float3 size_min;
  HLSL::float3 size_max;
  HLSL::float3 blend_mins;
  HLSL::float3 blend_maxs;
  ShadowQuality shadowQuality;
  bool mExcludeNeverUpdateFromDedicated;
  bool mShadowUseFxAxis;
  bool spotShadows;
  float penumbra_inches;
  ShadowUpdateType shadowUpdate;
  ShadowResolutionScaleType mShadowResolutionScale;
  vec4_t cookieTransform[2];
  char cookieName[64];
  float cookieAngle;
  float cookieRotation;
  HLSL::float2 cookieScale;
  HLSL::float2 cookieScroll;
  HLSL::float2 cookieOffset;
  bool volumetric;
  bool volumetricCookies;
  uint32_t volumetricSampleCount;
  float volumetricIntensityScale;
  uint32_t lightStateMask;
  float mCullingFalloff;
  float mCullingCutoff;
  float cullRadius;
  HLSL::float3 wldCullMin;
  HLSL::float3 wldCullMax;
  uint32_t lightPriority;
  uint32_t shadowPriority;
  int umbraId;
  vec3_t mAxisFromFx[3];
  bool exploderDisabled;
  float exploderFade;
};

class __attribute__((aligned(4))) GfxConfig_Lensflare {
public:
  vec3_t pos;
  bool isEnabled;
  char lensFlareDefName[64];
};

struct GfxSceneLight {
  GfxConfig_Light light;
  ComCookieInfo cookieInfo;
  bool isDynamic;
  GfxSceneLightStats runtimeStats;
};

struct GfxFilm {
  float vkTT;
  float vkTS;
  vec4_t vkTC;
  vec3_t vkTO;
  vec3_t vkRGB0;
  float vkL0;
  float vkM0;
  vec3_t vkRGB1;
  float vkL1;
  float vkM1;
  vec3_t vkRGB2;
  float vkL2;
  vec3_t vkRGB3;
  float vkL3;
  float vkM3;
  vec3_t vkRGB4;
  float vkL4;
  float vkM4;
  float vkRM;
  int16_t lutA;
  int16_t lutB;
  int16_t lutC;
  int16_t lutD;
  vec4_t lutBlendWeights;
  vec4_t sunFlareTint;
};

struct GfxReviveFx {
  bool enabled;
  float reviveEdgeColorTemp;
  float reviveEdgeSaturation;
  vec3_t reviveEdgeScale;
  vec3_t reviveEdgeContrast;
  vec3_t reviveEdgeOffset;
  float reviveEdgeMaskAdjust;
  float reviveEdgeAmount;
};

struct GfxVisionSet {
  GfxFilm film;
  GfxReviveFx reviveFx;
};

struct GfxDoubleVision {
  vec3_t direction;
  float motionBlurMagnitude;
  float deltaPerMS;
  float cur;
  float targ;
};

struct __attribute__((aligned(4))) GfxCompositeFx {
  vec2_t distortionScale;
  float blurRadius;
  float distortionMagnitude;
  float frameRate;
  int lastUpdate;
  int frame;
  int startMSec;
  int currentTime;
  int duration;
  bool enabled;
  bool scriptEnabled;
};

struct GfxPoison {
  float curAmountTarget;
  float curAmount;
};

struct GfxViewport {
  int x;
  int y;
  int width;
  int height;
};

struct WaterFogDef {
  int startTime;
  int finishTime;
  vec4_t color;
  float fogStart;
  float density;
  float heightDensity;
  float baseHeight;
  vec4_t sunFogColor;
  vec3_t sunFogDir;
  float sunFogStartAng;
  float sunFogEndAng;
};

struct GfxRippleWave {
  vec3_t wavePosition;
  float waveWidth;
  float waveSpeedScale;
  float waveAmplitudeRatio;
  float waveInnerStart;
  float waveInnerEnd;
  float waveOuterStart;
  float waveOuterEnd;
  int waveTimeStamp;
};

struct __attribute__((aligned(4))) GfxDepthOfField {
  float viewModelStart;
  float viewModelEnd;
  float nearStart;
  float nearEnd;
  float farStart;
  float farEnd;
  float nearBlur;
  float farBlur;
  bool useAltMaterial;
};

struct __attribute__((aligned(4))) GfxSaveScreenParam {
  float s0;
  float t0;
  float ds;
  float dt;
  LocalClientNum_t screenTimerId;
  uint8_t mode;
};

struct __attribute__((aligned(4))) GfxBlendSaveScreenBlurredParam {
  int32_t fadeMsec;
  float s0;
  float t0;
  float ds;
  float dt;
  int32_t screenTimerId;
  uint8_t enabled;
};

struct __attribute__((aligned(4))) GfxBlendSaveScreenFlashedParam {
  float intensityWhiteout;
  float intensityScreengrab;
  float s0;
  float t0;
  float ds;
  float dt;
  int32_t screenTimerId;
  uint8_t enabled;
};

struct GfxSaveScreenFx {
  GfxSaveScreenParam saveScreenParam;
  GfxBlendSaveScreenBlurredParam blendBlurredParam;
  GfxBlendSaveScreenFlashedParam blendFlashedParam;
};

struct GfxDataBuffer {
  GfxTexture image;
};

struct __attribute__((aligned(8))) GfxSiegeAnimUncompressed {
  db::xpak::StreamWrappedBuffer data;
  GfxDataBuffer base;
  GfxDataBuffer quat;
  GfxDataBuffer pos;
  uint32_t baseStride;
  uint32_t quatStride;
  uint32_t posStride;
  uint32_t baseOffset;
  uint32_t quatOffset;
  uint32_t posOffset;
  uint32_t width;
  uint32_t height;
  uint32_t totalSize;
  GfxPixelFormat quatFormat;
  GfxPixelFormat posFormat;
};

struct GfxSplineOffsets {
  uint32_t rotKeyStart;
  uint32_t transKeyStart;
  uint16_t rotKeyCount;
  uint16_t transKeyCount;
};

struct GfxSiegeSplineRotKey {
  vec4_t p0;
  vec4_t p1;
  vec4_t p2;
};

struct GfxSiegeSplineTransKey {
  vec3_t p0;
  vec3_t p1;
  vec3_t p2;
};

struct __attribute__((aligned(8))) GfxSiegeAnimCompressed {
  GfxSplineOffsets *splineOffsets;
  uint16_t *splineRotFrames;
  uint16_t *splineTransFrames;
  GfxSiegeSplineRotKey *splineRotKey;
  GfxSiegeSplineTransKey *splineTransKey;
  vec4_t *splineTransBase;
  uint32_t numSplineOffsets;
  uint32_t numRotKeys;
  uint32_t numTransKeys;
};

struct GfxSiegeAnimShot {
  uint16_t startFrame;
  uint16_t endFrame;
  uint16_t flags;
  scr::ScrString_t name;
};

struct GfxSiegeAnimEvent {
  float frame;
  vec3_t pos;
  scr::ScrString_t node;
  scr::ScrString_t trigger;
  scr::ScrString_t param;
};

struct __attribute__((aligned(4))) XBoneInfo {
  vec3_t bounds[2];
  vec3_t offset;
  float radiusSquared;
  uint8_t collmap;
};

struct __attribute__((aligned(4))) GfxSiegeAnim {
  const char *name;
  GfxSiegeAnimUncompressed *uncomp;
  GfxSiegeAnimCompressed *comp;
  GfxSiegeAnimShot *shots;
  GfxSiegeAnimEvent *events;
  XBoneInfo bonesBounds;
  uint32_t numBones;
  uint32_t numFrames;
  float timeScale;
  scr::ScrString_t scrName;
  uint16_t flags;
  uint16_t numShots;
  uint16_t numEvents;
  uint16_t startupEventsEnd;
  uint16_t shutdownEventsEnd;
};

struct FastCriticalSection {
  volatile int32_t readCount;
  volatile int32_t writeCount;
};

struct GfxPackedPlacement {
  vec3_t origin;
  vec3_t axis[3];
  float scale;
};

struct GfxStaticModelBone {
  vec4_t m[3];
  vec4_t extra;
};

struct GfxSiegeAnimState;
struct GfxStaticModelDrawInst {
  vec3_t mins;
  vec3_t maxs;
  db::xasset::XModel *model;
  uint8_t hidden;
  uint8_t noVolumeDecal;
  uint16_t modelSeed;
  uint16_t smid;
  uint16_t numPosedBones;
  uint32_t constantOverride;
  float scaledRadius;
  union {
    GfxSiegeAnimState *sanimPlayback;
    GfxSiegeAnim *sanim;
  };
  GfxPackedPlacement placement;
  uint16_t flags;
  int16_t mapGateIndex;
  GfxStaticModelBone *posedBones;
  float invScaleSq;
  scr::ScrString_t targetname;
};

struct GfxSiegeAnimState {

  struct EntInfo {
    uint16_t entNo;
    LocalClientNum_t localClientNum;
  };

  struct __attribute__((aligned(8))) Anim {
    GfxSiegeAnim *anim;
    GfxSiegeAnimShot shot;
    float tStart;
    uint32_t flags;
  };

  Anim anims[4];
  int animNow;
  int animPlay;
  float tNow;
  float tNext;
  float timeScale;
  uint16_t numEvents;
  uint16_t flags;
  float blendAmount;
  float blendTChange;
  uint32_t boneArrayOffsetFrameAllocated;
  uint32_t boneArrayOffset;
  FastCriticalSection critSec;
  union {
    GfxStaticModelDrawInst *inst;
    EntInfo entInfo;
  } u;
  GfxSiegeAnimState *blendSource;
};

} // namespace gfx
} // namespace game