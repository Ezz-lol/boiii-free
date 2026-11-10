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
// Two widescreen views are shrunk to 80% along the side they don't split, and
// centered on it
inline constexpr double SPLITSCREEN_LETTERBOX_RATIO = 0.8;
inline constexpr float SPLITSCREEN_LETTERBOX_SCALE =
    static_cast<float>(SPLITSCREEN_LETTERBOX_RATIO);
inline constexpr float SPLITSCREEN_LETTERBOX_OFFSET =
    static_cast<float>((1.0 - SPLITSCREEN_LETTERBOX_RATIO) / 2.0);

const ClientViewParams *
CG_GetLocalClientViewParams_Impl(LocalClientNum_t localClientNum,
                                 int32_t viewIndex, int32_t viewCount,
                                 bool letterbox);
void CG_FreeCGEnts_Impl(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients =
                                                  game::LOCAL_CLIENT_COUNT);
void CG_AllocateClientMemory_Impl(
    hunk::HunkUser *hunk,
    LocalClientNum_t maxLocalClients = game::LOCAL_CLIENT_COUNT);

inline bool CG_IsFullyInitialized_Impl(LocalClientNum_t localClientNum) {
  return localClientNum < *cl::cl_maxLocalClients &&
         cgArray[localClientNum].nextSnap != nullptr;
}
} // namespace cg
} // namespace game
