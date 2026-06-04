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
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
                 int32_t maxKillCamsInSplitscreen)>
    CG_InitAndAllocCGEntsArray{0x14085B990};

WEAK symbol<void(hunk::HunkUser *user, LocalClientNum_t maxLocalClients)>
    CG_FreeCGEnts{0x140873A30};
WEAK symbol<void()> CG_ClearCGEnts{0x142D47490};

extern std::atomic<level::cl::cgPool *> cgArray_store;
level::cl::cgPool *get_cgArray();
WEAK symbol<level::cl::cgPool *> cgArray{0x144D17C80};
extern std::atomic<level::cl::cgsPool *> cgsArray_store;
level::cl::cgsPool *get_cgsArray();
WEAK symbol<level::cl::cgsPool *> cgsArray{0x144D17B70};

extern level::cl::AtomicLocalClientCentityPools cg_entitiesArray_store;
level::cl::LocalClientCentityPools get_cg_entitiesArray();
void clear_cgEntitiesArray_store();
WEAK symbol<level::cl::LocalClientCentityPools> cg_entitiesArray{0x144D17B60};

extern std::atomic<anim::ViewModelInfo *> cg_viewModelArray_store;
anim::ViewModelInfo *get_cg_viewModelArray();
WEAK symbol<anim::ViewModelInfo *> cg_viewModelArray{0x1449D9428};
extern std::atomic<ClientPlayerAttachmentInfo *> cg_attachmentsArray_store;
ClientPlayerAttachmentInfo *get_cg_attachmentsArray();
WEAK symbol<ClientPlayerAttachmentInfo *> cg_attachmentsArray{0x1449D9420};

WEAK symbol<CGFakeEntitiesInuseBitArray> cg_fakeEntitiesInuseBitArray{
    0x144D17B80};

} // namespace cg
} // namespace game

#endif