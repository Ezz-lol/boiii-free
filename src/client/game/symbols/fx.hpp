#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace fx {
WEAK symbol<void()> FX_ShutdownLensFlareSystem{0x1414ED6A0};
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
                 ClientNum_t maxClients, cl::clientAllocFlags_t allocFlags)>
    FX_AllocateClientMemory{0x1414D4700};
} // namespace fx
} // namespace game

#endif