#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace lobby {
namespace vm {
WEAK symbol<void()> LobbyVM_Reload{0x141EE4640, 0x1404A1F50};
WEAK symbol<void(ControllerIndex_t controllerIndex,
                 game::lobby::LobbyType lobbyType, ClientNum_t maxClients)>
    LobbyVM_LobbyHostLeftEvent{0x141EE3130, 0x1404A0CC0};
WEAK symbol<void(hks::lua_Integer value)> LobbyVM_CheckAdvertiseSteamServer{
    0x0, 0x1404A03D0};
WEAK symbol<bool(ControllerIndex_t controllerIndex)>
    LobbyVM_CheckAdvertizeSteamServer{0x141EE2870, 0x0};
WEAK symbol<void(hks::lua_Integer value)> LobbyVM_CheckInitSteamServer{
    0x0, 0x1404A0480};
WEAK symbol<void()> LobbyVM_ClientLaunch_ClearLaunchScript{0x141EC1320, 0x0};
WEAK symbol<void(const ControllerIndex_t controllerIndex)> LobbyVM_DevmapEvent{
    0x141EE2A50, 0x0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession,
                 const char *lobbyVMEvent)>
    LobbyVM_FireEvent{0x141EE2B10, 0x0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession, XUID xuid,
                 const char *lobbyVMEvent, const char *customData)>
    LobbyVM_FireEventWithXuid{0x141EE2BE0, 0x1404A0740};
WEAK symbol<int(jq::jqBatch *batch)> LobbyVM_GCCallback{0x0, 0x1404A0840};
WEAK symbol<int(jq::jqBatch *batch, void *data)>
    LobbyVM_GCCallback_Implementation{0x0, 0x1404A08A0};
WEAK symbol<int64_t()> LobbyVM_GetHWM{0x141EE2E70, 0x0};
WEAK symbol<int64_t()> LobbyVM_GetMemoryUsage{0x141EE2E90, 0x0};
WEAK symbol<hks::lua_State *()> LobbyVM_GetVM{0x141EE2EB0, 0x1404A0A40};
WEAK symbol<void()> LobbyVM_Init{0x141EE2EC0, 0x1404A0A50};
WEAK symbol<void(ControllerIndex_t controllerIndex)>
    LobbyVM_LobbyHostLeft_InGameMigrateFinished{0x141EE3270, 0x0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession, XUID xuid,
                 const char *gamertag)>
    LobbyVM_OnClientAdded{0x141EE3310, 0x0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession, XUID xuid,
                 const char *gamertag)>
    LobbyVM_OnClientRemoved{0x141EE3320, 0x1404A0EB0};
WEAK symbol<void(ErrorCode errorCode, const char *errorMsg,
                 bool comErrorInProgress, bool isInCleanup)>
    LobbyVM_OnComError{0x141EE3330, 0x1404A0EC0};
WEAK symbol<void()> LobbyVM_OnDediQosReady{0x141E8A3E0, 0x0};
WEAK symbol<void(
    game::lobby::session::LobbySession *lobbySession, XUID disconnectClientXuid,
    game::lobby::LobbyDisconnectClient disconnectClient, const char *errorMsg)>
    LobbyVM_OnDisconnect{0x141EE3550, 0x1404A10C0};
WEAK symbol<void(ControllerIndex_t controllerIndex)> LobbyVM_OnDWDisconnect{
    0x141EE34B0, 0x1404A1020};
WEAK symbol<void()> LobbyVM_OnErrorShutdown{0x141EE3710, 0x1404A1280};
WEAK symbol<void(ControllerIndex_t controllerIndex)> LobbyVM_OnExperimentReset{
    0x141EBF990, 0x1404808C0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession)>
    LobbyVM_OnGametypeSettingsChange{0x141EE37B0, 0x1404A1320};
WEAK symbol<void()> LobbyVM_OnInit{0x141EE37C0, 0x1404A1330};
WEAK symbol<void()> LobbyVM_OnKVSFlush{0x141EE38D0, 0x0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession)>
    LobbyVM_OnLeaveWithParty{0x141EE3950, 0x1404A1440};
WEAK symbol<bool(ControllerIndex_t controllerIndex, XUID leavingClientXuid)>
    LobbyVM_OnLocalClientLeave{0x141EE3B40, 0x1404A14F0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession,
                 const char *currentGametype, const char *nextGametype)>
    LobbyVM_OnMatchChangeGameType{0x141EE3C10, 0x1404A15C0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession,
                 const char *currentMap, const char *nextMap)>
    LobbyVM_OnMatchChangeMap{0x141EE3D10, 0x1404A16C0};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession)>
    LobbyVM_OnMatchEnd{0x141EE3E10, 0x0};
WEAK symbol<void(hks::lua_Integer currentMax, hks::lua_Integer newMax)>
    LobbyVM_OnMaxClientsChanged{0x141EE3EE0, 0x0};
WEAK symbol<void()> LobbyVM_OnPartyPrivacyChange{0x141EE3FA0, 0x1404A1A10};
WEAK symbol<void()> LobbyVM_OnPostExecFFOTD{0x141EE4100, 0x1404A1AB0};
WEAK symbol<void()> LobbyVM_OnPreExecFFOTD{0x141EE41A0, 0x1404A1B50};
WEAK symbol<void(game::lobby::session::LobbySession *lobbySession)>
    LobbyVM_OnSessionStart{0x141EE42F0, 0x0};
WEAK symbol<void(bool init)> LobbyVM_OnUILoad{0x141EE4300, 0x0};
WEAK symbol<void(const ControllerIndex_t controllerIndex, byte *buff,
                 int32_t maxSize, uint16_t *outSize)>
    LobbyVM_PopulateMutableClientDDLBuff{0x141EE43A0, 0x0};
WEAK symbol<void(XUID clientXuid, byte *buff, int32_t maxSize, int32_t size)>
    LobbyVM_ReceiveMutableClientDDLBuff{0x141EE44F0, 0x0};
WEAK symbol<void()> LobbyVM_Update{0x141EE46C0, 0x0};

} // namespace vm
} // namespace lobby
} // namespace lua
} // namespace game
