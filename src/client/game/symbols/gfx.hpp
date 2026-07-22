#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace gfx {

WEAK symbol<GfxImage *> loadedGfxImage{0x14F4E2910, 0x147D09590};
WEAK symbol<void(GfxTexture texture, int32_t)> Gfx_TexturePool_ReleaseRef{
    0x141CE3370};
WEAK symbol<void(GfxTexture texture, int32_t)> Gfx_TexturePool_AddRef{
    0x141CE2B70};
WEAK symbol<GfxTexture(const char *path)> Gfx_LoadTextureFromPng{0x1420D6E60,
                                                                 0x1404E2CC0};
WEAK symbol<bool(GfxImage *to, GfxImage *from)> Image_AssignDefaultTexture{
    0x141CB7010, 0x140383430};
} // namespace gfx
} // namespace game