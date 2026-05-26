#pragma once
#include <cstdint>

#include "../../../game/game.hpp"

namespace game {
namespace cl {
int32_t CL_HighestPriorityStatPacket_Impl(clientConnection_t *clc,
                                          int32_t numCompressedStatsPackets);
clientConnection_t *
CL_GetLocalClientConnection(LocalClientNum_t LocalClientNum);
void CL_CheckForResend_Impl(game::LocalClientNum_t localClientNum);
} // namespace cl
} // namespace game