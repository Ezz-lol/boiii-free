#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cg {
WEAK symbol<void(int32_t localClientNum)> CG_LUIHUDRestart{0x140F7E970};
WEAK symbol<clientUIActive_t> clientUIActives{0x1453D8BC0};
WEAK symbol<void(int localClientNum, float *fov_x,
                 float *dxDzAtDefaultAspectRatio, float *dxDz, float *dyDz)>
    CG_CalcFOVfromLens{0x1404D6230};
} // namespace cg
} // namespace game

#endif