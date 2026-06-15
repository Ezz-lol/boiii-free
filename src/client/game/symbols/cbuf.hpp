#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cbuf {
WEAK symbol<void(int localClientNum, const char *text)> Cbuf_AddText{
    0x1420EC010, 0x1404F75B0};
WEAK symbol<void(int localClientNum, ControllerIndex_t controllerIndex,
                 const char *buffer)>
    Cbuf_ExecuteBuffer{0x14133BE10, 0x1404F78D0};
} // namespace cbuf
} // namespace game

#endif