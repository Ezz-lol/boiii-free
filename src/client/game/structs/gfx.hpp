#pragma once

#include <cstdint>
#include "sce/sce.hpp"
namespace game {
namespace gfx {

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
static_assert(sizeof(XPakEntryInfo) == 0x20,
              "XPakEntryInfo size must be 32 bytes");

struct GfxStreamedPartInfo {
  uint32_t levelCountAndSize;
  uint16_t width;
  uint16_t height;
  XPakEntryInfo xpakEntry;
};

struct GfxTexture {
  sce::gnm::Texture basemap;
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
// sizeof=0x130
struct GfxImage {
  GfxStreamedPartInfo streamedParts[4];
  uint8_t streamedPartCount;
  GfxImageSemantic semantic;
  MapType mapType;
  GfxImageCategory category;
  uint8_t useFallback;
  uint8_t _paddingA5[3];
  GfxTexture texture;
  GfxTexture fallbackTexture;
  uint32_t flags;
  bool delayLoadPixels;
  uint8_t _paddingED[1];
  uint16_t width;
  uint16_t height;
  uint16_t depth;
  int32_t alignment;
  uint8_t levelCount;
  uint8_t fallbackLevel;
  uint8_t streaming;
  uint8_t _paddingFB[5];
  uint8_t *pixels;
  uint8_t *fallbackPixels;
  uint32_t totalSize;
  uint32_t fallbackSize;
  GfxPixelFormat format;
  uint8_t _padding11C[4];
  const char *name;
  uint32_t hash;
  uint8_t _padding12C[4];
};
static_assert(sizeof(GfxImage) == 0x130, "GfxImage size must be 304 bytes");
#pragma pack(pop)
} // namespace gfx
} // namespace game