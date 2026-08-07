#pragma once

#include <game/symbols/sym_include.hpp>

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
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients)>
    CG_AllocateClientMemory{0x1408408F0};
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
                 int32_t maxKillCamsInSplitscreen)>
    CG_InitAndAllocCGEntsArray{0x14085B990};
WEAK symbol<bool(LocalClientNum_t localClientNum)> CG_IsFullyInitialized{
    0x1400AB1A0};

WEAK symbol<void(hunk::HunkUser *user, LocalClientNum_t maxLocalClients)>
    CG_FreeCGEnts{0x140873A30};
WEAK symbol<void()> CG_ClearCGEnts{0x142D47490};
WEAK symbol<bool(int32_t *hitNum, const vec3_t *start, const vec3_t *end,
                 contents_t mask, cm::trace_t *trace)>
    CG_SightTracePoint{0x1412B4380};

WEAK symbol<float> cg_hudSplitscreenScale{0x1457FA6E8, 0x142CF12BC};

extern LocalClientPool<level::cl::cg_t> cgArray;
extern LocalClientPool<level::cl::cgs_t> cgsArray;
extern LocalClientPool<game::level::cl::centityPool_t> cg_entitiesArray;
extern anim::ViewModelInfoPool cg_viewModelArray;
extern ClientPlayerAttachmentInfoPool cg_attachmentsArray;
extern LocalClientPool<ClientPlayerWeaponInfo> cg_weaponsArray;
extern LocalClientPool<phys::Destructible> cg_destructibles;
extern LocalClientPool<ik::IkBuf> cg_ikBuf;

WEAK symbol<CGFakeEntitiesInuseBitArray> cg_fakeEntitiesInuseBitArray{
    0x144D17B80};

/*
    The builtin pools below are replaced with the above, statically allocated
  pools in boiii.

    In the client, these were originally statically allocated, but are intended
  to be heap allocated in the released client engine to allow the allocation
  address to be protected by TAC at runtime. Boiii modifies the engine to
  instead use these statically allocated pools. This is done both for the sake
  of performance and to allow convenient access to the otherwise TAC-protected
  pools.

    In dedicated server, these are - in the base game - never allocated, so the
  pool pointers stored in the below globals are either `nullptr` and unused, or
  filled by boiii with the addresses to the above statically allocated pools.
*/
WEAK symbol<level::cl::cgPool *> builtin_cgArray{0x144D17C80, 0x14222BCB0};
WEAK symbol<level::cl::cgsPool *> builtin_cgsArray{0x144D17B70, 0x14222BCB8};
WEAK symbol<level::cl::LocalClientCentityPools> builtin_cg_entitiesArray{
    0x144D17B60, 0x14222BCC0};
WEAK symbol<anim::ViewModelInfoPool *> builtin_cg_viewModelArray{0x1449D9428};
WEAK symbol<ClientPlayerAttachmentInfoPool *> builtin_cg_attachmentsArray{
    0x1449D9420};

WEAK symbol<LocalClientCgWeaponsPools> builtin_cg_weaponsArray{0x1449D9410};
WEAK symbol<LocalClientCgDestructiblesPools> builtin_cg_destructibles{
    0x157F00FF0};
WEAK symbol<LocalClientIkBufs> builtin_cg_ikBuf{0x144A315C0};

} // namespace cg
} // namespace game