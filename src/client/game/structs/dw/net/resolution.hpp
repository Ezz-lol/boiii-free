#pragma once

#include <game/structs/dw/core.hpp>
#include <game/structs/dw/net/core.hpp>
#include <game/structs/dw/thread.hpp>

namespace game {
namespace dw {
namespace net {
namespace resolution {

class bdGetHostByNameConfig {
public:
  bdFloat32 m_timeout;
};

class bdGetHostByName : public thread::bdRunnable {
public:
  enum class bdStatus : uint32_t {
    BD_LOOKUP_UNINITIALIZED = 0x0,
    BD_LOOKUP_PENDING = 0x1,
    BD_LOOKUP_SUCCEEDED = 0x2,
    BD_LOOKUP_FAILED = 0x3,
    BD_LOOKUP_CANCELLED = 0x4,
    BD_LOOKUP_TIMED_OUT = 0x5,
    BD_LOOKUP_ERROR = 0x6,
  };

  bdStopwatch m_timer;
  bdGetHostByName::bdStatus m_status;
  thread::bdThread *m_thread;
  bdGetHostByNameConfig m_config;
  bdInAddr m_addresses[4];
  bdUInt m_numAddresses;
  bdDNSHandle m_handle;
  bdUInt m_threadStackSize;
};

} // namespace resolution
} // namespace net
} // namespace dw
} // namespace game
