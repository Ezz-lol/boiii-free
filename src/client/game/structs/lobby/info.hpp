#pragma once

#include <game/structs/core.hpp>
#include <game/structs/lobby/core.hpp>
#include <game/structs/net/core.hpp>

namespace game {
namespace lobby {
namespace info {

// Verified
PACKED(struct LobbyUGCInfo {
  char publisherId[32];
  uint32_t version;
});
ASSERT_SIZE(LobbyUGCInfo, 0x24);

// Verified
PACKED(struct InfoResponseLobby {
  bool isValid;
  uint8_t _padding01[7];
  XUID hostXuid;
  str32_t hostName;
  net::bdSecurityID secId;
  net::bdSecurityKey secKey;
  net::SerializedAdr serializedAdr;
  uint8_t _padding6E[2];
  LobbyParams lobbyParams;
  LobbyUGCInfo ugcInfo;
  uint8_t _padding9C[4];
});
ASSERT_SIZE(InfoResponseLobby, 0xA0);

} // namespace info
} // namespace lobby
} // namespace game
