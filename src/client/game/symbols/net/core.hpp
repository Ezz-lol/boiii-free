#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {

WEAK symbol<SOCKET> ip_socket{0x157E75818, 0x14A640988};

// NET
WEAK symbol<uint8_t *()> Netchan_AllocMessage{0x14211AED0, 0x142173990,
                                              0x140516000};
WEAK symbol<bool(netsrc_t sock, const netadr_t *adr, const void *data,
                 int32_t len)>
    NET_OutOfBandData{0x142173600, 0x140515C80};
WEAK symbol<bool(PacketQueuePtr queue, uint32_t flags, netsrc_t sock,
                 const netadr_t *addr, int32_t length, const void *data)>
    NET_EnqueuePacket{0x0, 0x14051A570};
WEAK symbol<bool(netsrc_t sock, int32_t length, const void *data,
                 const netadr_t *to)>
    NET_SendPacket{0x1423323B0, 0x140596E40};
WEAK symbol<bool(const char *str, netadr_t *adr)> NET_StringToAdr{0x142172780,
                                                                  0x140515110};
WEAK symbol<bool(netsrc_t sock, netadr_t *adr, const char *data)>
    NET_OutOfBandPrint{0x142173710};
} // namespace net
} // namespace game