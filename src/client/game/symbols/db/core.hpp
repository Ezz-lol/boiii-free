#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace db {

WEAK symbol<uint32_t[0x26400]> db_hashTable{0x149410980, 0x146904070};
WEAK symbol<void()> DB_Init{0x141422050, 0x1401D7160};
WEAK symbol<void()> DB_AssetPoolInit{0x1413E9F00, 0x14019F170};
WEAK symbol<bool(const char *zoneName, int source)> DB_ValidFastFile{
    0x141420B60, 0x0};
WEAK symbol<void(PMemStack *preloadPMemSide)> DB_ActivatePreloadedMap{
    0x14141F3A0, 0x0};
WEAK symbol<float()> DB_GetLoadedFraction{0x1413EF4B0, 0x0};

} // namespace db
} // namespace game
