#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace render {
// Rendering
WEAK symbol<void(const char *, int, const void *, float, float, float, float,
                 float, const float *, int)>
    R_AddCmdDrawText{0x141CD98D0};
} // namespace render
} // namespace game

#endif
