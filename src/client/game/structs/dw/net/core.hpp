#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/net/core.hpp>

namespace game {
namespace dw {
namespace net {

typedef int32_t bdDNSHandle;

enum class bdNATType : uint8_t {
  NAT_UNKNOWN = 0x0,
  NAT_OPEN = 0x1,
  NAT_MODERATE = 0x2,
  NAT_STRICT = 0x3,
};

inline constexpr const char *serialize(const bdNATType type) {
  switch (type) {
  case bdNATType::NAT_UNKNOWN:
    return "NAT_TYPE_UNKNOWN";
  case bdNATType::NAT_OPEN:
    return "NAT_TYPE_OPEN";
  case bdNATType::NAT_MODERATE:
    return "NAT_TYPE_MODERATE";
  case bdNATType::NAT_STRICT:
    return "NAT_TYPE_STRICT";
  default:
    return "NAT_TYPE_INVALID";
  }
}

PACKED(class bdSockAddr {
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

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"m_family\": {}, \"m_iaddr\": {}, \"m_iaddr6\": "
                       "{}, \"m_sockaddr_storage\": {} }}",
                       m_family, inUn.m_iaddr, ::serialize(inUn.m_iaddr6),
                       ::serialize(inUn.m_sockaddr_storage));
  }
});
ASSERT_SIZE(bdSockAddr, 0x84);

typedef class bdSockAddr bdInAddr;

class bdInetAddr {
public:
  bdInAddr m_addr;

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"m_addr\": {} }}", m_addr.serialize());
  }
};

PACKED(class bdAddr {
public:
  bdInetAddr m_address;
  bdPort m_port;
  uint8_t _padding86[2];

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format("{{ \"m_address\": {}, \"m_port\": {} }}",
                       m_address.serialize(), m_port);
  }
});

PACKED(class bdCommonAddr : public bdReferencable {
public:
  struct syms {
    static inline constexpr symbol<void(const bdCommonAddr *, bdUByte8 *buffer)>
        serialize{0x142887830, 0x1429009A0};
  };

  bdArray<bdAddr> m_localAddrs;
  bdAddr m_publicAddr;
  bdNATType m_natType;
  bdUInt m_hash;
  bdBool m_isLoopback;
  uint8_t _paddingB1[7];

  inline void serialize(game::net::XNADDR & buf) const {
    return syms::serialize(this, reinterpret_cast<bdUByte8 *>(&buf));
  }

  [[nodiscard]] inline std::string serialize() const noexcept {
    return std::format(
        "{{ \"m_localAddrs\": {}, \"m_publicAddr\": {}, \"m_natType\": {}, "
        "\"m_hash\": {}, \"m_isLoopback\": {} }}",
        ::serialize(m_localAddrs), m_publicAddr.serialize(),
        net::serialize(m_natType), m_hash, ::serialize(m_isLoopback));
  }
});

typedef bdReference<bdCommonAddr> bdCommonAddrRef;

} // namespace net
} // namespace dw
} // namespace game
