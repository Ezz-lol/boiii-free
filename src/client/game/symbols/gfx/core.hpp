#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace gfx {

WEAK symbol<GfxImage *> loadedGfxImage{0x14F463990, 0x147D09590};
WEAK symbol<void(GfxTexture texture, int32_t)> Gfx_TexturePool_ReleaseRef{
    0x141CD6FA0, 0x0};
WEAK symbol<bool(GfxTexture texture)> Gfx_TexturePool_Contains{0x1413EC500,
                                                               0x1401A16E0};
WEAK symbol<void(GfxTexture texture, int32_t)> Gfx_TexturePool_AddRef{
    0x141CD67A0, 0x0};
WEAK symbol<GfxTexture(const char *path)> Gfx_LoadTextureFromPng{0x1420CA6E0,
                                                                 0x1404E2CC0};
WEAK symbol<bool(GfxImage *to, GfxImage *from)> GfxImage_AssignDefaultTexture{
    0x141CAAC40, 0x140383430};
} // namespace gfx
} // namespace game
