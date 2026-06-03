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
                 level::entityEvent_t event)>
    CG_EntityEvent{0x140776DA0, 0x1400CC160};

extern std::atomic<level::cl::cgPool *> cgArray_store;
level::cl::cgPool *get_cgArray();
WEAK symbol<level::cl::cgPool *> cgArray{0x144D17C80};
extern std::atomic<level::cl::cgsPool *> cgsArray_store;
level::cl::cgsPool *get_cgsArray();
WEAK symbol<level::cl::cgsPool *> cgsArray{0x144D17B70};

extern std::atomic<anim::ViewModelInfo *> cg_viewModelArray_store;
anim::ViewModelInfo *get_cg_viewModelArray();
WEAK symbol<anim::ViewModelInfo *> cg_viewModelArray{0x1449D9428};
extern std::atomic<ClientPlayerAttachmentInfo *> cg_attachmentsArray_store;
ClientPlayerAttachmentInfo *get_cg_attachmentsArray();
WEAK symbol<ClientPlayerAttachmentInfo *> cg_attachmentsArray{0x1449D9420};

} // namespace cg
} // namespace game

#endif