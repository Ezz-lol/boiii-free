#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace cl {
WEAK symbol<void(game::LocalClientNum_t localClientNum)>
    CL_CheckKeepDrawingConnectScreen{0x1413CCB00, 0x0};
WEAK symbol<void(game::ControllerIndex_t controllerIndex,
                 net::XSESSION_INFO *hostInfo, const net::netadr_t *addr,
                 int numPublicSlots, int numPrivateSlots, const char *mapname,
                 const char *gametype, const char *somethingWithUserMaps)>
    CL_ConnectFromLobby{0x14134C590, 0x14018E420};
WEAK symbol<bool(game::LocalClientNum_t localClientNum, int index, char *buf,
                 int size, bool addClanName)>
    CL_GetClientName{0x1413E3160, 0x0};
WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    CL_LocalClient_IsActive{0x1427C18E0, 0x0};
WEAK symbol<float(void *key)> CL_KeyState{0x1412FF880, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, int32_t key, int32_t down,
                 uint32_t time)>
    CL_KeyEvent{0x141342220, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *pszMapName,
                 const char *pszGametype)>
    CL_SetupForNewServerMap{0x14135CD40, 0x0};
WEAK symbol<void(game::LocalClientNum_t localClientNum, bool active)>
    CL_LocalClient_SetActive{0x1427C1940, 0x0};

WEAK symbol<const char *(int32_t configStringIndex)> CL_GetConfigString{
    0x141321130, 0x14017CCC0};
WEAK symbol<int64_t(LocalClientNum_t localClientNum)> CL_ClearClientThinkPacket{
    0x1412F33F0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *mapname,
                 const char *gametype)>
    CL_PreloadMap{0x14135A200, 0x0};
WEAK symbol<bool(const char *map)> CL_MapSwitch_IsMapPreloaded{0x14131EA10,
                                                               0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool isServer)>
    CL_MapSwitch_SwitchNow{0x14131EC80, 0x0};
WEAK symbol<void(bool setupViewport)> CL_InitRenderer{0x141359710, 0x0};
WEAK symbol<void()> CL_StartHunkUsers{0x14135D1B0, 0x0};
WEAK symbol<CLMapSwitch> cl_switch{0x1452C4A00, 0x142850830};

WEAK symbol<void(int64_t localClientNum, int32_t msec)> CL_Frame{0x141351400,
                                                                 0x0};
WEAK symbol<void(game::LocalClientNum_t localClientNum)> CL_CheckForResend{
    0x14134B990, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool checkStatus)>
    CL_LoadoutChanged{0x141359980, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool deactivateClient)>
    CL_Disconnect{0x14135D880, 0x0};
WEAK symbol<void()> CL_Disconnect_f{0x14134CCD0, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum, net::netadr_t *from,
                 const char *reason)>
    CL_DisconnectPacket{0x14134CBB0, 0x0};
WEAK symbol<void(hunk::HunkUser *hunk, LocalClientNum_t maxLocalClients,
                 ClientNum_t maxClients, clientAllocFlags_t allocFlags)>
    CL_AllocateClientMemory{0x1413484E0, 0x0};

WEAK symbol<void(LocalClientNum_t maxLocalClients, ClientNum_t maxClients,
                 clientAllocFlags_t flags)>
    AllocatePerLocalClientMemory{0x14135D330, 0x0};
WEAK symbol<bool(const char *mapname, clientAllocFlags_t flags)>
    CL_AllocatePerLocalClientMemory{0x14135D670, 0x0};
WEAK symbol<void(bool restartUI)> CL_FreePerLocalClientMemory{0x14135DC20, 0x0};
WEAK symbol<void(const LocalClientNum_t localClientNum, unsigned __int8 c,
                 vec4_t *color)>
    CL_LookupColor{0x141321F40, 0x0};

WEAK symbol<void()> CL_CompositeRender{0x141335BF0, 0x0};
// Actual name is unknown - gives an image back to CL_CompositeRender's pool
WEAK symbol<void(gfx::GfxImage *image)> CL_CompositeRender_ReleaseImage{
    0x141335B90, 0x0};
// Actual name is unknown
WEAK symbol<CompositeJob[16]> compositeJobs{0x1452CA460, 0x0};

WEAK symbol<ClientConnections *> clientConnections{0x145359BB8, 0x0};
WEAK symbol<ClientActives *> clients{0x14535D188, 0x0};
WEAK symbol<clientStatic_t> cls{0x14535D190, 0x1428E2E40};
// TODO: fix the clientStatic_t struct and remove these symbols
WEAK symbol<gameState_t> cls_gamestate{0x1456a3828, 0x142C294D8};
WEAK symbol<int32_t> cls_serverId{0x14561C250, 0x142BA1F00};
WEAK symbol<qboolean> cls_forceFullscreen{0x14561C254, 0x142BA1F04};
WEAK symbol<qboolean> cls_exitFullscreen{0x14561C258, 0x142BA1F08};
WEAK symbol<r::vidConfig_t> cls_vidConfig{0x14536571C, 0x1428EB3CC};

WEAK symbol<ClientNum_t> cl_allocatedClients{0x1432A7EE8, 0x0};
WEAK symbol<clientAllocFlags_t> cl_lastAllocFlags{0x14535598C, 0x0};
WEAK symbol<void(LocalClientNum_t localClientNum)> CL_FirstSnapshot{0x141320E80,
                                                                    0x0};

WEAK symbol<LocalClientNum_t> primaryLocalClientNum{0x1433A256C, 0x14112E01C};
WEAK symbol<LocalClientNum_t> cl_maxLocalClients{0x145323720, 0x0};
WEAK symbol<bool> cl_serverLoadingMap{0x14532373F, 0x0};
} // namespace cl
} // namespace game
