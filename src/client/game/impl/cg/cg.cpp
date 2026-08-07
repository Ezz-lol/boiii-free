#include <std_include.hpp>
#include "cg.hpp"

namespace game {
namespace cg {

void CG_InitAndAllocCGEntsArray_Impl(
    hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
    [[maybe_unused]] int32_t maxKillCamsInSplitScreen) {

  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {

    // ORIGINAL:
    // level::cl::centityPool_t *alloc =
    //     reinterpret_cast<level::cl::centityPool_t *>(hunk::Hunk_UserAlloc(
    //         hunk, sizeof(level::cl::centityPool_t), 8, "cg_entitiesArray"));

    // PATCH: use statically allocated pool
    builtin_cg_entitiesArray->pools[localClientNum] =
        &cg_entitiesArray[localClientNum];
    cg_fakeEntitiesInuseBitArray->inUse[localClientNum].reset();
  }
}

void CG_ClearCGEnts_Impl() {
  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < game::LOCAL_CLIENT_COUNT; ++localClientNum) {
    builtin_cg_entitiesArray->pools[localClientNum] = nullptr;
  }
}

void CG_FreeCGEnts_Impl(hunk::HunkUser *hunk,
                        LocalClientNum_t maxLocalClients) {
  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {
    // ORIGINAL:
    // hunk::Hunk_UserFree(hunk,
    //                     reinterpret_cast<void *>(
    //                         builtin_cg_entitiesArray->pools[localClientNum]));

    // PATCH: clear static pool allocation
    memset(cg_entitiesArray, 0, sizeof(cg_entitiesArray));
  }
}

void CG_AllocateClientMemory_Impl(hunk::HunkUser *hunk,
                                  LocalClientNum_t maxLocalClients) {

  // cgArray
  {
    // ORIGINAL:
    // level::cl::cgPool *cgPoolAlloc =
    //     reinterpret_cast<level::cl::cgPool *>(hunk::Hunk_UserAlloc(
    //         hunk, sizeof(level::cl::cg_t) * maxLocalClients, 0x10,
    //         "cgArray"));
    // *cgArray = cgPoolAlloc;

    // PATCH: store the address to our static pool allocation
    *game::cg::builtin_cgArray =
        reinterpret_cast<game::level::cl::cgPool *>(&cgArray);
  }

  // cgsArray
  {
    // ORIGINAL:
    // level::cl::cgsPool *cgsPoolAlloc = reinterpret_cast<level::cl::cgsPool
    // *>(
    //     hunk::Hunk_UserAlloc(hunk, 0x1E940 * maxLocalClients, 8,
    //     "cgsArray"));
    // *cgsArray = cgsPoolAlloc;

    // PATCH: store the address to our static pool allocation
    *game::cg::builtin_cgsArray =
        reinterpret_cast<game::level::cl::cgsPool *>(&cgsArray);
  }

  // cg_viewModelArray
  {
    // ORIGINAL:
    // anim::ViewModelInfo *cg_viewModelArrayAlloc =
    //     reinterpret_cast<anim::ViewModelInfo *>(hunk::Hunk_UserAlloc(
    //         hunk, 0x3A0 * maxLocalClients, 8, "cg_viewModelArray"));
    // *cg_viewModelArray = cg_viewModelArrayAlloc;

    // PATCH: store the address to our static pool allocation
    *builtin_cg_viewModelArray = &cg_viewModelArray;
  }

  // cg_attachmentsArray
  {
    // ORIGINAL:
    // ClientPlayerAttachmentInfo *cg_attachmentsArrayAlloc =
    //     reinterpret_cast<ClientPlayerAttachmentInfo *>(
    //         hunk::Hunk_UserAlloc(hunk, 0x200, 8, "cg_attachmentsArray"));
    // *cg_attachmentsArray = cg_attachmentsArrayAlloc;

    // PATCH: store the address to our static pool allocation
    *builtin_cg_attachmentsArray = &cg_attachmentsArray;
  }

  for (LocalClientNum_t localClientNum = game::LOCAL_CLIENT_0;
       localClientNum < maxLocalClients; ++localClientNum) {

    // cg_weaponsArray
    {
      // ORIGINAL:
      // cg_weaponsArray->pools[localClientNum] =
      //     reinterpret_cast<ClientPlayerWeaponInfo *>(
      //         hunk::Hunk_UserAlloc(hunk, 0x7000u, 8, "cg_weaponsArray"));

      // PATCH: store the address to our static pool allocation
      builtin_cg_weaponsArray->pools[localClientNum] =
          &cg_weaponsArray[localClientNum];
    }

    // cg_destructibles
    {
      // ORIGINAL:
      // cg_destructibles->pools[localClientNum] =
      //     reinterpret_cast<phys::Destructible *>(
      //         hunk::Hunk_UserAlloc(hunk, 0x11880u, 8, "cg_destrutibles"));

      // PATCH: store the address to our static pool allocation
      builtin_cg_destructibles->pools[localClientNum] =
          &cg_destructibles[localClientNum];
    }

    // cg_ikBuf
    {
      // ORIGINAL:
      // ik::IkBuf ikStatesArray = reinterpret_cast<ik::IkBuf>(
      //     hunk::Hunk_UserAlloc(hunk, 0xDB7F0u, 0x10, "ikStatesArray"));
      // cg_ikBuf->bufs[localClientNum] = ikStatesArray;

      // PATCH: store the address to our static pool allocation
      ik::IkBufRef ikStatesArray = {.ref = &cg_ikBuf[localClientNum]};
      builtin_cg_ikBuf->bufs[localClientNum] = ikStatesArray;

      ik::IK_AllocateLocalClientMemory(ikStatesArray, localClientNum);
    }
    cg_fakeEntitiesInuseBitArray->inUse[localClientNum].reset();
  }

  CG_InitAndAllocCGEntsArray_Impl(hunk, maxLocalClients, 0);
}

} // namespace cg
} // namespace game