#include <std_include.hpp>

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace info {

bool InfoResponseLobby::package(msg::LobbyMsg &lobbyMsg) {
  // Not sure why this is tried twice - this is how Treyarch implemented it.
  constexpr size_t PACKAGE_ATTEMPTS = 2;

  bool result = true;
  lobbyMsg.package_arrayStart("lobbies");
  for (size_t i = 0; lobbyMsg.package_element(i < PACKAGE_ATTEMPTS); ++i) {
    result &= lobbyMsg.package_bool("valid", &isValid);
    if (isValid) {
      result &=
          lobbyMsg.package_xuid("hostxuid", &hostXuid) &&
          lobbyMsg.package_string("hostname", hostName) &&
          lobbyMsg.package_int("networkmode",
                               reinterpret_cast<int32_t *>(&lobbyParams)) &&
          lobbyMsg.package_int(
              "mainmode", reinterpret_cast<int32_t *>(&lobbyParams.mainMode)) &&
          lobbyMsg.package_glob("secid", &secId, sizeof(secId)) &&
          lobbyMsg.package_glob("seckey", &secKey, sizeof(secKey)) &&
          lobbyMsg.package_glob("addrbuff", serializedAdr.addrBuff,
                                sizeof(serializedAdr.addrBuff)) &&
          lobbyMsg.package_string("ugcName", ugcInfo.publisherId) &&
          lobbyMsg.package_uint("ugcVersion", &ugcInfo.version);
    }
  }
  return result;
}
} // namespace info
} // namespace lobby
} // namespace game
