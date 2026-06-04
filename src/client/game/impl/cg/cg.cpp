#include "cg.hpp"
#include <atomic>

namespace game {
namespace cg {

void CG_InitAndAllocCGEntsArray_Impl(
    hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
    [[maybe_unused]] int maxKillCamsInSplitScreen) {

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
       usage when arxan has temporarily moved and set the pointer to 0xFFFFFFFF
       during (consistently) obfuscated access.
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
                                  cg_entitiesArray->pools[localClientNum]));

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
} // namespace cg
} // namespace game