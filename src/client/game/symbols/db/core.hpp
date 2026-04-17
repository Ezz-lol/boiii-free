#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "../macros.hpp"

namespace game {
namespace db {

WEAK symbol<uint32_t[0x26400]> db_hashTable{0x149410980, 146904070};
WEAK symbol<void()> DB_Init{0x141422030, 0x1401D7160};
WEAK symbol<bool(const char *zoneName, int source)> DB_FileExists{0x141420B40};

} // namespace db
} // namespace game
#endif