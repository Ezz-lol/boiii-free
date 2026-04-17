#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace snd {
// SD (Sound Driver) - client only
namespace sd {
WEAK symbol<sd_byte *(const char *name, uint32_t size, uint32_t align)>
    SD_Alloc{0x142265820};
WEAK symbol<void(sd_byte *ptr)> SD_Free{0x142265CB0};
} // namespace sd
} // namespace snd
} // namespace game

#endif