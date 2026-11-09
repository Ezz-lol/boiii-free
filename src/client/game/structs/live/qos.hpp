#pragma once

#include <game/structs/core.hpp>
#include <game/structs/macros.hpp>

#include <cstdint>

namespace game {
namespace live {
namespace qos {

PACKED(struct echoServer_t {
  uint32_t serverAddr;
  uint16_t serverPort;
  uint8_t _padding06[2];
});

PACKED(struct serverStats_t {
  uint8_t numPacketsRecvd;
  uint8_t _padding01[1];
  uint16_t numPacketsSent;
  uint32_t packetsRecvd;
  int32_t firstPingDeadline;
});

PACKED(struct qosServer_t {
  enum class Type : uint32_t {
    ANY = 0x0,
    GAMESERVERS = 0x1,
    THUNDERHEAD = 0x2,
  };

  union {
    str<4> serverName;
    uint32_t serverId;
  };
  uint16_t shortId;
  str16_t thunderheadServerName;
  uint8_t _padding16[2];
  echoServer_t echoServers[8];
  uint8_t numServerAddrs;
  uint8_t _padding59[1];
  uint16_t nonce;
  uint16_t currentServer;
  uint8_t _padding5E[2];
  Type type;
  serverStats_t stats;
  uint16_t ping;
  uint8_t _padding72[2];
});
ASSERT_SIZE(qosServer_t, 0x74);

} // namespace qos
} // namespace live
} // namespace game
