#pragma once
#ifdef GAME_HPP

#include "macros.hpp"

namespace game {
namespace cl {
WEAK symbol<void(int localClientNum)> CL_CheckKeepDrawingConnectScreen{
    0x1413CCAE0};
WEAK symbol<void(int controllerIndex, net::XSESSION_INFO *hostInfo,
                 const net::netadr_t *addr, int numPublicSlots,
                 int numPrivateSlots, const char *mapname, const char *gametype,
                 const char *somethingWithUserMaps)>
    CL_ConnectFromLobby{0x14134C570};
WEAK symbol<bool(int localClientNum, int index, char *buf, int size,
                 bool addClanName)>
    CL_GetClientName{0x1413E3140};
WEAK symbol<bool(int localClientNum)> CL_LocalClient_IsActive{0x14283AA50};
WEAK symbol<float(void *key)> CL_KeyState{0x1412FF860};
WEAK symbol<void(LocalClientNum_t localClientNum, const char *pszMapName,
                 const char *pszGametype)>
    CL_SetupForNewServerMap{0x14135CD20};
} // namespace cl
} // namespace game

#endif