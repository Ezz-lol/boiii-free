#pragma once

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace msg {
namespace host {

bool LobbyHostMsg_SendAgreementRequest_Impl(ControllerIndex_t controllerIndex,
                                            net::HostInfo *hostInfo,
                                            uint64_t reservationKey, int nonce,
                                            LobbyType srcLobbyType,
                                            LobbyType targetLobbyType,
                                            LobbyParams params);

} // namespace host
} // namespace msg
} // namespace lobby
} // namespace game
