#pragma once
#include "core.hpp"

namespace game {
namespace dw {
namespace net {

enum class bdNATType : uint8_t {
  NAT_UNKNOWN = 0x0,
  NAT_OPEN = 0x1,
  NAT_MODERATE = 0x2,
  NAT_STRICT = 0x3,
};

#pragma pack(push, 1)
class bdSockAddr {
public:
  union {
    struct {
      bdUByte8 m_b1;
      bdUByte8 m_b2;
      bdUByte8 m_b3;
      bdUByte8 m_b4;
    } m_caddr;
    bdUInt32 m_iaddr;
    struct {
      bdUInt16 m_w1;
      bdUInt16 m_w2;
      bdUInt16 m_w3;
      bdUInt16 m_w4;
      bdUInt16 m_w5;
      bdUInt16 m_w6;
      bdUInt16 m_w7;
      bdUInt16 m_w8;
    } m_caddr6;
    bdUByte8 m_iaddr6[16];
    bdUByte8 m_sockaddr_storage[128];
  } inUn;
  bdUInt16 m_family;
  uint8_t _padding82[2];
};
ASSERT_SIZE(bdSockAddr, 0x84);
#pragma pack(pop)

typedef class bdSockAddr bdInAddr;

class bdInetAddr {
public:
  bdInAddr m_addr;
};

#pragma pack(push, 1)
class bdAddr {
public:
  bdInetAddr m_address;
  bdPort m_port;
  uint8_t _padding86[2];
};
#pragma pack(pop)

#pragma pack(push, 1)
class bdCommonAddr : bdReferencable {
public:
  bdArray<bdAddr> m_localAddrs;
  bdAddr m_publicAddr;
  bdNATType m_natType;
  bdUInt m_hash;
  bdBool m_isLoopback;
  uint8_t _paddingB1[7];
};
#pragma pack(pop)

typedef bdReference<bdCommonAddr> bdCommonAddrRef;

} // namespace net
} // namespace dw
} // namespace game