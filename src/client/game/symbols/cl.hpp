#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cl {
WEAK symbol<void(game::LocalClientNum_t localClientNum)>
    CL_CheckKeepDrawingConnectScreen{0x1413CCB00, 0x1413CCAE0, 0x0};
WEAK symbol<void(game::ControllerIndex_t controllerIndex,
                 net::XSESSION_INFO *hostInfo, const net::netadr_t *addr,
                 int numPublicSlots, int numPrivateSlots, const char *mapname,
                 const char *gametype, const char *somethingWithUserMaps)>
    CL_ConnectFromLobby{0x14134C590, 0x14134C570, 0x14018E420};
WEAK symbol<bool(game::LocalClientNum_t localClientNum, int index, char *buf,
                 int size, bool addClanName)>
    CL_GetClientName{0x1413E3160, 0x1413E3140, 0x0};
WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    CL_LocalClient_IsActive{0x1427C18E0, 0x14283AA50, 0x0};
WEAK symbol<float(void *key)> CL_KeyState{0x1412FF880, 0x1412FF860, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *pszMapName,
                 const char *pszGametype)>
    CL_SetupForNewServerMap{0x14135CD40, 0x14135CD20, 0x0};
WEAK symbol<void(game::LocalClientNum_t localClientNum, bool active)>
    CL_LocalClient_SetActive{0x1427C1940, 0x14283AAB0, 0x0};

WEAK symbol<const char *(int32_t configStringIndex)> CL_GetConfigString{
    0x141321130, 0x141321110, 0x14017CCC0};
WEAK symbol<int64_t(LocalClientNum_t localClientNum)> CL_ClearClientThinkPacket{
    0x1412F33F0, 0x1412F33D0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *mapname,
                 const char *gametype)>
    CL_PreloadMap{0x14135A200, 0x14135A1E0, 0x0};
WEAK symbol<bool(const char *map)> CL_MapSwitch_IsMapPreloaded{
    0x14131EA10, 0x14131E9F0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool isServer)>
    CL_MapSwitch_SwitchNow{0x14131EC80, 0x14131EC60, 0x0};
WEAK symbol<void(bool setupViewport)> CL_InitRenderer{0x141359710, 0x1413596F0,
                                                      0x0};
WEAK symbol<void()> CL_StartHunkUsers{0x14135D1B0, 0x14135D190, 0x0};
WEAK symbol<CLMapSwitch> cl_switch{0x1452C4A00, 0x145343A00, 0x142850830};
WEAK symbol<int32_t> g_qport{0x156DCAB3C, 0x156E49AAC, 0x0};

WEAK symbol<void(int64_t localClientNum, int32_t msec)> CL_Frame{
    0x141351400, 0x1413513E0, 0x0};
WEAK symbol<void(game::LocalClientNum_t localClientNum)> CL_CheckForResend{
    0x14134B990, 0x14134B970, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool checkStatus)>
    CL_LoadoutChanged{0x141359980, 0x141359960, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool deactivateClient)>
    CL_Disconnect{0x14135D880, 0x14135D860, 0x0};
WEAK symbol<void()> CL_Disconnect_f{0x14134CCD0, 0x14134CCB0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, net::netadr_t *from,
                 const char *reason)>
    CL_DisconnectPacket{0x14134CBB0, 0x14134CB90, 0x0};
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
                 ClientNum_t maxClients, clientAllocFlags_t allocFlags)>
    CL_AllocateClientMemory{0x1413484E0, 0x1413484C0, 0x0};

WEAK symbol<void(LocalClientNum_t maxLocalClients, ClientNum_t maxClients,
                 clientAllocFlags_t flags)>
    AllocatePerLocalClientMemory{0x14135D330, 0x14135D310, 0x0};
WEAK symbol<bool(const char *mapname, clientAllocFlags_t flags)>
    CL_AllocatePerLocalClientMemory{0x14135D670, 0x14135D650, 0x0};
WEAK symbol<void(bool restartUI)> CL_FreePerLocalClientMemory{0x14135DC20,
                                                              0x14135DC00, 0x0};

WEAK symbol<ClientConnections *> clientConnections{0x145359BB8, 0x1453D8BB8,
                                                   0x0};
WEAK symbol<ClientActives *> clients{0x14535D188, 0x1453DC188, 0x0};
WEAK symbol<clientStatic_t> cls{0x14535D190, 0x1453DC190, 0x1428E2E40};
// TODO: fix the clientStatic_t struct and remove these symbols
WEAK symbol<gameState_t> cls_gamestate{0x1456a3828, 0x145722828, 0x142C294D8};
WEAK symbol<int32_t> cls_serverId{0x14561C250, 0x14569B250, 0x142BA1F00};
WEAK symbol<qboolean> cls_forceFullscreen{0x14561C254, 0x14569B254,
                                          0x142BA1F04};
WEAK symbol<qboolean> cls_exitFullscreen{0x14561C258, 0x14569B258, 0x142BA1F08};
WEAK symbol<r::vidConfig_t> cls_vidConfig{0x14536571c, 0x1453E471C,
                                          0x1428EB3CC};

WEAK symbol<ClientNum_t> cl_allocatedClients{0x1432A7EE8, 0x143326EE8, 0x0};
WEAK symbol<clientAllocFlags_t> cl_lastAllocFlags{0x14535598C, 0x1453D498C,
                                                  0x0};
WEAK symbol<void(LocalClientNum_t localClientNum)> CL_FirstSnapshot{
    0x141320E80, 0x141320E60, 0x0};

WEAK symbol<LocalClientNum_t> primaryLocalClientNum{0x1433A256C, 0x14342155C,
                                                    0x0};
WEAK symbol<LocalClientNum_t> cl_maxLocalClients{0x145323720, 0x1453A2720, 0x0};
WEAK symbol<bool> cl_serverLoadingMap{0x14532373F, 0x1453A273F, 0x0};
} // namespace cl
} // namespace game
