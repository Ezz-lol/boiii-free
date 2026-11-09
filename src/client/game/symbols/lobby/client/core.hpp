#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace client {
WEAK symbol<void(session::LobbyGameData *gameData)> ServerStatusNotRunning{
    0x141EC1DB0, 0x0, 0x0};
WEAK symbol<int32_t(const LobbyType lobbyType)>
    LobbyClientSession_GetConnectivityBits{0x141EBE5C0, 0x0, 0x14047F680};
WEAK symbol<void(LobbyType lobbyType, int32_t connectivityBits)>
    LobbyClientSession_SetConnectivityBits{0x141EBFB00, 0x0, 0x140480A30};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession, int32_t connectivityBits)>
    SendUpdatedConnectivityToHost{0x141EED370, 0x0, 0x1404A7690};
WEAK symbol<void(const ControllerIndex_t controllerIndex,
                 session::LobbySession *lobbySession,
                 msg::PrivateData *privateDataMsg)>
    LobbyClient_UpdatePrivateData{0x141EBFD90, 0x0, 0x140480B80};
WEAK symbol<bool()> LobbyClientLaunch_IsInGame{0x141EC1BB0, 0x0, 0x1404825A0};
WEAK symbol<void(ControllerIndex_t controllerIndex, const LobbyType lobbyType)>
    LobbyClientMsg_SendClientHeartbeat{0x141EC7470, 0x0, 0x140485C40};

WEAK symbol<int32_t(active::ActiveClient *activeClient)>
    LobbyClientActiveList_GetQPort{0x141EF1B00, 0x141EFE280, 0x0};
WEAK symbol<const char *(LobbyType lobbyType,
                         clientcontent::ClientContentType contentType,
                         XUID xuid)>
    LobbyClientContent_GetDebugString{0x141EC0730, 0x141ECD1C0, 0x140481480};
WEAK symbol<uint16_t(LobbyType lobbyType, XUID xuid,
                     clientcontent::ClientContentType clientContentType)>
    LobbyClientContent_GetTotalContentReceived{0x141EC0890, 0x0, 0x0};
WEAK symbol<void()> LobbyClientContent_Init{0x141EC08F0, 0x141ECD380, 0x0};
WEAK symbol<bool(ControllerIndex_t controllerIndex,
                 clientcontent::ClientContentType contentType)>
    LobbyClientContent_IsBufferChecksumChanged{0x141EC0910, 0x0, 0x0};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 clientcontent::ClientContentType contentType, bool dirty)>
    LobbyClientContent_SetActiveLobbyClientContentDataDirty{0x141EC0A30, 0x0,
                                                            0x0};
WEAK symbol<void(LobbyType lobbyType, XUID xuid, uint16_t totalContentReceived,
                 clientcontent::ClientContentType contentType)>
    LobbyClientContent_SetTotalContentReceived{0x141EC0AA0, 0x0, 0x0};
WEAK symbol<void()> LobbyClientContent_Update{0x141EC0B30, 0x141ECD5C0, 0x0};
WEAK symbol<session::ClientSession *(LobbyType lobbyType)>
    LobbyClientData_GetClientSession{0x141EC0F00, 0x141ECD990, 0x140481B60};
WEAK symbol<session::LobbySession *(LobbyType lobbyType)>
    LobbyClientData_GetSession{0x141EC0F90, 0x141ECDA20, 0x140481BA0};
WEAK symbol<void(LobbyType lobbyType, bool updateUI)>
    LobbyClientData_UpdateLastHostMsgTimestamp{0x141EC1200, 0x141ECDC90,
                                               0x140481DE0};
WEAK symbol<void(const ControllerIndex_t controllerIndex)>
    LobbyClient_DownloadDemo{0x141EBE4B0, 0x141ECAF40, 0x14047F570};
WEAK symbol<void(const ControllerIndex_t controllerIndex, int32_t baseQPort,
                 FixedClientInfo *fixedClientInfo)>
    LobbyClient_GetFixedClientInfo{0x141EBE5E0, 0x0, 0x14047F6A0};
WEAK symbol<session::LobbyMigrateClientData *(LobbyType lobbyType)>
    LobbyClient_GetLobbyClientMigrationData{0x141EBE840, 0x141ECB2D0, 0x0};
WEAK symbol<void(const ControllerIndex_t controllerIndex,
                 MutableClientInfo *mutableClientInfo)>
    LobbyClient_GetMutableClientInfo{0x141EBEB00, 0x141ECB590, 0x14047FB40};
WEAK symbol<session::LobbySession *(const LobbyType lobbyType)>
    LobbyClient_GetSession{0x141EBF500, 0x141ECBF90, 0x140480520};
WEAK symbol<void(void *payload)> LobbyClient_HandleClientEnd_Disconnect{
    0x141ECA050, 0x141ED6A60, 0x140488280};
WEAK symbol<session::LobbySession *(void *lobbyType)>
    LobbyClient_HandleTaskPending{0x141ECA130, 0x141ED6B40, 0x140488340};
WEAK symbol<int64_t()> LobbyClient_Init{0x141EBF6C0, 0x141ECC150, 0x0};
WEAK symbol<bool(const LobbyType lobbyType)> LobbyClient_IsActive{
    0x141EBF6E0, 0x141ECC170, 0x140480680};
WEAK symbol<bool(const LobbyType lobbyType)> LobbyClient_IsDedicated{
    0x0, 0x0, 0x140480720};
WEAK symbol<bool(const LobbyType lobbyType)> LobbyClient_IsHost{
    0x141EBF7B0, 0x141ECC240, 0x140480750};
WEAK symbol<bool(const LobbyType lobbyType)> LobbyClient_IsStateAvailable{
    0x141EBF880, 0x0, 0x0};
WEAK symbol<void()> LobbyClientLaunch_Clear{0x141EC1AB0, 0x141ECE540, 0x0};
WEAK symbol<void(LobbyType lobbyType)> LobbyClientLaunch_Update{
    0x141EC1C00, 0x141ECE690, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 session::GameData *gameDataMsg)>
    LobbyClientLaunch_UpdateDemoInfo{0x141EC1C30, 0x0, 0x1404825F0};
WEAK symbol<void(clientcontent::ClientContent *content, LobbyType lobbyType,
                 XUID fromXUID, XUID clientXUID,
                 clientcontent::ClientContentType contentType)>
    LobbyClientMsg_DecompressClientContentDataReceived{0x141EC6FA0, 0x0,
                                                       0x140485780};
WEAK symbol<bool(ControllerIndex_t controllerIndex, net::netadr_t *fromAdr,
                 XUID fromXUID, msg::LobbyMsg *lobbyMsg)>
    LobbyClientMsg_HandleHostMsg{0x141EC70F0, 0x141ED3B80, 0x1404858D0};
WEAK symbol<void()> LobbyClientMsg_Init{0x141EC7160, 0x141ED3BF0, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 LobbyLeaderActivity activity)>
    LobbyClientMsg_SendActivity{0x141EC7170, 0x0, 0x0};
WEAK symbol<bool(const ControllerIndex_t controllerIndex, LobbyType lobbyType,
                 XUID xuid)>
    LobbyClientMsg_SendDisconnect{0x141EC7530, 0x141ED3FC0, 0x140485D00};
WEAK symbol<bool(const ControllerIndex_t controllerIndex,
                 const LobbyType lobbyType)>
    LobbyClientMsg_SendModifiedStats{0x141EC7780, 0x0, 0x0};
WEAK symbol<void(LobbyType lobbyType)> LobbyClientMsg_Update{0x141EC7A40,
                                                             0x141ED44D0, 0x0};
WEAK symbol<bool(net::msg::msg_t *msg, int32_t sequenceNumber, XUID fromXUID)>
    LobbyClientMsg_WriteDataToClientContent{0x141EC7A70, 0x0, 0x0};
WEAK symbol<int32_t(const LobbyType lobbyType, lua::hks::lua_State *luaVM)>
    LobbyClient_PushSessionClientsToVM{0x141EBF8C0, 0x141ECC350, 0x0};
WEAK symbol<void(const LobbyType lobbyType, lua::hks::lua_State *luaVM)>
    LobbyClient_PushSessionToVM{0x141EBF910, 0x0, 0x0};
WEAK symbol<int32_t(LobbyType lobbyType)>
    LobbyClientSession_GetLastHostMsgTimestamp{0x141EC0F20, 0x0, 0x0};
WEAK symbol<session::LobbyMigrateClientData *(LobbyType lobbyType)>
    LobbyClientSession_GetLobbyClientMigrationData{0x141EC0F40, 0x0, 0x0};
WEAK symbol<session::PeerToPeerInfo *(LobbyType lobbyType)>
    LobbyClientSession_GetPeerToPeerInfo{0x141EBF330, 0x141ECBDC0, 0x140480350};
WEAK symbol<voice::SharedVoiceSessionData *(LobbyType lobbyType)>
    LobbyClientSession_GetSharedVoiceData{0x141EBF6A0, 0x0, 0x0};
WEAK symbol<void()> LobbyClientSession_Init{0x141EC1010, 0x141ECDAA0, 0x0};
WEAK symbol<void(LobbyType lobbyType,
                 voice::SharedVoiceSessionData *platformVoiceData)>
    LobbyClientSession_SetSharedVoiceData{0x141EBFB60, 0x0, 0x0};
WEAK symbol<void(const int32_t actionId, LobbyType lobbyType,
                 LobbyMainMode mainMode, LobbyNetworkMode networkMode,
                 net::HostInfo *hostInfo)>
    LobbyClient_Start{0x141EBFB80, 0x141ECC610, 0x140480A90};
WEAK symbol<void(LobbyType lobbyType)> LobbyClient_TheaterUpdate{
    0x141EBFB90, 0x141ECC620, 0x0};
WEAK symbol<void(LobbyType lobbyType)> LobbyClient_Update{0x141EBFC20,
                                                          0x141ECC6B0, 0x0};
WEAK symbol<uint32_t(const ControllerIndex_t controllerIndex,
                     session::LobbySession *lobbySession,
                     session::GameData *gameDataMsg)>
    LobbyClient_UpdateGameData{0x141EBFCA0, 0x141ECC730, 0x140480AA0};
} // namespace client
} // namespace lobby
} // namespace game
