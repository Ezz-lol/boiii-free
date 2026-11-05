#pragma once

#include <cstdint>
#include <windows.h>

#include "sys.hpp"

namespace game {
namespace uhttp {
typedef int32_t Socket_t;

struct __attribute__((aligned(8))) uHttp_Server {
  uint16_t minPort;
  uint16_t maxPort;
  uint64_t inaddr;
  int32_t backlog;
  sys::FastCriticalSection cs;
  Socket_t socket;
  uint16_t port;
};

class uHttp_Session;

class uHttp_Session {
public:
  struct Header {
    char *name;
    char *value;
  };

  uHttp_Session *next;
  Socket_t socket;
  sockaddr_in addr;
  bool wsFragment;
  union {
    uint64_t i;
    void *p;
  } aUser[4];
  char userData[10240];
  char *path;
  uint8_t _unknown2848[8];
  char headerString[10240];          // Verified
  uHttp_Session::Header headers[20]; // Verified
};
ASSERT_OFFSET(uHttp_Session, headerString, 0x2850);
ASSERT_OFFSET(uHttp_Session, headers, 0x5050);

class HTTPD_WebSocketHandler;
typedef fastcall_t<bool(HTTPD_WebSocketHandler *handler, uHttp_Session *session,
                        uint32_t, void *, size_t)>
    HTTPD_WebSocketHandlerCB;

class HTTPD_WebSocketHandler {
public:
  HTTPD_WebSocketHandler *next;
  HTTPD_WebSocketHandlerCB *cb;
  uHttp_Session *session;
  const char *path;
};

} // namespace uhttp
} // namespace game
