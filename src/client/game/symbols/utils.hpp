#pragma once
#ifdef GAME_HPP

#include <cstddef>
#include "macros.hpp"

namespace game {

// Re-implementations
bool I_islower(int c);
bool I_isupper(int c);

WEAK symbol<const char *(char *str)> I_CleanStr{0x1422E9050, 0x140580E80};
WEAK symbol<int(const char *s0, const char *s1)> I_stricmp{0x1422E9530,
                                                           0x140581300};
WEAK symbol<void(char *dest, size_t destsize, const char *src)> I_strcpy{
    0x1422E9410, 0x1405811E0};

} // namespace game

#endif
