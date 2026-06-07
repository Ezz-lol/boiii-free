#include <std_include.hpp>
#include "cg.hpp"
#include <atomic>

namespace game {
namespace cg {

void CG_InitAndAllocCGEntsArray_Impl(
    hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
    [[maybe_unused]] int32_t maxKillCamsInSplitScreen) {

  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {

    level::cl::centityPool_t *alloc =
        reinterpret_cast<level::cl::centityPool_t *>(hunk::Hunk_UserAlloc(
            hunk, sizeof(level::cl::centityPool_t), 8, "cg_entitiesArray"));
    cg_entitiesArray->pools[localClientNum] = alloc;
    reset(&cg_fakeEntitiesInuseBitArray->inUse[localClientNum]);
    /*
       ## PATCH
       Store allocation pointer in boiii's global atomic storage to allow safe
       usage when Treyarch's anticheat (TAC) has temporarily moved and set the
       pointer to 0xFFFFFFFF during (consistently) encrypted access.
    */
    cg_entitiesArray_store[localClientNum].store(alloc,
                                                 std::memory_order_seq_cst);
  }
}

void CG_ClearCGEnts_Impl() {
  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < game::LOCAL_CLIENT_COUNT; ++localClientNum) {
    cg_entitiesArray->pools[localClientNum] = nullptr;

    /*
      ## PATCH

      Clear allocation pointer in global atomic store.
   */
    cg_entitiesArray_store[localClientNum].store(nullptr,
                                                 std::memory_order_seq_cst);
  }
}

void CG_FreeCGEnts_Impl(hunk::HunkUser *hunk,
                        LocalClientNum_t maxLocalClients) {
  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {
    hunk::Hunk_UserFree(hunk, reinterpret_cast<void *>(
                                  cg_entitiesArray_store[localClientNum].load(
                                      std::memory_order_seq_cst)));

    /*
       ## Patch
       Clear allocation pointer in global atomic store.

       The engine's allocation pointers are not cleared in this function, but
       are all cleared in proceeding code in its sole caller.
    */
    cg_entitiesArray_store[localClientNum].store(nullptr,
                                                 std::memory_order_seq_cst);
  }
}

void CG_AllocateClientMemory_Impl(hunk::HunkUser *hunk,
                                  LocalClientNum_t maxLocalClients) {

  // cgArray
  level::cl::cgPool *cgPoolAlloc =
      reinterpret_cast<level::cl::cgPool *>(hunk::Hunk_UserAlloc(
          hunk, sizeof(level::cl::cg_t) * maxLocalClients, 0x10, "cgArray"));
  *cgArray = cgPoolAlloc;
  // PATCH: store in our global isolated from TAC address encryption
  cgArray_store.store(cgPoolAlloc, std::memory_order_seq_cst);

  // cgsArray
  level::cl::cgsPool *cgsPoolAlloc = reinterpret_cast<level::cl::cgsPool *>(
      hunk::Hunk_UserAlloc(hunk, 0x1E940 * maxLocalClients, 8, "cgsArray"));
  *cgsArray = cgsPoolAlloc;
  // PATCH: store in our global isolated from TAC address encryption
  cgsArray_store.store(cgsPoolAlloc, std::memory_order_seq_cst);

  // cg_viewModelArray
  anim::ViewModelInfo *cg_viewModelArrayAlloc =
      reinterpret_cast<anim::ViewModelInfo *>(hunk::Hunk_UserAlloc(
          hunk, 0x3A0 * maxLocalClients, 8, "cg_viewModelArray"));
  *cg_viewModelArray = cg_viewModelArrayAlloc;
  // PATCH: store in our global isolated from TAC address encryption
  cg_viewModelArray_store.store(cg_viewModelArrayAlloc,
                                std::memory_order_seq_cst);

  // cg_attachmentsArray
  ClientPlayerAttachmentInfo *cg_attachmentsArrayAlloc =
      reinterpret_cast<ClientPlayerAttachmentInfo *>(
          hunk::Hunk_UserAlloc(hunk, 0x200, 8, "cg_attachmentsArray"));
  *cg_attachmentsArray = cg_attachmentsArrayAlloc;
  // PATCH: store in our global isolated from TAC address encryption
  cg_attachmentsArray_store.store(cg_attachmentsArrayAlloc,
                                  std::memory_order_seq_cst);

  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {
    cg_weaponsArray->pools[localClientNum] =
        reinterpret_cast<ClientPlayerWeaponInfo *>(
            hunk::Hunk_UserAlloc(hunk, 0x7000u, 8, "cg_weaponsArray"));
    cg_destructibles->pools[localClientNum] =
        reinterpret_cast<phys::Destructible *>(
            hunk::Hunk_UserAlloc(hunk, 0x11880u, 8, "cg_destrutibles"));
    ik::IkBuf ikStatesArray = reinterpret_cast<ik::IkBuf>(
        hunk::Hunk_UserAlloc(hunk, 0xDB7F0u, 0x10, "ikStatesArray"));
    cg_ikBuf->bufs[localClientNum] = ikStatesArray;
    ik::IK_AllocateLocalClientMemory(ikStatesArray, localClientNum);
    reset(&cg_fakeEntitiesInuseBitArray->inUse[localClientNum]);
  }

  CG_InitAndAllocCGEntsArray_Impl(hunk, maxLocalClients, 0);
}

bool CG_IsFullyInitialized_Impl(LocalClientNum_t localClientNum) {
  level::cl::cgPool *pools = get_cgArray();
  if (pools) {
    if (localClientNum < *cl::cl_maxLocalClients) {
      return pools->pool[localClientNum].nextSnap != nullptr;
    }
  }
  return false;
}
} // namespace cg
} // namespace game