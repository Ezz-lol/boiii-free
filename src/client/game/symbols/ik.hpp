#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace ik {
WEAK symbol<void(IkBuf ikStatesBuf, LocalClientNum_t localClientNum)>
    IK_AllocateLocalClientMemory{0x142470CB0};
} // namespace ik
} // namespace game

#endif