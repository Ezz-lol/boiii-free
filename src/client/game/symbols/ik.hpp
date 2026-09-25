#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace ik {
WEAK symbol<void(IkBufRef ikStatesBuf, LocalClientNum_t localClientNum)>
    IK_AllocateLocalClientMemory{0x1423F7B40, 0x142470CB0, 0x0};
} // namespace ik
} // namespace game
