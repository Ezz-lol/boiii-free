#pragma once
#include <cstdint>

#include <game/game.hpp>

namespace game {
namespace cg {
void CG_InitAndAllocCGEntsArray_Impl(
    hunk::HunkUser *hunk,
    LocalClientNum_t maxLocalClients = game::LOCAL_CLIENT_COUNT,
    int32_t maxKillCamsInSplitScreen = 0);

void CG_ClearCGEnts_Impl();
void CG_FreeCGEnts_Impl(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients =
                                                  game::LOCAL_CLIENT_COUNT);
void CG_AllocateClientMemory_Impl(
    hunk::HunkUser *hunk,
    LocalClientNum_t maxLocalClients = game::LOCAL_CLIENT_COUNT);

bool CG_IsFullyInitialized_Impl(LocalClientNum_t localClientNum);
} // namespace cg
} // namespace game