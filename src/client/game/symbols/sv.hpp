#pragma once

#include <game/symbols/sym_include.hpp>

#include "sym_include.hpp"

namespace game {
namespace sv {
WEAK symbol<void(net::netadr_t *from, net::msg::msg_t *msg)> SV_AuthClient{
    0x0, 0x140475BB0};
WEAK symbol<CmdArgs> sv_cmd_args{0x15689AE30, 0x14944C740};
inline CmdArgs *SV_Cmd_Argv() { return sv_cmd_args.get(); }

inline int32_t SV_Cmd_Argc() { return sv_cmd_args->argc[sv_cmd_args->nesting]; }
// Client and dedi struct size differs :(
WEAK symbol<client_s_cl *> svs_clients_cl{0x1576F9318, 0};
WEAK symbol<client_s *> svs_clients{0x0, 0x14A178E98};

WEAK symbol<server_t> sv{0x1576F9C00, 0x14A0BC710};
WEAK symbol<serverStatic_t> svs{0x1576F8700, 0x14A178280};
WEAK symbol<serverStaticHeader_t> svsHeader{0x157906580, 0x14A2CE280};

WEAK symbol<bool()> SV_Loaded{0x142252250, 0x140535460};
WEAK symbol<void *()> SV_AddTestClient{0x142248F40, 0x14052E3E0};
WEAK symbol<void(net::netadr_t from)> SV_DirectConnect{0x142249880,
                                                       0x14052EC60};
WEAK
    symbol<void(ClientNum_t clientNum, net::svscmd_type type, const char *text)>
        SV_GameSendServerCommand{0x14224F580, 0x140532CA0};
WEAK symbol<void(client_s *cl_0, net::svscmd_type type, const char *fmt, ...)>
    SV_SendServerCommand{0x142254D30, 0x140537F10};
WEAK symbol<bool(ClientNum_t clientNum)> SV_IsTestClient{0x14224AB60,
                                                         0x14052FF40};
WEAK symbol<void(ControllerIndex_t controllerIndex, const char *server,
                 MapPreload preload, bool savegame)>
    SV_SpawnServer{0x1422528C0, 0x140535B20};
WEAK symbol<void(const char *text_in)> SV_Cmd_TokenizeString{0x1420EF130,
                                                             0x1404FA6C0};
WEAK symbol<void()> SV_Cmd_EndTokenizedString{0x1420EF0E0, 0x1404FA670};
WEAK symbol<void(client_s *client, net::msg::msg_t *msg)>
    SV_ExecuteClientMessage{0x14224A460, 0x14052F840};

WEAK symbol<void(int index, const char *val)> SV_SetConfigstring{0x1422513A0,
                                                                 0x1405355B0};

WEAK symbol<void(client_s *drop, const char *reason, bool tellThem,
                 bool removeFromLobby)>
    SV_DropClient{0x14224A050, 0x14052F430};
WEAK symbol<void(client_s *cl_0, const char *reason)> SV_Live_RemoveClient{
    0x142242510, 0x140527530};
/*
  Server only. Function exists on client but requires accessing areas of memory
  which are only initialized when running as dedicated server. Also might not
  (?) take the second `reason` argument, but it is entirely unused anyway, so
  this is of no consequence.
*/
WEAK symbol<void(client_s *cl_0, const char *reason)>
    SV_Live_RemoveAllClientsFromAddress{0x142254630, 0x1405379E0};
WEAK symbol<void(client_s *client, net::svscmd_type type, const char *cmd)>
    SV_AddServerCommand{0x142253460, 0x140536660};

WEAK symbol<void(client_s *client, user::usercmd_t *cmd)> SV_ClientEnterWorld{
    0x142249370, 0x14052E810};
WEAK symbol<void(level::gentity_t *ent)> SV_DObjDumpInfo{0x14224E980,
                                                         0x1405320C0};
WEAK symbol<BoneIndex(const level::gentity_t *ent, scr::ScrString_t boneName)>
    SV_DObjGetBoneIndex{0x14224E9E0, 0x140532120};

} // namespace sv
} // namespace game