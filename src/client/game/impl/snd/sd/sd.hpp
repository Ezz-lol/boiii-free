#pragma once
#include <game/game.hpp>
#include <cstdint>
namespace game {
namespace snd {
namespace sd {
sd_byte *SD_HeapAlloc(const char *name, uint32_t size, uint32_t align);
void SD_HeapFree(sd_byte *ptr);
template <typename T>
  requires(!std::same_as<T, sd_byte>)
inline void SD_HeapFree(T *ptr) {
  return SD_HeapFree(reinterpret_cast<sd_byte *>(ptr));
}

inline void SD_HeapFree(const uintptr_t ptr) {
  return SD_HeapFree(reinterpret_cast<sd_byte *>(ptr));
}

inline void SD_HeapFree(const intptr_t ptr) {
  return SD_HeapFree(static_cast<uintptr_t>(ptr));
}

} // namespace sd
} // namespace snd
} // namespace game