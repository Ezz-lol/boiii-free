#pragma once

#include <game/structs/lobby/core.hpp>
#include <game/structs/lobby/msg/msg.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace lobby {
namespace serverlist {

// Verified
PACKED(struct LobbyServerList {
  int32_t count;
  DEF_PAD(04, 4);
  msg::Msg_InfoResponseLobbyServer serverInfo[128];
  int32_t refreshTime;
  DEF_PAD(2480C, 4);
  IMPL_SLICE_METHODS(serverInfo, count);
});
ASSERT_SIZE(LobbyServerList, 0x24810);

} // namespace serverlist
} // namespace lobby
} // namespace game
