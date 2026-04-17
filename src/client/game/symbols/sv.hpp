#pragma once
#ifdef GAME_HPP

#include "macros.hpp"
#include <cstdint>

namespace game {
namespace sv {
WEAK symbol<CmdArgs> sv_cmd_args{0x15689AE30, 0x14944C740};
// Client and dedi struct size differs :(
WEAK symbol<net::client_s_cl *> svs_clients_cl{0x1576F9318, 0};
WEAK symbol<net::client_s *> svs_clients{0x0, 0x14A178E98};
WEAK symbol<uint32_t> svs_time{0x0, 0x14A178E84};

WEAK symbol<bool()> SV_Loaded{0x142252250, 0x140535460};
WEAK symbol<void *()> SV_AddTestClient{0x142248F40, 0x14052E3E0};
WEAK symbol<void(net::netadr_t from)> SV_DirectConnect{0x142249880,
                                                       0x14052EC60};
WEAK symbol<void(int clientNum, net::svscmd_type type, const char *text)>
    SV_GameSendServerCommand{0x14224F580, 0x140532CA0};
WEAK symbol<void(net::client_s *cl_0, net::svscmd_type type, const char *fmt,
                 ...)>
    SV_SendServerCommand{0x142254D30, 0x140537F10};
WEAK symbol<bool(int clientNum)> SV_IsTestClient{0x14224AB60, 0x14052FF40};
WEAK symbol<void(int controllerIndex, const char *server, MapPreload preload,
                 bool savegame)>
    SV_SpawnServer{0x1422528C0, 0x140535B20};
WEAK symbol<void(const char *text_in)> SV_Cmd_TokenizeString{0x1420EF130,
                                                             0x1404FA6C0};
WEAK symbol<void()> SV_Cmd_EndTokenizedString{0x1420EF0E0, 0x1404FA670};
WEAK symbol<void(void *client, net::msg_t *msg)> SV_ExecuteClientMessage{
    0x14224A460, 0x14052F840};

WEAK symbol<void(net::client_s *drop, const char *reason, bool tellThem,
                 bool removeFromLobby)>
    SV_DropClient{0x14224A050, 0x14052F430};
WEAK symbol<void(net::client_s *cl_0, const char *reason)> SV_Live_RemoveClient{
    0x142242510, 0x140527530};
/*
  Server only. Function exists on client but requires accessing areas of memory
  which are only initialized when running as dedicated server. Also might not
  (?) take the second `reason` argument, but it is entirely unused anyway, so
  this is of no consequence.
*/
WEAK symbol<void(net::client_s *cl_0, const char *reason)>
    SV_Live_RemoveAllClientsFromAddress{0x142254630, 0x1405379E0};
WEAK symbol<void(net::client_s *client, net::svscmd_type type, const char *cmd)>
    SV_AddServerCommand{0x142253460, 0x140536660};
} // namespace sv
} // namespace game

#endif
