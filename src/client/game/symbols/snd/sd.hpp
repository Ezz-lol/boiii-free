#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace snd {
// SD (Sound Driver) - client only
namespace sd {
WEAK symbol<sd_byte *(const char *name, uint32_t size, uint32_t align)>
    SD_Alloc{0x142208CF0, 0x0};

WEAK symbol<void()> SD_AllocSanity{0x1422090C0, 0x0};
WEAK symbol<sd_byte *(uint64_t alignedSize, const char *name)> SD_AllocBlock{
    0x142208E50, 0x0};
WEAK symbol<void(sd_byte *ptr)> SD_Free{0x142209180, 0x0};
} // namespace sd
} // namespace snd
} // namespace game
