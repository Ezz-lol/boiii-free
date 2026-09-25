#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace con {

WEAK symbol<void(char *text, int maxSize)> Con_GetTextCopy{
    0x14133A7F0, 0x14133A7D0, 0x140182C40};
WEAK symbol<void(LocalClientNum_t localClientNum)> Con_DrawInput{
    0x141339900, 0x1413398E0, 140182010};
} // namespace con
} // namespace game
