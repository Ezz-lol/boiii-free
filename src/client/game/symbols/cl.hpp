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
} // namespace cl
} // namespace game

#endif