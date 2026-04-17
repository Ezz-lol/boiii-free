#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace info {
WEAK symbol<const char *(const char *, const char *key)> Info_ValueForKey{
    0x1422E87B0};
WEAK symbol<void(char *s, const char *key, const char *value)>
    Info_SetValueForKey{0x1422E8410, 0x1405802D0};

} // namespace info
} // namespace game

#endif