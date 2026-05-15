#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <atomic>
#include <utils/string.hpp>
#include "game/game.hpp"
#include "game/structs/structs.hpp"
#include "game/utils.hpp"
#include "../../../component/auth.hpp"

namespace game {
namespace cl {
int32_t CL_HighestPriorityStatPacket_Impl(clientConnection_t *clc,
                                          int32_t numCompressedStatsPackets);
clientConnection_t *
CL_GetLocalClientConnection(LocalClientNum_t LocalClientNum);
void CL_CheckForResend_Impl(LocalClientNum_t localClientNum);
} // namespace cl
} // namespace game