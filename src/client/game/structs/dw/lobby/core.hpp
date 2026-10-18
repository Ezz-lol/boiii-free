#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/crypt.hpp>
#include <game/structs/dw/task/core.hpp>

namespace game {
namespace dw {
namespace lobby {

class bdLobbyConnectionListener {
public:
  // TODO
  struct VTbl {};
  VTbl *vtable;
};

PACKED(struct bdSockAddr {
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
});

typedef bdSockAddr bdInAddr;

struct bdInetAddr {
  bdInAddr m_addr;
};

PACKED(struct bdAddr {
  bdInetAddr m_address;
  bdPort m_port;
  uint8_t _padding86[2];
});

typedef bdUInt32 bdTitleID;

class bdAuthInfo {
public:
  bdTitleID m_titleID;
  bdUInt32 m_IVSeed;
  bdByte8 m_data[128];
  bdUByte8 m_sessionKey[24];
};

class __declspec(align(4)) bdPendingBufferTransfer : bdReferencable {
public:
  bdByteBufferRef m_buffer;
  bdUByte8 *m_txPtr;
  bdUInt32 m_txAvail;
  bdBool m_isThrottled;
};

typedef bdReference<bdPendingBufferTransfer> bdPendingBufferTransferRef;

class bdStreamSocket {
public:
  bdInt m_handle;
};

PACKED(class bdTokenBucket {
public:
  bdUInt32 m_tokensPerMs;
  bdUInt32 m_capacity;
  bdUInt32 m_minFragment;
  bdUInt32 m_tokens;
  bdUInt64 m_lastUpdateTime;
  bdBool m_isInfinite;
  uint8_t _padding1A[7];
});

class bdLobbyConnection : public bdReferencable {
public:
  enum class RecvState : uint32_t {
    INIT = 0x0,
    SIZE = 0x1,
    ENCRYPT = 0x2,
    MESSAGE = 0x3,
    COMPLETE = 0x4,
  };

  enum class Status : uint32_t {
    NOT_CONNECTED = 0x0,
    CONNECTING = 0x1,
    CONNECTED = 0x2,
    DISCONNECTING = 0x3,
    DISCONNECTED = 0x4,
  };

  enum Crypto210State : uint32_t {
    INVALID = 0x0,
    WAIT_SERVER_ACK = 0x1,
    WAIT_SERVER_AUTH_DONE = 0x2,
    READY = 0x3,
  };

  bdAddr m_lobbyAddr;
  bdBool m_isLSGConnection;
  bdBool m_isPlaintext;
  bdUInt m_maxSendMessageSize;
  bdUInt m_maxRecvMessageSize;
  bdLobbyConnection::RecvState m_recvState;
  bdUByte8 m_msgSizeBuffer[4];
  bdUInt m_recvCount;
  bdUByte8 m_recvEncryptType;
  bdUInt32 m_messageSize;
  task::bdTaskByteBufferRef m_recvMessage;
  bdPendingBufferTransferRef m_recvTransfer;
  bdQueue<bdPendingBufferTransfer> m_outgoingBuffers;
  bdStreamSocket m_socket;
  bdLobbyConnection::Status m_status;
  bdLobbyConnectionListener *m_connectionListener;
  bdUInt m_encryptionType;
  crypt::bdCypher *m_cypher;
  bdUByte8 m_sessionKey[24];
  bdUInt32 m_messageCount;
  bdStopwatch m_keepAliveTimer;
  bdStopwatch m_lastReceivedTimer;
  bdStopwatch m_asyncConnectTimer;
  bdTokenBucket m_throttleBucket;
  bdBool m_tasksAreThrottled;
  bdAuthInfo m_authInfo;
  bdLobbyConnection::Crypto210State m_cryptoState;
  bdUInt32 m_cypher210RecvCounter;
  bdUInt32 m_cypher210SendCounter;
  bdUByte8 m_clientNonce[8];
  bdUByte8 m_serverNonce[8];
  bdUByte8 m_expectedServerVerify[8];
  crypt::bdCypherAES *m_cypher210Client;
  crypt::bdCypherAES *m_cypher210Server;
  bdUByte8 m_cypher210ClientMacKey[20];
  bdUByte8 m_cypher210ServerMacKey[20];
  bdUInt64 m_cypher210ConnID;
};
typedef bdReference<bdLobbyConnection> bdLobbyConnectionRef;

class bdLobbyEventHandler {
public:
  // TODO
  struct VTbl {};

  VTbl *vtbl;
};

} // namespace lobby
} // namespace dw
} // namespace game
