#pragma once

#include "game/structs/clientcontent.hpp"
#include "game/structs/lobby/core.hpp"
#include "game/structs/lobby/session.hpp"

#include <cstdint>

namespace game {
namespace lobby {
namespace active {

PACKED(struct ActiveClient {
  MutableClientInfo mutableClientInfo;
  FixedClientInfo fixedClientInfo;
  session::SessionInfo sessionInfo[3];
  XUID probedXuid;
  int32_t probedXuidTimestamp;
  int32_t uploadBandwidth;
  net::connectionType_e connectionType;
  clientcontent::ClientContent clientContent;
  LobbyClientPool<clientcontent::ClientContentTracking[2]> tracking;
  uint8_t _padding28894[4];
  clientcontent::ClientContentFragments fragments[128];
});
// Correct size is 0x2E7B20.
// TODO: correct this struct.
// ASSERT_SIZE(ActiveClient, 0x2E7B20);

struct ActiveList {
  LobbyModule module;
  uint8_t _padding04[4];
  const char *name;
  LobbyClientPool<ActiveClient *> client;
};
} // namespace active
} // namespace lobby
} // namespace game
