#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
// Quake functions
WEAK symbol<void(void *Base, size_t NumOfElements, size_t SizeOfElements,
                 _CoreCrtNonSecureSearchSortCompareFunction CompareFunction)>
    qsort{0x142C3CB30, 0x140AB6020};
// All I_ prefixed functions had a `Q_` prefix in quake.
// They were likely renamed `I_` for "IW" engine.
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcat{
    0x1422E9340, 140581110};
} // namespace game

#endif