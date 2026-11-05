#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace migrate {
WEAK symbol<void(session::LobbySession *lobbySession,
                 msg::Msg_LobbyMigrateStart *msgData)>
    LobbyMigrate_HandleFindBestCommand{0x141EDEA90, 0x0, 0x14049C740};
WEAK symbol<void(session::LobbySession *lobbySession, XUID migrateTo)>
    LobbyMigrate_HandleLobbyMigrateTo{0x141EDEAC0, 0x0, 0x14049C770};
WEAK symbol<void(session::LobbySession *hostSession,
                 msg::MsgClientMigrateInfo *migrateInfo, XUID fromXUID)>
    LobbyMigrateHost_ReadClientMigrateInfo{0x141ED1FC0, 0x0, 0x140490200};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, net::netadr_t *fromAdr,
                 XUID fromXUID, msg::Msg_LobbyMigrateBandwidthTest *msgData)>
    LobbyMigrateClient_HandleTestPacket{0x141EC2660, 0x0, 0x140482CD0};

} // namespace migrate
} // namespace lobby
} // namespace game
