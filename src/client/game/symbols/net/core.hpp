#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>

namespace game {
namespace net {

WEAK symbol<SOCKET> ip_socket{0x157DF68E8, 0x14A640988};
WEAK symbol<SOCKET> server_ip_socket{0x14A5B09D8, 0x0};

// NET
WEAK symbol<uint8_t *()> Netchan_AllocMessage{0x14211AED0, 0x140516000};
WEAK symbol<bool(netsrc_t sock, const netadr_t *adr, const void *data,
                 int32_t len)>
    NET_OutOfBandData{0x14211AB40, 0x140515C80};
WEAK symbol<bool(PacketQueuePtr queue, uint32_t flags, netsrc_t sock,
                 const netadr_t *addr, int32_t length, const void *data)>
    NET_EnqueuePacket{0x0, 0x14051A570};
WEAK symbol<bool(netsrc_t sock, int32_t length, const void *data,
                 const netadr_t *to)>
    NET_SendPacket{0x1422B9240, 0x140596E40};
WEAK symbol<bool(const char *str, netadr_t *adr)> NET_StringToAdr{0x142119CC0, 0x140515110};
WEAK symbol<bool(netsrc_t sock, netadr_t *adr, const char *data)>
    NET_OutOfBandPrint{0x14211AC50, 0x140515D90};
WEAK symbol<uint16_t()> NET_GetDefaultPort{0x142119C60, 0x0};
WEAK symbol<bool(const netadr_t *a, const netadr_t *b)> NetAdr_Compare{0x142119DB0, 0x140515200};
WEAK symbol<int32_t> g_qport{0x156DCAB3C, 0x1498C18E0};

} // namespace net
} // namespace game
