#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace con {

WEAK symbol<void(char *text, int maxSize)> Con_GetTextCopy{0x14133A7D0,
                                                           0x140182C40};
}
} // namespace game

#endif