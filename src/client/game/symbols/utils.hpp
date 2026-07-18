#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstddef>

namespace game {

// Re-implementations
bool I_islower(int32_t c);
bool I_isupper(int32_t c);

WEAK symbol<const char *(char *str)> I_CleanStr{0x1422E9050, 0x140580E80};
WEAK symbol<int32_t(const char *s0, const char *s1)> I_stricmp{0x1422E9530,
                                                               0x140581300};
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcpy{
    0x1422E9410, 0x1405811E0};

} // namespace game