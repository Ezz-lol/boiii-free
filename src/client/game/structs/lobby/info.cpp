#include <std_include.hpp>

#include "game/game.hpp"

namespace game {
namespace lobby {
namespace info {

bool InfoResponseLobby::package(msg::LobbyMsg &lobbyMsg) {
  bool result = true;
  lobbyMsg.package_arrayStart("lobbies");
  for (LobbyModule lobbyModule = LobbyModule::HOST;
       lobbyMsg.package_next_element(lobbyModule < LobbyModule::COUNT, result);
       ++lobbyModule) {
    result &=
        lobbyMsg.package_bool("valid", &isValid) &&
        (!isValid ||
         (lobbyMsg.package_xuid("hostxuid", &hostXuid) &&
          lobbyMsg.package_string("hostname", hostName) &&
          lobbyMsg.package_int("networkmode", reinterpret_cast<int32_t *>(
                                                  &lobbyParams.networkMode)) &&
          validate(lobbyParams.networkMode) &&
          lobbyMsg.package_int(
              "mainmode", reinterpret_cast<int32_t *>(&lobbyParams.mainMode)) &&
          validate(lobbyParams.mainMode) &&
          lobbyMsg.package_glob("secid", &secId, sizeof(secId)) &&
          lobbyMsg.package_glob("seckey", &secKey, sizeof(secKey)) &&
          lobbyMsg.package_glob("addrbuff", serializedAdr.addrBuff,
                                sizeof(serializedAdr.addrBuff)) &&
          lobbyMsg.package_string("ugcName", ugcInfo.publisherId) &&
          lobbyMsg.package_uint("ugcVersion", &ugcInfo.version)));
  }
  return result;
}
} // namespace info
} // namespace lobby
} // namespace game
