#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {

// Re-implementations
bool I_islower(int32_t c);
bool I_isupper(int32_t c);

// Quake functions
WEAK symbol<void(void *Base, size_t NumOfElements, size_t SizeOfElements,
                 _CoreCrtNonSecureSearchSortCompareFunction CompareFunction)>
    qsort{0x142BC39C0, 0x142C3CB30, 0x140AB6020};
// All I_ prefixed functions had a `Q_` prefix in quake.
// They were likely renamed `I_` for "IW" engine.
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcat{
    0x14227C270, 0x1422E9340, 140581110};

WEAK symbol<const char *(char *str)> I_CleanStr{0x14227BF80, 0x1422E9050,
                                                0x140580E80};
WEAK symbol<int32_t(const char *s0, const char *s1)> I_stricmp{
    0x14227C460, 0x1422E9530, 0x140581300};
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcpy{
    0x14227C340, 0x1422E9410, 0x1405811E0};

} // namespace game
