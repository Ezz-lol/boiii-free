#pragma once
#include "../../../../game/game.hpp"
#include <cstdint>
namespace game {
namespace snd {
namespace sd {
sd_byte *SD_HeapAlloc(const char *name, uint32_t size, uint32_t align);
void SD_HeapFree(sd_byte *ptr);
} // namespace sd
} // namespace snd
} // namespace game