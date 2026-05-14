#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cl {
WEAK symbol<void(game::LocalClientNum_t localClientNum)>
    CL_CheckKeepDrawingConnectScreen{0x1413CCAE0};
WEAK symbol<void(game::ControllerIndex_t controllerIndex,
                 net::XSESSION_INFO *hostInfo, const net::netadr_t *addr,
                 int numPublicSlots, int numPrivateSlots, const char *mapname,
                 const char *gametype, const char *somethingWithUserMaps)>
    CL_ConnectFromLobby{0x14134C570};
WEAK symbol<bool(game::LocalClientNum_t localClientNum, int index, char *buf,
                 int size, bool addClanName)>
    CL_GetClientName{0x1413E3140};
WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    CL_LocalClient_IsActive{0x14283AA50};
WEAK symbol<float(void *key)> CL_KeyState{0x1412FF860};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *pszMapName,
                 const char *pszGametype)>
    CL_SetupForNewServerMap{0x14135CD20};
WEAK symbol<uint32_t> cl_maxLocalClients{0x1453A2720};
WEAK symbol<bool> cl_serverLoadingMap{0x1453A273F};
WEAK symbol<void(game::LocalClientNum_t localClientNum, bool active)>
    CL_LocalClient_SetActive{0x14283AAB0};

WEAK symbol<int64_t(LocalClientNum_t localClientNum)> CL_ClearClientThinkPacket{
    0x1412F33D0};
WEAK symbol<bool(const char *map)> CL_MapSwitch_IsMapPreloaded{0x14131E9F0};
WEAK symbol<void(LocalClientNum_t localClientNum, bool isServer)>
    CL_MapSwitch_SwitchNow{0x14131EC60};
WEAK symbol<void(bool setupViewport)> CL_InitRenderer{0x1413596F0};
WEAK symbol<void()> CL_StartHunkUsers{0x14135D190};
WEAK symbol<CLMapSwitch> cl_switch{0x145343A00};
WEAK symbol<int32_t> g_qport{0x156E49AAC};

WEAK symbol<void(int64_t localClientNum, int32_t msec)> CL_Frame{0x1413513E0};
WEAK symbol<void(game::LocalClientNum_t localClientNum)> CL_CheckForResend{
    0x14134B970};
WEAK symbol<clientConnection_t (*)[2]> clientConnections{0x1453D8BB8};
WEAK symbol<void(LocalClientNum_t localClientNum, bool checkStatus)>
    CL_LoadoutChanged{0x141359960};
WEAK symbol<void(LocalClientNum_t localClientNum, bool deactivateClient)>
    CL_Disconnect{0x14135D860};
WEAK symbol<void(LocalClientNum_t localClientNum, net::netadr_t *from,
                 const char *reason)>
    CL_DisconnectPacket{0x14134CB90};

WEAK symbol<clientStatic_t> cls{0x1453DC190};
} // namespace cl
} // namespace game

#endif