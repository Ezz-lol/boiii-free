#pragma once

#include <game/structs/dw/core.hpp>

namespace game {
namespace dw {
namespace relay {

enum class bdRelayServiceConnTypeHint : uint32_t {
  UDP = 0x1,
  XLSP = 0x2,
};

enum class bdRelayServicePlatform : int32_t {
  PC = 0x1,
  PS3 = 0x2,
  IPHONE = 0x3,
  ANDROID = 0x4,
  XENON = 0x5,
  PS4 = 0x6,
  XBOXONE = 0x7,
  WII = 0x8,
  WIIU = 0x9,
  WP8 = 0xA,
  UNIX = 0xB,
  OSX = 0xC,
  UNKNOWN = -1
};

class bdRelayService {
public:
  task::manage::bdRemoteTaskManager *m_remoteTaskManager;
};

} // namespace relay
} // namespace dw
} // namespace game
