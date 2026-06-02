#pragma once
#ifdef GAME_HPP

#include "../macros.hpp"

namespace game {
namespace cg {
WEAK symbol<void(LocalClientNum_t localClientNum)> CG_LUIHUDRestart{
    0x140F7E970};
WEAK symbol<ClientUIActives> clientUIActives{0x1453D8BC0};
WEAK symbol<void(LocalClientNum_t localClientNum, float *fov_x,
                 float *dxDzAtDefaultAspectRatio, float *dxDz, float *dyDz)>
    CG_CalcFOVfromLens{0x1404D6230};

WEAK symbol<void(LocalClientNum_t localClientNum, level::cl::centity_t *cent,
                 level::entityEvent32_t event)>
    CG_EntityEvent{0x140776DA0, 0x1400CC160};
} // namespace cg
} // namespace game

#endif