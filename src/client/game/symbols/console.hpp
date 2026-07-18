#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace con {

WEAK symbol<void(char *text, int maxSize)> Con_GetTextCopy{0x14133A7D0,
                                                           0x140182C40};
}
} // namespace game
