#pragma once
#include <cstdint>

#include <game/game.hpp>

namespace game {
namespace cl {
int32_t CL_HighestPriorityStatPacket_Impl(clientConnection_t *clc,
                                          int32_t numCompressedStatsPackets);
clientConnection_t *
CL_GetLocalClientConnection(LocalClientNum_t localClientNum);
void CL_CheckForResend_Impl(game::LocalClientNum_t localClientNum);
void AllocatePerLocalClientMemory_Impl(LocalClientNum_t maxLocalClients,
                                       ClientNum_t maxClients,
                                       clientAllocFlags_t flags);
void CL_AllocateClientMemory_Impl(hunk::HunkUser *hunk,
                                  LocalClientNum_t maxLocalClients,
                                  ClientNum_t maxClients,
                                  clientAllocFlags_t flags);
} // namespace cl
} // namespace game