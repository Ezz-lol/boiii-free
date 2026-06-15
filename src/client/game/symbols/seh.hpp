#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace seh {
WEAK symbol<const char *(const char *reference)> SEH_SafeTranslateString{
    0x142279510};
}
} // namespace game
#endif