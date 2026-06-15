#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
// Quake functions
WEAK symbol<void(void *Base, size_t NumOfElements, size_t SizeOfElements,
                 _CoreCrtNonSecureSearchSortCompareFunction CompareFunction)>
    qsort{0x142C3CB30, 0x140AB6020};
} // namespace game

#endif
