#pragma once

#include <game/symbols/sym_include.hpp>
#include <windows.h>

namespace game {
namespace sys {
WEAK symbol<int()> Sys_Milliseconds{0x1422B9700, 0x142332870, 0x1405972F0};
WEAK symbol<void(HINSTANCE h_instance)> Sys_CreateConsole{
    0x1422B9C90, 0x142332E00, 0x140597880};
WEAK symbol<void()> Sys_ShowConsole{0x1422BA250, 0x1423333C0, 0x140597E40};

WEAK symbol<TLSData *()> Sys_GetTLS{0x14212AD10, 0x1421837B0, 0x140525EB0};
inline int32_t Cmd_Argc() {
  TLSData *tls = Sys_GetTLS();
  return tls->cmdArgs->argc[tls->cmdArgs->nesting];
}
inline CmdArgs *Cmd_Argv() { return Sys_GetTLS()->cmdArgs; }

WEAK symbol<qboolean(net::netadr_t *net_from, net::msg::msg_t *net_message)>
    Sys_GetPacket{0x1422B9040, 0x1423321B0, 0x140596CF0};
WEAK symbol<qboolean()> Sys_IsDatabaseReady{0x14212AFC0, 0x142183A60, 0x0};
WEAK symbol<void(CriticalSection critsect)> Sys_EnterCriticalSection{
    0x140119190, 0x140119190, 0x140055230};
WEAK symbol<void(CriticalSection critsect)> Sys_LeaveCriticalSection{
    0x1401191E0, 0x1401191E0, 0x140055280};
WEAK symbol<const char *()> Sys_Cwd{0x142247CE0, 0x1422A4800, 0x140564EB0};
WEAK symbol<void(HANDLE *event)> Sys_WaitForSingleObject{
    0x14212BC50, 0x142C7A8E0, 0x140526DC0};

WEAK symbol<fs::PathList(char *directory, const char *extension, char *filter,
                         int *numfiles, qboolean wantsubs)>
    Sys_ListFiles{0x14227D940, 0x1422EAA10, 0x1405827C0};
WEAK symbol<void(const char *fmt, ...)> Sys_Error{0x142287930, 0x1422F4A00,
                                                  0x140584F50};

WEAK symbol<const char *()> Sys_GetAbsZoneDir{0x142247CF0, 0x1422A4810,
                                              0x140564EC0};
WEAK symbol<str<272>> s_absZoneDir{0x1579EC960, 0x157A6B860, 0x14A3A2BA0};

} // namespace sys
} // namespace game
