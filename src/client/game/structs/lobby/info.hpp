#pragma once

#include <game/structs/core.hpp>
#include <game/structs/lobby/core.hpp>
#include <game/structs/net/core.hpp>
#include <structs/str.hpp>

namespace game {
namespace lobby {
namespace msg {
struct LobbyMsg;
}
namespace info {

// Verified
PACKED(struct LobbyUGCInfo {
  str32_t publisherId = {0};
  uint32_t version = 0;
});
ASSERT_SIZE(LobbyUGCInfo, 0x24);

// Verified
PACKED(struct InfoResponseLobby {
  bool isValid = false;
  uint8_t _padding01[7];
  XUID hostXuid = 0;
  str32_t hostName = {0};
  net::bdSecurityID secId = {};
  net::bdSecurityKey secKey = {};
  net::SerializedAdr serializedAdr = {};
  uint8_t _padding6E[2];
  LobbyParams lobbyParams = {};
  LobbyUGCInfo ugcInfo = {};
  uint8_t _padding9C[4];

  bool package(msg::LobbyMsg & msg);
});
ASSERT_SIZE(InfoResponseLobby, 0xA0);

} // namespace info
} // namespace lobby
} // namespace game
