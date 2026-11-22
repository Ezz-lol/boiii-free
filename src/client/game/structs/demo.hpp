#pragma once

#include "macros.hpp"
#include <cstdint>

namespace game {
namespace demo {
struct DemoPlayback {
  uint8_t unk0[0xA97494];
  int32_t clipDuration;
};
ASSERT_OFFSET(DemoPlayback, clipDuration, 0xA97494);
} // namespace demo
} // namespace game
