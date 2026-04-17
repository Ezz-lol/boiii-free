#pragma once
#ifdef GAME_HPP

#include <cstdint>
#include "macros.hpp"

namespace game {
namespace net {

WEAK symbol<SOCKET> ip_socket{0x157E75818, 0x14A640988};

// MSG
WEAK symbol<uint8_t(msg_t *msg)> MSG_ReadByte{0x142155450, 0x14050D1B0};

// NET
WEAK symbol<bool(netsrc_t sock, netadr_t *adr, const void *data, int len)>
    NET_OutOfBandData{0x142173600};
WEAK symbol<bool(netsrc_t sock, int length, const void *data,
                 const netadr_t *to)>
    NET_SendPacket{0x1423323B0, 0x140596E40};
WEAK symbol<bool(const char *, netadr_t *)> NET_StringToAdr{0x142172780,
                                                            0x140515110};
} // namespace net
} // namespace game

#endif
