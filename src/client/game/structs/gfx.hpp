#pragma once

#include <cstdint>
#include "core.hpp"
#include "quake.hpp"
namespace game {
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
  int flags;
  int format;
  int resourceSize;
  char data[1];
  uint8_t _padding011[3];
};
ASSERT_SIZE(GfxImageLoadDef, 0x14);
#pragma pack(pop)

union GfxTexture {
  ID3D11ShaderResourceView *basemap;
  GfxImageLoadDef *loadDef;
};

enum class GfxImageSemantic : int8_t {
  IMG_SEMANTIC_UNKNOWN = 0x0,
  IMG_SEMANTIC_2D = 0x1,
  IMG_SEMANTIC_DIFFUSE_MAP = 0x2,
  IMG_SEMANTIC_EFFECT_MAP = 0x3,
  IMG_SEMANTIC_NORMAL_MAP = 0x4,
  IMG_SEMANTIC_SPECULAR_MASK = 0x5,
  IMG_SEMANTIC_SPECULAR_MAP = 0x6,
  IMG_SEMANTIC_GLOSS_MAP = 0x7,
  IMG_SEMANTIC_OCCLUSION_MAP = 0x8,
  IMG_SEMANTIC_REVEAL_MAP = 0x9,
  IMG_SEMANTIC_MULTIPLE_MASK = 0xA,
  IMG_SEMANTIC_THICKNESS_MAP = 0xB,
  IMG_SEMANTIC_CAMO_MAP = 0xC,
  IMG_SEMANTIC_ONE_CHANNEL = 0xD,
  IMG_SEMANTIC_TWO_CHANNEL = 0xE,
  IMG_SEMANTIC_EMBLEM = 0xF,
  IMG_SEMANTIC_CUSTOM = 0x10,
  IMG_SEMANTIC_LUT_TPAGE = 0x11,
  IMG_SEMANTIC_LIGHT_COOKIE = 0x12,
  IMG_SEMANTIC_HDR = 0x13,
  IMG_SEMANTIC_EYE_CAUSTIC = 0x14,
  IMG_SEMANTIC_COUNT = 0x15,
};

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
  int alignment;
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
} // namespace gfx
} // namespace game