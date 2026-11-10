#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace migrate {
WEAK symbol<void(session::LobbySession *lobbySession,
                 msg::Msg_LobbyMigrateStart *msgData)>
    LobbyMigrate_HandleFindBestCommand{0x141EDEA90, 0x14049C740};
WEAK symbol<void(session::LobbySession *lobbySession, XUID migrateTo)>
    LobbyMigrate_HandleLobbyMigrateTo{0x141EDEAC0, 0x14049C770};
WEAK symbol<void(session::LobbySession *hostSession,
                 msg::MsgClientMigrateInfo *migrateInfo, XUID fromXUID)>
    LobbyMigrateHost_ReadClientMigrateInfo{0x141ED1FC0, 0x140490200};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, net::netadr_t *fromAdr,
                 XUID fromXUID, msg::Msg_LobbyMigrateBandwidthTest *msgData)>
    LobbyMigrateClient_HandleTestPacket{0x141EC2660, 0x140482CD0};
WEAK symbol<void(session::LobbySession *lobbySession,
                 msg::Msg_LobbyMigrateAnnounceHost *msgData, XUID fromXUID)>
    LobbyMigrateClient_HandleHostAnnounce{0x141EC2590, 0x140482C00};
WEAK symbol<void(session::LobbySession *clientSession,
                 msg::MsgHostMigrateInfo *migrateInfo, bool stateMsg)>
    LobbyMigrateClient_ReadMigrateInfo{0x141EC2870, 0x140482EE0};
WEAK symbol<bool(session::LobbySession *clientSession)>
    LobbyMigrateClient_MigrateActive{0x141EC2770, 0x140482DE0};
WEAK symbol<bool(session::LobbySession *lobbySession)>
    LobbyMigrate_CanLobbyMigrate{0x141EDE700, 0x14049C3B0};
WEAK symbol<void(session::LobbySession *lobbySession, bool activate)>
    LobbyMigrateClient_SetMigrateActive{0x141EC2960, 0x140482FD0};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 session::LobbySession *clientSession,
                 msg::Msg_LobbyIngameMigrateNewHost *msgData)>
    LobbyInGameMigrateClient_HandleSessionInfoMsg{0x141EC2450, 0x140482AD0};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 session::LobbySession *clientSession,
                 msg::Msg_LobbyIngameMigrateMigrateTo *msgData)>
    LobbyInGameMigrateClient_HandleMigrateTo{0x141EC23C0, 0x140482A40};
} // namespace migrate
} // namespace lobby
} // namespace game
