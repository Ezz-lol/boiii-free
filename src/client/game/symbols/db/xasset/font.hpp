#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace db {
namespace xasset {
namespace font {
WEAK symbol<game::db::xasset::MaterialHandle(int32_t height)>
    FontCache_GetMaterial{0x141CA0570, 0x0, 0x0};
WEAK symbol<const ttf::CachedGlyph *(ttf::TTFDef *ttf, int height,
                                     const int32_t charCode,
                                     FontCacheFXFlags fontCacheFXFlags)>
    FontCache_GetCachedGlyph{0x141C9F710, 0x0, 0x0};
WEAK symbol<int32_t(ttf::TTFDef *ttf, int32_t height,
                    const ttf::CachedGlyph *prevGlyph,
                    const ttf::CachedGlyph *nextGlyph)>
    FontCache_GetKerningAmount{0x141CA02D0, 0x0, 0x0};
} // namespace font
} // namespace xasset
} // namespace db
} // namespace game
